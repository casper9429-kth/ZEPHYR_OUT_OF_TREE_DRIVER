# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(pyd1598_driver)


add_subdirectory(drivers)


target_sources(app PRIVATE 
    src/main.cpp
)

set(ZEPHYR_CPLUSPLUS ON)