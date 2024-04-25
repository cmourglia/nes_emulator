#include "cpu.h"

#include <stdio.h>

#include "disassembler.h"
#include "opcode.h"

CPU init_cpu() {
    CPU cpu = {};

    // TODO: Reset CPU state

    return cpu;
}

enum CPU_Status : u8 {
    CS_CarryFlag = 1 << 0,
    CS_ZeroFlag = 1 << 1,
    CS_InterruptDisable = 1 << 2,
    CS_DecimalModeFlag = 1 << 3,
    CS_OverflowFlag = 1 << 4,
    CS_NegativeFlag = 1 << 5,
};

void update_cpu_status(CPU* cpu, u8 value, u8 status_flags) {
    if (status_flags & CS_CarryFlag) {
        // TODO:
    }

    if (status_flags & CS_ZeroFlag) {
        cpu->zero_flag = (value == 0);
    }

    if (status_flags & CS_InterruptDisable) {
        // TODO:
    }

    if (status_flags & CS_DecimalModeFlag) {
        // TODO:
    }

    if (status_flags & CS_OverflowFlag) {
        // TODO:
    }

    if (status_flags & CS_NegativeFlag) {
        cpu->negative_flag = ((value & 0b10000000) != 0);
    }
}

void interpret_program(u8* code, u16 code_size) {
    CPU cpu = init_cpu();

    bool quit = false;

    while (!quit) {
        OpCode opcode = opcode_get_next(cpu.program_counter, code);

        switch (opcode.code) {
            case 0xA9: {
                cpu.accumulator = opcode.byte;
                update_cpu_status(&cpu, cpu.accumulator,
                                  CS_ZeroFlag | CS_NegativeFlag);
            } break;

            case 0xAA: {
                cpu.x = cpu.accumulator;
                update_cpu_status(&cpu, cpu.x, CS_ZeroFlag | CS_NegativeFlag);
            } break;

            case 0xE8: {
                cpu.x += 1;
                update_cpu_status(&cpu, cpu.x, CS_ZeroFlag | CS_NegativeFlag);
            } break;

            case 0x00: {
                quit = true;
            } break;

            default: {
                printf("Opcode `%02x` is not implemented\n", opcode.code);
            } break;
        }

        cpu.program_counter += opcode.bytes;

        if (cpu.program_counter >= code_size) {
            break;
        }
    }

    dump_cpu(&cpu);
}
