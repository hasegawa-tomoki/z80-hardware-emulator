#include <cstdint>
#include <cstdio>
#include "mcycle.hpp"
#include "cpu.hpp"
#include "log.hpp"
#include <unistd.h>

void Mcycle::int_m1t1t2(Cpu* cpu){
    // t1
    cpu->waitClockRising();
    cpu->_bus.setAddress(cpu->_special_registers.pc);
    cpu->pin_o_mreq = true;
    cpu->pin_o_rd = true;
    cpu->pin_o_m1 = false;
    cpu->waitClockFalling();
    // t2
    cpu->waitClockRising();
    cpu->waitClockFalling();
    // tw
    cpu->waitClockRising();
    cpu->waitClockFalling();
    // tw
    cpu->waitClockRising();
    cpu->waitClockFalling();
    while (! cpu->readGpio(Cpu::Z80_GPIO_WAIT)){
        cpu->waitClockFalling();
    }
}

void Mcycle::m1t1(Cpu* cpu){
    // T1: Output PC's address
    cpu->waitClockRising();
    if (cpu->halt){
        return;
    }
    cpu->_bus.setAddress(cpu->_special_registers.pc);
    if (cpu->enable_virtual_memory){
        cpu->executing = cpu->virtual_memory[cpu->_special_registers.pc];
    }
    cpu->_special_registers.pc++;
    cpu->pin_o_mreq = false;
    cpu->pin_o_rd = false;
    cpu->pin_o_m1 = false;
    cpu->updateControlSignals();
    cpu->waitClockFalling();
}

void Mcycle::m1t2(Cpu* cpu){
    // T2: Wait memory until WAIT is inactive
    cpu->waitClockRising();
    if (cpu->halt){
        return;
    }
    cpu->waitClockFalling();
    while (! cpu->readGpio(Cpu::Z80_GPIO_WAIT)){
        cpu->waitClockFalling();
    }
}

void Mcycle::m1t3(Cpu* cpu) {
    // T3-rising: Fetch data. Output refresh address. Update control signals
    cpu->waitClockRising();
    if (! cpu->halt){
        if (! cpu->enable_virtual_memory){
            cpu->executing = cpu->_bus.read();
        }

        cpu->pin_o_mreq = true;
        cpu->pin_o_rd = true;
        cpu->pin_o_m1 = true;
        cpu->pin_o_rfsh = false;
        cpu->updateControlSignals();
    } else {
        cpu->executing = 0x00;
    }
    auto refreshAddr = (uint16_t)((cpu->_special_registers.i << 8) | cpu->_special_registers.r);
    //printf("Refresh addr: %04x\n", refreshAddr);
    cpu->_bus.setAddress(refreshAddr);
    // T3-falling: Activate MREQ
    cpu->waitClockFalling();
    cpu->pin_o_mreq = false;
    cpu->updateControlSignals();
}

