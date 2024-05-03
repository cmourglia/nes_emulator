#include "cpu.h"

#include <stdio.h>
#include <string.h>

#include "common.h"
#include "disassembler.h"
#include "opcode.h"

enum Flags : u8 {
    CarryFlag = 1 << 0,         // Carry
    ZeroFlag = 1 << 1,          // Zero
    InterruptDisable = 1 << 2,  // Disable interrupts
    DecimalModeFlag = 1 << 3,   // Decimal mode (unused on the NES)
    BreakCommand = 1 << 4,      // Break
    UnusedFlag = 1 << 5,        // Unused
    OverflowFlag = 1 << 6,      // Overflow
    NegativeFlag = 1 << 7,      // Negative
};

enum Consts {
    STACK_START = 0x0100,
};

static void set_flag(CPU *cpu, Flags flag, bool v) {
    if (v) {
        cpu->status |= flag;
    } else {
        cpu->status &= ~flag;
    }
}

static void update_zero_and_negative_flags(CPU *cpu, u8 value) {
    cpu->zero_flag = (value == 0) ? 1 : 0;
    cpu->negative_flag = ((value & 0x80) == 0) ? 0 : 1;
}

static u8 read_mem(CPU *cpu, u16 addr) {
    return cpu->memory[addr];
}

static void write_mem(CPU *cpu, u16 addr, u8 value) {
    cpu->memory[addr] = value;
}

static u16 read_mem_u16(CPU *cpu, u16 addr) {
    u16 lo = (u16)read_mem(cpu, addr) & 0x00FF;
    u16 hi = (u16)read_mem(cpu, addr + 1) & 0x00FF;

    return (hi << 8) | lo;
}

static void write_mem_u16(CPU *cpu, u16 addr, u16 value) {
    write_mem(cpu, addr, value & 0xFF);
    write_mem(cpu, addr + 1, (value >> 8) & 0xFF);
}

static u8 pop_stack(CPU *cpu) {
    cpu->stack_pointer += 1;
    return read_mem(cpu, STACK_START + cpu->stack_pointer);
}

static void push_stack(CPU *cpu, u8 value) {
    write_mem(cpu, STACK_START + cpu->stack_pointer, value);
    cpu->stack_pointer -= 1;
}

static u16 pop_stack_u16(CPU *cpu) {
    return pop_stack(cpu) | (pop_stack(cpu) << 8);
}

static void push_stack_u16(CPU *cpu, u16 value) {
    push_stack(cpu, (value >> 8) & 0x00FF);
    push_stack(cpu, value & 0x00FF);
}

CPU init_cpu(const std::vector<u8> &code) {
    CPU cpu = {};

    if (code.size() > 0) {
        load_program(&cpu, code);
        reset_cpu(&cpu);
    }

    return cpu;
}

void reset_cpu(CPU *cpu) {
    cpu->accumulator = 0;
    cpu->x = 0;
    cpu->y = 0;
    cpu->status = 0 | UnusedFlag;
    // TODO: Find out why everyone puts sets that as 0xFD
    cpu->stack_pointer = 0xFD;

    u16 lo = (u16)read_mem(cpu, 0xFFFC);
    u16 hi = (u16)read_mem(cpu, 0xFFFC + 1);

    cpu->program_counter = (hi << 8) | lo;

    // TODO: Reset cycle count (8)
}

void load_program(CPU *cpu, const std::vector<u8> &code) {
    // TODO: Temp address
    memcpy(cpu->memory + 0x8042, code.data(), code.size());
    write_mem_u16(cpu, 0xFFFC, 0x8042);
}

