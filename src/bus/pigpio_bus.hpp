#ifndef Z80EMU_PIGPIOBUS_HPP
#define Z80EMU_PIGPIOBUS_HPP

#include <array>
#include <ctime>
#include "bus.hpp"

class PigpioBus : public Bus {
public:
    PigpioBus();
    /*
    static void intReset(int gpio, int level, uint32_t tick, void *busObj);
    static void intNmi(int gpio, int level, uint32_t tick, void *busObj);
    static void intInt(int gpio, int level, uint32_t tick, void *busObj);
     */

    void setAddress(uint16_t addr) override;
    void setData(uint8_t data) override;
    uint8_t getData() override;
    void setControl(uint8_t z80PinName, bool level) override;
    bool getInput(uint8_t z80PinName) override;
    void syncControl() override;

    void waitClockRising() override;
    void waitClockFalling() override;

    uint32_t gpio_bank1 = 0;
    //uint32_t gpio_bank2 = 0;
    uint8_t currentDataBusMode = 0xff;

    static const uint8_t RPi_GPIO_L_A0 = 0;
    static const uint8_t RPi_GPIO_L_A1 = 1;
    static const uint8_t RPi_GPIO_L_A2 = 2;
    static const uint8_t RPi_GPIO_L_A3 = 3;
    static const uint8_t RPi_GPIO_L_A4 = 4;
    static const uint8_t RPi_GPIO_L_A5 = 5;
    static const uint8_t RPi_GPIO_L_A6 = 6;
    static const uint8_t RPi_GPIO_L_A7 = 7;
    static const uint8_t RPi_GPIO_D0 = 8;
    static const uint8_t RPi_GPIO_D1 = 9;
    static const uint8_t RPi_GPIO_D2 = 10;
    static const uint8_t RPi_GPIO_D3 = 11;
    static const uint8_t RPi_GPIO_D4 = 12;
    static const uint8_t RPi_GPIO_D5 = 13;
    static const uint8_t RPi_GPIO_D6 = 14;
    static const uint8_t RPi_GPIO_D7 = 15;
    static const uint8_t RPi_GPIO_I_RESET = 16;
    static const uint8_t RPi_GPIO_I_CLK = 17;
    static const uint8_t RPi_GPIO_I_NMI = 18;
    static const uint8_t RPi_GPIO_I_INT = 19;
    static const uint8_t RPi_GPIO_I_WAIT = 20;
    static const uint8_t RPi_GPIO_I_BUSRQ = 21;
    static const uint8_t RPi_GPIO_DATA_BUS_DIR = 22;
    static const uint8_t RPi_GPIO_LE_ADDRESS_LOW = 23;
    static const uint8_t RPi_GPIO_LE_ADDRESS_HIGH = 24;
    static const uint8_t RPi_GPIO_LE_CONTROL = 25;

    static const uint8_t L_M1 = 0;
    static const uint8_t L_RFSH = 1;
    static const uint8_t L_HALT = 2;
    static const uint8_t L_RD = 3;
    static const uint8_t L_WR = 4;
    static const uint8_t L_MREQ = 5;
    static const uint8_t L_IORQ = 6;
    static const uint8_t L_BUSACK = 7;

    static const uint8_t DATA_BUS_DIR_OUT = 1;
    static const uint8_t DATA_BUS_DIR_IN = 0;
};


#endif //Z80EMU_PIGPIOBUS_HPP
