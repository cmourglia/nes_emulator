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

static bool opcode_is_illegal(u8 opcode) {
    switch (opcode) {
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x07:
        case 0x0B:
        case 0x0C:
        case 0x0F:
        case 0x12:
        case 0x13:
        case 0x14:
        case 0x17:
        case 0x1A:
        case 0x1B:
        case 0x1C:
        case 0x1F:
        case 0x22:
        case 0x23:
        case 0x27:
        case 0x2B:
        case 0x2F:
        case 0x32:
        case 0x33:
        case 0x34:
        case 0x37:
        case 0x3A:
        case 0x3B:
        case 0x3C:
        case 0x3F:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x47:
        case 0x4B:
        case 0x4F:
        case 0x52:
        case 0x53:
        case 0x54:
        case 0x57:
        case 0x5A:
        case 0x5B:
        case 0x5C:
        case 0x5F:
        case 0x62:
        case 0x63:
        case 0x64:
        case 0x67:
        case 0x6B:
        case 0x6F:
        case 0x72:
        case 0x73:
        case 0x74:
        case 0x77:
        case 0x7A:
        case 0x7B:
        case 0x7C:
        case 0x7F:
        case 0x80:
        case 0x82:
        case 0x83:
        case 0x87:
        case 0x89:
        case 0x8B:
        case 0x8F:
        case 0x92:
        case 0x93:
        case 0x97:
        case 0x9B:
        case 0x9C:
        case 0x9E:
        case 0x9F:
        case 0xA3:
        case 0xA7:
        case 0xAB:
        case 0xAF:
        case 0xB2:
        case 0xB3:
        case 0xB7:
        case 0xBB:
        case 0xBF:
        case 0xC2:
        case 0xC3:
        case 0xC7:
        case 0xCB:
        case 0xCF:
        case 0xD2:
        case 0xD3:
        case 0xD4:
        case 0xD7:
        case 0xDA:
        case 0xDB:
        case 0xDC:
        case 0xDF:
        case 0xE2:
        case 0xE3:
        case 0xE7:
        case 0xEB:
        case 0xEF:
        case 0xF2:
        case 0xF3:
        case 0xF4:
        case 0xF7:
        case 0xFA:
        case 0xFB:
        case 0xFC:
        case 0xFF: return true;

        default: return false;
    }
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
            if (opcode->byte == 0xFF) {
                u16 lo = (u16)read_mem(cpu->bus, opcode->word);
                u16 hi = (u16)read_mem(cpu->bus, opcode->word & 0xFF00);

                return (hi << 8) | lo;
            } else {
                u16 lo = (u16)read_mem(cpu->bus, opcode->word);
                u16 hi = (u16)read_mem(cpu->bus, opcode->word + 1);

                return (hi << 8) | lo;
            }
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

    printf(
        " %c",
        (opcode_is_illegal(read_mem(bus, cpu->program_counter)) ? '*' : ' '));

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
            printf("$%04X,X @ %04X", opcode->word, addr);
            len = 14;
        } break;
        case AM_Absolute_Y: {
            printf("$%04X,Y @ %04X", opcode->word, addr);
            len = 14;
        } break;
        case AM_Immediate: {
            printf("#$%02X", opcode->byte);
            len = 4;
        } break;
        case AM_Implied: {
        } break;
        case AM_Indirect: {
            printf("($%04X) = %04X", opcode->word, addr);
            len = 14;
        } break;
        case AM_Indirect_X: {
            printf("($%02X,X) @ %02X = %04X", opcode->byte,
                   (opcode->byte + cpu->x) & 0xFF, addr);
            len = 19;
        } break;
        case AM_Indirect_Y: {
            u16 base = opcode->byte;

            u16 lo_byte = read_mem(cpu->bus, base);
            u16 hi_byte = read_mem(cpu->bus, (base + 1) & 0x00FF);

            u16 base_addr = lo_byte | (hi_byte << 8);

            printf("($%02X),Y = %04X @ %04X", opcode->byte, base_addr, addr);
            len = 21;
        } break;
        case AM_Relative: {
            printf("$%04X", (cpu->program_counter + 2 + addr) & 0xFFFF);
            len = 5;
        } break;
        case AM_ZeroPage: {
            printf("$%02X", addr & 0x00FF);
            len = 3;
        } break;
        case AM_ZeroPage_X: {
            printf("$%02X,X @ %02X", opcode->byte, addr & 0x00FF);
            len = 10;
        } break;
        case AM_ZeroPage_Y: {
            printf("$%02X,Y @ %02X", opcode->byte, addr & 0x00FF);
            len = 10;
        } break;
    }

    if (opcode->addressing_mode != AM_Immediate &&
        opcode->addressing_mode != AM_Accumulator &&
        opcode->addressing_mode != AM_Implied) {
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
            case I_NOP:
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
