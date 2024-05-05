#include <catch2/catch_test_macros.hpp>

#include "cpu.h"
#include "disassembler.h"

TEST_CASE("a9_lda_immediate_load_data", "[opcodes][lda]") {
    CPU cpu = init_cpu({0xa9, 0x05, 0x00});
    run_cpu(&cpu);

    fprintf(stderr, "Status: 0x%x (0x%x)\n", cpu.status, 0b00110000);

    REQUIRE(cpu.accumulator == 0x05);
    REQUIRE(cpu.status == (BreakCommand | UnusedFlag));
}

TEST_CASE("a9_lda_zero_flag", "[opcodes][lda]") {
    CPU cpu = init_cpu({0xa9, 0x00, 0x00});
    run_cpu(&cpu);

    REQUIRE(cpu.accumulator == 0x00);
    REQUIRE(cpu.status == (BreakCommand | UnusedFlag | ZeroFlag));
}

TEST_CASE("a9_lda_negative_flag", "[opcodes][lda]") {
    CPU cpu = init_cpu({0xa9, 0xff, 0x00});
    run_cpu(&cpu);

    REQUIRE(cpu.accumulator == 0xFF);
    REQUIRE(cpu.status == (BreakCommand | UnusedFlag | NegativeFlag));
}

TEST_CASE("aa_tax_move_a_to_x", "[opcodes][tax]") {
    CPU cpu = init_cpu({0xaa, 0x00});
    cpu.accumulator = 42;
    run_cpu(&cpu);

    REQUIRE(cpu.accumulator == 42);
    REQUIRE(cpu.x == 42);
    REQUIRE(cpu.status == (BreakCommand | UnusedFlag));
}

TEST_CASE("aa_tax_zero_flag", "[opcodes][tax]") {
    CPU cpu = init_cpu({0xaa, 0x00});
    cpu.accumulator = 0;
    run_cpu(&cpu);

    REQUIRE(cpu.x == 0);
    REQUIRE(cpu.status == (BreakCommand | UnusedFlag | ZeroFlag));
}

TEST_CASE("aa_tax_negative_flag", "[opcodes][tax]") {
    CPU cpu = init_cpu({0xaa, 0x00});
    cpu.accumulator = 0xff;
    run_cpu(&cpu);

    REQUIRE(cpu.x == 0xff);
    REQUIRE(cpu.status == (BreakCommand | UnusedFlag | NegativeFlag));
}

TEST_CASE("e8_inx_increment_x", "[opcodes][inx]") {
    CPU cpu = init_cpu({0xe8, 0x00});
    cpu.x = 42;

    run_cpu(&cpu);

    REQUIRE(cpu.x == 43);
}

TEST_CASE("e8_inx_overflow", "[opcodes][inx]") {
    CPU cpu = init_cpu({0xe8, 0xe8, 0x00});
    cpu.x = 0xff;

    run_cpu(&cpu);
    REQUIRE(cpu.x == 1);
}

TEST_CASE("simple_program_1", "[programs]") {
    CPU cpu = init_cpu({0xa9, 0xc0, 0xaa, 0xe8, 0x00});
    run_cpu(&cpu);

    REQUIRE(cpu.accumulator == 0xc0);
    REQUIRE(cpu.x == 0xc1);
    REQUIRE(cpu.y == 0);
    REQUIRE(cpu.status == (BreakCommand | UnusedFlag | NegativeFlag));
}

TEST_CASE("10 times 3", "[programs]") {
    std::vector<u8> prg = {0xa2, 0x0a, 0x8e, 0x00, 0x00, 0xa2, 0x03, 0x8e,
                           0x01, 0x00, 0xac, 0x00, 0x00, 0xa9, 0x00, 0x18,
                           0x6d, 0x01, 0x00, 0x88, 0xd0, 0xfa, 0x8d, 0x02,
                           0x00, 0xea, 0xea, 0xea, 0x00};

    CPU cpu = init_cpu(prg);

    run_cpu(&cpu);

    // REQUIRE(cpu.accumulator == 30);
    // REQUIRE(cpu.x == 3);
    // REQUIRE(cpu.y == 0);

    REQUIRE(cpu.memory[2] == 30);
}
