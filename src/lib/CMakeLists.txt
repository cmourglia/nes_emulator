project(nes)

add_library(nes STATIC
    common.h
    cpu.cpp cpu.h
    disassembler.cpp disassembler.h
    opcode.cpp opcode.h
)

target_include_directories(nes PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
