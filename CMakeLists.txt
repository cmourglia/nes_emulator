cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(nes_emu)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(vendors)

add_subdirectory(src)
