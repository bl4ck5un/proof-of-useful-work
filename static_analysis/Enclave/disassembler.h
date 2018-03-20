//
// Created by fanz on 10/11/16.
//

#ifndef PARSER_DISASSEMBLER_H
#define PARSER_DISASSEMBLER_H


#include <stdint.h>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <config.h>

#include "distorm.h"
#include "config.h"
#include "../Common/Log.h"


using namespace std;

typedef uint64_t Offset;


class BasicBlock {
private:
    Offset startAddress;
    unsigned int nInstructions;
    vector<_DecodedInst*> instList;

public:
    BasicBlock(Offset startAddress, _DecodedInst* instStart, unsigned int nInstructions)
    {
        this->startAddress = startAddress;
        this->nInstructions = nInstructions;
        for (int i = 0; i < nInstructions; i++)
        {
            this->instList.push_back(instStart + i);
        }
    }

    ~BasicBlock(){}

    bool verify() const;

    void display() const
    {
        LL_DEBUG("BB at %p", this->startAddress);
        for (int i = 0; i < this->instList.size(); i++)
        {
            LL_DEBUG("%s %s",
            instList[i]->mnemonic.p,
            instList[i]->operands.p);
        }
    }

};

#endif //PARSER_DISASSEMBLER_H
