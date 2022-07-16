#include <cstdio>
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

    int idClock = callback_ex(this->_pi, Z80_GPIO_CLK, EITHER_EDGE, Cpu::intClock, this);
    if (idClock < 0){
        printf("Error callback_ex() for Z80_GPIO_CLK");
        return false;
    }
    int idReset = callback_ex(this->_pi, Z80_GPIO_RESET, EITHER_EDGE, Cpu::intReset, this);
    if (idReset < 0){
        printf("Error callback_ex() for Z80_GPIO_RESET");
        return false;
    }
    int idWait = callback_ex(this->_pi, Z80_GPIO_WAIT, EITHER_EDGE, Cpu::intWait, this);
    if (idWait < 0){
        printf("Error callback_ex() for Z80_GPIO_WAIT");
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
void Cpu::intClock(int pi, unsigned gpio, unsigned level, uint32_t tick, void *cpuObj)
{
    Cpu* cpu = static_cast<Cpu*>(cpuObj);

//    static int c = 1;
//    if (c % 10 == 0){
//        printf("Interrupt #%d level %d at %u\n", c, level, tick);
//    }
//    c++;
    cpu->tick = tick;
    cpu->pin_i_clk_prev = cpu->pin_i_clk;
    cpu->pin_i_clk = level;
}
#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedParameter"
void Cpu::intWait(int pi, unsigned gpio, unsigned level, uint32_t tick, void *cpuObj){
    Cpu* cpu = static_cast<Cpu*>(cpuObj);
    cpu->pin_i_wait = level;
}
#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedParameter"
void Cpu::intNmi(int pi, unsigned gpio, unsigned level, uint32_t tick, void *cpuObj){
    Cpu* cpu = static_cast<Cpu*>(cpuObj);
    cpu->pin_i_nmi_prev = cpu->pin_i_nmi;
    cpu->pin_i_nmi = level;
}
#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedParameter"
void Cpu::intInt(int pi, unsigned gpio, unsigned level, uint32_t tick, void *cpuObj){
    Cpu* cpu = static_cast<Cpu*>(cpuObj);
    cpu->pin_i_int_prev = cpu->pin_i_int;
    cpu->pin_i_int = level;
}
#pragma clang diagnostic pop


bool Cpu::clockRising(){
    if (!this->pin_i_clk_prev && this->pin_i_clk){
        this->pin_i_clk_prev = this->pin_i_clk;
        return true;
    }
    return false;
}
bool Cpu::clockFalling(){
    if (this->pin_i_clk_prev && !this->pin_i_clk){
        this->pin_i_clk_prev = this->pin_i_clk;
        return true;
    }
    return false;
}
void Cpu::waitClockRising(){
    while(! this->clockRising());
}
void Cpu::waitClockFalling(){
    while(! this->clockFalling());
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
    while(true){
        if (this->resetting){
            printf("Resetting");
            this->reset();
        }
        Mcycle::m1t1(this);
        Mcycle::m1t2(this);
        Mcycle::m1t3(this);
        Mcycle::m1t4(this);
        this->_opCode.execute(this->executing);
        Log::dump(this);

        // Disable / Enable interrupt
        if (this->waitingEI > 0){
            this->waitingEI--;
            if (this->waitingEI == 0){
                this->iff1 = true;
                this->iff2 = true;
            }
        }
        if (this->waitingDI > 0){
            this->waitingDI--;
            if (this->waitingDI == 0){
                this->iff1 = false;
                this->iff2 = false;
            }
        }

        // NMI
        if (this->NMI_activated){
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
                    uint16_t int_vector_pointer = (this->_special_registers.i << 8) + int_vector;
                    uint16_t int_vector_addr = Mcycle::m2(this, (int_vector_pointer + 1) << 8) +
                                               Mcycle::m2(this, int_vector_pointer);
                    this->_special_registers.pc = int_vector_addr;
                    break;
                }
                default:
                    throw std::runtime_error("Invalid interrupt mode.");
            }
        }
    }
    #pragma clang diagnostic pop
}