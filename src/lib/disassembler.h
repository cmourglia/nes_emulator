#pragma once

#include "common.h"

#include <string>
#include <span>

#include "cpu.h"

std::vector<std::string> disassemble_code(Bus* bus);

void dump_cpu(CPU *cpu);

struct OpCode;
void disassemble_cpu(CPU* cpu, OpCode* opcode);
