cmake_minimum_required(VERSION 3.31)

find_program(JEV_CCACHE_BINARY ccache REQUIRED)
set(C_COMPILER_LAUNCHER ${JEV_CCACHE_BINARY})
set(CXX_COMPILER_LAUNCHER ${JEV_CCACHE_BINARY})

find_package(Zephyr REQUIRED)
project(stm32f1-usb-gofast)

target_sources(app PRIVATE src/main.c)
