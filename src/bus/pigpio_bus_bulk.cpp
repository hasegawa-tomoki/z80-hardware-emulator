#include <stdexcept>
#include <pigpio.h>
#include <unistd.h>
#include "pigpio_bus_bulk.hpp"
#include "../log.hpp"

PigpioBusBulk::PigpioBusBulk() {
    if (gpioInitialise() < 0){
        throw std::runtime_error("pigpio initialization failed");
    }
    // Latch
    for (int i = RPi_GPIO_L_A0; i <= RPi_GPIO_L_A7; i++){
        gpioSetMode(i, PI_OUTPUT);
        gpioSetPullUpDown(i, PI_PUD_OFF);
        gpioWrite(i, PI_LOW);
    }
    // Latch selector
    gpioSetMode(RPi_GPIO_LE_ADDRESS_LOW, PI_OUTPUT);
    gpioWrite(RPi_GPIO_LE_ADDRESS_LOW, PI_HIGH);
    gpioWrite(RPi_GPIO_LE_ADDRESS_LOW, PI_LOW);
    gpioSetMode(RPi_GPIO_LE_ADDRESS_HIGH, PI_OUTPUT);
    gpioWrite(RPi_GPIO_LE_ADDRESS_HIGH, PI_HIGH);
    gpioWrite(RPi_GPIO_LE_ADDRESS_HIGH, PI_LOW);
    gpioSetMode(RPi_GPIO_LE_CONTROL, PI_OUTPUT);
    gpioWrite(RPi_GPIO_LE_CONTROL, PI_HIGH);
    gpioWrite(RPi_GPIO_LE_CONTROL, PI_LOW);
    // Data bus Isolation (0: Enable 1: Isolated)
    gpioSetMode(RPi_GPIO_DATA_BUS_OE, PI_OUTPUT);
    gpioWrite(RPi_GPIO_DATA_BUS_OE, DATA_BUS_ISOLATED);
    // Data bus Direction (0: input, 1: output)
    gpioSetMode(RPi_GPIO_DATA_BUS_DIR, PI_OUTPUT);
    gpioWrite(RPi_GPIO_DATA_BUS_DIR, DATA_BUS_DIR_OUT);
    // Data bus
    for (int i = RPi_GPIO_D0; i <= RPi_GPIO_D7; i++){
        gpioSetMode(i, PI_INPUT);
        gpioSetPullUpDown(i, PI_PUD_OFF);
        gpioWrite(i, PI_LOW);
    }
    // Input Pins
    for (int i = RPi_GPIO_I_RESET; i <= RPi_GPIO_I_BUSRQ; i++){
        gpioSetMode(i, PI_INPUT);
        gpioSetPullUpDown(i, PI_PUD_OFF);
    }
}

void PigpioBusBulk::setAddress(uint16_t addr){
    if ((addr & 0xff00) != (this->address & 0xff00)){
        uint8_t high = addr >> 8;
        // 76ns
        gpioWrite_Bits_0_31_Set(high);
        gpioWrite_Bits_0_31_Clear(~high & 0x000000ff);
        // 118ns
        gpioWrite(RPi_GPIO_LE_ADDRESS_HIGH, PI_HIGH);
        // 118ns
        gpioWrite(RPi_GPIO_LE_ADDRESS_HIGH, PI_LOW);
    }

    if ((addr & 0x00ff) != (this->address & 0x00ff)) {
        uint8_t low = (addr & 0xff);
        // 76ns
        gpioWrite_Bits_0_31_Set(low);
        gpioWrite_Bits_0_31_Clear(~low & 0x000000ff);
        // 118ns
        gpioWrite(RPi_GPIO_LE_ADDRESS_LOW, PI_HIGH);
        // 118ns
        gpioWrite(RPi_GPIO_LE_ADDRESS_LOW, PI_LOW);
    }

    this->address = addr;
}

void PigpioBusBulk::setDataBegin(uint8_t data){
    if (this->currentDataBusMode != DATA_BUS_DIR_OUT){
        this->currentDataBusMode = DATA_BUS_DIR_OUT;

        for (int i = RPi_GPIO_D0; i <= RPi_GPIO_D7; i++){
            gpioSetMode(i, PI_OUTPUT);
        }
        gpioWrite(RPi_GPIO_DATA_BUS_DIR, DATA_BUS_DIR_OUT);
    }
    gpioWrite(RPi_GPIO_DATA_BUS_OE, DATA_BUS_ENABLED);
    gpioWrite_Bits_0_31_Set(data << 8);
    gpioWrite_Bits_0_31_Clear(((~data) << 8) & 0x0000ff00);
}

