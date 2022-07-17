#include "log.hpp"
#include "cpu.hpp"
#include <string>
#include <fstream>
#include "config.hpp"

void Log::write(Cpu* cpu, char* string){
#ifdef Z80EMU_ENABLE_LOG
    static std::ofstream stream("log.txt");
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "tick:%010u\t%s", cpu->tick, string);

    stream << buffer << std::endl;
#endif //Z80EMU_ENABLE_LOG
}

void Log::dump_registers(Cpu *cpu) {
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "step:registers\taf:%04x bc:%04x de:%04x hl:%04x i:%02x r:%02x ix:%02x iy:%04x sp:%04x pc:%04x",
             cpu->_registers.af(),
             cpu->_registers.bc(),
             cpu->_registers.de(),
             cpu->_registers.hl(),
             cpu->_special_registers.i,
             cpu->_special_registers.r,
             cpu->_special_registers.ix,
             cpu->_special_registers.iy,
             cpu->_special_registers.sp,
             cpu->_special_registers.pc);
    write(cpu, buffer);
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
