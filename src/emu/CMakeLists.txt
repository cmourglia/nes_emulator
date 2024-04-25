project(emu)

add_executable(emu main.cpp)
target_link_libraries(emu nes SDL3::SDL3 imgui)