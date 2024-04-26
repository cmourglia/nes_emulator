#include <catch2/catch_test_macros.hpp>

#include "cpu.h"
#include "disassembler.h"

TEST_CASE("0xa9_lda_immediate_load_data", "[opcodes]") {
    u8 code[] = {0xa9, 0x05, 0x00};

    CPU cpu = init_cpu();
    interpret_program(&cpu, code, sizeof(code));

    REQUIRE(cpu.accumulator == 0x05);
    REQUIRE(cpu.status == 0);
}

TEST_CASE("0xa9_lda_zero_flag", "[opcodes]") {
    u8 code[] = {0xa9, 0x00, 0x00};

    CPU cpu = init_cpu();
    interpret_program(&cpu, code, sizeof(code));

    REQUIRE(cpu.accumulator == 0x00);
    REQUIRE((cpu.status & 0b00000010) != 0);
    REQUIRE(cpu.zero_flag == 1);
}

TEST_CASE("simple_program_1", "[opcodes]") {
    u8 code[] = {0xa9, 0xc0, 0xaa, 0xe8, 0x00};

    CPU cpu = init_cpu();
    interpret_program(&cpu, code, sizeof(code));

    REQUIRE(cpu.accumulator == 0xc0);
    REQUIRE(cpu.x == 0xc1);
    REQUIRE(cpu.y == 0);
    REQUIRE(cpu.status == 0b10000000);
}