void PigpioBusBulk::setDataEnd(){
    gpioWrite(RPi_GPIO_DATA_BUS_OE, DATA_BUS_ISOLATED);
}

uint8_t PigpioBusBulk::getData(){
    if (this->currentDataBusMode != DATA_BUS_DIR_IN){
        this->currentDataBusMode = DATA_BUS_DIR_IN;

        for (int i = RPi_GPIO_D0; i <= RPi_GPIO_D7; i++){
            gpioSetMode(i, PI_INPUT);
        }
        gpioWrite(RPi_GPIO_DATA_BUS_DIR, DATA_BUS_DIR_IN);
    }
    gpioWrite(RPi_GPIO_DATA_BUS_OE, DATA_BUS_ENABLED);
    uint32_t bits = gpioRead_Bits_0_31();
    uint8_t data = 0x000000ff & (bits >> 8);
    gpioWrite(RPi_GPIO_DATA_BUS_OE, DATA_BUS_ISOLATED);

    return data;
}

void PigpioBusBulk::setControl(uint8_t z80PinName, bool level){
    switch (z80PinName){
        case Z80_PIN_O_HALT:    this->pin_o_halt = level;   break;
        case Z80_PIN_O_MERQ:    this->pin_o_mreq = level;   break;
        case Z80_PIN_O_IORQ:    this->pin_o_iorq = level;   break;
        case Z80_PIN_O_RD:      this->pin_o_rd = level;     break;
        case Z80_PIN_O_WR:      this->pin_o_wr = level;     break;
        case Z80_PIN_O_BUSACK:  this->pin_o_busack = level; break;
        case Z80_PIN_O_M1:      this->pin_o_m1 = level;     break;
        case Z80_PIN_O_RFSH:    this->pin_o_rfsh = level;   break;
        default:
            throw std::logic_error("Invalid Z80 pin (setControl)");
    }
}

bool PigpioBusBulk::getInput(uint8_t z80PinName){
    switch (z80PinName){
        case Z80_PIN_I_CLK:
            return gpioRead(RPi_GPIO_I_CLK);
        case Z80_PIN_I_INT:
            return gpioRead(RPi_GPIO_I_INT);
        case Z80_PIN_I_NMI:
            return gpioRead(RPi_GPIO_I_NMI);
        case Z80_PIN_I_WAIT:
            return gpioRead(RPi_GPIO_I_WAIT);
        case Z80_PIN_I_BUSRQ:
            return gpioRead(RPi_GPIO_I_BUSRQ);
        case Z80_PIN_I_RESET:
            return gpioRead(RPi_GPIO_I_RESET);
        default:
            throw std::logic_error("Invalid Z80 pin (getInput)");
    }
}

void PigpioBusBulk::syncControl(){
    uint8_t control = 0;
    if (this->pin_o_m1){ control |= (1 << L_M1); }
    if (this->pin_o_rfsh){ control |= (1 << L_RFSH); }
    if (this->pin_o_halt){ control |= (1 << L_HALT); }
    if (this->pin_o_rd){ control |= (1 << L_RD); }
    if (this->pin_o_wr){ control |= (1 << L_WR); }
    if (this->pin_o_mreq){ control |= (1 << L_MREQ); }
    if (this->pin_o_iorq){ control |= (1 << L_IORQ); }
    if (this->pin_o_busack){ control |= (1 << L_BUSACK); }

    // 76ns
    gpioWrite_Bits_0_31_Set(control & 0xff);
    gpioWrite_Bits_0_31_Clear(~control & 0xff);
    // 118ns
    gpioWrite(RPi_GPIO_LE_CONTROL, PI_HIGH);
    // 118ns
    gpioWrite(RPi_GPIO_LE_CONTROL, PI_LOW);
}

void PigpioBusBulk::waitClockRising(){
    // Commented out because the operating speed is significantly slow relative to the clock.
    /*
    if (gpioRead(RPi_GPIO_I_CLK)){
        while(gpioRead(RPi_GPIO_I_CLK));
    }
    while(! gpioRead(RPi_GPIO_I_CLK));
     */
}
void PigpioBusBulk::waitClockFalling(){
    // Commented out because the operating speed is significantly slow relative to the clock.
    /*
    if (! gpioRead(RPi_GPIO_I_CLK)){
        while(! gpioRead(RPi_GPIO_I_CLK));
    }
    while(gpioRead(RPi_GPIO_I_CLK));
     */
}
