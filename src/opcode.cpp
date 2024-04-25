#include "opcode.h"

#include <unordered_map>

// There is probably tons of better ways to handle that,
// lets KISS for now and see it that causes an issue in practice
// Filled in using https://www.masswerk.at/6502/6502_instruction_set.html
static std::unordered_map<u8, OpCode> opcodes = {
    {0x00, {0x00, 1, 7, AM_Implied}},     {0x01, {0x01, 2, 6, AM_Indirect_X}},
    {0x05, {0x05, 2, 3, AM_ZeroPage}},    {0x06, {0x06, 2, 5, AM_ZeroPage}},
    {0x08, {0x08, 1, 3, AM_Implied}},     {0x09, {0x09, 2, 2, AM_Immediate}},
    {0x0A, {0x0A, 1, 2, AM_Accumulator}}, {0x0D, {0x0D, 3, 4, AM_Absolute}},
    {0x0E, {0x0E, 3, 6, AM_Absolute}},

    {0x10, {0x10, 2, 2, AM_Relative}},    {0x11, {0x11, 2, 5, AM_Indirect_Y}},
    {0x15, {0x15, 2, 4, AM_ZeroPage_X}},  {0x16, {0x16, 2, 6, AM_ZeroPage_X}},
    {0x18, {0x18, 1, 2, AM_Implied}},     {0x19, {0x19, 3, 4, AM_Absolute_Y}},
    {0x1D, {0x1D, 3, 4, AM_Absolute_X}},  {0x1E, {0x1E, 3, 7, AM_Absolute_X}},

    {0x20, {0x20, 3, 6, AM_Absolute}},    {0x21, {0x21, 2, 6, AM_Indirect_X}},
    {0x24, {0x24, 2, 3, AM_ZeroPage}},    {0x25, {0x25, 2, 3, AM_ZeroPage}},
    {0x26, {0x26, 2, 5, AM_ZeroPage}},    {0x28, {0x28, 1, 4, AM_Implied}},
    {0x29, {0x29, 2, 2, AM_Immediate}},   {0x2A, {0x2A, 1, 2, AM_Accumulator}},
    {0x2C, {0x2C, 3, 4, AM_Absolute}},    {0x2D, {0x2D, 3, 4, AM_Absolute}},
    {0x2E, {0x2E, 3, 6, AM_Absolute}},

    {0x30, {0x30, 2, 2, AM_Relative}},    {0x31, {0x31, 2, 5, AM_Indirect_Y}},
    {0x35, {0x35, 2, 4, AM_ZeroPage_X}},  {0x36, {0x36, 2, 6, AM_ZeroPage_X}},
    {0x38, {0x38, 1, 2, AM_Implied}},     {0x39, {0x39, 3, 4, AM_Absolute_Y}},
    {0x3D, {0x3D, 3, 4, AM_Absolute_X}},  {0x3E, {0x3E, 3, 7, AM_Absolute_X}},

    {0x40, {0x40, 1, 6, AM_Implied}},     {0x41, {0x41, 2, 6, AM_Indirect_X}},
    {0x45, {0x45, 2, 3, AM_ZeroPage}},    {0x46, {0x46, 2, 5, AM_ZeroPage}},
    {0x48, {0x48, 1, 3, AM_Implied}},     {0x49, {0x49, 2, 2, AM_Immediate}},
    {0x4A, {0x4A, 1, 2, AM_Accumulator}}, {0x4C, {0x4C, 3, 3, AM_Absolute}},
    {0x4D, {0x4D, 3, 4, AM_Absolute}},    {0x4E, {0x4E, 3, 6, AM_Absolute}},

    {0x50, {0x50, 2, 2, AM_Relative}},    {0x51, {0x51, 2, 5, AM_Indirect_Y}},
    {0x55, {0x55, 2, 4, AM_ZeroPage_X}},  {0x56, {0x56, 2, 6, AM_ZeroPage_X}},
    {0x58, {0x58, 1, 2, AM_Implied}},     {0x59, {0x59, 3, 4, AM_Absolute_Y}},
    {0x5D, {0x5D, 3, 4, AM_Absolute_X}},  {0x5E, {0x5E, 3, 7, AM_Absolute_X}},

    {0x60, {0x60, 1, 6, AM_Implied}},     {0x61, {0x61, 2, 6, AM_Indirect_X}},
    {0x65, {0x65, 2, 3, AM_ZeroPage}},    {0x66, {0x66, 2, 5, AM_ZeroPage}},
    {0x68, {0x68, 1, 4, AM_Implied}},     {0x69, {0x69, 2, 2, AM_Immediate}},
    {0x6A, {0x6A, 1, 2, AM_Accumulator}}, {0x6C, {0x6C, 3, 5, AM_Indirect}},
    {0x6D, {0x6D, 3, 4, AM_Absolute}},    {0x6E, {0x6E, 3, 6, AM_Absolute}},

    {0x70, {0x70, 2, 2, AM_Relative}},    {0x71, {0x71, 2, 5, AM_Indirect_Y}},
    {0x75, {0x75, 2, 4, AM_ZeroPage_X}},  {0x76, {0x76, 2, 6, AM_ZeroPage_X}},
    {0x78, {0x78, 1, 2, AM_Implied}},     {0x79, {0x79, 3, 4, AM_Absolute_Y}},
    {0x7D, {0x7D, 3, 4, AM_Absolute_X}},  {0x7E, {0x7E, 3, 7, AM_Absolute_X}},

    {0x81, {0x81, 2, 6, AM_Indirect_X}},  {0x84, {0x84, 2, 3, AM_ZeroPage}},
    {0x85, {0x85, 2, 3, AM_ZeroPage}},    {0x86, {0x86, 2, 3, AM_ZeroPage}},
    {0x88, {0x88, 1, 2, AM_Implied}},     {0x8A, {0x8A, 1, 2, AM_Implied}},
    {0x8C, {0x8C, 3, 4, AM_Absolute}},    {0x8D, {0x8D, 3, 4, AM_Absolute}},
    {0x8E, {0x8E, 3, 4, AM_Absolute}},

    {0x90, {0x90, 2, 2, AM_Relative}},    {0x91, {0x91, 2, 6, AM_Indirect_Y}},
    {0x94, {0x94, 2, 4, AM_ZeroPage_X}},  {0x95, {0x95, 2, 4, AM_ZeroPage_X}},
    {0x96, {0x96, 2, 4, AM_ZeroPage_Y}},  {0x98, {0x98, 1, 2, AM_Implied}},
    {0x99, {0x99, 3, 5, AM_Absolute_Y}},  {0x9A, {0x9A, 1, 2, AM_Implied}},
    {0x9D, {0x9D, 3, 5, AM_Absolute_X}},

    {0xA0, {0xA0, 2, 2, AM_Immediate}},   {0xA1, {0xA1, 2, 6, AM_Indirect_X}},
    {0xA2, {0xA2, 2, 2, AM_Immediate}},   {0xA4, {0xA4, 2, 3, AM_ZeroPage}},
    {0xA5, {0xA5, 2, 3, AM_ZeroPage}},    {0xA6, {0xA6, 2, 3, AM_ZeroPage}},
    {0xA8, {0xA8, 1, 2, AM_Implied}},     {0xA9, {0xA9, 2, 2, AM_Immediate}},
    {0xAA, {0xAA, 1, 2, AM_Implied}},     {0xAC, {0xAC, 3, 4, AM_Absolute}},
    {0xAD, {0xAD, 3, 4, AM_Absolute}},    {0xAE, {0xAE, 3, 4, AM_Absolute}},

    {0xB0, {0xB0, 2, 2, AM_Relative}},    {0xB1, {0xB1, 2, 5, AM_Indirect_Y}},
    {0xB4, {0xB4, 2, 4, AM_ZeroPage_X}},  {0xB5, {0xB5, 2, 4, AM_ZeroPage_X}},
    {0xB6, {0xB6, 2, 4, AM_ZeroPage_Y}},  {0xB8, {0xB8, 1, 2, AM_Implied}},
    {0xB9, {0xB9, 3, 4, AM_Absolute_Y}},  {0xBA, {0xBA, 1, 2, AM_Implied}},
    {0xBC, {0xBC, 3, 4, AM_Absolute_X}},  {0xBD, {0xBD, 3, 4, AM_Absolute_X}},
    {0xBE, {0xBE, 3, 4, AM_Absolute_Y}},

    {0xC0, {0xC0, 2, 2, AM_Immediate}},   {0xC1, {0xC1, 2, 6, AM_Indirect_X}},
    {0xC4, {0xC4, 2, 3, AM_ZeroPage}},    {0xC5, {0xC5, 2, 3, AM_ZeroPage}},
    {0xC6, {0xC6, 2, 5, AM_ZeroPage}},    {0xC8, {0xC8, 1, 2, AM_Implied}},
    {0xC9, {0xC9, 2, 2, AM_Immediate}},   {0xCA, {0xCA, 1, 2, AM_Implied}},
    {0xCC, {0xCC, 3, 4, AM_Absolute}},    {0xCD, {0xCD, 3, 4, AM_Absolute}},
    {0xCE, {0xCE, 3, 6, AM_Absolute}},

    {0xD0, {0xD0, 2, 2, AM_Relative}},    {0xD1, {0xD1, 2, 5, AM_Indirect_Y}},
    {0xD5, {0xD5, 2, 4, AM_ZeroPage_X}},  {0xD6, {0xD6, 2, 6, AM_ZeroPage_X}},
    {0xD8, {0xD8, 1, 2, AM_Implied}},     {0xD9, {0xD9, 3, 4, AM_Absolute_Y}},
    {0xDD, {0xDD, 3, 4, AM_Absolute_X}},  {0xDE, {0xDE, 3, 7, AM_Absolute_X}},

    {0xE0, {0xE0, 2, 2, AM_Immediate}},   {0xE1, {0xE1, 2, 6, AM_Indirect_X}},
    {0xE4, {0xE4, 2, 3, AM_ZeroPage}},    {0xE5, {0xE5, 2, 3, AM_ZeroPage}},
    {0xE6, {0xE6, 2, 5, AM_ZeroPage}},    {0xE8, {0xE8, 1, 2, AM_Implied}},
    {0xE9, {0xE9, 2, 2, AM_Immediate}},   {0xEA, {0xEA, 1, 2, AM_Implied}},
    {0xEC, {0xEC, 3, 4, AM_Absolute}},    {0xED, {0xED, 3, 4, AM_Absolute}},
    {0xEE, {0xEE, 3, 6, AM_Absolute}},

    {0xF0, {0xF0, 2, 2, AM_Relative}},    {0xF1, {0xF1, 2, 5, AM_Indirect_Y}},
    {0xF5, {0xF5, 2, 4, AM_ZeroPage_X}},  {0xF6, {0xF6, 2, 6, AM_ZeroPage_X}},
    {0xF8, {0xF8, 1, 2, AM_Implied}},     {0xF9, {0xF9, 3, 4, AM_Absolute_Y}},
    {0xFD, {0xFD, 3, 4, AM_Absolute_X}},  {0xFE, {0xFE, 3, 7, AM_Absolute_X}},
};

