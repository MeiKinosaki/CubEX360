cmake_minimum_required(VERSION 3.25)
project(CubEX360 C)

add_executable(cubex360
    main.c
    ../core-src/src/cpu.c
    ../core-src/src/bus.c
    ../core-src/src/memory.c
)
target_include_directories(cubex360 PRIVATE ../core-src/include)
set_target_properties(cubex360 PROPERTIES C_STANDARD 17 C_EXTENSIONS ON)
target_compile_options(cubex360 PRIVATE -Wall -Wextra -Werror)
xsdk_add_xex(cubex360
    TITLE_ID 0x43554258
    NAME "CubEX360"
    VERSION 1.99.0.0
)
