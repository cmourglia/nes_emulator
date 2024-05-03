#include "opcode.h"

#include <stdlib.h>
#include <unordered_map>

extern void adc_fn(CPU *, OpCode *, u16);
extern void and_fn(CPU *, OpCode *, u16);
extern void asl_fn(CPU *, OpCode *, u16);
extern void bcc_fn(CPU *, OpCode *, u16);
extern void bcs_fn(CPU *, OpCode *, u16);
extern void beq_fn(CPU *, OpCode *, u16);
extern void bit_fn(CPU *, OpCode *, u16);
extern void bmi_fn(CPU *, OpCode *, u16);
extern void bne_fn(CPU *, OpCode *, u16);
extern void bpl_fn(CPU *, OpCode *, u16);
extern void brk_fn(CPU *, OpCode *, u16);
extern void bvc_fn(CPU *, OpCode *, u16);
extern void bvs_fn(CPU *, OpCode *, u16);
extern void clc_fn(CPU *, OpCode *, u16);
extern void cld_fn(CPU *, OpCode *, u16);
extern void cli_fn(CPU *, OpCode *, u16);
extern void clv_fn(CPU *, OpCode *, u16);
extern void cmp_fn(CPU *, OpCode *, u16);
extern void cpx_fn(CPU *, OpCode *, u16);
extern void cpy_fn(CPU *, OpCode *, u16);
extern void dec_fn(CPU *, OpCode *, u16);
extern void dex_fn(CPU *, OpCode *, u16);
extern void dey_fn(CPU *, OpCode *, u16);
extern void eor_fn(CPU *, OpCode *, u16);
extern void inc_fn(CPU *, OpCode *, u16);
extern void inx_fn(CPU *, OpCode *, u16);
extern void iny_fn(CPU *, OpCode *, u16);
extern void jmp_fn(CPU *, OpCode *, u16);
extern void jsr_fn(CPU *, OpCode *, u16);
extern void lda_fn(CPU *, OpCode *, u16);
extern void ldx_fn(CPU *, OpCode *, u16);
extern void ldy_fn(CPU *, OpCode *, u16);
extern void lsr_fn(CPU *, OpCode *, u16);
extern void nop_fn(CPU *, OpCode *, u16);
extern void ora_fn(CPU *, OpCode *, u16);
extern void pha_fn(CPU *, OpCode *, u16);
extern void php_fn(CPU *, OpCode *, u16);
extern void pla_fn(CPU *, OpCode *, u16);
extern void plp_fn(CPU *, OpCode *, u16);
extern void rol_fn(CPU *, OpCode *, u16);
extern void ror_fn(CPU *, OpCode *, u16);
extern void rti_fn(CPU *, OpCode *, u16);
extern void rts_fn(CPU *, OpCode *, u16);
extern void sbc_fn(CPU *, OpCode *, u16);
extern void sec_fn(CPU *, OpCode *, u16);
extern void sed_fn(CPU *, OpCode *, u16);
extern void sei_fn(CPU *, OpCode *, u16);
extern void sta_fn(CPU *, OpCode *, u16);
extern void stx_fn(CPU *, OpCode *, u16);
extern void sty_fn(CPU *, OpCode *, u16);
extern void tax_fn(CPU *, OpCode *, u16);
extern void tay_fn(CPU *, OpCode *, u16);
extern void tsx_fn(CPU *, OpCode *, u16);
extern void txa_fn(CPU *, OpCode *, u16);
extern void txs_fn(CPU *, OpCode *, u16);
extern void tya_fn(CPU *, OpCode *, u16);

// Special cases: Accumulator addressing mode
extern void asl_acc_fn(CPU *, OpCode *, u16);
extern void lsr_acc_fn(CPU *, OpCode *, u16);
extern void rol_acc_fn(CPU *, OpCode *, u16);
extern void ror_acc_fn(CPU *, OpCode *, u16);