static std::unordered_map<u8, const char*> opcode_mnemonics = {
    {0x00, "BRK"}, {0x01, "ORA"}, {0x05, "ORA"}, {0x06, "ASL"}, {0x08, "PHP"},
    {0x09, "ORA"}, {0x0A, "ASL"}, {0x0D, "ORA"}, {0x0E, "ASL"}, {0x10, "BPL"},
    {0x11, "ORA"}, {0x15, "ORA"}, {0x16, "ASL"}, {0x18, "CLC"}, {0x19, "ORA"},
    {0x1D, "ORA"}, {0x1E, "ASL"}, {0x20, "JSR"}, {0x21, "AND"}, {0x24, "BIT"},
    {0x25, "AND"}, {0x26, "ROL"}, {0x28, "PLP"}, {0x29, "AND"}, {0x2A, "ROL"},
    {0x2C, "BIT"}, {0x2D, "AND"}, {0x2E, "ROL"}, {0x30, "BMI"}, {0x31, "AND"},
    {0x35, "AND"}, {0x36, "ROL"}, {0x38, "SEC"}, {0x39, "AND"}, {0x3D, "AND"},
    {0x3E, "ROL"}, {0x40, "RTI"}, {0x41, "EOR"}, {0x45, "EOR"}, {0x46, "LSR"},
    {0x48, "PHA"}, {0x49, "EOR"}, {0x4A, "LSR"}, {0x4C, "JMP"}, {0x4D, "EOR"},
    {0x4E, "LSR"}, {0x50, "BVC"}, {0x51, "EOR"}, {0x55, "EOR"}, {0x56, "LSR"},
    {0x58, "CLI"}, {0x59, "EOR"}, {0x5D, "EOR"}, {0x5E, "LSR"}, {0x60, "RTS"},
    {0x61, "ADC"}, {0x65, "ADC"}, {0x66, "ROR"}, {0x68, "PLA"}, {0x69, "ADC"},
    {0x6A, "ROR"}, {0x6C, "JMP"}, {0x6D, "ADC"}, {0x6E, "ROR"}, {0x70, "BVS"},
    {0x71, "ADC"}, {0x75, "ADC"}, {0x76, "ROR"}, {0x78, "SEI"}, {0x79, "ADC"},
    {0x7D, "ADC"}, {0x7E, "ROR"}, {0x81, "STA"}, {0x84, "STY"}, {0x85, "STA"},
    {0x86, "STX"}, {0x88, "DEY"}, {0x8A, "TXA"}, {0x8C, "STY"}, {0x8D, "STA"},
    {0x8E, "STX"}, {0x90, "BCC"}, {0x91, "STA"}, {0x94, "STY"}, {0x95, "STA"},
    {0x96, "STX"}, {0x98, "TYA"}, {0x99, "STA"}, {0x9A, "TXS"}, {0x9D, "STA"},
    {0xA0, "LDY"}, {0xA1, "LDA"}, {0xA2, "LDX"}, {0xA4, "LDY"}, {0xA5, "LDA"},
    {0xA6, "LDX"}, {0xA8, "TAY"}, {0xA9, "LDA"}, {0xAA, "TAX"}, {0xAC, "LDY"},
    {0xAD, "LDA"}, {0xAE, "LDX"}, {0xB0, "BCS"}, {0xB1, "LDA"}, {0xB4, "LDY"},
    {0xB5, "LDA"}, {0xB6, "LDX"}, {0xB8, "CLV"}, {0xB9, "LDA"}, {0xBA, "TSX"},
    {0xBC, "LDY"}, {0xBD, "LDA"}, {0xBE, "LDX"}, {0xC0, "CPY"}, {0xC1, "CMP"},
    {0xC4, "CPY"}, {0xC5, "CMP"}, {0xC6, "DEC"}, {0xC8, "INY"}, {0xC9, "CMP"},
    {0xCA, "DEX"}, {0xCC, "CPY"}, {0xCD, "CMP"}, {0xCE, "DEC"}, {0xD0, "BNE"},
    {0xD1, "CMP"}, {0xD5, "CMP"}, {0xD6, "DEC"}, {0xD8, "CLD"}, {0xD9, "CMP"},
    {0xDD, "CMP"}, {0xDE, "DEC"}, {0xE0, "CPX"}, {0xE1, "SBC"}, {0xE4, "CPX"},
    {0xE5, "SBC"}, {0xE6, "INC"}, {0xE8, "INX"}, {0xE9, "SBC"}, {0xEA, "NOP"},
    {0xEC, "CPX"}, {0xED, "SBC"}, {0xEE, "INC"}, {0xF0, "BEQ"}, {0xF1, "SBC"},
    {0xF5, "SBC"}, {0xF6, "INC"}, {0xF8, "SED"}, {0xF9, "SBC"}, {0xFD, "SBC"},
    {0xFE, "INC"},
};

OpCode opcode_get_next(u16 program_counter, u8* memory) {
    OpCode code = opcodes.at(memory[program_counter]);

    switch (code.bytes) {
        case 1: {
            // No-op
        } break;

        case 2: {
            code.lo_byte = memory[program_counter + 1];
        } break;

        case 3: {
            code.lo_byte = memory[program_counter + 1];
            code.hi_byte = memory[program_counter + 2];
        } break;

        default: {
            abort();
        }
    }

    return code;
}

const char* opcode_get_mnemonic(u8 code) { return opcode_mnemonics.at(code); }