#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#ifndef Z80EMU_Z80_HPP
#define Z80EMU_Z80_HPP
#include <array>
#include "bus.hpp"
#include "registers.hpp"
#include "special_registers.hpp"
#include "opcode.hpp"

class Cpu
{
public:
    int _pi;
    Mcp23s17 _io;
    Bus _bus;
    OpCode _opCode;
    SpecialRegisters _special_registers;
    Registers _registers;
    Registers _registers_alternate;

    bool enable_virtual_memory = false;
    std::array<uint8_t, 32> virtual_memory{
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };

    uint32_t tick = 0;

    bool iff1 = false;
    bool iff2 = false;
    bool halt = false;

    uint8_t waitingEI = 0;
    uint8_t waitingDI = 0;
    bool NMI_activated = false;
    bool INT_activated = false;

    bool pin_o_m1 = true;
    bool pin_o_rfsh = true;
    bool pin_o_halt = true;
    bool pin_o_rd = true;
    bool pin_o_wr = true;
    bool pin_o_mreq = true;
    bool pin_o_iorq = true;
    bool pin_o_busack = true;

    volatile bool pin_i_reset = true;
    volatile bool pin_i_clk = true;
    volatile bool pin_i_clk_prev = false;
    volatile bool pin_i_nmi = true;
    volatile bool pin_i_nmi_prev = true;
    volatile bool pin_i_int = true;
    volatile bool pin_i_int_prev = true;
    volatile bool pin_i_wait = true;
    volatile bool pin_i_busrq = true;

    uint8_t interrupt_mode = 0;
    volatile bool resetting = false;

    uint8_t executing = 0;

    static const int IO_ADDRESS_BUS = 0;
    static const int GPIO_HIGH = Mcp23s17::MCP23S17_GPIOA;
    static const int GPIO_LOW = Mcp23s17::MCP23S17_GPIOB;
    static const int IO_CONTROL_DATA_BUS = 1;
    static const int GPIO_DATA_BUS = Mcp23s17::MCP23S17_GPIOA;
    static const int GPIO_CONTROL = Mcp23s17::MCP23S17_GPIOB;

    static const uint8_t Z80_GPIO_RESET = 22;
    static const uint8_t Z80_GPIO_CLK = 23;
    static const uint8_t Z80_GPIO_NMI = 24;
    static const uint8_t Z80_GPIO_INT = 25;
    static const uint8_t Z80_GPIO_WAIT = 26;
    static const uint8_t Z80_GPIO_BUSRQ = 27;

    explicit Cpu(int pi);

    bool init();
    void reset();

    void updateControlSignals();

    static void intReset(int pi, unsigned gpio, unsigned level, uint32_t tick, void *z80obj);
    //static void intClock(int pi, unsigned gpio, unsigned level, uint32_t tick, void *z80obj);
    static void intWait(int pi, unsigned gpio, unsigned level, uint32_t tick, void *z80obj);
    static void intNmi(int pi, unsigned gpio, unsigned level, uint32_t tick, void *z80obj);
    static void intInt(int pi, unsigned gpio, unsigned level, uint32_t tick, void *z80obj);

    //bool clockRising();
    //bool clockFalling();
    void waitClockRising() const;
    void waitClockFalling() const;
    [[nodiscard]] bool readGpio(uint8_t gpio) const;
    bool nmiFalling();
    bool intFalling();

    void instructionCycle();
};

#endif //Z80EMU_Z80_HPP

#pragma clang diagnostic pop