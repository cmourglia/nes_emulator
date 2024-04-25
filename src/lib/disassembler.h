#pragma once

#include "common.h"

#include <string>
#include <vector>

#include "cpu.h"

std::vector<std::string> disassemble_code(u8* code, u16 code_size);

void dump_cpu(CPU* cpu);