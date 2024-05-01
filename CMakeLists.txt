cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(gbemu)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(vendors)

add_subdirectory(src)
