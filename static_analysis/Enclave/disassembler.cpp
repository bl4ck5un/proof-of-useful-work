// diStorm64 library sample
// http://ragestorm.net/distorm/
// Arkon, Stefan, 2005
// Mikhail, 2006
// JvW, 2007

#include <stdio.h>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <config.h>

#include "distorm.h"
#include "disassembler.h"
#include "../Common/Log.h"
#include "../Common/Debug.h"

#define EX_OK           0
#define EX_USAGE       64
#define EX_DATAERR     65
#define EX_NOINPUT     66
#define EX_NOUSER      67
#define EX_NOHOST      68
#define EX_UNAVAILABLE 69
#define EX_SOFTWARE    70
#define EX_OSERR       71
#define EX_OSFILE      72
#define EX_CANTCREAT   73
#define EX_IOERR       74
#define EX_TEMPFAIL    75
#define EX_PROTOCOL    76
#define EX_NOPERM      77
#define EX_CONFIG      78


using namespace std;
int disassembler(
		const unsigned char* addr,
		uint64_t offset,
		uint64_t len,
		_DecodedInst* insts,
		int inst_cap,
		unsigned int* n_decoded)
{
	_DecodeResult res;
	_DecodeType dt = Decode64Bits;

    *n_decoded = 0;
	unsigned int bytes_decoded;


	while (1) {
		res = distorm_decode(offset, addr, len, dt, insts, inst_cap, n_decoded);
		if (res == DECRES_INPUTERR) {
			LL_CRITICAL("Input error, halting!\n");
			return EX_SOFTWARE;
		}

		if (res == DECRES_SUCCESS) break;
		else if (*n_decoded == 0) break;

		bytes_decoded = (unsigned int)(insts[*n_decoded-1].offset - offset);
		bytes_decoded += insts[*n_decoded-1].size;
		addr += bytes_decoded;
		len -= bytes_decoded;
		offset += bytes_decoded;
	}
	return EX_OK;
}

inline bool isCtrlflow(const _WString *inst_ws)
{
    return inst_ws->p[0] == 'J' ||
           strncmp("RET", (const char*) inst_ws->p, 3) == 0 ||
           strncmp("CALL", (const char*) inst_ws->p, min<unsigned int>(inst_ws->length, 4)) == 0;
}

inline bool isLEA(const _DecodedInst *inst)
{
    LL_DEBUG("mnemonic: %s", inst->mnemonic.p);
    LL_DEBUG("operands: %s", inst->operands.p);
    return (strncmp("LEA", (const char*) inst->mnemonic.p, 3) == 0) && (strncmp("R15", (const char*) inst->operands.p, 3) == 0);
}

inline bool isNop(const _DecodedInst* inst)
{
    return (strncmp("NOP", (const char*) inst->mnemonic.p, 3) == 0);
}


inline long extractLEAOprand (const _DecodedInst* inst)
{
    // LEA R15, [R15+0x4]
    // find 0x4 in the above expression
    const char* posOfIncrement = strchr((const char*) inst->operands.p, '+');
    long increment = strtol(posOfIncrement, NULL, 0);

    return increment;
}

/*
 * Verify a basicblock
 * Note that because the disassembly losses all of the labels, a basic block here might
 * correspond to multiple basic blocks at the instrumentation time.
 * Therefore the correct logic of verification is
 *
 * bb.size() <= sum of arguments to LEA's + number of LEAs
 *
 */
