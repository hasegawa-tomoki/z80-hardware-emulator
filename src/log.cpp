#include "log.hpp"
#include "cpu.hpp"
#include <string>
#include <fstream>

void Log::write(Cpu* cpu, char* string){
    static std::ofstream stream("log.txt");
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "tick:%010u\t%s", cpu->tick, string);

    stream << buffer << std::endl;
//    stream.flush();
}

void Log::execute(Cpu* cpu, uint8_t op_code, const char* mnemonic){
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "step:execute\tpc:%04x\topcode:%02x\tmnemonic:%s", cpu->_special_registers.pc - 1, op_code, mnemonic);
    write(cpu, buffer);
}

void Log::step(Cpu* cpu, const char* step){
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "step:%s", step);
    write(cpu, buffer);
}

void Log::target_register(Cpu* cpu, const char* reg_name){
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "step:register\tregister:%s", reg_name);
    write(cpu, buffer);
}

void Log::error(Cpu* cpu, const char* error){
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "error:%s", error);
    write(cpu, buffer);
}

void Log::mem_read(Cpu* cpu, uint16_t addr, uint8_t data){
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "step:m2(read)\taddr:%04x\tdata:%02x", addr, data);
    write(cpu, buffer);
}

void Log::mem_write(Cpu* cpu, uint16_t addr, uint8_t data){
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "step:m3(write)\taddr:%04x\tdata:%02x", addr, data);
    write(cpu, buffer);
}

void Log::io_read(Cpu* cpu, uint16_t addr, uint8_t data){
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "step:io(read)\taddr:%04x\tdata:%02x", addr, data);
    write(cpu, buffer);
}

void Log::io_write(Cpu* cpu, uint16_t addr, uint8_t data){
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "step:io(write)\taddr:%04x\tdata:%02x", addr, data);
    write(cpu, buffer);
}
