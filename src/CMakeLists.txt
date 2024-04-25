project(nes)

set(CMAKE_CXX_STANDARD 20)

add_executable(nes main.cpp cpu.cpp opcode.cpp disassembler.cpp)
target_link_libraries(nes SDL3::SDL3 imgui)
