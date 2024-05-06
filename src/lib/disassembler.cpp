#include "disassembler.h"

#include "bus.h"
#include "opcode.h"

std::string dec_to_hex(int d, int size) {
    std::string result;
    result.resize(size);

    int i = 0;
    while (d > 0) {
        char c = 0;

        if (d % 16 <= 9) {
            c = '0' + (d % 16);
        } else {
            c = 'A' + ((d % 16) - 10);
        }

        result[size - i - 1] = c;
        i += 1;
        d /= 16;
    }

    while (i < size) {
        result[size - i - 1] = '0';
        i += 1;
    }

    return result;
}

std::vector<std::string> disassemble_code(Bus *bus) {
    std::vector<std::string> result;

    u32 pc = 0;

    std::string str;

    while (pc <= 0x10000) {
        OpCode opcode = get_next_opcode(bus, pc);

        str.clear();

        str += dec_to_hex((u16)pc, 4);

        str += "   ";

        str += dec_to_hex(read_mem(bus, pc), 2);
        str += " ";
        str += opcode.bytes >= 2 ? dec_to_hex(read_mem(bus, pc + 1), 2) : "  ";
        str += " ";
        str += opcode.bytes >= 3 ? dec_to_hex(read_mem(bus, pc + 2), 2) : "  ";

        str += "   ";

        str += get_instruction_mnemonic(opcode.instruction);
        str += " ";

        switch (opcode.addressing_mode) {
            case AM_Accumulator: {
                str += "A      ";
            } break;
            case AM_Absolute: {
                str += "$" + dec_to_hex(opcode.word, 4) + "  ";
            } break;
            case AM_Absolute_X: {
                str += "$" + dec_to_hex(opcode.word, 4) + ",X";
            } break;
            case AM_Absolute_Y: {
                str += "$" + dec_to_hex(opcode.word, 4) + ",Y";
            } break;
            case AM_Immediate: {
                str += "#$" + dec_to_hex(opcode.byte, 2) + "   ";
            } break;
            case AM_Implied: {
                str += "       ";
            } break;
            case AM_Indirect: {
                str += "($" + dec_to_hex(opcode.word, 4) + ")";
            } break;
            case AM_Indirect_X: {
                str += "($" + dec_to_hex(opcode.byte, 2) + ",X)";
            } break;
            case AM_Indirect_Y: {
                str += "($" + dec_to_hex(opcode.byte, 2) + "),Y";
            } break;
            case AM_Relative: {
                str += "$" + dec_to_hex(opcode.byte, 2) + "    ";
            } break;
            case AM_ZeroPage: {
                str += "$" + dec_to_hex(opcode.byte, 2) + "    ";
            } break;
            case AM_ZeroPage_X: {
                str += "$" + dec_to_hex(opcode.byte, 2) + ",X  ";
            } break;
            case AM_ZeroPage_Y: {
                str += "$" + dec_to_hex(opcode.byte, 2) + ",Y  ";
            } break;
        }

        str += "   ;";

        result.push_back(str);

        pc += opcode.bytes;
    }

    return result;
}

