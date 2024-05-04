project(nes)

set(CMAKE_CXX_STANDARD 20)

if (MSVC)
    add_compile_options(/W4)
else()
    set(DISABLED_WARNINGS -Wno-missing-field-initializers)
    add_compile_options(-Wall -Wextra -Werror ${DISABLED_WARNINGS})
endif()

add_subdirectory(lib)
add_subdirectory(emu)

enable_testing()
add_subdirectory(tests)
