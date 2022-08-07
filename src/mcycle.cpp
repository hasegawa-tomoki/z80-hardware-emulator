#include <cstdint>
#include <cstdio>
#include "mcycle.hpp"
#include "cpu.hpp"
#include "log.hpp"
#include <unistd.h>

void Mcycle::int_m1t1t2(Cpu* cpu){
    // t1
    cpu->bus->waitClockRising();
    cpu->bus->setAddress(cpu->special_registers.pc);
    cpu->bus->pin_o_mreq = Bus::PIN_HIGH;
    cpu->bus->pin_o_rd = Bus::PIN_HIGH;
    cpu->bus->pin_o_m1 = Bus::PIN_LOW;
    cpu->bus->syncControl();
    cpu->bus->waitClockFalling();
    // t2
    cpu->bus->waitClockRising();
    cpu->bus->waitClockFalling();
    // tw
    cpu->bus->waitClockRising();
    cpu->bus->waitClockFalling();
    cpu->bus->pin_o_iorq = Bus::PIN_LOW;
    cpu->bus->syncControl();
    // tw
    cpu->bus->waitClockRising();
    cpu->bus->waitClockFalling();
    while (!cpu->bus->getInput(Bus::Z80_PIN_I_WAIT)){
        cpu->bus->waitClockFalling();
    }
}

void Mcycle::m1t1(Cpu* cpu){
    // T1: Output PC's address
    cpu->bus->waitClockRising();
    if (cpu->halt){
        return;
    }

    if (cpu->enable_virtual_memory){
        cpu->executing = cpu->virtual_memory[cpu->special_registers.pc];
    } else {
        cpu->bus->setAddress(cpu->special_registers.pc);
    }
    cpu->special_registers.pc++;
    cpu->bus->pin_o_mreq = Bus::PIN_LOW;
    cpu->bus->pin_o_rd = Bus::PIN_LOW;
    cpu->bus->pin_o_m1 = Bus::PIN_LOW;
    cpu->bus->syncControl();
    cpu->bus->waitClockFalling();
}

void Mcycle::m1t2(Cpu* cpu){
    // T2: Wait memory until WAIT is inactive
    cpu->bus->waitClockRising();
    if (cpu->halt){
        return;
    }
    cpu->bus->waitClockFalling();
    while (!cpu->bus->getInput(Bus::Z80_PIN_I_WAIT)){
        cpu->bus->waitClockFalling();
    }
}

void Mcycle::m1t3(Cpu* cpu) {
    // T3-rising: Fetch data. Output refresh address. Update control signals
    cpu->bus->waitClockRising();
    if ((! cpu->halt) && (! cpu->enable_virtual_memory)){
        cpu->executing = cpu->bus->getData();
    }
    if (! cpu->enable_virtual_memory) {
        auto refreshAddr = (uint16_t)((cpu->special_registers.i << 8) | cpu->special_registers.r);
        cpu->bus->setAddress(refreshAddr);

        cpu->bus->pin_o_mreq = Bus::PIN_HIGH;
        cpu->bus->pin_o_rd = Bus::PIN_HIGH;
        cpu->bus->pin_o_m1 = Bus::PIN_HIGH;
        cpu->bus->pin_o_rfsh = Bus::PIN_LOW;
        cpu->bus->syncControl();

        // T3-falling: Activate MREQ
        cpu->bus->waitClockFalling();
        cpu->bus->pin_o_mreq = Bus::PIN_LOW;
        cpu->bus->syncControl();
    }
}

void Mcycle::m1t4(Cpu* cpu) {
    // T4: Inactivate MREQ, RFSH. Increment R resistor.
    if (! cpu->enable_virtual_memory){
        cpu->bus->waitClockRising();
        cpu->bus->waitClockFalling();
        cpu->bus->pin_o_mreq = Bus::PIN_HIGH;
        cpu->bus->syncControl();

        cpu->bus->pin_o_rfsh = Bus::PIN_HIGH;
        cpu->bus->syncControl();
    }

    uint8_t r1 = (cpu->special_registers.r & 0b10000000);
    uint8_t r7 = (cpu->special_registers.r + 1 & 0b01111111);
    cpu->special_registers.r = r1 | r7;
}

