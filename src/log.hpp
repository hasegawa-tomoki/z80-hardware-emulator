#ifndef Z80EMU_LOG_HPP
#define Z80EMU_LOG_HPP

#include <cstdint>
#include "bus/bus.hpp"

class Cpu;
class Mcycle;

class Log {
public:
    static void write(char* string);
    static void dump_registers(Cpu *cpu);
    static void execute(Cpu* cpu, uint8_t op_code, const char* mnemonic);
    static void general(Cpu* cpu, const char* step);
    static void target_register(Cpu* cpu, const char* reg_name);
    static void error(Cpu* cpu, const char* step);
    static void mem_read(Cpu* cpu, uint16_t addr, uint8_t data);
    static void mem_write(Cpu* cpu, uint16_t addr, uint8_t data);
    static void io_read(Cpu* cpu, uint16_t addr, uint8_t data);
    static void io_write(Cpu* cpu, uint16_t addr, uint8_t data);
    static void bus(Bus* bus, const char* step);
};

#endif //Z80EMU_LOG_HPP