bool BasicBlock::verify() const {
    unsigned long starting = 0, i;
    // allow BB begins with NOP and XOR
    if (strncmp("NOP", (const char*) instList[0]->mnemonic.p, 3) == 0)
        starting++;
    if (strncmp("XOR", (const char*) instList[0]->mnemonic.p, 3) == 0)
        starting++;

    // After any XOR or NOP, a BB must begins with a LEA
    if (!isLEA(instList.at(starting))) {
        LL_CRITICAL("BB does not begin with LEA. Begins with %s", instList.at(starting)->mnemonic.p);
        return false;
    }

    int numberOfInstructionsCounted = 0;
    int numberOfLEAInstructions = 0;
    // Compiler might insert random NOPs after instrumentation
    // so no there is no way to count them and we have to prevent we have
    int numberOfNopInstructions = 0;
    for (i = starting; i < instList.size() - 1; i++)
    {
        // basic blocks contain no control flow instructions
        if (isCtrlflow(&instList[i]->mnemonic)) { return false; }
        // how many instructions are counted so far?
        if (isLEA(instList.at(i))) {
            numberOfInstructionsCounted += extractLEAOprand(instList.at(starting));
            numberOfLEAInstructions++;
        }

        if (isNop(instList.at(i))) {
            numberOfNopInstructions++;
        }
    }

    // if RET is preceeded by a MOV RAX R15, th
    if (strcmp("RET", (const char*) instList.back()->mnemonic.p)==0 &&
        strcmp("MOV", (const char*) instList.end()[-2]->mnemonic.p) == 0 &&
        strcmp("RAX, R15", (const char*) instList.end()[-2]->operands.p) == 0)
    {
        // pretend we have counted the last MOV
        numberOfInstructionsCounted++;
    }
    LL_DEBUG("numberOfInstructionsCounted=%d, numberOfLEA=%d, numberOfNop=%d", numberOfInstructionsCounted, numberOfLEAInstructions, numberOfNopInstructions);
    numberOfInstructionsCounted += numberOfNopInstructions;
    return (numberOfInstructionsCounted >= instList.size() - starting - numberOfLEAInstructions);
}

int verify_inst_counters (const unsigned char* asm_buffer, uint64_t asm_offset, uint64_t asm_len)
{
    _DecodedInst insts[10240];
    unsigned int n_decoded = 0;
    disassembler(asm_buffer, asm_offset, asm_len, insts, 10240, &n_decoded);

    map<uint64_t, uint64_t> offsetToSeq;
    for (int i = 0; i < n_decoded; i++)
    {
        offsetToSeq[insts[i].offset] = i;
        LL_DEBUG("%0*llx (%02d) %-24s %s%s%s", 16,
               insts[i].offset,
               insts[i].size,
               (char*)insts[i].instructionHex.p,
               (char*)insts[i].mnemonic.p,
               insts[i].operands.length != 0 ? " " : "",
               (char*)insts[i].operands.p);
    }

    uint64_t  op = 0;
    int ret = 0;

    vector<int> bpSeqs;

    bpSeqs.push_back(0);

    uint64_t addr = asm_offset;
    for (int i = 0; i < n_decoded; addr += insts[i].size, i++)
    {
        if (isCtrlflow(&insts[i].mnemonic))
        {
            // bb begins right after the control flow instructions
            bpSeqs.push_back(i + 1);

            if (insts[i].operands.length > 0)
            {
                // find jump targets
                op = strtol((const char*) insts[i].operands.p, NULL, 0);
                if (op == 0 || op == LONG_MAX || op == LONG_MIN)
                    continue;

                map<uint64_t, uint64_t>::iterator it = offsetToSeq.find(op);
                if (it != offsetToSeq.end())
                {
                    bpSeqs.push_back(it->second);
                }
            }
        }
    }

    // unique
    sort(bpSeqs.begin(), bpSeqs.end());
    bpSeqs.erase(unique(bpSeqs.begin(), bpSeqs.end()), bpSeqs.end());

    vector<BasicBlock> basicBlocks;

    for (int i = 0; i < bpSeqs.size() - 1; i++)
    {
        int bpSeq = bpSeqs[i];
        BasicBlock nb(insts[bpSeq].offset, insts + bpSeq, bpSeqs[i+1]-bpSeqs[i]);
        basicBlocks.push_back(nb);
    }


    for (int i = 0; i < basicBlocks.size(); i++)
    {
        basicBlocks[i].display();
        if (!basicBlocks[i].verify())
            return false;
    }

    return true;
}
