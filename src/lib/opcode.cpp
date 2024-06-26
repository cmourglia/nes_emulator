#include "opcode.h"

#include <stdlib.h>
#include <unordered_map>

#include "bus.h"

extern void adc_fn(CPU *, u16);
extern void and_fn(CPU *, u16);
extern void asl_fn(CPU *, u16);
extern void bcc_fn(CPU *, u16);
extern void bcs_fn(CPU *, u16);
extern void beq_fn(CPU *, u16);
extern void bit_fn(CPU *, u16);
extern void bmi_fn(CPU *, u16);
extern void bne_fn(CPU *, u16);
extern void bpl_fn(CPU *, u16);
extern void brk_fn(CPU *, u16);
extern void bvc_fn(CPU *, u16);
extern void bvs_fn(CPU *, u16);
extern void clc_fn(CPU *, u16);
extern void cld_fn(CPU *, u16);
extern void cli_fn(CPU *, u16);
extern void clv_fn(CPU *, u16);
extern void cmp_fn(CPU *, u16);
extern void cpx_fn(CPU *, u16);
extern void cpy_fn(CPU *, u16);
extern void dec_fn(CPU *, u16);
extern void dex_fn(CPU *, u16);
extern void dey_fn(CPU *, u16);
extern void eor_fn(CPU *, u16);
extern void inc_fn(CPU *, u16);
extern void inx_fn(CPU *, u16);
extern void iny_fn(CPU *, u16);
extern void jmp_fn(CPU *, u16);
extern void jsr_fn(CPU *, u16);
extern void lda_fn(CPU *, u16);
extern void ldx_fn(CPU *, u16);
extern void ldy_fn(CPU *, u16);
extern void lsr_fn(CPU *, u16);
extern void nop_fn(CPU *, u16);
extern void ora_fn(CPU *, u16);
extern void pha_fn(CPU *, u16);
extern void php_fn(CPU *, u16);
extern void pla_fn(CPU *, u16);
extern void plp_fn(CPU *, u16);
extern void rol_fn(CPU *, u16);
extern void ror_fn(CPU *, u16);
extern void rti_fn(CPU *, u16);
extern void rts_fn(CPU *, u16);
extern void sbc_fn(CPU *, u16);
extern void sec_fn(CPU *, u16);
extern void sed_fn(CPU *, u16);
extern void sei_fn(CPU *, u16);
extern void sta_fn(CPU *, u16);
extern void stx_fn(CPU *, u16);
extern void sty_fn(CPU *, u16);
extern void tax_fn(CPU *, u16);
extern void tay_fn(CPU *, u16);
extern void tsx_fn(CPU *, u16);
extern void txa_fn(CPU *, u16);
extern void txs_fn(CPU *, u16);
extern void tya_fn(CPU *, u16);

// Special cases: Accumulator addressing mode
extern void asl_acc_fn(CPU *, u16);
extern void lsr_acc_fn(CPU *, u16);
extern void rol_acc_fn(CPU *, u16);
extern void ror_acc_fn(CPU *, u16);

// Unofficial functions
extern void jam_fn(CPU *, u16);
extern void slo_fn(CPU *, u16);
extern void anc_fn(CPU *, u16);
extern void rla_fn(CPU *, u16);
extern void sre_fn(CPU *, u16);
extern void alr_fn(CPU *, u16);
extern void rra_fn(CPU *, u16);
extern void arr_fn(CPU *, u16);
extern void sax_fn(CPU *, u16);
extern void ane_fn(CPU *, u16);
extern void sha_fn(CPU *, u16);
extern void tas_fn(CPU *, u16);
extern void shy_fn(CPU *, u16);
extern void shx_fn(CPU *, u16);
extern void lax_fn(CPU *, u16);
extern void lxa_fn(CPU *, u16);
extern void las_fn(CPU *, u16);
extern void dcp_fn(CPU *, u16);
extern void sbx_fn(CPU *, u16);
extern void isb_fn(CPU *, u16);