// There is probably tons of better ways to handle that,
// lets KISS for now and see it that causes an issue in practice
// Filled in using https://www.masswerk.at/6502/6502_instruction_set.html
static std::unordered_map<u8, OpCode> opcodes = {
    {0x00, {brk_fn, I_BRK, 1, 7, AM_Implied}},
    {0x01, {ora_fn, I_ORA, 2, 6, AM_Indirect_X}},
    {0x05, {ora_fn, I_ORA, 2, 3, AM_ZeroPage}},
    {0x06, {asl_fn, I_ASL, 2, 5, AM_ZeroPage}},
    {0x08, {php_fn, I_PHP, 1, 3, AM_Implied}},
    {0x09, {ora_fn, I_ORA, 2, 2, AM_Immediate}},
    {0x0A, {asl_acc_fn, I_ASL, 1, 2, AM_Accumulator}},
    {0x0D, {ora_fn, I_ORA, 3, 4, AM_Absolute}},
    {0x0E, {asl_fn, I_ASL, 3, 6, AM_Absolute}},

    {0x10, {bpl_fn, I_BPL, 2, 2, AM_Relative}},
    {0x11, {ora_fn, I_ORA, 2, 5, AM_Indirect_Y}},
    {0x15, {ora_fn, I_ORA, 2, 4, AM_ZeroPage_X}},
    {0x16, {asl_fn, I_ASL, 2, 6, AM_ZeroPage_X}},
    {0x18, {clc_fn, I_CLC, 1, 2, AM_Implied}},
    {0x19, {ora_fn, I_ORA, 3, 4, AM_Absolute_Y}},
    {0x1D, {ora_fn, I_ORA, 3, 4, AM_Absolute_X}},
    {0x1E, {asl_fn, I_ASL, 3, 7, AM_Absolute_X}},

    {0x20, {jsr_fn, I_JSR, 3, 6, AM_Absolute}},
    {0x21, {and_fn, I_AND, 2, 6, AM_Indirect_X}},
    {0x24, {bit_fn, I_BIT, 2, 3, AM_ZeroPage}},
    {0x25, {and_fn, I_AND, 2, 3, AM_ZeroPage}},
    {0x26, {rol_fn, I_ROL, 2, 5, AM_ZeroPage}},
    {0x28, {plp_fn, I_PLP, 1, 4, AM_Implied}},
    {0x29, {and_fn, I_AND, 2, 2, AM_Immediate}},
    {0x2A, {rol_acc_fn, I_ROL, 1, 2, AM_Accumulator}},
    {0x2C, {bit_fn, I_BIT, 3, 4, AM_Absolute}},
    {0x2D, {and_fn, I_AND, 3, 4, AM_Absolute}},
    {0x2E, {rol_fn, I_ROL, 3, 6, AM_Absolute}},

    {0x30, {bmi_fn, I_BMI, 2, 2, AM_Relative}},
    {0x31, {and_fn, I_AND, 2, 5, AM_Indirect_Y}},
    {0x35, {and_fn, I_AND, 2, 4, AM_ZeroPage_X}},
    {0x36, {rol_fn, I_ROL, 2, 6, AM_ZeroPage_X}},
    {0x38, {sec_fn, I_SEC, 1, 2, AM_Implied}},
    {0x39, {and_fn, I_AND, 3, 4, AM_Absolute_Y}},
    {0x3D, {and_fn, I_AND, 3, 4, AM_Absolute_X}},
    {0x3E, {rol_fn, I_ROL, 3, 7, AM_Absolute_X}},

    {0x40, {rti_fn, I_RTI, 1, 6, AM_Implied}},
    {0x41, {eor_fn, I_EOR, 2, 6, AM_Indirect_X}},
    {0x45, {eor_fn, I_EOR, 2, 3, AM_ZeroPage}},
    {0x46, {lsr_fn, I_LSR, 2, 5, AM_ZeroPage}},
    {0x48, {pha_fn, I_PHA, 1, 3, AM_Implied}},
    {0x49, {eor_fn, I_EOR, 2, 2, AM_Immediate}},
    {0x4A, {lsr_acc_fn, I_LSR, 1, 2, AM_Accumulator}},
    {0x4C, {jmp_fn, I_JMP, 3, 3, AM_Absolute}},
    {0x4D, {eor_fn, I_EOR, 3, 4, AM_Absolute}},
    {0x4E, {lsr_fn, I_LSR, 3, 6, AM_Absolute}},

    {0x50, {bvc_fn, I_BVC, 2, 2, AM_Relative}},
    {0x51, {eor_fn, I_EOR, 2, 5, AM_Indirect_Y}},
    {0x55, {eor_fn, I_EOR, 2, 4, AM_ZeroPage_X}},
    {0x56, {lsr_fn, I_LSR, 2, 6, AM_ZeroPage_X}},
    {0x58, {cli_fn, I_CLI, 1, 2, AM_Implied}},
    {0x59, {eor_fn, I_EOR, 3, 4, AM_Absolute_Y}},
    {0x5D, {eor_fn, I_EOR, 3, 4, AM_Absolute_X}},
    {0x5E, {lsr_fn, I_LSR, 3, 7, AM_Absolute_X}},

    {0x60, {rts_fn, I_RTS, 1, 6, AM_Implied}},
    {0x61, {adc_fn, I_ADC, 2, 6, AM_Indirect_X}},
    {0x65, {adc_fn, I_ADC, 2, 3, AM_ZeroPage}},
    {0x66, {ror_fn, I_ROR, 2, 5, AM_ZeroPage}},
    {0x68, {pla_fn, I_PLA, 1, 4, AM_Implied}},
    {0x69, {adc_fn, I_ADC, 2, 2, AM_Immediate}},
    {0x6A, {ror_acc_fn, I_ROR, 1, 2, AM_Accumulator}},
    {0x6C, {jmp_fn, I_JMP, 3, 5, AM_Indirect}},
    {0x6D, {adc_fn, I_ADC, 3, 4, AM_Absolute}},
    {0x6E, {ror_fn, I_ROR, 3, 6, AM_Absolute}},

    {0x70, {bvs_fn, I_BVS, 2, 2, AM_Relative}},
    {0x71, {adc_fn, I_ADC, 2, 5, AM_Indirect_Y}},
    {0x75, {adc_fn, I_ADC, 2, 4, AM_ZeroPage_X}},
    {0x76, {ror_fn, I_ROR, 2, 6, AM_ZeroPage_X}},
    {0x78, {sei_fn, I_SEI, 1, 2, AM_Implied}},
    {0x79, {adc_fn, I_ADC, 3, 4, AM_Absolute_Y}},
    {0x7D, {adc_fn, I_ADC, 3, 4, AM_Absolute_X}},
    {0x7E, {ror_fn, I_ROR, 3, 7, AM_Absolute_X}},

    {0x81, {sta_fn, I_STA, 2, 6, AM_Indirect_X}},
    {0x84, {sty_fn, I_STY, 2, 3, AM_ZeroPage}},
    {0x85, {sta_fn, I_STA, 2, 3, AM_ZeroPage}},
    {0x86, {stx_fn, I_STX, 2, 3, AM_ZeroPage}},
    {0x88, {dey_fn, I_DEY, 1, 2, AM_Implied}},
    {0x8A, {txa_fn, I_TXA, 1, 2, AM_Implied}},
    {0x8C, {sty_fn, I_STY, 3, 4, AM_Absolute}},
    {0x8D, {sta_fn, I_STA, 3, 4, AM_Absolute}},
    {0x8E, {stx_fn, I_STX, 3, 4, AM_Absolute}},

    {0x90, {bcc_fn, I_BCC, 2, 2, AM_Relative}},
    {0x91, {sta_fn, I_STA, 2, 6, AM_Indirect_Y}},
    {0x94, {sty_fn, I_STY, 2, 4, AM_ZeroPage_X}},
    {0x95, {sta_fn, I_STA, 2, 4, AM_ZeroPage_X}},
    {0x96, {stx_fn, I_STX, 2, 4, AM_ZeroPage_Y}},
    {0x98, {tya_fn, I_TYA, 1, 2, AM_Implied}},
    {0x99, {sta_fn, I_STA, 3, 5, AM_Absolute_Y}},
    {0x9A, {txs_fn, I_TXS, 1, 2, AM_Implied}},
    {0x9D, {sta_fn, I_STA, 3, 5, AM_Absolute_X}},

    {0xA0, {ldy_fn, I_LDY, 2, 2, AM_Immediate}},
    {0xA1, {lda_fn, I_LDA, 2, 6, AM_Indirect_X}},
    {0xA2, {ldx_fn, I_LDX, 2, 2, AM_Immediate}},
    {0xA4, {ldy_fn, I_LDY, 2, 3, AM_ZeroPage}},
    {0xA5, {lda_fn, I_LDA, 2, 3, AM_ZeroPage}},
    {0xA6, {ldx_fn, I_LDX, 2, 3, AM_ZeroPage}},
    {0xA8, {tay_fn, I_TAY, 1, 2, AM_Implied}},
    {0xA9, {lda_fn, I_LDA, 2, 2, AM_Immediate}},
    {0xAA, {tax_fn, I_TAX, 1, 2, AM_Implied}},
    {0xAC, {ldy_fn, I_LDY, 3, 4, AM_Absolute}},
    {0xAD, {lda_fn, I_LDA, 3, 4, AM_Absolute}},
    {0xAE, {ldx_fn, I_LDX, 3, 4, AM_Absolute}},

    {0xB0, {bcs_fn, I_BCS, 2, 2, AM_Relative}},
    {0xB1, {lda_fn, I_LDA, 2, 5, AM_Indirect_Y}},
    {0xB4, {ldy_fn, I_LDY, 2, 4, AM_ZeroPage_X}},
    {0xB5, {lda_fn, I_LDA, 2, 4, AM_ZeroPage_X}},
    {0xB6, {ldx_fn, I_LDX, 2, 4, AM_ZeroPage_Y}},
    {0xB8, {clv_fn, I_CLV, 1, 2, AM_Implied}},
    {0xB9, {lda_fn, I_LDA, 3, 4, AM_Absolute_Y}},
    {0xBA, {tsx_fn, I_TSX, 1, 2, AM_Implied}},
    {0xBC, {ldy_fn, I_LDY, 3, 4, AM_Absolute_X}},
    {0xBD, {lda_fn, I_LDA, 3, 4, AM_Absolute_X}},
    {0xBE, {ldx_fn, I_LDX, 3, 4, AM_Absolute_Y}},

    {0xC0, {cpy_fn, I_CPY, 2, 2, AM_Immediate}},
    {0xC1, {cmp_fn, I_CMP, 2, 6, AM_Indirect_X}},
    {0xC4, {cpy_fn, I_CPY, 2, 3, AM_ZeroPage}},
    {0xC5, {cmp_fn, I_CMP, 2, 3, AM_ZeroPage}},
    {0xC6, {dec_fn, I_DEC, 2, 5, AM_ZeroPage}},
    {0xC8, {iny_fn, I_INY, 1, 2, AM_Implied}},
    {0xC9, {cmp_fn, I_CMP, 2, 2, AM_Immediate}},
    {0xCA, {dex_fn, I_DEX, 1, 2, AM_Implied}},
    {0xCC, {cpy_fn, I_CPY, 3, 4, AM_Absolute}},
    {0xCD, {cmp_fn, I_CMP, 3, 4, AM_Absolute}},
    {0xCE, {dec_fn, I_DEC, 3, 6, AM_Absolute}},

    {0xD0, {bne_fn, I_BNE, 2, 2, AM_Relative}},
    {0xD1, {cmp_fn, I_CMP, 2, 5, AM_Indirect_Y}},
    {0xD5, {cmp_fn, I_CMP, 2, 4, AM_ZeroPage_X}},
    {0xD6, {dec_fn, I_DEC, 2, 6, AM_ZeroPage_X}},
    {0xD8, {cld_fn, I_CLD, 1, 2, AM_Implied}},
    {0xD9, {cmp_fn, I_CMP, 3, 4, AM_Absolute_Y}},
    {0xDD, {cmp_fn, I_CMP, 3, 4, AM_Absolute_X}},
    {0xDE, {dec_fn, I_DEC, 3, 7, AM_Absolute_X}},

    {0xE0, {cpx_fn, I_CPX, 2, 2, AM_Immediate}},
    {0xE1, {sbc_fn, I_SBC, 2, 6, AM_Indirect_X}},
    {0xE4, {cpx_fn, I_CPX, 2, 3, AM_ZeroPage}},
    {0xE5, {sbc_fn, I_SBC, 2, 3, AM_ZeroPage}},
    {0xE6, {inc_fn, I_INC, 2, 5, AM_ZeroPage}},
    {0xE8, {inx_fn, I_INX, 1, 2, AM_Implied}},
    {0xE9, {sbc_fn, I_SBC, 2, 2, AM_Immediate}},
    {0xEA, {nop_fn, I_NOP, 1, 2, AM_Implied}},
    {0xEC, {cpx_fn, I_CPX, 3, 4, AM_Absolute}},
    {0xED, {sbc_fn, I_SBC, 3, 4, AM_Absolute}},
    {0xEE, {inc_fn, I_INC, 3, 6, AM_Absolute}},

    {0xF0, {beq_fn, I_BEQ, 2, 2, AM_Relative}},
    {0xF1, {sbc_fn, I_SBC, 2, 5, AM_Indirect_Y}},
    {0xF5, {sbc_fn, I_SBC, 2, 4, AM_ZeroPage_X}},
    {0xF6, {inc_fn, I_INC, 2, 6, AM_ZeroPage_X}},
    {0xF8, {sed_fn, I_SED, 1, 2, AM_Implied}},
    {0xF9, {sbc_fn, I_SBC, 3, 4, AM_Absolute_Y}},
    {0xFD, {sbc_fn, I_SBC, 3, 4, AM_Absolute_X}},
    {0xFE, {inc_fn, I_INC, 3, 7, AM_Absolute_X}},
};

