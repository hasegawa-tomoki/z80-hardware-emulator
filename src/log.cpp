#include "log.hpp"
#include "cpu.hpp"
#include <string>
#include <fstream>

void Log::write(Cpu* cpu, char* string){
    static std::ofstream stream("log.txt");
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "tick:%010u\t%s", cpu->tick, string);
    //printf("%s\n", buffer);

    stream << buffer << std::endl;
    stream.flush();
}

void Log::execute(Cpu* cpu, uint8_t op_code, const char* mnemonic){
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "step:execute\tpc:%04x\topcode:%02x\tmnemonic:%s", cpu->_special_registers.pc - 1, op_code, mnemonic);
    write(cpu, buffer);
}

void Log::before_m1(Cpu* cpu){
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "step:before-m1\tpc:%04x", cpu->_special_registers.pc);
    write(cpu, buffer);
}

void Log::after_m1(Cpu* cpu){
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "step:after-m1\topcode:%02x", cpu->executing);
    write(cpu, buffer);
}

void Log::step(Cpu* cpu, const char* step){
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "step:%s", step);
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
