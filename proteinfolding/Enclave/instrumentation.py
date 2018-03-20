import sys
import os

asm_file = open(sys.argv[1], 'r')

import re

# match the GNU AS label syntax:
# https://sourceware.org/binutils/docs/as/Symbol-Names.html
LABEL = re.compile(r'^[a-zA-Z._][\w\$]*:')
# match a function name label (i.e. a label but not starting with .)
FUNC_LABEL = re.compile(r'^[a-zA-Z_][\w\$]*:')
# match the pouw_main function
MAIN_FUNC = re.compile(r'.*pouw_main.*')

LEADING_S = re.compile(r'\s*')

def has_instruction(line):
    line = line.strip()
    if len(line) == 0 or is_label(line) or is_comment(line) or is_derivitives(line):
        return False
    else:
        return True

def is_comment(line):
    line = line.strip()
    return line.startswith('#')

def is_label(line):
    line = line.strip()
    return LABEL.match(line) != None

def is_derivitives(line):
    line = line.strip()
    return not is_label(line) and line.startswith('.')

def is_ctrlflow(line):
    line = line.strip()
    if line.startswith('j') or \
        line.startswith('call') or \
        line.startswith('ret'):
        return True
    else:
        return False

def is_main_label(line):
    line = line.strip()
    return FUNC_LABEL.match(line) and MAIN_FUNC.match(line)

def verbatim(line):
    sys.stdout.write(line)
    sys.stdout.flush()

def is_ret(line):
    line = line.strip()
    return line.startswith('ret')

def next_inst(start, asm):
    for j in range(start, len(asm)):
        if has_instruction(asm[j]):
            break
    return j


first = True
in_main = False
ret_marked = False
beginning_found = False

next_potential_beginning = 0
asm = asm_file.readlines()
for i in range(0, len(asm)):
    line = asm[i]
    if LEADING_S.match(line):
        leading_s = LEADING_S.match(line).group()
    else:
        leading_s = ""

    # a very very special case
    # when dealing with the last ret in pouw_main, we can need to do something speical
    if not ret_marked and in_main and is_ret(line):
        print leading_s + 'movq\t%r15, %rax\t# added by PoUW'
        in_main = False
        ret_marked = True

    if not ret_marked and is_main_label(line):
        in_main = True

    if i < next_potential_beginning:
        verbatim(line)
        continue

    if is_derivitives(line) or is_comment(line):
        verbatim(line)
        continue
    if is_label(line) and is_label(asm[i+1]):
        verbatim(line)
        continue

    # now we are at the line of potential interest
    # but not always.
    if is_label(line) and not is_label(asm[i+1]):
        next_potential_beginning = next_inst(i + 1, asm)
        beginning_found = True
        verbatim(line)
        continue

    # real beginning found
    if in_main and first:
        # reset the r15
        print leading_s + 'xorq\t%r15, %r15\t# added by PoUW'
        first = False
    n_travel = 0
    # note the loop starts at i to include the beginning
    for j in range(i, len(asm)):
        if is_label(asm[j]):
            next_potential_beginning = j
            break
        if has_instruction(asm[j]):
            n_travel += 1
        if is_ctrlflow(asm[j]):
            next_potential_beginning = next_inst(j+1, asm)
            break

    if n_travel > 0:
        print leading_s + 'leaq\t%d(%%r15), %%r15\t# added by PoUW' % n_travel

    verbatim(line)

asm_file.close()
