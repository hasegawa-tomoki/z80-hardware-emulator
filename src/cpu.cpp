#include <cstdio>
#include <ctime>
#include <utility>
#include <pigpio.h>
#include "stdexcept"
#include "cpu.hpp"
#include "mcycle.hpp"
#include "opcode.hpp"
#include "log.hpp"

Cpu::Cpu(Bus *_bus)
{
    this->bus = _bus;

    OpCode _opCode(this);
    this->opCode = _opCode;

    Registers resistors;
    this->registers = resistors;
    Registers resistorsAlternate;
    this->registers_alternate = resistorsAlternate;
    SpecialRegisters specialResistors;
    this->special_registers = specialResistors;
}

/**
 * Reset (input, active Low). RESET initializes the CPU as follows:
 * it resets the interrupt enable flip-flop, clears the PC and registers I and R, and sets the interrupt status to Mode 0.
 * During reset time, the address and data bus go to a high-impedance state, and all control output signals go to the inactive state.
 * Notice that RESET must be active for a minimum of three full clock cycles before the reset operation is complete.
 */
void Cpu::reset()
{
    // it resets the interrupt enable flip-flop
    this->iff1 = false;
    this->iff2 = false;
    // clears the PC and registers I and R
    this->special_registers.pc = 0;
    this->special_registers.i = 0;
    this->special_registers.r = 0;
    // sets the interrupt status to Mode 0
    this->interrupt_mode = 0;
    // TODO: the address and data bus go to a high-impedance state
    // all control output signals go to the inactive state
    this->bus->pin_o_m1 = true;
    this->bus->pin_o_rfsh = true;
    this->bus->pin_o_halt = true;
    this->bus->pin_o_rd = true;
    this->bus->pin_o_wr = true;
    this->bus->pin_o_mreq = true;
    this->bus->pin_o_iorq = true;
    this->bus->pin_o_busack = true;

    this->bus->syncControl();
}

void Cpu::instructionCycle(){
    #pragma clang diagnostic push
    #pragma ide diagnostic ignored "EndlessLoop"
    int instructions = 0;
    clock_t start = clock();
    clock_t lastReset = start;
    while(true){
        if (!this->bus->getInput(Bus::Z80_PIN_I_RESET)){
            while(!this->bus->getInput(Bus::Z80_PIN_I_RESET));

            const double time = static_cast<double>(clock() - lastReset) / CLOCKS_PER_SEC * 1000.0;
            if (time > 1000){
                printf("Resetting\n");
                Log::general(this, "Reset");
                this->reset();
            }
        }
        Mcycle::m1t1(this);
        Mcycle::m1t2(this);
        Mcycle::m1t3(this);
        Mcycle::m1t4(this);
        this->opCode.execute(this->executing);

        // Disable / Enable interrupt
        if (this->waitingEI > 0){
            Log::general(this, "Enable interrupt");
            this->waitingEI--;
            if (this->waitingEI == 0){
                this->iff1 = true;
                this->iff2 = true;
            }
        }
        if (this->waitingDI > 0){
            Log::general(this, "Disable interrupt");
            this->waitingDI--;
            if (this->waitingDI == 0){
                this->iff1 = false;
                this->iff2 = false;
            }
        }

        // NMI
        if ((!this->bus->getInput(Bus::Z80_PIN_I_NMI))){
            Log::general(this, "NMI-activated");
            this->iff2 = this->iff1;
            this->iff1 = false;

            uint16_t nmi_jump_addr = 0x0066;
            this->special_registers.sp--;
            Mcycle::m3(this, this->special_registers.sp, this->special_registers.pc >> 8);
            this->special_registers.sp--;
            Mcycle::m3(this, this->special_registers.sp, this->special_registers.pc & 0xff);
            this->special_registers.pc = nmi_jump_addr;
        }
        // INT
        if ((!this->bus->getInput(Bus::Z80_PIN_I_INT)) && this->iff1 && this->bus->getInput(Bus::Z80_PIN_I_BUSRQ)){
            Log::general(this, "INT-activated");
            Mcycle::int_m1t1t2(this);
            Mcycle::m1t3(this);
            Mcycle::m1t4(this);

            this->special_registers.sp--;
            Mcycle::m3(this, this->special_registers.sp, this->special_registers.pc >> 8);
            this->special_registers.sp--;
            Mcycle::m3(this, this->special_registers.sp, this->special_registers.pc & 0xff);

            uint8_t int_vector = this->executing;
            Log::io_read(this, this->special_registers.pc, int_vector);
            switch (this->interrupt_mode){
                case 0:
                    this->opCode.execute(int_vector);
                    break;
                case 1:
                    this->special_registers.pc = 0x0038;
                    break;
                case 2: {
                    uint16_t int_vector_pointer = (this->special_registers.i << 8) + (int_vector & 0b11111110);
                    uint16_t int_vector_addr =
                            Mcycle::m2(this, int_vector_pointer) +
                            (Mcycle::m2(this, int_vector_pointer + 1) << 8);
                    printf("Int vector pointer: %04x\n", int_vector_pointer);
                    printf("Int vector addr: %04x\n", int_vector_addr);
                    this->special_registers.pc = int_vector_addr;
                    break;
                }
                default:
                    throw std::runtime_error("Invalid interrupt mode.");
            }
        }

        instructions++;
        if (instructions == 1000){
            const double time = static_cast<double>(clock() - start) / CLOCKS_PER_SEC * 1000.0;
            printf("1,000 instructions in %lf msec.\n", time);
            start = clock();
            instructions = 0;
        }
    }
    #pragma clang diagnostic pop
}