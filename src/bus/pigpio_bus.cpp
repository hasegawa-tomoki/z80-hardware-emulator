#include <stdexcept>
#include <pigpio.h>
#include <unistd.h>
#include "pigpio_bus.hpp"
#include "../log.hpp"

PigpioBus::PigpioBus() {
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
    waitNanoSec(5);
    gpioWrite(RPi_GPIO_LE_ADDRESS_LOW, PI_LOW);
    gpioSetMode(RPi_GPIO_LE_ADDRESS_HIGH, PI_OUTPUT);
    gpioWrite(RPi_GPIO_LE_ADDRESS_HIGH, PI_HIGH);
    waitNanoSec(5);
    gpioWrite(RPi_GPIO_LE_ADDRESS_HIGH, PI_LOW);
    gpioSetMode(RPi_GPIO_LE_CONTROL, PI_OUTPUT);
    gpioWrite(RPi_GPIO_LE_CONTROL, PI_HIGH);
    waitNanoSec(5);
    gpioWrite(RPi_GPIO_LE_CONTROL, PI_LOW);
    // Data bus Direction (0: input, 1: output)
    gpioSetMode(RPi_GPIO_DATA_BUS_DIR, PI_OUTPUT);
    gpioWrite(RPi_GPIO_DATA_BUS_DIR, DATA_BUS_DIR_OUT);
    waitNanoSec(5);
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

    /*
    int idReset = gpioSetAlertFuncEx(RPi_GPIO_I_RESET, PigpioBus::intReset, this);
    if (idReset < 0){
        throw std::runtime_error("Error gpioSetAlertFuncEx() for Z80_GPIO_RESET");
    }
    int idNmi = gpioSetAlertFuncEx(RPi_GPIO_I_NMI, PigpioBus::intNmi, this);
    if (idNmi < 0){
        throw std::runtime_error("Error gpioSetAlertFuncEx() for Z80_GPIO_NMI");
    }
    int idInt = gpioSetAlertFuncEx(RPi_GPIO_I_INT, PigpioBus::intInt, this);
    if (idInt < 0){
        throw std::runtime_error("Error gpioSetAlertFuncEx() for Z80_GPIO_INT");
    }
     */
}
/*
#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedParameter"
void PigpioBus::intReset(int gpio, int level, uint32_t tick, void *busObj){
    static uint32_t prev_tick = 0;

    if (tick - prev_tick > 1000){
        printf("intReset()\n");
        auto* bus = static_cast<PigpioBus*>(busObj);
        bus->resetting = (level == 0);
        bus->pin_i_reset = level;
    }
    prev_tick = tick;
}
#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedParameter"
void PigpioBus::intNmi(int gpio, int level, uint32_t tick, void *busObj){
    auto* bus = static_cast<PigpioBus*>(busObj);
    printf("intNmi()\n");
    Log::bus(bus, "intNmi");
    bus->pin_i_nmi_prev = bus->pin_i_nmi;
    bus->pin_i_nmi = level;
}
#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedParameter"
void PigpioBus::intInt(int gpio, int level, uint32_t tick, void *busObj){
    auto* bus = static_cast<PigpioBus*>(busObj);
    printf("intInt()\n");
    Log::bus(bus, "intInt");
    bus->pin_i_int_prev = bus->pin_i_int;
    bus->pin_i_int = level;
}
#pragma clang diagnostic pop
*/

void PigpioBus::setAddress(uint16_t addr){
    this->address = addr;

    uint8_t high = addr >> 8;
    for (int i = 0; i <= 7; i++){
        gpioWrite(RPi_GPIO_L_A0 + i, (high & (1 << i)) > 0);
    }
    gpioWrite(RPi_GPIO_LE_ADDRESS_HIGH, PI_HIGH);
    this->waitNanoSec(5);
    gpioWrite(RPi_GPIO_LE_ADDRESS_HIGH, PI_LOW);
    this->waitNanoSec(5);

    uint8_t low = (addr & 0xff);
    for (int i = 0; i <= 7; i++){
        gpioWrite(RPi_GPIO_L_A0 + i, (low & (1 << i)) > 0);
    }
    gpioWrite(RPi_GPIO_LE_ADDRESS_LOW, PI_HIGH);
    this->waitNanoSec(5);
    gpioWrite(RPi_GPIO_LE_ADDRESS_LOW, PI_LOW);
    this->waitNanoSec(5);
}

void PigpioBus::setDataBegin(uint8_t data){
    if (true or this->currentDataBusMode != DATA_BUS_DIR_OUT){
        this->currentDataBusMode = DATA_BUS_DIR_OUT;
        for (int i = RPi_GPIO_D0; i <= RPi_GPIO_D7; i++){
            gpioSetMode(i, PI_OUTPUT);
        }
        gpioWrite(RPi_GPIO_DATA_BUS_DIR, DATA_BUS_DIR_OUT);
        waitNanoSec(2);
    }
    for (int i = 0; i <= 7; i++){
        uint8_t bit = ((data & (1 << i)) > 0);
        gpioWrite(RPi_GPIO_D0 +  i, bit);
    }
}

uint8_t PigpioBus::getData(){
    if (this->currentDataBusMode != DATA_BUS_DIR_IN){
        this->currentDataBusMode = DATA_BUS_DIR_IN;
        for (int i = RPi_GPIO_D0; i <= RPi_GPIO_D7; i++){
            gpioSetMode(i, PI_INPUT);
        }
        gpioWrite(RPi_GPIO_DATA_BUS_DIR, DATA_BUS_DIR_IN);
        waitNanoSec(2);
    }
    uint8_t data = 0;
    for (int i = 0; i <= 7; i++){
        if (gpioRead(RPi_GPIO_D0 + i) == PI_HIGH){
            data |= (1 << i);
        }
    }
    return data;
}

void PigpioBus::setControl(uint8_t z80PinName, bool level){
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

bool PigpioBus::getInput(uint8_t z80PinName){
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

void PigpioBus::syncControl(){
    gpioWrite(L_M1, this->pin_o_m1);
    gpioWrite(L_RFSH, this->pin_o_rfsh);
    gpioWrite(L_HALT, this->pin_o_halt);
    gpioWrite(L_RD, this->pin_o_rd);
    gpioWrite(L_WR, this->pin_o_wr);
    gpioWrite(L_MREQ, this->pin_o_mreq);
    gpioWrite(L_IORQ, this->pin_o_iorq);
    gpioWrite(L_BUSACK, this->pin_o_busack);

    gpioWrite(RPi_GPIO_LE_CONTROL, PI_HIGH);
    waitNanoSec(5);
    gpioWrite(RPi_GPIO_LE_CONTROL, PI_LOW);
    waitNanoSec(5);
}

void PigpioBus::waitClockRising(){
    if (gpioRead(RPi_GPIO_I_CLK)){
        while(gpioRead(RPi_GPIO_I_CLK));
    }
    while(! gpioRead(RPi_GPIO_I_CLK));
}
void PigpioBus::waitClockFalling(){
    if (! gpioRead(RPi_GPIO_I_CLK)){
        while(! gpioRead(RPi_GPIO_I_CLK));
    }
    while(gpioRead(RPi_GPIO_I_CLK));
}

