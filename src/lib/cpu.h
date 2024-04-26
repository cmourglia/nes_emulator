#pragma once

#include "common.h"

// https://www.nesdev.org/obelisk-6502-guide/
struct CPU {
    u16 program_counter;
    u8 stack_pointer;

    u8 accumulator;
    u8 x;
    u8 y;

    union {
        u8 status;
        struct {
            u8 carry_flag : 1;
            u8 zero_flag : 1;
            u8 interrupt_disable : 1;
            u8 decimal_mode : 1;  // Disabled on NES
            u8 break_command : 1;
            u8 unused : 1;
            u8 overflow_flag : 1;
            u8 negative_flag : 1;
        };
    };

    u8 memory[4096];
};

enum AddressingMode {
    AM_Accumulator,
    AM_Absolute,
    AM_Absolute_X,
    AM_Absolute_Y,
    AM_Immediate,
    AM_Implied,
    AM_Indirect,
    AM_Indirect_X,
    AM_Indirect_Y,
    AM_Relative,
    AM_ZeroPage,
    AM_ZeroPage_X,
    AM_ZeroPage_Y,
};

CPU init_cpu();

// TODO: This is temporary, to get started. In the future
// we will need a proper loop, with a bus and so on
void interpret_program(CPU* cpu, u8* code, u16 code_size);
