#ifndef Z80EMU_MCYCLE_HPP
#define Z80EMU_MCYCLE_HPP

class Cpu;

class Mcycle {
public:
    static void int_m1t1t2(Cpu* cpu);

    static void m1t1(Cpu* cpu);
    static void m1t2(Cpu* cpu);
    static void m1t3(Cpu* cpu);
    static void m1t4(Cpu* cpu);

    static uint8_t m2(Cpu* cpu, uint16_t addr);
    static void m3(Cpu* cpu, uint16_t addr, uint8_t data);

    static uint8_t in(Cpu* cpu, uint8_t portL, uint8_t portH);
    static void out(Cpu* cpu, uint8_t portL, uint8_t portH, uint8_t data);
};

#endif //Z80EMU_MCYCLE_HPP
