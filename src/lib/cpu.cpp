#include "cpu.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "opcode.h"

enum Consts {
    STACK_START = 0x0100,
};

void tick(CPU *cpu, int cycles) {
    UNUSED(cpu);
    UNUSED(cycles);

    // TODO: Move this in the BUS probably
    // TODO: Implement me please
    fprintf(stderr, "TODO: tick()\n");
}

static void set_flag(CPU *cpu, StatusFlags flag, bool v) {
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

    if (!code.empty()) {
        load_program(&cpu, code);
        reset_cpu(&cpu);
    }

    return cpu;
}

void reset_cpu(CPU *cpu) {
    cpu->accumulator = 0;
    cpu->x = 0;
    cpu->y = 0;
    cpu->status = UnusedFlag;
    // TODO: Find out why everyone puts sets that as 0xFD
    cpu->stack_pointer = 0xFD;

    u16 lo = (u16)read_mem(cpu, 0xFFFC);
    u16 hi = (u16)read_mem(cpu, 0xFFFC + 1);

    cpu->program_counter = (hi << 8) | lo;

    // TODO: Reset cycle count (8)
}

void load_program(CPU *cpu, const std::vector<u8> &code) {
    // TODO: Temp address
    if (sizeof(cpu->memory) - 0x8000 >= code.size()) {
        memcpy(cpu->memory + 0x8000, code.data(), code.size());
    } else {
        abort();
    }
    write_mem_u16(cpu, 0xFFFC, 0x8000);
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
            cpu->program_counter += 2;
            return 0;
        }

        case AM_Absolute_X: {
            *out_address = opcode->word + cpu->x;
            u16 old_hi = opcode->word & 0xFF00;
            u16 new_hi = *out_address & 0xFF00;

            cpu->program_counter += 2;

            // Crossing boundaries means 1 additional cycle
            return old_hi == new_hi ? 0 : 1;
        }

        case AM_Absolute_Y: {
            *out_address = opcode->word + cpu->y;
            u16 old_hi = opcode->word & 0xFF00;
            u16 new_hi = *out_address & 0xFF00;

            cpu->program_counter += 2;

            // Crossing boundaries means 1 additional cycle
            return old_hi == new_hi ? 0 : 1;
        }

        case AM_Immediate: {
            *out_address = cpu->program_counter;
            cpu->program_counter += 1;
            return 0;
        }

        case AM_Indirect: {
            u16 lo = (u16)read_mem(cpu, opcode->word);
            u16 hi = (u16)read_mem(cpu, opcode->word + 1);

            *out_address = (hi << 8) | lo;

            cpu->program_counter += 2;

            return 0;
        }

        case AM_Indirect_X: {
            u16 base = opcode->byte;
            u16 ptr = (base + cpu->x) & 0x00FF;

            u16 lo_byte = read_mem(cpu, ptr);
            u16 hi_byte = read_mem(cpu, (ptr + 1) & 0x00FF);

            *out_address = (hi_byte << 8) | lo_byte;

            cpu->program_counter += 1;

            return 0;
        }

        case AM_Indirect_Y: {
            u16 base = opcode->byte;

            u16 lo_byte = read_mem(cpu, base);
            u16 hi_byte = read_mem(cpu, (base + 1) & 0x00FF);

            *out_address = ((hi_byte << 8) | lo_byte) + cpu->y;

            u16 old_hi = hi_byte << 8;
            u16 new_hi = *out_address & 0xFF00;

            cpu->program_counter += 1;

            return old_hi == new_hi ? 0 : 1;
        }

        case AM_Relative: {
            u16 relative_address = (u16)opcode->byte;
            if (relative_address & 0x80) {
                relative_address |= 0xFF00;
            }

            *out_address = relative_address;

            cpu->program_counter += 1;

            return 0;
        }

        case AM_ZeroPage: {
            *out_address = opcode->byte;
            cpu->program_counter += 1;
            return 0;
        }

        case AM_ZeroPage_X: {
            *out_address = (u16)(opcode->byte + cpu->x) & 0x00FF;
            cpu->program_counter += 1;
            return 0;
        }

        case AM_ZeroPage_Y: {
            *out_address = (u16)(opcode->byte + cpu->y) & 0x00FF;
            cpu->program_counter += 1;
            return 0;
        }
    }

    return 0;
}