// There is probably tons of better ways to handle that,
// lets KISS for now and see it that causes an issue in practice
// Filled in using https://www.masswerk.at/6502/6502_instruction_set.html
static OpCode opcodes[0x100] = {
    /* 0x00 */ {brk_fn, I_BRK, 1, 7, AM_Implied},
    /* 0x01 */ {ora_fn, I_ORA, 2, 6, AM_Indirect_X},
    /* 0x02 */ {jam_fn, I_JAM, 1, 1, AM_Implied},
    /* 0x03 */ {slo_fn, I_SLO, 2, 8, AM_Indirect_X},
    /* 0x04 */ {nop_fn, I_NOP, 2, 3, AM_ZeroPage},
    /* 0x05 */ {ora_fn, I_ORA, 2, 3, AM_ZeroPage},
    /* 0x06 */ {asl_fn, I_ASL, 2, 5, AM_ZeroPage},
    /* 0x07 */ {slo_fn, I_SLO, 2, 5, AM_ZeroPage},
    /* 0x08 */ {php_fn, I_PHP, 1, 3, AM_Implied},
    /* 0x09 */ {ora_fn, I_ORA, 2, 2, AM_Immediate},
    /* 0x0A */ {asl_acc_fn, I_ASL, 1, 2, AM_Accumulator},
    /* 0x0B */ {anc_fn, I_ANC, 2, 2, AM_Immediate},
    /* 0x0C */ {nop_fn, I_NOP, 3, 4, AM_Absolute},
    /* 0x0D */ {ora_fn, I_ORA, 3, 4, AM_Absolute},
    /* 0x0E */ {asl_fn, I_ASL, 3, 6, AM_Absolute},
    /* 0x0F */ {slo_fn, I_SLO, 3, 6, AM_Absolute},

    /* 0x10 */ {bpl_fn, I_BPL, 2, 2, AM_Relative},
    /* 0x11 */ {ora_fn, I_ORA, 2, 5, AM_Indirect_Y},
    /* 0x12 */ {jam_fn, I_JAM, 1, 1, AM_Implied},
    /* 0x13 */ {slo_fn, I_SLO, 2, 8, AM_Indirect_Y},
    /* 0x14 */ {nop_fn, I_NOP, 2, 4, AM_ZeroPage_X},
    /* 0x15 */ {ora_fn, I_ORA, 2, 4, AM_ZeroPage_X},
    /* 0x16 */ {asl_fn, I_ASL, 2, 6, AM_ZeroPage_X},
    /* 0x17 */ {slo_fn, I_SLO, 2, 6, AM_ZeroPage_X},
    /* 0x18 */ {clc_fn, I_CLC, 1, 2, AM_Implied},
    /* 0x19 */ {ora_fn, I_ORA, 3, 4, AM_Absolute_Y},
    /* 0x1A */ {nop_fn, I_NOP, 1, 2, AM_Implied},
    /* 0x1B */ {slo_fn, I_SLO, 3, 7, AM_Absolute_Y},
    /* 0x1C */ {nop_fn, I_NOP, 3, 4, AM_Absolute_X},
    /* 0x1D */ {ora_fn, I_ORA, 3, 4, AM_Absolute_X},
    /* 0x1E */ {asl_fn, I_ASL, 3, 7, AM_Absolute_X},
    /* 0x1F */ {slo_fn, I_SLO, 3, 7, AM_Absolute_X},

    /* 0x20 */ {jsr_fn, I_JSR, 3, 6, AM_Absolute},
    /* 0x21 */ {and_fn, I_AND, 2, 6, AM_Indirect_X},
    /* 0x22 */ {jam_fn, I_JAM, 1, 1, AM_Implied},
    /* 0x23 */ {rla_fn, I_RLA, 2, 8, AM_Indirect_X},
    /* 0x24 */ {bit_fn, I_BIT, 2, 3, AM_ZeroPage},
    /* 0x25 */ {and_fn, I_AND, 2, 3, AM_ZeroPage},
    /* 0x26 */ {rol_fn, I_ROL, 2, 5, AM_ZeroPage},
    /* 0x27 */ {rla_fn, I_RLA, 2, 5, AM_ZeroPage},
    /* 0x28 */ {plp_fn, I_PLP, 1, 4, AM_Implied},
    /* 0x29 */ {and_fn, I_AND, 2, 2, AM_Immediate},
    /* 0x2A */ {rol_acc_fn, I_ROL, 1, 2, AM_Accumulator},
    /* 0x2B */ {anc_fn, I_ANC, 2, 2, AM_Immediate},
    /* 0x2C */ {bit_fn, I_BIT, 3, 4, AM_Absolute},
    /* 0x2D */ {and_fn, I_AND, 3, 4, AM_Absolute},
    /* 0x2E */ {rol_fn, I_ROL, 3, 6, AM_Absolute},
    /* 0x2F */ {rla_fn, I_RLA, 3, 6, AM_Absolute},

    /* 0x30 */ {bmi_fn, I_BMI, 2, 2, AM_Relative},
    /* 0x31 */ {and_fn, I_AND, 2, 5, AM_Indirect_Y},
    /* 0x32 */ {jam_fn, I_JAM, 1, 1, AM_Implied},
    /* 0x33 */ {rla_fn, I_RLA, 2, 8, AM_Indirect_Y},
    /* 0x34 */ {nop_fn, I_NOP, 2, 4, AM_ZeroPage_X},
    /* 0x35 */ {and_fn, I_AND, 2, 4, AM_ZeroPage_X},
    /* 0x36 */ {rol_fn, I_ROL, 2, 6, AM_ZeroPage_X},
    /* 0x37 */ {rla_fn, I_RLA, 2, 6, AM_ZeroPage_X},
    /* 0x38 */ {sec_fn, I_SEC, 1, 2, AM_Implied},
    /* 0x39 */ {and_fn, I_AND, 3, 4, AM_Absolute_Y},
    /* 0x3A */ {nop_fn, I_NOP, 1, 2, AM_Implied},
    /* 0x3B */ {rla_fn, I_RLA, 3, 7, AM_Absolute_Y},
    /* 0x3C */ {nop_fn, I_NOP, 3, 4, AM_Absolute_X},
    /* 0x3D */ {and_fn, I_AND, 3, 4, AM_Absolute_X},
    /* 0x3E */ {rol_fn, I_ROL, 3, 7, AM_Absolute_X},
    /* 0x3F */ {rla_fn, I_RLA, 3, 7, AM_Absolute_X},

    /* 0x40 */ {rti_fn, I_RTI, 1, 6, AM_Implied},
    /* 0x41 */ {eor_fn, I_EOR, 2, 6, AM_Indirect_X},
    /* 0x42 */ {jam_fn, I_JAM, 1, 1, AM_Implied},
    /* 0x43 */ {sre_fn, I_SRE, 2, 8, AM_Indirect_X},
    /* 0x44 */ {nop_fn, I_NOP, 2, 3, AM_ZeroPage},
    /* 0x45 */ {eor_fn, I_EOR, 2, 3, AM_ZeroPage},
    /* 0x46 */ {lsr_fn, I_LSR, 2, 5, AM_ZeroPage},
    /* 0x47 */ {sre_fn, I_SRE, 2, 5, AM_ZeroPage},
    /* 0x48 */ {pha_fn, I_PHA, 1, 3, AM_Implied},
    /* 0x49 */ {eor_fn, I_EOR, 2, 2, AM_Immediate},
    /* 0x4A */ {lsr_acc_fn, I_LSR, 1, 2, AM_Accumulator},
    /* 0x4B */ {alr_fn, I_ALR, 2, 2, AM_Immediate},
    /* 0x4C */ {jmp_fn, I_JMP, 3, 3, AM_Absolute},
    /* 0x4D */ {eor_fn, I_EOR, 3, 4, AM_Absolute},
    /* 0x4E */ {lsr_fn, I_LSR, 3, 6, AM_Absolute},
    /* 0x4F */ {sre_fn, I_SRE, 3, 6, AM_Absolute},

    /* 0x50 */ {bvc_fn, I_BVC, 2, 2, AM_Relative},
    /* 0x51 */ {eor_fn, I_EOR, 2, 5, AM_Indirect_Y},
    /* 0x52 */ {jam_fn, I_JAM, 1, 1, AM_Implied},
    /* 0x53 */ {sre_fn, I_SRE, 2, 8, AM_Indirect_Y},
    /* 0x54 */ {nop_fn, I_NOP, 2, 4, AM_ZeroPage_X},
    /* 0x55 */ {eor_fn, I_EOR, 2, 4, AM_ZeroPage_X},
    /* 0x56 */ {lsr_fn, I_LSR, 2, 6, AM_ZeroPage_X},
    /* 0x57 */ {sre_fn, I_SRE, 2, 6, AM_ZeroPage_X},
    /* 0x58 */ {cli_fn, I_CLI, 1, 2, AM_Implied},
    /* 0x59 */ {eor_fn, I_EOR, 3, 4, AM_Absolute_Y},
    /* 0x5A */ {nop_fn, I_NOP, 1, 2, AM_Implied},
    /* 0x5B */ {sre_fn, I_SRE, 3, 7, AM_Absolute_Y},
    /* 0x5C */ {nop_fn, I_NOP, 3, 4, AM_Absolute_X},
    /* 0x5D */ {eor_fn, I_EOR, 3, 4, AM_Absolute_X},
    /* 0x5E */ {lsr_fn, I_LSR, 3, 7, AM_Absolute_X},
    /* 0x5F */ {sre_fn, I_SRE, 3, 7, AM_Absolute_X},

    /* 0x60 */ {rts_fn, I_RTS, 1, 6, AM_Implied},
    /* 0x61 */ {adc_fn, I_ADC, 2, 6, AM_Indirect_X},
    /* 0x62 */ {jam_fn, I_JAM, 1, 1, AM_Implied},
    /* 0x63 */ {rra_fn, I_RRA, 2, 8, AM_Indirect_X},
    /* 0x64 */ {nop_fn, I_NOP, 2, 3, AM_ZeroPage},
    /* 0x65 */ {adc_fn, I_ADC, 2, 3, AM_ZeroPage},
    /* 0x66 */ {ror_fn, I_ROR, 2, 5, AM_ZeroPage},
    /* 0x67 */ {rra_fn, I_RRA, 2, 5, AM_ZeroPage},
    /* 0x68 */ {pla_fn, I_PLA, 1, 4, AM_Implied},
    /* 0x69 */ {adc_fn, I_ADC, 2, 2, AM_Immediate},
    /* 0x6A */ {ror_acc_fn, I_ROR, 1, 2, AM_Accumulator},
    /* 0x6B */ {arr_fn, I_ARR, 2, 2, AM_Immediate},
    /* 0x6C */ {jmp_fn, I_JMP, 3, 5, AM_Indirect},
    /* 0x6D */ {adc_fn, I_ADC, 3, 4, AM_Absolute},
    /* 0x6E */ {ror_fn, I_ROR, 3, 6, AM_Absolute},
    /* 0x6F */ {rra_fn, I_RRA, 3, 6, AM_Absolute},

    /* 0x70 */ {bvs_fn, I_BVS, 2, 2, AM_Relative},
    /* 0x71 */ {adc_fn, I_ADC, 2, 5, AM_Indirect_Y},
    /* 0x72 */ {jam_fn, I_JAM, 1, 1, AM_Implied},
    /* 0x73 */ {rra_fn, I_RRA, 2, 8, AM_Indirect_Y},
    /* 0x74 */ {nop_fn, I_NOP, 2, 4, AM_ZeroPage_X},
    /* 0x75 */ {adc_fn, I_ADC, 2, 4, AM_ZeroPage_X},
    /* 0x76 */ {ror_fn, I_ROR, 2, 6, AM_ZeroPage_X},
    /* 0x77 */ {rra_fn, I_RRA, 2, 6, AM_ZeroPage_X},
    /* 0x78 */ {sei_fn, I_SEI, 1, 2, AM_Implied},
    /* 0x79 */ {adc_fn, I_ADC, 3, 4, AM_Absolute_Y},
    /* 0x7A */ {nop_fn, I_NOP, 1, 2, AM_Implied},
    /* 0x7B */ {rra_fn, I_RRA, 3, 7, AM_Absolute_Y},
    /* 0x7C */ {nop_fn, I_NOP, 3, 4, AM_Absolute_X},
    /* 0x7D */ {adc_fn, I_ADC, 3, 4, AM_Absolute_X},
    /* 0x7E */ {ror_fn, I_ROR, 3, 7, AM_Absolute_X},
    /* 0x7F */ {rra_fn, I_RRA, 3, 7, AM_Absolute_X},

    /* 0x80 */ {nop_fn, I_NOP, 2, 2, AM_Immediate},
    /* 0x81 */ {sta_fn, I_STA, 2, 6, AM_Indirect_X},
    /* 0x82 */ {nop_fn, I_NOP, 2, 2, AM_Immediate},
    /* 0x83 */ {sax_fn, I_SAX, 2, 6, AM_Indirect_X},
    /* 0x84 */ {sty_fn, I_STY, 2, 3, AM_ZeroPage},
    /* 0x85 */ {sta_fn, I_STA, 2, 3, AM_ZeroPage},
    /* 0x86 */ {stx_fn, I_STX, 2, 3, AM_ZeroPage},
    /* 0x87 */ {sax_fn, I_SAX, 2, 3, AM_ZeroPage},
    /* 0x88 */ {dey_fn, I_DEY, 1, 2, AM_Implied},
    /* 0x89 */ {nop_fn, I_NOP, 2, 2, AM_Immediate},
    /* 0x8A */ {txa_fn, I_TXA, 1, 2, AM_Implied},
    /* 0x8B */ {ane_fn, I_ANE, 2, 2, AM_Immediate},
    /* 0x8C */ {sty_fn, I_STY, 3, 4, AM_Absolute},
    /* 0x8D */ {sta_fn, I_STA, 3, 4, AM_Absolute},
    /* 0x8E */ {stx_fn, I_STX, 3, 4, AM_Absolute},
    /* 0x8F */ {sax_fn, I_SAX, 3, 4, AM_Absolute},

    /* 0x90 */ {bcc_fn, I_BCC, 2, 2, AM_Relative},
    /* 0x91 */ {sta_fn, I_STA, 2, 6, AM_Indirect_Y},
    /* 0x92 */ {jam_fn, I_JAM, 1, 1, AM_Implied},
    /* 0x93 */ {sha_fn, I_SHA, 2, 6, AM_Indirect_Y},
    /* 0x94 */ {sty_fn, I_STY, 2, 4, AM_ZeroPage_X},
    /* 0x95 */ {sta_fn, I_STA, 2, 4, AM_ZeroPage_X},
    /* 0x96 */ {stx_fn, I_STX, 2, 4, AM_ZeroPage_Y},
    /* 0x97 */ {sax_fn, I_SAX, 2, 4, AM_ZeroPage_Y},
    /* 0x98 */ {tya_fn, I_TYA, 1, 2, AM_Implied},
    /* 0x99 */ {sta_fn, I_STA, 3, 5, AM_Absolute_Y},
    /* 0x9A */ {txs_fn, I_TXS, 1, 2, AM_Implied},
    /* 0x9B */ {tas_fn, I_TAS, 3, 5, AM_Absolute_Y},
    /* 0x9C */ {shy_fn, I_SHY, 3, 5, AM_Absolute_X},
    /* 0x9D */ {sta_fn, I_STA, 3, 5, AM_Absolute_X},
    /* 0x9E */ {shx_fn, I_SHX, 3, 5, AM_Absolute_Y},
    /* 0x9F */ {sha_fn, I_SHA, 3, 5, AM_Absolute_Y},

    /* 0xA0 */ {ldy_fn, I_LDY, 2, 2, AM_Immediate},
    /* 0xA1 */ {lda_fn, I_LDA, 2, 6, AM_Indirect_X},
    /* 0xA2 */ {ldx_fn, I_LDX, 2, 2, AM_Immediate},
    /* 0xA3 */ {lax_fn, I_LAX, 2, 6, AM_Indirect_X},
    /* 0xA4 */ {ldy_fn, I_LDY, 2, 3, AM_ZeroPage},
    /* 0xA5 */ {lda_fn, I_LDA, 2, 3, AM_ZeroPage},
    /* 0xA6 */ {ldx_fn, I_LDX, 2, 3, AM_ZeroPage},
    /* 0xA7 */ {lax_fn, I_LAX, 2, 3, AM_ZeroPage},
    /* 0xA8 */ {tay_fn, I_TAY, 1, 2, AM_Implied},
    /* 0xA9 */ {lda_fn, I_LDA, 2, 2, AM_Immediate},
    /* 0xAA */ {tax_fn, I_TAX, 1, 2, AM_Implied},
    /* 0xAB */ {lxa_fn, I_LXA, 2, 2, AM_Immediate},
    /* 0xAC */ {ldy_fn, I_LDY, 3, 4, AM_Absolute},
    /* 0xAD */ {lda_fn, I_LDA, 3, 4, AM_Absolute},
    /* 0xAE */ {ldx_fn, I_LDX, 3, 4, AM_Absolute},
    /* 0xAF */ {lax_fn, I_LAX, 3, 4, AM_Absolute},

    /* 0xB0 */ {bcs_fn, I_BCS, 2, 2, AM_Relative},
    /* 0xB1 */ {lda_fn, I_LDA, 2, 5, AM_Indirect_Y},
    /* 0xB2 */ {jam_fn, I_JAM, 1, 1, AM_Implied},
    /* 0xB3 */ {lax_fn, I_LAX, 2, 5, AM_Indirect_Y},
    /* 0xB4 */ {ldy_fn, I_LDY, 2, 4, AM_ZeroPage_X},
    /* 0xB5 */ {lda_fn, I_LDA, 2, 4, AM_ZeroPage_X},
    /* 0xB6 */ {ldx_fn, I_LDX, 2, 4, AM_ZeroPage_Y},
    /* 0xB7 */ {lax_fn, I_LAX, 2, 4, AM_ZeroPage_Y},
    /* 0xB8 */ {clv_fn, I_CLV, 1, 2, AM_Implied},
    /* 0xB9 */ {lda_fn, I_LDA, 3, 4, AM_Absolute_Y},
    /* 0xBA */ {tsx_fn, I_TSX, 1, 2, AM_Implied},
    /* 0xBB */ {las_fn, I_LAS, 3, 4, AM_Absolute_Y},
    /* 0xBC */ {ldy_fn, I_LDY, 3, 4, AM_Absolute_X},
    /* 0xBD */ {lda_fn, I_LDA, 3, 4, AM_Absolute_X},
    /* 0xBE */ {ldx_fn, I_LDX, 3, 4, AM_Absolute_Y},
    /* 0xBF */ {lax_fn, I_LAX, 3, 4, AM_Absolute_Y},

    /* 0xC0 */ {cpy_fn, I_CPY, 2, 2, AM_Immediate},
    /* 0xC1 */ {cmp_fn, I_CMP, 2, 6, AM_Indirect_X},
    /* 0xC2 */ {nop_fn, I_NOP, 2, 2, AM_Immediate},
    /* 0xC3 */ {dcp_fn, I_DCP, 2, 8, AM_Indirect_X},
    /* 0xC4 */ {cpy_fn, I_CPY, 2, 3, AM_ZeroPage},
    /* 0xC5 */ {cmp_fn, I_CMP, 2, 3, AM_ZeroPage},
    /* 0xC6 */ {dec_fn, I_DEC, 2, 5, AM_ZeroPage},
    /* 0xC7 */ {dcp_fn, I_DCP, 2, 5, AM_ZeroPage},
    /* 0xC8 */ {iny_fn, I_INY, 1, 2, AM_Implied},
    /* 0xC9 */ {cmp_fn, I_CMP, 2, 2, AM_Immediate},
    /* 0xCA */ {dex_fn, I_DEX, 1, 2, AM_Implied},
    /* 0xCB */ {sbx_fn, I_SBX, 2, 2, AM_Immediate},
    /* 0xCC */ {cpy_fn, I_CPY, 3, 4, AM_Absolute},
    /* 0xCD */ {cmp_fn, I_CMP, 3, 4, AM_Absolute},
    /* 0xCE */ {dec_fn, I_DEC, 3, 6, AM_Absolute},
    /* 0xCF */ {dcp_fn, I_DCP, 3, 6, AM_Absolute},

    /* 0xD0 */ {bne_fn, I_BNE, 2, 2, AM_Relative},
    /* 0xD1 */ {cmp_fn, I_CMP, 2, 5, AM_Indirect_Y},
    /* 0xD2 */ {jam_fn, I_JAM, 1, 1, AM_Implied},
    /* 0xD3 */ {dcp_fn, I_DCP, 2, 8, AM_Indirect_Y},
    /* 0xD4 */ {nop_fn, I_NOP, 2, 4, AM_ZeroPage_X},
    /* 0xD5 */ {cmp_fn, I_CMP, 2, 4, AM_ZeroPage_X},
    /* 0xD6 */ {dec_fn, I_DEC, 2, 6, AM_ZeroPage_X},
    /* 0xD7 */ {dcp_fn, I_DCP, 2, 6, AM_ZeroPage_X},
    /* 0xD8 */ {cld_fn, I_CLD, 1, 2, AM_Implied},
    /* 0xD9 */ {cmp_fn, I_CMP, 3, 4, AM_Absolute_Y},
    /* 0xDA */ {nop_fn, I_NOP, 1, 2, AM_Implied},
    /* 0xDB */ {dcp_fn, I_DCP, 3, 7, AM_Absolute_Y},
    /* 0xDC */ {nop_fn, I_NOP, 3, 4, AM_Absolute_X},
    /* 0xDD */ {cmp_fn, I_CMP, 3, 4, AM_Absolute_X},
    /* 0xDE */ {dec_fn, I_DEC, 3, 7, AM_Absolute_X},
    /* 0xDF */ {dcp_fn, I_DCP, 3, 7, AM_Absolute_X},

    /* 0xE0 */ {cpx_fn, I_CPX, 2, 2, AM_Immediate},
    /* 0xE1 */ {sbc_fn, I_SBC, 2, 6, AM_Indirect_X},
    /* 0xE2 */ {nop_fn, I_NOP, 2, 2, AM_Immediate},
    /* 0xE3 */ {isb_fn, I_ISB, 2, 8, AM_Indirect_X},
    /* 0xE4 */ {cpx_fn, I_CPX, 2, 3, AM_ZeroPage},
    /* 0xE5 */ {sbc_fn, I_SBC, 2, 3, AM_ZeroPage},
    /* 0xE6 */ {inc_fn, I_INC, 2, 5, AM_ZeroPage},
    /* 0xE7 */ {isb_fn, I_ISB, 2, 5, AM_ZeroPage},
    /* 0xE8 */ {inx_fn, I_INX, 1, 2, AM_Implied},
    /* 0xE9 */ {sbc_fn, I_SBC, 2, 2, AM_Immediate},
    /* 0xEA */ {nop_fn, I_NOP, 1, 2, AM_Implied},
    /* 0xEB */ {sbc_fn, I_SBC, 2, 2, AM_Immediate},
    /* 0xEC */ {cpx_fn, I_CPX, 3, 4, AM_Absolute},
    /* 0xED */ {sbc_fn, I_SBC, 3, 4, AM_Absolute},
    /* 0xEE */ {inc_fn, I_INC, 3, 6, AM_Absolute},
    /* 0xEF */ {isb_fn, I_ISB, 3, 6, AM_Absolute},

    /* 0xF0 */ {beq_fn, I_BEQ, 2, 2, AM_Relative},
    /* 0xF1 */ {sbc_fn, I_SBC, 2, 5, AM_Indirect_Y},
    /* 0xF2 */ {jam_fn, I_JAM, 1, 1, AM_Implied},
    /* 0xF3 */ {isb_fn, I_ISB, 2, 8, AM_Indirect_Y},
    /* 0xF4 */ {nop_fn, I_NOP, 2, 4, AM_ZeroPage_X},
    /* 0xF5 */ {sbc_fn, I_SBC, 2, 4, AM_ZeroPage_X},
    /* 0xF6 */ {inc_fn, I_INC, 2, 6, AM_ZeroPage_X},
    /* 0xF7 */ {isb_fn, I_ISB, 2, 6, AM_ZeroPage_X},
    /* 0xF8 */ {sed_fn, I_SED, 1, 2, AM_Implied},
    /* 0xF9 */ {sbc_fn, I_SBC, 3, 4, AM_Absolute_Y},
    /* 0xFA */ {nop_fn, I_NOP, 1, 2, AM_Implied},
    /* 0xFB */ {isb_fn, I_ISB, 3, 7, AM_Absolute_Y},
    /* 0xFC */ {nop_fn, I_NOP, 3, 4, AM_Absolute_X},
    /* 0xFD */ {sbc_fn, I_SBC, 3, 4, AM_Absolute_X},
    /* 0xFE */ {inc_fn, I_INC, 3, 7, AM_Absolute_X},
    /* 0xFF */ {isb_fn, I_ISB, 3, 7, AM_Absolute_X},
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

    {I_ALR, "ALR"}, {I_ANC, "ANC"}, {I_ANE, "ANE"}, {I_ARR, "ARR"},
    {I_DCP, "DCP"}, {I_ISB, "ISB"}, {I_JAM, "JAM"}, {I_LAS, "LAS"},
    {I_LAX, "LAX"}, {I_LXA, "LXA"}, {I_RLA, "RLA"}, {I_SLO, "SLO"},
    {I_SRE, "SRE"}, {I_RRA, "RRA"}, {I_SAX, "SAX"}, {I_SBX, "SBX"},
    {I_SHA, "SHA"}, {I_SHX, "SHX"}, {I_SHY, "SHY"}, {I_TAS, "TAS"},
};

OpCode get_next_opcode(Bus *bus, u16 program_counter) {
    u8 mem = read_mem(bus, program_counter);
    auto code = opcodes[mem];
    switch (code.bytes) {
        case 1: {
            // No-op
        } break;

        case 2: {
            code.byte = read_mem(bus, program_counter + 1);
        } break;

        case 3: {
            code.byte = read_mem(bus, program_counter + 1);
            code._ignored = read_mem(bus, program_counter + 2);
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

bool instruction_has_additional_cycle_on_page_crossing(
    Instruction instruction) {
    switch (instruction) {
        case I_ADC:
        case I_AND:
        case I_CMP:
        case I_EOR:
        case I_LDA:
        case I_LDX:
        case I_LDY:
        case I_NOP:
        case I_ORA:
        case I_SBC:
        case I_LAS:
        case I_LAX: return true;
        default: return false;
    }
}