void dump_cpu(CPU *cpu) {
    printf(" -=-=-=- CPU Registers -=-=-=-\n");

    printf("  Program Counter: %04x\n", cpu->program_counter);
    printf("  Accumulator:     %02x (%d)\n", cpu->accumulator,
           cpu->accumulator);
    printf("  Index X:         %02x (%d)\n", cpu->x, cpu->x);
    printf("  Index Y:         %02x (%d)\n", cpu->y, cpu->y);
    printf("  CPU Status:\n");
    printf("    Carry Flag:        %d\n", cpu->carry_flag);
    printf("    Zero Flag:         %d\n", cpu->zero_flag);
    printf("    Interrupt Disable: %d\n", cpu->interrupt_disable);
    printf("    Decimal Mode:      %d\n", cpu->decimal_mode);
    printf("    Break Command:     %d\n", cpu->break_command);
    printf("    Overflow Flag:     %d\n", cpu->overflow_flag);
    printf("    Negative Flag:     %d\n", cpu->negative_flag);

    printf(" -=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
}

static u16 get_operand_address(CPU *cpu, OpCode *opcode) {
    switch (opcode->addressing_mode) {
        case AM_Accumulator: return 0;
        case AM_Implied: return 0;
        case AM_Absolute: return opcode->word;
        case AM_Absolute_X: return opcode->word + cpu->x;
        case AM_Absolute_Y: return opcode->word + cpu->y;
        case AM_Immediate: return cpu->program_counter;
        case AM_ZeroPage: return opcode->byte;
        case AM_ZeroPage_X: return (u16)(opcode->byte + cpu->x) & 0x00FF;
        case AM_ZeroPage_Y: return (u16)(opcode->byte + cpu->y) & 0x00FF;

        case AM_Indirect: {
            u16 lo = (u16)read_mem(cpu->bus, opcode->word);
            u16 hi = (u16)read_mem(cpu->bus, opcode->word + 1);

            return (hi << 8) | lo;
        }

        case AM_Indirect_X: {
            u16 base = opcode->byte;
            u16 ptr = (base + cpu->x) & 0x00FF;

            u16 lo_byte = read_mem(cpu->bus, ptr);
            u16 hi_byte = read_mem(cpu->bus, (ptr + 1) & 0x00FF);

            return (hi_byte << 8) | lo_byte;
        }

        case AM_Indirect_Y: {
            u16 base = opcode->byte;

            u16 lo_byte = read_mem(cpu->bus, base);
            u16 hi_byte = read_mem(cpu->bus, (base + 1) & 0x00FF);

            return ((hi_byte << 8) | lo_byte) + cpu->y;
        }

        case AM_Relative: {
            u16 relative_address = (u16)opcode->byte;
            if (relative_address & 0x80) {
                relative_address |= 0xFF00;
            }

            return relative_address;
        }
    }

    return 0;
}

void disassemble_cpu(CPU *cpu, OpCode *opcode) {
    printf("%04X  ", cpu->program_counter);

    Bus *bus = cpu->bus;

    switch (opcode->bytes) {
        case 1:
            printf("%02X      ", read_mem(bus, cpu->program_counter));
            break;
        case 2:
            printf("%02X %02X   ", read_mem(bus, cpu->program_counter),
                   read_mem(bus, cpu->program_counter + 1));
            break;
        case 3:
            printf("%02X %02X %02X", read_mem(bus, cpu->program_counter),
                   read_mem(bus, cpu->program_counter + 1),
                   read_mem(bus, cpu->program_counter + 2));
            break;
    }

    printf("  ");

    printf("%s ", get_instruction_mnemonic(opcode->instruction));

    int len = 0;

    u16 addr = get_operand_address(cpu, opcode);

    switch (opcode->addressing_mode) {
        case AM_Accumulator: {
            printf("A");
            len = 1;
        } break;
        case AM_Absolute: {
            printf("$%04X", addr);
            len = 5;
        } break;
        case AM_Absolute_X: {
            //                str += "$" + dec_to_hex(opcode.word, 4) + ",X";
        } break;
        case AM_Absolute_Y: {
            //                str += "$" + dec_to_hex(opcode.word, 4) + ",Y";
        } break;
        case AM_Immediate: {
            printf("#$%02X", opcode->byte);
            len = 4;
        } break;
        case AM_Implied: {
            //                str += "       ";
        } break;
        case AM_Indirect: {
            printf("($%04X) = %04X", opcode->word, addr);
            len = 14;
            //                str += "($" + dec_to_hex(opcode.word, 4) + ")";
        } break;
        case AM_Indirect_X: {
            printf("($%02X,X) @ %02X = %04X", opcode->byte,
                   (opcode->byte + cpu->x) & 0xFF, addr);
            len = 19;
            //                str += "($" + dec_to_hex(opcode.byte, 2) + ",X)";
        } break;
        case AM_Indirect_Y: {
            u16 base = opcode->byte;

            u16 lo_byte = read_mem(cpu->bus, base);
            u16 hi_byte = read_mem(cpu->bus, (base + 1) & 0x00FF);

            u16 base_addr = lo_byte | (hi_byte << 8);

            printf("($%02X),Y = %04X @ %04X", opcode->byte,
                   base_addr, addr);
            len = 21;
            //                printf("$" + dec_to_hex(opcode.byte, 2) + "),Y";
        } break;
        case AM_Relative: {
            printf("$%04X", cpu->program_counter + 2 + addr);
            len = 5;
            //                str += "$" + dec_to_hex(opcode.byte, 2) + "    ";
        } break;
        case AM_ZeroPage: {
            printf("$%02X", addr);
            len = 3;
            //                str += "$" + dec_to_hex(opcode.byte, 2) + "    ";
        } break;
        case AM_ZeroPage_X: {
            //                str += "$" + dec_to_hex(opcode.byte, 2) + ",X  ";
        } break;
        case AM_ZeroPage_Y: {
            //                str += "$" + dec_to_hex(opcode.byte, 2) + ",Y  ";
        } break;
    }

    if (opcode->addressing_mode != AM_Immediate &&
        opcode->addressing_mode != AM_Accumulator) {
        switch (opcode->instruction) {
            case I_STX:
            case I_STA:
            case I_BIT:
            case I_LDX:
            case I_LDA:
            case I_ORA:
            case I_AND:
            case I_EOR:
            case I_ADC:
            case I_CMP:
            case I_SBC:
            case I_LDY:
            case I_STY:
            case I_CPX:
            case I_CPY:
            case I_LSR:
            case I_ASL:
            case I_ROR:
            case I_ROL:
            case I_INC:
            case I_DEC:
                printf(" = %02X", read_mem(bus, addr));
                len += 5;
                break;

            default: break;
        }
    }

    while (len++ < 28) {
        printf(" ");
    }

    printf("A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%lu", cpu->accumulator,
           cpu->x, cpu->y, cpu->status, cpu->stack_pointer, cpu->cycles);

    printf("\n");
}
