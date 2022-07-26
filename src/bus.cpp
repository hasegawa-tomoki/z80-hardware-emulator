#include "iostream"
#include "bus.hpp"
#include "cpu.hpp"

Bus::Bus()
{
}

Bus::Bus(int io)
{
    this->_io = io;
}

void Bus::tick(){

}

void Bus::setAddress(uint16_t addr) {
//    this->_io.write_gpio(Cpu::IO_ADDRESS_BUS, addr);
}

uint8_t Bus::read(){
    return 0;
//    this->_io.set_direction_8(Cpu::IO_CONTROL_DATA_BUS, Cpu::GPIO_DATA_BUS, Mcp23s17::DIR_INPUT);
//    return this->_io.read_gpio8(Cpu::IO_CONTROL_DATA_BUS, Cpu::GPIO_DATA_BUS);
}

void Bus::write(uint8_t data){
//    this->_io.set_direction_8(Cpu::IO_CONTROL_DATA_BUS, Cpu::GPIO_DATA_BUS, Mcp23s17::DIR_OUTPUT);
//    this->_io.write_gpio8(Cpu::IO_CONTROL_DATA_BUS, Cpu::GPIO_DATA_BUS, data);
}

