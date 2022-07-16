#ifndef Z80EMU_LOG_HPP
#define Z80EMU_LOG_HPP

#include <cstdint>

class Cpu;
class Mcycle;

class Log {
public:
    static void write(Cpu* cpu, char* string);
    static void execute(Cpu* cpu, uint8_t op_code, const char* mnemonic);
    static void before_m1(Cpu* cpu);
    static void after_m1(Cpu* cpu);
    static void step(Cpu* cpu, const char* step);
    static void error(Cpu* cpu, const char* step);
    static void mem_read(Cpu* cpu, uint16_t addr, uint8_t data);
    static void mem_write(Cpu* cpu, uint16_t addr, uint8_t data);
};

#endif //Z80EMU_LOG_HPP