void Mcycle::m1t4(Cpu* cpu) {
    // T4: Inactivate MREQ, RFSH. Increment R resistor.
    cpu->waitClockRising();
    if (! cpu->halt){
        cpu->waitClockFalling();
        cpu->pin_o_mreq = true;
        cpu->updateControlSignals();
    }
    cpu->pin_o_rfsh = true;

    uint8_t r1 = (cpu->_special_registers.r & 0b10000000);
    uint8_t r7 = (cpu->_special_registers.r + 1 & 0b01111111);
    cpu->_special_registers.r = r1 | r7;
    //printf("R: %02x\n", cpu->_special_registers.r);

    if (cpu->nmiFalling()){
        cpu->NMI_activated = true;
    }
    if (cpu->intFalling() && cpu->iff1 && cpu->pin_i_busrq){
        cpu->INT_activated = true;
    }
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "Simplify"
uint8_t Mcycle::m2(Cpu* cpu, uint16_t addr){
    if (cpu->enable_virtual_memory){
        Log::mem_read(cpu, addr, cpu->virtual_memory[addr]);
        return cpu->virtual_memory[addr];
    }

    // T1
    cpu->waitClockRising();
    cpu->_bus.setAddress(addr);
    cpu->pin_o_mreq = false;
    cpu->pin_o_rd = false;
    cpu->updateControlSignals();
    cpu->waitClockFalling();
    // T2
    cpu->waitClockRising();
    cpu->waitClockFalling();
    while (! cpu->readGpio(Cpu::Z80_GPIO_WAIT)){
        cpu->waitClockFalling();
    }
    // T3
    cpu->waitClockRising();
    uint8_t data = cpu->_bus.read();
    cpu->pin_o_mreq = true;
    cpu->pin_o_rd = true;
    cpu->updateControlSignals();
    cpu->waitClockFalling();

    Log::mem_read(cpu, addr, data);

    return data;
}
#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma ide diagnostic ignored "Simplify"
void Mcycle::m3(Cpu* cpu, uint16_t addr, uint8_t data){
    if (cpu->enable_virtual_memory){
        cpu->virtual_memory[addr] = data;
        Log::mem_write(cpu, addr, data);
        return;
    }

    // T1
    cpu->waitClockRising();
    cpu->_bus.setAddress(addr);
    cpu->waitClockFalling();
    cpu->pin_o_mreq = false;
    cpu->updateControlSignals();
    cpu->_bus.write(data);
    // T2
    cpu->waitClockRising();
    cpu->waitClockFalling();
    cpu->pin_o_wr = false;
    cpu->updateControlSignals();
    while (! cpu->readGpio(Cpu::Z80_GPIO_WAIT)){
        cpu->waitClockFalling();
    }
    // T3
    cpu->waitClockRising();
    cpu->waitClockFalling();
    cpu->pin_o_mreq = true;
    cpu->pin_o_wr = true;
    cpu->updateControlSignals();

    Log::mem_write(cpu, addr, data);
    /*
    uint8_t read = Mcycle::m2(cpu, addr);
    if (data != read){
        printf("Memory write failed. write: %02x  read: %02x\n", data, read);
    }
    uint8_t read2 = Mcycle::m2(cpu, addr);
    if (read != read2){
        printf("Memory read unstable. read1: %02x  read2: %02x\n", read, read2);
    }
     */
}
#pragma clang diagnostic pop

uint8_t Mcycle::in(Cpu* cpu, uint8_t portL, uint8_t portH){
    // T1
    cpu->waitClockRising();
    uint16_t port = (portH << 8) | portL;
    cpu->_bus.setAddress(port);
    cpu->waitClockFalling();
    // T2
    cpu->waitClockRising();
    cpu->pin_o_iorq = false;
    cpu->pin_o_rd = false;
    cpu->updateControlSignals();
    cpu->waitClockFalling();
    // TW
    cpu->waitClockRising();
    cpu->waitClockFalling();
    while (! cpu->readGpio(Cpu::Z80_GPIO_WAIT)){
        cpu->waitClockFalling();
    }
    // T3
    cpu->waitClockRising();
    cpu->waitClockFalling();
    uint8_t data = cpu->_bus.read();
    cpu->pin_o_iorq = true;
    cpu->pin_o_rd = true;
    cpu->updateControlSignals();

    Log::io_read(cpu, port, data);

    return data;
}

void Mcycle::out(Cpu* cpu, uint8_t portL, uint8_t portH, uint8_t data){
    // T1
    cpu->waitClockRising();
    uint16_t port = (portH << 8) | portL;
    cpu->_bus.setAddress(port);
    cpu->waitClockFalling();
    // T2
    cpu->waitClockRising();
    cpu->pin_o_iorq = false;
    cpu->pin_o_wr = false;
    cpu->updateControlSignals();
    cpu->waitClockFalling();
    // TW
    cpu->waitClockRising();
    cpu->waitClockFalling();
    while (! cpu->readGpio(Cpu::Z80_GPIO_WAIT)){
        cpu->waitClockFalling();
    }
    // T3
    cpu->waitClockRising();
    cpu->waitClockFalling();
    cpu->pin_o_iorq = true;
    cpu->pin_o_wr = true;
    cpu->updateControlSignals();

    Log::io_write(cpu, port, data);
}