// Return true if page was crossed
int get_operand_address(CPU *cpu, OpCode *opcode, u16 *out_address) {
    switch (opcode->addressing_mode) {
        case AM_Accumulator:
        case AM_Implied:
            // Handled by specific functions directly
            return 0;

        case AM_Absolute: {
            *out_address = opcode->word;
            return 0;
        }

        case AM_Absolute_X: {
            *out_address = opcode->word + cpu->x;
            u16 old_hi = opcode->word & 0xFF00;
            u16 new_hi = *out_address & 0xFF00;
            // Crossing boundaries means 1 additional cycle
            return old_hi == new_hi ? 0 : 1;
        }

        case AM_Absolute_Y: {
            *out_address = opcode->word + cpu->y;
            u16 old_hi = opcode->word & 0xFF00;
            u16 new_hi = *out_address & 0xFF00;
            // Crossing boundaries means 1 additional cycle
            return old_hi == new_hi ? 0 : 1;
        }

        case AM_Immediate: {
            *out_address = cpu->program_counter + 1;
            return 0;
        }

        case AM_Indirect: {
            u16 lo = (u16)read_mem(cpu, opcode->word);
            u16 hi = (u16)read_mem(cpu, opcode->word + 1);

            *out_address = (hi << 8) | lo;

            return 0;
        }

        case AM_Indirect_X: {
            u16 base = opcode->byte;
            u16 ptr = (base + cpu->x) & 0x00FF;

            u16 lo_byte = read_mem(cpu, ptr);
            u16 hi_byte = read_mem(cpu, (ptr + 1) & 0x00FF);

            *out_address = (hi_byte << 8) | lo_byte;
            return 0;
        }

        case AM_Indirect_Y: {
            u16 base = opcode->byte;

            u16 lo_byte = read_mem(cpu, base);
            u16 hi_byte = read_mem(cpu, (base + 1) & 0x00FF);

            *out_address = ((hi_byte << 8) | lo_byte) + cpu->y;

            u16 old_hi = hi_byte << 8;
            u16 new_hi = *out_address & 0xFF00;

            return old_hi == new_hi ? 0 : 1;
        }

        case AM_Relative: {
            u16 relative_address = (u16)opcode->byte;
            if (relative_address & 0x80) {
                relative_address |= 0xFF00;
            }

            *out_address = relative_address;
        }

        case AM_ZeroPage: {
            *out_address = opcode->byte;
            return 0;
        }

        case AM_ZeroPage_X: {
            *out_address = (u16)(opcode->byte + cpu->x) & 0x00FF;
            return 0;
        }

        case AM_ZeroPage_Y: {
            *out_address = (u16)(opcode->byte + cpu->y) & 0x00FF;
            return 0;
        }
    }

    return 0;
}