static std::unordered_map<Instruction, const char *> opcode_mnemonics = {
    {I_ADC, "ADC"}, {I_AND, "AND"}, {I_ASL, "ASL"}, {I_BCC, "BCC"},
    {I_BCS, "BCS"}, {I_BEQ, "BEQ"}, {I_BIT, "BIT"}, {I_BMI, "BMI"},
    {I_BNE, "BNE"}, {I_BPL, "BPL"}, {I_BRK, "BRK"}, {I_BVC, "BVC"},
    {I_BVS, "BVS"}, {I_CLC, "CLC"}, {I_CLD, "CLD"}, {I_CLI, "CLI"},
    {I_CLV, "CLV"}, {I_CMP, "CMP"}, {I_CPX, "CPX"}, {I_CPY, "CPY"},
    {I_DEC, "DEC"}, {I_DEX, "DEX"}, {I_DEY, "DEY"}, {I_EOR, "EOR"},
    {I_INC, "INC"}, {I_INX, "INX"}, {I_INY, "INY"}, {I_JMP, "JMP"},
    {I_JSR, "JSR"}, {I_LDA, "LDA"}, {I_LDX, "LDX"}, {I_LDY, "LDY"},
    {I_LSR, "LSR"}, {I_NOP, "NOP"}, {I_ORA, "ORA"}, {I_PHA, "PHA"},
    {I_PHP, "PHP"}, {I_PLA, "PLA"}, {I_PLP, "PLP"}, {I_ROL, "ROL"},
    {I_ROR, "ROR"}, {I_RTI, "RTI"}, {I_RTS, "RTS"}, {I_SBC, "SBC"},
    {I_SEC, "SEC"}, {I_SED, "SED"}, {I_SEI, "SEI"}, {I_STA, "STA"},
    {I_STX, "STX"}, {I_STY, "STY"}, {I_TAX, "TAX"}, {I_TAY, "TAY"},
    {I_TSX, "TSX"}, {I_TXA, "TXA"}, {I_TXS, "TXS"}, {I_TYA, "TYA"},
};

OpCode get_next_opcode(u16 program_counter, const u8 *memory) {
    OpCode code = opcodes.at(memory[program_counter]);

    switch (code.bytes) {
        case 1: {
            // No-op
        } break;

        case 2: {
            code.byte = memory[program_counter + 1];
        } break;

        case 3: {
            code.byte = memory[program_counter + 1];
            code._ignored = memory[program_counter + 2];
        } break;

        default: {
            abort();
        }
    }

    return code;
}

const char *get_instruction_mnemonic(Instruction instruction) {
    return opcode_mnemonics.at(instruction);
}
