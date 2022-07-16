#ifndef Z80EMU_LOG_HPP
#define Z80EMU_LOG_HPP

class Cpu;
class Mcycle;

class Log {
public:
    static void write(Cpu* cpu, char* string);
    static void dump(Cpu* cpu);
};

#endif //Z80EMU_LOG_HPP
