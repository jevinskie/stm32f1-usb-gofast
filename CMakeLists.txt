cmake_minimum_required(VERSION 3.31)

find_package(Zephyr REQUIRED)
project(stm32f1-usb-gofast)

target_sources(app PRIVATE src/main.c)
