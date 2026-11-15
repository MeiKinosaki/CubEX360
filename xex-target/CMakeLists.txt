cmake_minimum_required(VERSION 3.25)
project(cubex360_xex C)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_executable(cubex360
  main.c
  ../core-src/src/memory.c
  ../core-src/src/bus.c
  ../core-src/src/cpu.c
)
target_include_directories(cubex360 PRIVATE ../core-src/include)
target_compile_options(cubex360 PRIVATE -Wall -Wextra -Werror)
xsdk_add_xex(cubex360 TITLE_ID 0x43554258 NAME "CubEX360" VERSION 1.9.0.0)
