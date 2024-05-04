#pragma once

#include "common.h"

#include <string>
#include <vector>

#include "cpu.h"

std::vector<std::string> disassemble_code(const std::vector<u8> &code);

void dump_cpu(CPU *cpu);
