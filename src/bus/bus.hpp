#ifndef Z80EMU_BUS_HPP
#define Z80EMU_BUS_HPP

#include <cinttypes>

class Bus {
public:
    virtual void setAddress(uint16_t addr) = 0;
    virtual void setDataBegin(uint8_t data) = 0;
    virtual void setDataEnd() = 0;
    virtual uint8_t getData() = 0;
    virtual void setControl(uint8_t z80PinName, bool level) = 0;
    virtual bool getInput(uint8_t z80PinName) = 0;
    virtual void syncControl() = 0;

    virtual void waitClockRising() = 0;
    virtual void waitClockFalling() = 0;
    static void waitNanoSec(int ns);

    uint16_t address = 0;

    uint8_t pin_o_m1 = PIN_HIGH;
    uint8_t pin_o_rfsh = PIN_HIGH;
    uint8_t pin_o_halt = PIN_HIGH;
    uint8_t pin_o_rd = PIN_HIGH;
    uint8_t pin_o_wr = PIN_HIGH;
    uint8_t pin_o_mreq = PIN_HIGH;
    uint8_t pin_o_iorq = PIN_HIGH;
    uint8_t pin_o_busack = PIN_HIGH;

    volatile uint8_t pin_i_reset = PIN_HIGH;
    volatile uint8_t pin_i_nmi = PIN_HIGH;
    volatile uint8_t pin_i_nmi_prev = PIN_HIGH;
    volatile uint8_t pin_i_int = PIN_HIGH;
    volatile uint8_t pin_i_int_prev = PIN_HIGH;
    volatile uint8_t pin_i_wait = PIN_HIGH;
    volatile uint8_t pin_i_busrq = PIN_HIGH;

    static const uint8_t Z80_PIN_I_CLK = 6;
    static const uint8_t Z80_PIN_I_INT = 16;
    static const uint8_t Z80_PIN_I_NMI = 17;
    static const uint8_t Z80_PIN_O_HALT = 18;
    static const uint8_t Z80_PIN_O_MERQ = 19;
    static const uint8_t Z80_PIN_O_IORQ = 20;
    static const uint8_t Z80_PIN_O_RD = 21;
    static const uint8_t Z80_PIN_O_WR = 22;
    static const uint8_t Z80_PIN_O_BUSACK = 23;
    static const uint8_t Z80_PIN_I_WAIT = 24;
    static const uint8_t Z80_PIN_I_BUSRQ = 25;
    static const uint8_t Z80_PIN_I_RESET = 26;
    static const uint8_t Z80_PIN_O_M1 = 27;
    static const uint8_t Z80_PIN_O_RFSH = 28;
    static const uint8_t Z80_PIN_A0 = 30;
    static const uint8_t Z80_PIN_A1 = 31;
    static const uint8_t Z80_PIN_A2 = 32;
    static const uint8_t Z80_PIN_A3 = 33;
    static const uint8_t Z80_PIN_A4 = 34;
    static const uint8_t Z80_PIN_A5 = 35;
    static const uint8_t Z80_PIN_A6 = 36;
    static const uint8_t Z80_PIN_A7 = 37;
    static const uint8_t Z80_PIN_A8 = 38;
    static const uint8_t Z80_PIN_A9 = 39;
    static const uint8_t Z80_PIN_A10 = 40;
    static const uint8_t Z80_PIN_A11 = 1;
    static const uint8_t Z80_PIN_A12 = 2;
    static const uint8_t Z80_PIN_A13 = 3;
    static const uint8_t Z80_PIN_A14 = 4;
    static const uint8_t Z80_PIN_A15 = 5;
    static const uint8_t Z80_PIN_D0 = 14;
    static const uint8_t Z80_PIN_D1 = 15;
    static const uint8_t Z80_PIN_D2 = 12;
    static const uint8_t Z80_PIN_D3 = 8;
    static const uint8_t Z80_PIN_D4 = 7;
    static const uint8_t Z80_PIN_D5 = 9;
    static const uint8_t Z80_PIN_D6 = 10;
    static const uint8_t Z80_PIN_D7 = 13;

    static const uint8_t PIN_HIGH = 1;
    static const uint8_t PIN_LOW = 0;
};


#endif //Z80EMU_BUS_HPP
