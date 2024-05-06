#pragma once

#include "cpu.h"

enum Instruction {
    I_ADC,
    I_AND,
    I_ASL,
    I_BCC,
    I_BCS,
    I_BEQ,
    I_BIT,
    I_BMI,
    I_BNE,
    I_BPL,
    I_BRK,
    I_BVC,
    I_BVS,
    I_CLC,
    I_CLD,
    I_CLI,
    I_CLV,
    I_CMP,
    I_CPX,
    I_CPY,
    I_DEC,
    I_DEX,
    I_DEY,
    I_EOR,
    I_INC,
    I_INX,
    I_INY,
    I_JMP,
    I_JSR,
    I_LDA,
    I_LDX,
    I_LDY,
    I_LSR,
    I_NOP,
    I_ORA,
    I_PHA,
    I_PHP,
    I_PLA,
    I_PLP,
    I_ROL,
    I_ROR,
    I_RTI,
    I_RTS,
    I_SBC,
    I_SEC,
    I_SED,
    I_SEI,
    I_STA,
    I_STX,
    I_STY,
    I_TAX,
    I_TAY,
    I_TSX,
    I_TXA,
    I_TXS,
    I_TYA,

    // Unofficial opcodes
    I_ALR,
    I_ANC,
    I_ANE,
    I_ARR,
    I_DCP,
    I_ISC,
    I_JAM,
    I_LAS,
    I_LAX,
    I_LXA,
    I_RLA,
    I_SLO,
    I_SRE,
    I_RRA,
    I_SAX,
    I_SBX,
    I_SHA,
    I_SHX,
    I_SHY,
    I_TAS,
    I_USC,
};

struct OpCode {
    void (*instruction_fn)(CPU *, u16);

    Instruction instruction;
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

struct Bus;

OpCode get_next_opcode(Bus* bus, u16 program_counter);
const char *get_instruction_mnemonic(Instruction instruction);