uint8_t Mcycle::m2(Cpu* cpu, uint16_t addr){
    if (cpu->enable_virtual_memory){
        Log::mem_read(cpu, addr, cpu->virtual_memory[addr]);
        return cpu->virtual_memory[addr];
    }

    // T1
    cpu->bus->waitClockRising();
    cpu->bus->setAddress(addr);
    cpu->bus->waitClockFalling();
    cpu->bus->pin_o_mreq = Bus::PIN_LOW;
    cpu->bus->pin_o_rd = Bus::PIN_LOW;
    cpu->bus->syncControl();
    // T2
    cpu->bus->waitClockRising();
    cpu->bus->waitClockFalling();
    while (!cpu->bus->getInput(Bus::Z80_PIN_I_WAIT)){
        cpu->bus->waitClockFalling();
    }
    // T3
    cpu->bus->waitClockRising();
    uint8_t data = cpu->bus->getData();
    cpu->bus->waitClockFalling();
    cpu->bus->pin_o_mreq = Bus::PIN_HIGH;
    cpu->bus->pin_o_rd = Bus::PIN_HIGH;
    cpu->bus->syncControl();

    Log::mem_read(cpu, addr, data);

    return data;
}

void Mcycle::m3(Cpu* cpu, uint16_t addr, uint8_t data){
    if (cpu->enable_virtual_memory){
        cpu->virtual_memory[addr] = data;
        Log::mem_write(cpu, addr, data);
        return;
    }

    // T1
    cpu->bus->waitClockRising();
    cpu->bus->setAddress(addr);
    cpu->bus->waitClockFalling();
    cpu->bus->setData(data);
    cpu->bus->pin_o_mreq = Bus::PIN_LOW;
    cpu->bus->syncControl();
    // T2
    cpu->bus->waitClockRising();
    cpu->bus->waitClockFalling();
    while (!cpu->bus->getInput(Bus::Z80_PIN_I_WAIT)){
        cpu->bus->waitClockFalling();
    }
    cpu->bus->pin_o_wr = Bus::PIN_LOW;
    cpu->bus->syncControl();
    // T3
    cpu->bus->waitClockRising();
    cpu->bus->waitClockFalling();
    cpu->bus->pin_o_mreq = Bus::PIN_HIGH;
    cpu->bus->pin_o_wr = Bus::PIN_HIGH;
    cpu->bus->syncControl();

    Log::mem_write(cpu, addr, data);
}

uint8_t Mcycle::in(Cpu* cpu, uint8_t portL, uint8_t portH){
    // T1
    cpu->bus->waitClockRising();
    uint16_t port = (portH << 8) | portL;
    cpu->bus->setAddress(port);
    cpu->bus->waitClockFalling();
    // T2
    cpu->bus->waitClockRising();
    cpu->bus->pin_o_iorq = Bus::PIN_LOW;
    cpu->bus->pin_o_rd = Bus::PIN_LOW;
    cpu->bus->syncControl();
    cpu->bus->waitClockFalling();
    // TW
    cpu->bus->waitClockRising();
    cpu->bus->waitClockFalling();
    while (!cpu->bus->getInput(Bus::Z80_PIN_I_WAIT)){
        cpu->bus->waitClockFalling();
    }
    // T3
    cpu->bus->waitClockRising();
    uint8_t data = cpu->bus->getData();
    cpu->bus->waitClockFalling();
    cpu->bus->pin_o_iorq = Bus::PIN_HIGH;
    cpu->bus->pin_o_rd = Bus::PIN_HIGH;
    cpu->bus->syncControl();

    Log::io_read(cpu, port, data);

    return data;
}

void Mcycle::out(Cpu* cpu, uint8_t portL, uint8_t portH, uint8_t data){
    // T1
    cpu->bus->waitClockRising();
    uint16_t port = (portH << 8) | portL;
    cpu->bus->setAddress(port);
    cpu->bus->waitClockFalling();
    cpu->bus->setData(data);
    // T2
    cpu->bus->waitClockRising();
    cpu->bus->pin_o_iorq = Bus::PIN_LOW;
    cpu->bus->pin_o_wr = Bus::PIN_LOW;
    cpu->bus->syncControl();
    cpu->bus->waitClockFalling();
    // TW
    cpu->bus->waitClockRising();
    cpu->bus->waitClockFalling();
    while (!cpu->bus->getInput(Bus::Z80_PIN_I_WAIT)){
        cpu->bus->waitClockFalling();
    }
    // T3
    cpu->bus->waitClockRising();
    cpu->bus->waitClockFalling();
    cpu->bus->pin_o_iorq = Bus::PIN_HIGH;
    cpu->bus->pin_o_wr = Bus::PIN_HIGH;
    cpu->bus->syncControl();

    Log::io_write(cpu, port, data);
}
