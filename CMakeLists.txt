cmake_minimum_required(VERSION 3.31 FATAL_ERROR)

find_program(JEV_CCACHE_BINARY ccache REQUIRED)
set(C_COMPILER_LAUNCHER ${CCACHE_BINARY})
set(CXX_COMPILER_LAUNCHER ${CCACHE_BINARY})

find_package(Zephyr REQUIRED)
project(stm32f1-usb-gofast)

target_sources(app PRIVATE src/main.c)
target_compile_options(app PRIVATE -Wno-unknown-pragmas -ggdb3)
add_compile_options(-ggdb3)

message("All targets POST: ${all_targets}")
