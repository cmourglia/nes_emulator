#pragma once

#include "cpu.h"

struct OpCode {
    u8 code;
    u8 bytes;
    u8 cycles;
    AddressingMode addressing_mode;

    union {
        struct {
            u8 byte;
            u8 _ignored;
        };
        u16 word;
    };
};

OpCode opcode_get_next(u16 program_counter, u8* memory);
const char* opcode_get_mnemonic(u8 code);