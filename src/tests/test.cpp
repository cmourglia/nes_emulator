#include <catch2/catch_test_macros.hpp>

#include "common.h"

u32 fact(u32 number) {
    return number <= 1 ? number : fact(number - 1) * number;
}

TEST_CASE("Factorials", "[factorial]") {
    REQUIRE(fact(1) == 1);
    REQUIRE(fact(2) == 2);
    REQUIRE(fact(3) == 6);
    REQUIRE(fact(10) == 3628800);
}