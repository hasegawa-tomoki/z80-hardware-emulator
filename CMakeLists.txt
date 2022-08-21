cmake_minimum_required(VERSION 3.0)

project(z80emu C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(Google_tests)

add_executable(z80emu
        src/z80emu.cpp
        src/cpu.cpp
        src/registers.cpp
        src/special_registers.cpp
        src/mcycle.cpp
        src/opcode.cpp
        src/log.cpp
        src/config.hpp
        src/bus/bus.cpp
        src/bus/pigpio_bus_bulk.cpp
        src/bus/pigpio_bus.cpp
        )

target_link_libraries(
        z80emu
        pigpio
        wiringPi
)
