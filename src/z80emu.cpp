#include <cstdio>
//#include <pigpio.h>
#include <pigpiod_if2.h>
#include <unistd.h>
#include "../libs/mcp23s17/mcp23s17.hpp"
#include "cpu.hpp"

int main(){
    printf("Hello z80\n");

    int pi = pigpio_start(nullptr, nullptr);
    if (pi < 0){
        printf("Failed to pigpio_start()\n");
        return 1;
    }

    for (int i = 22; i <= 27; i++){
        set_mode(pi, i, PI_INPUT);
        set_pull_up_down(pi, i, PI_PUD_OFF);
    }

    Cpu cpu(pi);
    if (! cpu.init()){
        return 1;
    }
    cpu.resetting = true;

    /*
    uint16_t a = 0x0000;
    uint16_t b = 0x0001;
    uint16_t c = a - b;
    printf("%04x\n", c);
    if ((int16_t)c < 0){
        printf("negative\n");
    }
     */

    /*
    z80._io.write_gpio(0, 0x0000);
    z80._io.write_gpio(1, 0x0000);

    while (true){
        cpu._io.write_gpio(1, 0xffff);
        usleep(200 * 1000);
        cpu._io.write_gpio(1, 0x0000);
        usleep(200 * 1000);
    }

    while(true){
        //uint8_t data = z80._io.read_gpio8(0, Mcp23s17::MCP23S17_GPIOA);
        z80._io.set_direction_8(Z80::IO_CONTROL_DATA_BUS, Z80::GPIO_DATA_BUS, Mcp23s17::DIR_INPUT);
        uint8_t data =  z80._io.read_gpio8(Z80::IO_CONTROL_DATA_BUS, Z80::GPIO_DATA_BUS);
        printf("%02x\n", data);
    }
    for (uint8_t i = 0; i <= 0xff; i++){
        cpu._bus.write(i);
        usleep(20 * 1000);
        cpu._bus.write(0);
        usleep(20 * 1000);
    }
    return 0;
     */

    cpu.instructionCycle();

    return 0;
}
