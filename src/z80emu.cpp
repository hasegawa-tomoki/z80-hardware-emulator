#include <cstdio>
#include <pigpio.h>
//#include <pigpiod_if2.h>
#include <unistd.h>
#include <ctime>
#include "../libs/mcp23s17/mcp23s17.hpp"
#include "cpu.hpp"
#include "mcycle.hpp"
#include "log.hpp"

void wait_nano_sec(int ns){
    struct timespec req{};
    req.tv_sec = 0;
    req.tv_nsec = ns;
    nanosleep(&req, nullptr);
}

int main(){
    printf("Hello z80\n");

    PigpioBus bus = PigpioBus();
    Cpu cpu(&bus);

    cpu.instructionCycle();
    return 0;

    /*
    printf("Initializing...\n");
    uint16_t start = 0x9000;
    uint16_t end = 0x91ff;
    for (uint16_t address = start; address <= end; address++) {
        if (address % 16 == 0){
            printf("\n%04x ", address);
        }
        uint8_t before = Mcycle::m2(&cpu, address);
        uint8_t flip = ~before;
        Mcycle::m3(&cpu, address, flip);
        uint8_t after = Mcycle::m2(&cpu, address);
        if (before != after){
            if (flip == after){
                printf("______ ");
            } else {
                printf("%02x%02x%02x ", before, flip, after);
            }
        } else {
            printf("====== ");
        }
        if (address == end){
            break;
        }
    }

    for (uint16_t address = 0x0000; address < 0x0040; address++) {
        if (address % 16 == 0){
            printf("\n");
        }
        uint8_t data = Mcycle::m2(&cpu, address);
        printf("%02x ", data);
    }

    for (uint16_t address = 0x8000; address < 0x8005; address++) {
        printf("Testing %04x...\n", address);
        for (uint8_t data = 0x00; data <= 0xff; data++){
            Mcycle::m3(&cpu, address, data);
            uint8_t rData = Mcycle::m2(&cpu, address);
            if (data != rData){
                printf("ERROR Address: %04x  Wrote: %02x  Read: %02x\n", address, data, rData);
            }
            if (data == 0xff){
                break;
            }
        }
        if (address == 0xffff){
            break;
        }
    }

    for (uint16_t address = 0x8000; address < 0x8040; address++) {
        Mcycle::m3(&cpu, address, 0x00);
    }

    for (uint16_t address = 0x8000; address < 0x8040; address++) {
        if (address % 16 == 0){
            printf("\n");
        }
        uint8_t data = Mcycle::m2(&cpu, address);
        printf("%02x ", data);
    }

    cpu.executing = 0xe3;
    cpu.special_registers.sp = 0x8000;
    Mcycle::m3(&cpu, 0x8000, 0x33);
    Mcycle::m3(&cpu, 0x8001, 0x55);
    cpu.registers.hl(0x7799);
    printf("hl: %04x  (sp): %02x  (sp + 1): %02x\n", cpu.registers.hl(), Mcycle::m2(&cpu, 0x8000), Mcycle::m2(&cpu, 0x8001));
    cpu.opCode.execute(0xe3);
    printf("hl: %04x  (sp): %02x  (sp + 1): %02x\n", cpu.registers.hl(), Mcycle::m2(&cpu, 0x8000), Mcycle::m2(&cpu, 0x8001));
    return 0;


    while(true){
        for (uint8_t value = 0; value <= 1; value++){
            cpu.bus->pin_o_m1 = value;
            cpu.bus->syncControl();
            cpu.bus->pin_o_rfsh = value;
            cpu.bus->syncControl();
            cpu.bus->pin_o_halt = value;
            cpu.bus->syncControl();
            cpu.bus->pin_o_rd = value;
            cpu.bus->syncControl();
            cpu.bus->pin_o_wr = value;
            cpu.bus->syncControl();
            cpu.bus->pin_o_mreq = value;
            cpu.bus->syncControl();
            cpu.bus->pin_o_iorq = value;
            cpu.bus->syncControl();
            cpu.bus->pin_o_busack = value;
            cpu.bus->syncControl();
            usleep(100);
        }
    }

    for (uint16_t address = 0; address < 0x7fff; address++) {
        if (address % 16 == 0){
            printf("\n");
        }
        uint8_t data = Mcycle::m2(&cpu, address);
        printf("%02x ", data);
    }
    clock_t start = clock();
    for (int i = 0; i < 1000; i++){
        gpioRead(10);
    }
    double time = static_cast<double>(clock() - start) / CLOCKS_PER_SEC * 1000.0;
    printf("1000 single I/O in %lf msec.\n", time);

    start = clock();
    for (int i = 0; i < 1000; i++){
        uint32_t data = gpioRead_Bits_0_31();
    }
    time = static_cast<double>(clock() - start) / CLOCKS_PER_SEC * 1000.0;
    printf("1000 multi I/O in %lf msec.\n", time);
    */
    /*
    uint32_t gpio_bank1 = 0;
    for (uint16_t address = 0; address < 0xfffe; address++){
        gpio_bank1 = (gpio_bank1 & 0b11111100011111111111111100000000) | ((uint8_t)(address >> 8)) | (1 << PigpioBus::RPi_GPIO_LE_ADDRESS_LOW);
        gpioWrite_Bits_0_31_Set(gpio_bank1);
        gpioWrite_Bits_0_31_Clear(~(gpio_bank1));
        wait_nano_sec(5);
        gpioWrite(PigpioBus::RPi_GPIO_LE_ADDRESS_LOW, PI_LOW);
        wait_nano_sec(5);

        gpio_bank1 = (gpio_bank1 & 0b11111100011111111111111100000000) | ((uint8_t)(address)) | (1 << PigpioBus::RPi_GPIO_LE_ADDRESS_HIGH);
        gpioWrite_Bits_0_31_Set(gpio_bank1);
        gpioWrite_Bits_0_31_Clear(~(gpio_bank1));
        wait_nano_sec(5);
        gpioWrite(PigpioBus::RPi_GPIO_LE_ADDRESS_HIGH, PI_LOW);
        waitNanoSec(5);

        bus.waitClockRising();
    }
     */


    // setAddress           151.271000 msec/1000 = 151,271 usec/1000 = 151 usec/1
    // updateControlSignals  79.728000 msec/1000 =  79,728 usec/1000 =  79 usec/1
    // 1clk = 0.00000025 sec = 0.00025 msec = 0.25 usec
    // M1 = 4clk = 1 usec
}
