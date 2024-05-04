#pragma once

#include "common.h"

#include <vector>

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

    u8 memory[0x10000];
};

enum StatusFlags : u8 {
    CarryFlag = 1 << 0,         // Carry
    ZeroFlag = 1 << 1,          // Zero
    InterruptDisable = 1 << 2,  // Disable interrupts
    DecimalModeFlag = 1 << 3,   // Decimal mode (unused on the NES)
    BreakCommand = 1 << 4,      // Break
    UnusedFlag = 1 << 5,        // Unused
    OverflowFlag = 1 << 6,      // Overflow
    NegativeFlag = 1 << 7,      // Negative
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

CPU init_cpu(const std::vector<u8> &code = {});

void reset_cpu(CPU *cpu);
void load_program(CPU *cpu, const std::vector<u8> &code);

// TODO: This is temporary, to get started. In the future
// we will need a proper loop, with a bus and so on
void run_cpu(CPU *cpu);
