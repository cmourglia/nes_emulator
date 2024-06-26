project(tests)

# Copy nestest.nes for CI
file(COPY ${CMAKE_SOURCE_DIR}/test_roms/nestest.nes
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

Include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.4.0 # or a later release
)

FetchContent_MakeAvailable(Catch2)

add_executable(tests test.cpp)
target_link_libraries(tests nes Catch2::Catch2WithMain)

list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(tests)