void run_cpu(CPU *cpu) {
    // TODO: We do not really want an infinite loop here...
    while (true) {
        OpCode opcode = get_next_opcode(cpu->program_counter, cpu->memory);

        u16 operand_address = 0;
        int cycles = opcode.cycles;

        cycles += get_operand_address(cpu, &opcode, &operand_address);

        opcode.instruction_fn(cpu, &opcode, operand_address);

        cpu->program_counter += opcode.bytes;

        if (cpu->break_command) {
            // TODO: Handle that more properly
            break;
        }
    }
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ADC
void adc_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
    UNUSED(cpu);
    UNUSED(opcode);
    UNUSED(operand_address);

    // TODO
    abort();
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#AND
// Logical AND
// A,Z,N = A&M
void and_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
    UNUSED(opcode);

    cpu->accumulator = cpu->accumulator & read_mem(cpu, operand_address);
    update_zero_and_negative_flags(cpu, cpu->accumulator);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ASL
// Arithmetic Shift Left
// M,Z,C,N = M*2
void asl_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
    UNUSED(opcode);

    u8 value = read_mem(cpu, operand_address);
    set_flag(cpu, CarryFlag, value & 0x80);
    value <<= 1;
    write_mem(cpu, operand_address, value);
    update_zero_and_negative_flags(cpu, value);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ASL
// Arithmetic Shift Left
// A,Z,C,N = M*2
void asl_acc_fn(CPU *cpu, OpCode *opcode, u16) {
    UNUSED(opcode);

    u8 value = cpu->accumulator;
    set_flag(cpu, CarryFlag, value & 0x80);
    value <<= 1;
    cpu->accumulator = value;
    update_zero_and_negative_flags(cpu, value);
}

void branch(CPU *cpu, OpCode *opcode, u16 relative_address) {
    opcode->cycles += 1;

    u16 new_pc = cpu->program_counter + 1 + relative_address;

    if (((cpu->program_counter + 1) & 0xFF00) != (new_pc & 0xFF00)) {
        opcode->cycles += 1;
    }

    cpu->program_counter = new_pc;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BCC
// Branch if Carry Clear
void bcc_fn(CPU *cpu, OpCode *opcode, u16 relative_address) {
    if (!cpu->carry_flag) {
        branch(cpu, opcode, relative_address);
    }
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BCS
// Branch if Carry Set
void bcs_fn(CPU *cpu, OpCode *opcode, u16 relative_address) {
    if (cpu->carry_flag) {
        branch(cpu, opcode, relative_address);
    }
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BEQ
// Branch if Equal
void beq_fn(CPU *cpu, OpCode *opcode, u16 relative_address) {
    if (cpu->zero_flag) {
        branch(cpu, opcode, relative_address);
    }
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BIT
// Bit Test
// A & M, N = M7, V = M6
void bit_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
    UNUSED(opcode);

    u8 memory = read_mem(cpu, operand_address);

    set_flag(cpu, ZeroFlag, (cpu->accumulator & memory) == 0);
    set_flag(cpu, OverflowFlag, memory & (1 << 6));
    set_flag(cpu, NegativeFlag, memory & (1 << 7));
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BMI
// Branch if Minus
void bmi_fn(CPU *cpu, OpCode *opcode, u16 relative_address) {
    if (cpu->negative_flag) {
        branch(cpu, opcode, relative_address);
    }
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BNE
// Branch if Not Equal
void bne_fn(CPU *cpu, OpCode *opcode, u16 relative_address) {
    if (!cpu->zero_flag) {
        branch(cpu, opcode, relative_address);
    }
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BPL
// Branch if Positive
void bpl_fn(CPU *cpu, OpCode *opcode, u16 relative_address) {
    if (!cpu->negative_flag) {
        branch(cpu, opcode, relative_address);
    }
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BRK
// Force Interrupt
void brk_fn(CPU *cpu, OpCode *opcode, u16) {
    UNUSED(opcode);

    push_stack_u16(cpu, cpu->program_counter + 1);

    set_flag(cpu, BreakCommand, true);

    push_stack(cpu, cpu->status);

    cpu->program_counter = read_mem_u16(cpu, 0xFFFE);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BVC
// Branch if Overflow Clear
void bvc_fn(CPU *cpu, OpCode *opcode, u16 relative_address) {
    if (!cpu->overflow_flag) {
        branch(cpu, opcode, relative_address);
    }
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BVS
// Branch if Overflow Set
void bvs_fn(CPU *cpu, OpCode *opcode, u16 relative_address) {
    if (cpu->overflow_flag) {
        branch(cpu, opcode, relative_address);
    }
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#CLC
// Clear Carry Flag
void clc_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
    UNUSED(opcode);
    UNUSED(operand_address);
    set_flag(cpu, CarryFlag, false);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#CLD
// Clear Decimal Mode
void cld_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
    UNUSED(opcode);
    UNUSED(operand_address);
    set_flag(cpu, DecimalModeFlag, false);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#CLI
// Clear Interrupt Disable
void cli_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
    UNUSED(opcode);
    UNUSED(operand_address);
    set_flag(cpu, InterruptDisable, false);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#CLV
// Clear Overflow Flag
void clv_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
    UNUSED(opcode);
    UNUSED(operand_address);
    set_flag(cpu, OverflowFlag, false);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#CMP
// Compare
// Z,C,N = A-M
void cmp_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
    UNUSED(opcode);

    u8 memory = read_mem(cpu, operand_address);
    set_flag(cpu, CarryFlag, cpu->accumulator >= memory);
    set_flag(cpu, ZeroFlag, cpu->accumulator == memory);
    set_flag(cpu, NegativeFlag, (cpu->accumulator - memory) & 0x80);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#CPX
// Compare X Register
// Z,C,N = X-M
void cpx_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
    UNUSED(opcode);

    u8 memory = read_mem(cpu, operand_address);
    set_flag(cpu, CarryFlag, cpu->x >= memory);
    set_flag(cpu, ZeroFlag, cpu->x == memory);
    set_flag(cpu, NegativeFlag, (cpu->x - memory) & 0x80);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#CPY
// Compare Y Register
// Z,C,N = Y-M
void cpy_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
    UNUSED(opcode);

    u8 memory = read_mem(cpu, operand_address);
    set_flag(cpu, CarryFlag, cpu->y >= memory);
    set_flag(cpu, ZeroFlag, cpu->y == memory);
    set_flag(cpu, NegativeFlag, (cpu->y - memory) & 0x80);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#DEC
void dec_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#DEX
void dex_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#DEY
void dey_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#EOR
void eor_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#INC
void inc_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#INX
void inx_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
    cpu->x += 1;
    update_zero_and_negative_flags(cpu, cpu->x);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#INY
void iny_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#JMP
void jmp_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#JSR
void jsr_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#LDA
void lda_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
    cpu->accumulator = read_mem(cpu, operand_address);
    update_zero_and_negative_flags(cpu, cpu->accumulator);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#LDX
void ldx_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#LDY
void ldy_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#LSR
void lsr_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#LSR
void lsr_acc_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#NOP
void nop_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ORA
void ora_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#PHA
void pha_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#PHP
void php_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#PLA
void pla_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#PLP
void plp_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ROL
void rol_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ROL
void rol_acc_fn(CPU *cpu, OpCode *opcode, u16) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ROR
void ror_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ROR
void ror_acc_fn(CPU *cpu, OpCode *opcode, u16) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#RTI
void rti_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#RTS
void rts_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#SBC
void sbc_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#SEC
void sec_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#SED
void sed_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#SEI
void sei_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#STA
void sta_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#STX
void stx_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#STY
void sty_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#TAX
void tax_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
    cpu->x = cpu->accumulator;
    update_zero_and_negative_flags(cpu, cpu->x);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#TAY
void tay_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#TSX
void tsx_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#TXA
void txa_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#TXS
void txs_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#TYA
void tya_fn(CPU *cpu, OpCode *opcode, u16 operand_address) {
}
