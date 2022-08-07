#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#ifndef Z80EMU_Z80_HPP
#define Z80EMU_Z80_HPP
#include <array>
#include "registers.hpp"
#include "special_registers.hpp"
#include "opcode.hpp"
#include "bus/pigpio_bus.hpp"

class Cpu
{
public:
    explicit Cpu(Bus *bus);

    Bus *bus;
    OpCode opCode;
    SpecialRegisters special_registers;
    Registers registers;
    Registers registers_alternate;

    bool enable_virtual_memory = false;
    std::array<uint8_t, 32> virtual_memory{
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    // refs: https://www.seasip.info/Cpm/bdos.html
    bool emulate_cpm_bdos_call = false;

    uint32_t tick = 0;

    bool iff1 = false;
    bool iff2 = false;
    bool halt = false;

    uint8_t waitingEI = 0;
    uint8_t waitingDI = 0;
    /*
    bool NMI_activated = false;
    bool INT_activated = false;
     */

    uint8_t interrupt_mode = 0;

    uint8_t executing = 0;

    void reset();

    void instructionCycle();
};

#endif //Z80EMU_Z80_HPP

#pragma clang diagnostic pop