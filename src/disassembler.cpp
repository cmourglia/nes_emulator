#include "disassembler.h"

#include <format>

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

std::vector<std::string> disassemble_code(u8* code, u16 code_size) {
    std::vector<std::string> result;

    u16 pointer = 0;

    std::string str;

    while (pointer < code_size) {
        OpCode opcode = opcode_get_next(pointer, code);

        str.clear();

        str += dec_to_hex(pointer, 4);

        str += "   ";

        str += dec_to_hex(code[pointer], 2);
        str += " ";
        str += opcode.bytes >= 2 ? dec_to_hex(code[pointer + 1], 2) : "  ";
        str += " ";
        str += opcode.bytes >= 3 ? dec_to_hex(code[pointer + 2], 2) : "  ";

        str += "   ";

        str += opcode_get_mnemonic(opcode.code);
        str += " ";

        switch (opcode.addressingMode) {
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
                str += "#$" + dec_to_hex(opcode.lo_byte, 2) + "   ";
            } break;
            case AM_Implied: {
                str += "       ";
            } break;
            case AM_Indirect: {
                str += "($" + dec_to_hex(opcode.word, 4) + ")";
            } break;
            case AM_Indirect_X: {
                str += "($" + dec_to_hex(opcode.lo_byte, 2) + ",X)";
            } break;
            case AM_Indirect_Y: {
                str += "($" + dec_to_hex(opcode.lo_byte, 2) + "),Y";
            } break;
            case AM_Relative: {
                str += "$" + dec_to_hex(opcode.lo_byte, 2) + "    ";
            } break;
            case AM_ZeroPage: {
                str += "$" + dec_to_hex(opcode.lo_byte, 2) + "    ";
            } break;
            case AM_ZeroPage_X: {
                str += "$" + dec_to_hex(opcode.lo_byte, 2) + ",X  ";
            } break;
            case AM_ZeroPage_Y: {
                str += "$" + dec_to_hex(opcode.lo_byte, 2) + ",Y  ";
            } break;
        }

        str += "   ;";

        result.push_back(str);

        pointer += opcode.bytes;
    }

    return result;
}