void run_cpu(CPU *cpu) {
    // TODO: We do not really want an infinite loop here...
    while (true) {
        OpCode opcode = get_next_opcode(cpu->program_counter, cpu->memory);

        cpu->program_counter += 1;

        u16 operand_address = 0;
        int cycles = opcode.cycles;

        cycles += get_operand_address(cpu, &opcode, &operand_address);

        opcode.instruction_fn(cpu, operand_address);

        tick(cpu, cycles);

        if (cpu->break_command) {
            // TODO: Handle that more properly
            break;
        }
    }
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ADC
// Add with Carry
// A,Z,C,N = A+M+C
// https://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
//
// Truth table for the overflow flag (7th bit shown for each operand)
// A M R | V
// 0 0 0 | 0
// 0 0 1 | 1
// 0 1 0 | 0
// 0 1 1 | 0
// 1 0 0 | 0
// 1 0 1 | 0
// 1 1 0 | 1
// 1 1 1 | 0
//
// So there is an overflow when both A and M are positive but R is negative
// or when both A and M are negative and R is positive.
// "Both are negative" or "Both are positive" is a XNOR gate (NOT XOR) between
// both operands A and M.
// We want to filter that out with the fact that R is different than M (resp. A)
// when V is 1, which is an XOR gate between M (resp. A) and V.
// We can then just binary AND both results to end up with our result
//
// A M R | V | A^M |~(A^M) | M^R | (M^R) & ~(A^M) |
// 0 0 0 | 0 |  0  |   1   |  0  |       0        |
// 0 0 1 | 1 |  0  |   1   |  1  |       1        |
// 0 1 0 | 0 |  1  |   0   |  1  |       0        |
// 0 1 1 | 0 |  1  |   0   |  0  |       0        |
// 1 0 0 | 0 |  1  |   0   |  0  |       0        |
// 1 0 1 | 0 |  1  |   0   |  1  |       0        |
// 1 1 0 | 1 |  0  |   1   |  1  |       1        |
// 1 1 1 | 0 |  0  |   1   |  0  |       0        |
void adc_fn(CPU *cpu, u16 operand_address) {
    u16 M = read_mem(cpu, operand_address);
    u16 A = cpu->accumulator;
    u16 C = cpu->carry_flag;
    u16 R = M + A + C;

    cpu->accumulator = (R & 0x00FF);

    set_flag(cpu, CarryFlag, R & 0xFF00);
    update_zero_and_negative_flags(cpu, cpu->accumulator);

    // Overflow, just keep 7th bit
    M = M & 0x80;
    A = A & 0x80;
    R = R & 0x80;
    set_flag(cpu, OverflowFlag, ~(A ^ M) & (M ^ R));
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#AND
// Logical AND
// A,Z,N = A&M
void and_fn(CPU *cpu, u16 operand_address) {
    cpu->accumulator = cpu->accumulator & read_mem(cpu, operand_address);
    update_zero_and_negative_flags(cpu, cpu->accumulator);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ASL
// Arithmetic Shift Left
// M,Z,C,N = M*2
void asl_fn(CPU *cpu, u16 operand_address) {
    u8 value = read_mem(cpu, operand_address);
    set_flag(cpu, CarryFlag, value & 0x80);
    value <<= 1;
    write_mem(cpu, operand_address, value);
    update_zero_and_negative_flags(cpu, value);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ASL
// Arithmetic Shift Left
// A,Z,C,N = M*2
void asl_acc_fn(CPU *cpu, u16 /* Address Mode Accumulator */) {
    set_flag(cpu, CarryFlag, cpu->accumulator & 0x80);
    cpu->accumulator <<= 1;
    update_zero_and_negative_flags(cpu, cpu->accumulator);
}

void branch(CPU *cpu, u16 relative_address) {
    tick(cpu, 1);

    u16 new_pc = cpu->program_counter + relative_address;

    if (((cpu->program_counter + 1) & 0xFF00) != (new_pc & 0xFF00)) {
        tick(cpu, 1);
    }

    cpu->program_counter = new_pc;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BCC
// Branch if Carry Clear
void bcc_fn(CPU *cpu, u16 relative_address) {
    if (!cpu->carry_flag) {
        branch(cpu, relative_address);
    }
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BCS
// Branch if Carry Set
void bcs_fn(CPU *cpu, u16 relative_address) {
    if (cpu->carry_flag) {
        branch(cpu, relative_address);
    }
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BEQ
// Branch if Equal
void beq_fn(CPU *cpu, u16 relative_address) {
    if (cpu->zero_flag) {
        branch(cpu, relative_address);
    }
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BIT
// Bit Test
// A & M, N = M7, V = M6
void bit_fn(CPU *cpu, u16 operand_address) {
    u8 memory = read_mem(cpu, operand_address);

    set_flag(cpu, ZeroFlag, (cpu->accumulator & memory) == 0);
    set_flag(cpu, OverflowFlag, memory & (1 << 6));
    set_flag(cpu, NegativeFlag, memory & (1 << 7));
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BMI
// Branch if Minus
void bmi_fn(CPU *cpu, u16 relative_address) {
    if (cpu->negative_flag) {
        branch(cpu, relative_address);
    }
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BNE
// Branch if Not Equal
void bne_fn(CPU *cpu, u16 relative_address) {
    if (!cpu->zero_flag) {
        branch(cpu, relative_address);
    }
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BPL
// Branch if Positive
void bpl_fn(CPU *cpu, u16 relative_address) {
    if (!cpu->negative_flag) {
        branch(cpu, relative_address);
    }
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BRK
// Force Interrupt
void brk_fn(CPU *cpu, u16 /* Address Mode Implied */) {
    push_stack_u16(cpu, cpu->program_counter + 1);

    set_flag(cpu, BreakCommand, true);

    push_stack(cpu, cpu->status);

    cpu->program_counter = read_mem_u16(cpu, 0xFFFE);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BVC
// Branch if Overflow Clear
void bvc_fn(CPU *cpu, u16 relative_address) {
    if (!cpu->overflow_flag) {
        branch(cpu, relative_address);
    }
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BVS
// Branch if Overflow Set
void bvs_fn(CPU *cpu, u16 relative_address) {
    if (cpu->overflow_flag) {
        branch(cpu, relative_address);
    }
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#CLC
// Clear Carry Flag
void clc_fn(CPU *cpu, u16 operand_address) {
    UNUSED(operand_address);
    set_flag(cpu, CarryFlag, false);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#CLD
// Clear Decimal Mode
void cld_fn(CPU *cpu, u16 operand_address) {
    UNUSED(operand_address);
    set_flag(cpu, DecimalModeFlag, false);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#CLI
// Clear Interrupt Disable
void cli_fn(CPU *cpu, u16 operand_address) {
    UNUSED(operand_address);
    set_flag(cpu, InterruptDisable, false);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#CLV
// Clear Overflow Flag
void clv_fn(CPU *cpu, u16 operand_address) {
    UNUSED(operand_address);
    set_flag(cpu, OverflowFlag, false);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#CMP
// Compare
// Z,C,N = A-M
void cmp_fn(CPU *cpu, u16 operand_address) {
    u8 memory = read_mem(cpu, operand_address);

    set_flag(cpu, CarryFlag, cpu->accumulator >= memory);
    update_zero_and_negative_flags(cpu, cpu->accumulator - memory);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#CPX
// Compare X Register
// Z,C,N = X-M
void cpx_fn(CPU *cpu, u16 operand_address) {
    u8 memory = read_mem(cpu, operand_address);

    set_flag(cpu, CarryFlag, cpu->x >= memory);
    update_zero_and_negative_flags(cpu, cpu->x - memory);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#CPY
// Compare Y Register
// Z,C,N = Y-M
void cpy_fn(CPU *cpu, u16 operand_address) {
    u8 memory = read_mem(cpu, operand_address);

    set_flag(cpu, CarryFlag, cpu->y >= memory);
    update_zero_and_negative_flags(cpu, cpu->y - memory);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#DEC
// Decrement memory
// M,Z,N = M-1
void dec_fn(CPU *cpu, u16 operand_address) {
    u8 memory = read_mem(cpu, operand_address);
    u8 result = memory - 1;

    update_zero_and_negative_flags(cpu, result);
    write_mem(cpu, operand_address, result);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#DEX
// Decrement X Register
// X,Z,N = X-1
void dex_fn(CPU *cpu, u16 /* Address Mode Implied */) {
    cpu->x -= 1;
    update_zero_and_negative_flags(cpu, cpu->x);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#DEY
// Decrement Y Register
// Y,Z,N = Y-1
void dey_fn(CPU *cpu, u16 /* Address Mode Implied */) {
    cpu->y -= 1;
    update_zero_and_negative_flags(cpu, cpu->y);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#EOR
// Exclusive OR
// A,Z,N = A^M
void eor_fn(CPU *cpu, u16 operand_address) {
    u8 memory = read_mem(cpu, operand_address);
    cpu->accumulator ^= memory;
    update_zero_and_negative_flags(cpu, cpu->accumulator);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#INC
// Increment memory
// M,Z,N = M + 1
void inc_fn(CPU *cpu, u16 operand_address) {
    u8 result = read_mem(cpu, operand_address) - 1;
    write_mem(cpu, operand_address, result);
    update_zero_and_negative_flags(cpu, result);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#INX
// Increment X Register
// X,Z,N = X+1
void inx_fn(CPU *cpu, u16 /* Address Mode Implied */) {
    cpu->x += 1;
    update_zero_and_negative_flags(cpu, cpu->x);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#INY
// Increment Y Register
// Y,Z,N = Y+1
void iny_fn(CPU *cpu, u16 /* Address Mode Implied */) {
    cpu->y += 1;
    update_zero_and_negative_flags(cpu, cpu->y);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#JMP
// Jump
void jmp_fn(CPU *cpu, u16 operand_address) {
    cpu->program_counter = operand_address;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#JSR
// Jump to Subroutine
void jsr_fn(CPU *cpu, u16 operand_address) {
    // TODO: Check pushed PC
    push_stack_u16(cpu, cpu->program_counter + 3 - 1);
    cpu->program_counter = operand_address;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#LDA
// Load Accumulator
// A,Z,N = M
void lda_fn(CPU *cpu, u16 operand_address) {
    cpu->accumulator = read_mem(cpu, operand_address);
    update_zero_and_negative_flags(cpu, cpu->accumulator);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#LDX
// Load X Register
// X,Z,N = M
void ldx_fn(CPU *cpu, u16 operand_address) {
    cpu->x = read_mem(cpu, operand_address);
    update_zero_and_negative_flags(cpu, cpu->x);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#LDY
// Load Y Register
// Y,Z,N = M
void ldy_fn(CPU *cpu, u16 operand_address) {
    cpu->y = read_mem(cpu, operand_address);
    update_zero_and_negative_flags(cpu, cpu->y);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#LSR
// Logical Shift Right
// M,C,Z,N = M/2
void lsr_fn(CPU *cpu, u16 operand_address) {
    u8 value = read_mem(cpu, operand_address);
    set_flag(cpu, CarryFlag, value & 0x01);
    value >>= 1;
    write_mem(cpu, operand_address, value);
    update_zero_and_negative_flags(cpu, value);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#LSR
// Logical Shift Right
// A,C,Z,N = A/2
void lsr_acc_fn(CPU *cpu, u16 /* Address Mode Accumulator */) {
    set_flag(cpu, CarryFlag, cpu->accumulator & 0x80);
    cpu->accumulator >>= 1;
    update_zero_and_negative_flags(cpu, cpu->accumulator);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#NOP
// No Operation
void nop_fn(CPU *cpu, u16 operand_address) {
    UNUSED(cpu);
    UNUSED(operand_address);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ORA
// Logical Inclusive OR
// A,Z,N = A|M
void ora_fn(CPU *cpu, u16 operand_address) {
    cpu->accumulator |= read_mem(cpu, operand_address);
    update_zero_and_negative_flags(cpu, cpu->accumulator);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#PHA
// Push Accumulator
void pha_fn(CPU *cpu, u16 /* Address Mode Implied */) {
    push_stack(cpu, cpu->accumulator);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#PHP
// Push Processor Status
void php_fn(CPU *cpu, u16 /* Address Mode Implied */) {
    push_stack(cpu, cpu->status);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#PLA
// Pull Accumulator
void pla_fn(CPU *cpu, u16 /* Address Mode Implied */) {
    cpu->accumulator = pop_stack(cpu);
    update_zero_and_negative_flags(cpu, cpu->accumulator);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#PLP
// Pull Processor Status
void plp_fn(CPU *cpu, u16 /* Address Mode Implied */) {
    cpu->status = pop_stack(cpu);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ROL
// Rotate Left
void rol_fn(CPU *cpu, u16 operand_address) {
    u8 value = read_mem(cpu, operand_address);

    bool carry = value & 0x80;
    value = (value << 1) | cpu->carry_flag;

    set_flag(cpu, CarryFlag, carry);
    write_mem(cpu, operand_address, value);
    update_zero_and_negative_flags(cpu, value);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ROL
// Rotate Left
void rol_acc_fn(CPU *cpu, u16 /* Address Mode Implied */) {
    bool carry = cpu->accumulator & 0x80;
    cpu->accumulator = (cpu->accumulator << 1) | cpu->carry_flag;

    set_flag(cpu, CarryFlag, carry);
    update_zero_and_negative_flags(cpu, cpu->accumulator);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ROR
// Rotate Right
void ror_fn(CPU *cpu, u16 operand_address) {
    u8 value = read_mem(cpu, operand_address);

    bool carry = value & 0x01;
    value = (value >> 1) | (cpu->carry_flag << 7);

    set_flag(cpu, CarryFlag, carry);
    write_mem(cpu, operand_address, value);
    update_zero_and_negative_flags(cpu, value);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ROR
void ror_acc_fn(CPU *cpu, u16 /* Address Mode Accumulator */) {
    bool carry = cpu->accumulator & 0x01;
    cpu->accumulator = (cpu->accumulator >> 1) | (cpu->carry_flag << 7);

    set_flag(cpu, CarryFlag, carry);
    update_zero_and_negative_flags(cpu, cpu->accumulator);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#RTI
// Return From Interrupt
void rti_fn(CPU *cpu, u16 /* Address Mode Implied */) {
    cpu->status = pop_stack(cpu);
    cpu->program_counter = pop_stack_u16(cpu);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#RTS
// Return From Subroutine
void rts_fn(CPU *cpu, u16 /* Address Mode Implied */) {
    cpu->program_counter = pop_stack_u16(cpu);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#SBC
// Subtract with Carry
// A,Z,C,N = A-M-(1-C)
// In practice,
// A - M - B <=> A - M - B + 256
//           <=> A - M - (1-C) + 256
//           <=> A + (255-M) + C
//           <=> A + ~M + C
// So, overall, same as the addition, just complement M
void sbc_fn(CPU *cpu, u16 operand_address) {
    u16 M = ~read_mem(cpu, operand_address) & 0x00FF;
    u16 A = cpu->accumulator;
    u16 C = cpu->carry_flag;
    u16 R = A + M + C;

    cpu->accumulator = (R & 0x00FF);

    set_flag(cpu, CarryFlag, R & 0xFF00);
    update_zero_and_negative_flags(cpu, cpu->accumulator);

    // Overflow, just keep 7th bit
    M = M & 0x80;
    A = A & 0x80;
    R = R & 0x80;
    set_flag(cpu, OverflowFlag, ~(A ^ M) & (M ^ R));
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#SEC
// Set Carry Flag
void sec_fn(CPU *cpu, u16 /* Address Mode Implied */) {
    set_flag(cpu, CarryFlag, true);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#SED
// Set Decimal Flag
void sed_fn(CPU *cpu, u16 /* Address Mode Implied */) {
    set_flag(cpu, DecimalModeFlag, true);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#SEI
// Set Interrupt Disable Flag
void sei_fn(CPU *cpu, u16 /* Address Mode Implied */) {
    set_flag(cpu, InterruptDisable, true);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#STA
// Store Accumulator
// M = A
void sta_fn(CPU *cpu, u16 operand_address) {
    write_mem(cpu, operand_address, cpu->accumulator);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#STX
// Store X Register
// M = X
void stx_fn(CPU *cpu, u16 operand_address) {
    write_mem(cpu, operand_address, cpu->x);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#STY
// Store Y Register
// M = Y
void sty_fn(CPU *cpu, u16 operand_address) {
    write_mem(cpu, operand_address, cpu->y);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#TAX
// Transfer Accumulator to X
// X = A
void tax_fn(CPU *cpu, u16 /* Address Mode Implied */) {
    cpu->x = cpu->accumulator;
    update_zero_and_negative_flags(cpu, cpu->x);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#TAY
// Transfer Accumulator to Y
// Y = A
void tay_fn(CPU *cpu, u16 /* Address Mode Implied */) {
    cpu->y = cpu->accumulator;
    update_zero_and_negative_flags(cpu, cpu->y);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#TSX
// Transfer Stack Pointer to X
// X = S
void tsx_fn(CPU *cpu, u16 /* Address Mode Implied */) {
    cpu->x = cpu->stack_pointer;
    update_zero_and_negative_flags(cpu, cpu->x);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#TXA
// Transfer X to Accumulator
// A = X
void txa_fn(CPU *cpu, u16 /* Address Mode Implied */) {
    cpu->accumulator = cpu->x;
    update_zero_and_negative_flags(cpu, cpu->accumulator);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#TXS
// Transfer X to Stack Pointer
// S = X
void txs_fn(CPU *cpu, u16 /* Address Mode Implied */) {
    cpu->stack_pointer = cpu->x;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#TYA
// Transfer Y to Accumulator
// A = Y
void tya_fn(CPU *cpu, u16 /* Address Mode Implied */) {
    cpu->accumulator = cpu->y;
    update_zero_and_negative_flags(cpu, cpu->accumulator);
}
