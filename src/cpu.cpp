#include <cstdio>
#include <ctime>
#include <pigpiod_if2.h>
#include "stdexcept"
#include "cpu.hpp"
#include "mcycle.hpp"
#include "opcode.hpp"
#include "log.hpp"

Cpu::Cpu(int pi)
{
    this->_pi = pi;
}

bool Cpu::init()
{
    Mcp23s17 io(this->_pi);
    io.init();
    this->_io = io;
    Bus bus(io);
    this->_bus = bus;

    OpCode opCode(this);
    this->_opCode = opCode;

    // resistors
    Registers resistors;
    this->_registers = resistors;
    Registers resistorsAlternate;
    this->_registers_alternate = resistorsAlternate;
    SpecialRegisters specialResistors;
    this->_special_registers = specialResistors;

    int idReset = callback_ex(this->_pi, Z80_GPIO_RESET, EITHER_EDGE, Cpu::intReset, this);
    if (idReset < 0){
        printf("Error callback_ex() for Z80_GPIO_RESET");
        return false;
    }
    int idNmi = callback_ex(this->_pi, Z80_GPIO_NMI, EITHER_EDGE, Cpu::intNmi, this);
    if (idNmi < 0){
        printf("Error callback_ex() for Z80_GPIO_NMI");
        return false;
    }
    int idInt = callback_ex(this->_pi, Z80_GPIO_INT, EITHER_EDGE, Cpu::intInt, this);
    if (idInt < 0){
        printf("Error callback_ex() for Z80_GPIO_INT");
        return false;
    }

    return true;
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
    this->_special_registers.pc = 0;
    this->_special_registers.i = 0;
    this->_special_registers.r = 0;
    // sets the interrupt status to Mode 0
    this->interrupt_mode = 0;
    // TODO: the address and data bus go to a high-impedance state
    // all control output signals go to the inactive state
    this->pin_o_m1 = true;
    this->pin_o_rfsh = true;
    this->pin_o_halt = true;
    this->pin_o_rd = true;
    this->pin_o_wr = true;
    this->pin_o_mreq = true;
    this->pin_o_iorq = true;
    this->pin_o_busack = true;

    this->updateControlSignals();
    this->resetting = false;
}

void Cpu::updateControlSignals(){
    uint8_t data = 0;
    if (this->pin_o_m1){ data |= 0b00000001; }
    if (this->pin_o_rfsh){ data |= 0b00000010; }
    if (this->pin_o_halt){ data |= 0b00000100; }
    if (this->pin_o_rd){ data |= 0b00001000; }
    if (this->pin_o_wr){ data |= 0b00010000; }
    if (this->pin_o_mreq){ data |= 0b00100000; }
    if (this->pin_o_iorq){ data |= 0b01000000; }
    if (this->pin_o_busack){ data |= 0b10000000; }

    this->_io.write_gpio8(IO_CONTROL_DATA_BUS, Mcp23s17::MCP23S17_GPIOB, data);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedParameter"
void Cpu::intReset(int pi, unsigned gpio, unsigned level, uint32_t tick, void *cpuObj){
    static uint32_t prev_tick = 0;

    if (tick - prev_tick > 1000){
        printf("intReset()\n");
        Cpu* cpu = static_cast<Cpu*>(cpuObj);
        cpu->resetting = (level == 0);
        cpu->pin_i_reset = level;
    }
    prev_tick = tick;
}
#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedParameter"
void Cpu::intNmi(int pi, unsigned gpio, unsigned level, uint32_t tick, void *cpuObj){
    Cpu* cpu = static_cast<Cpu*>(cpuObj);
    Log::general(cpu, "intNmi");
    cpu->pin_i_nmi_prev = cpu->pin_i_nmi;
    cpu->pin_i_nmi = level;
}
#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedParameter"
void Cpu::intInt(int pi, unsigned gpio, unsigned level, uint32_t tick, void *cpuObj){
    Cpu* cpu = static_cast<Cpu*>(cpuObj);
    Log::general(cpu, "intInt");
    cpu->pin_i_int_prev = cpu->pin_i_int;
    cpu->pin_i_int = level;
}
#pragma clang diagnostic pop

void Cpu::waitClockRising() const{
    while(! this->readGpio(Z80_GPIO_CLK));
}
void Cpu::waitClockFalling() const{
    while(this->readGpio(Z80_GPIO_CLK));
}

bool Cpu::readGpio(uint8_t gpio) const {
    uint32_t data;
    if (gpio <= 31){
        data = read_bank_1(this->_pi);
        return (data & (1 << gpio)) > 0;
    }
    data = read_bank_2(this->_pi);
    return (data & (1 << (gpio - 32))) > 0;
}

bool Cpu::nmiFalling(){
    if (this->pin_i_nmi_prev && !this->pin_i_nmi){
        this->pin_i_nmi_prev = this->pin_i_nmi;
        return true;
    }
    return false;
}
bool Cpu::intFalling(){
    if (this->pin_i_int_prev && !this->pin_i_int){
        this->pin_i_int_prev = this->pin_i_int;
        return true;
    }
    return false;
}

void Cpu::instructionCycle(){
    #pragma clang diagnostic push
    #pragma ide diagnostic ignored "EndlessLoop"
    int instructions = 0;
    clock_t start = clock();
    while(true){
        if (this->resetting){
            printf("Resetting\n");
            this->reset();
        }
        Mcycle::m1t1(this);
        Mcycle::m1t2(this);
        Mcycle::m1t3(this);
        Mcycle::m1t4(this);
        this->_opCode.execute(this->executing);

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
        if (this->NMI_activated){
            Log::general(this, "NMI-activated");
            this->NMI_activated = false;
            this->iff2 = this->iff1;
            this->iff1 = false;

            uint16_t nmi_jump_addr = 0x0066;
            this->_special_registers.sp--;
            Mcycle::m3(this, this->_special_registers.sp, this->_special_registers.pc >> 8);
            this->_special_registers.sp--;
            Mcycle::m3(this, this->_special_registers.sp, this->_special_registers.pc & 0xff);
            this->_special_registers.pc = nmi_jump_addr;
        }
        // INT
        if (this->INT_activated){
            Log::general(this, "INT-activated");
            this->INT_activated = false;
            Mcycle::int_m1t1t2(this);
            Mcycle::m1t3(this);
            Mcycle::m1t4(this);

            this->_special_registers.sp--;
            Mcycle::m3(this, this->_special_registers.sp, this->_special_registers.pc >> 8);
            this->_special_registers.sp--;
            Mcycle::m3(this, this->_special_registers.sp, this->_special_registers.pc & 0xff);

            uint8_t int_vector = this->executing;
            switch (this->interrupt_mode){
                case 0:
                    this->_opCode.execute(int_vector);
                    break;
                case 1:
                    this->_special_registers.pc = 0x0038;
                    break;
                case 2: {
                    uint16_t int_vector_pointer = (this->_special_registers.i << 8) + (int_vector & 0b11111110);
                    uint16_t int_vector_addr =
                            Mcycle::m2(this, int_vector_pointer) +
                            (Mcycle::m2(this, int_vector_pointer + 1) << 8);
                    this->_special_registers.pc = int_vector_addr;
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