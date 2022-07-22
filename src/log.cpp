#include "log.hpp"
#include "cpu.hpp"
#include <string>
#include <fstream>
#include "config.hpp"

void Log::write(Cpu* cpu, char* string){
#ifdef Z80EMU_ENABLE_LOG
    static std::ofstream stream("log.txt");
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "%s", string);

    stream << buffer << std::endl;
#endif //Z80EMU_ENABLE_LOG
}

void Log::dump_registers(Cpu *cpu) {
    char buffer[400];
    snprintf(buffer, sizeof(buffer),
             "type:registers\taf:%04x bc:%04x de:%04x hl:%04x i:%02x r:%02x ix:%02x iy:%04x sp:%04x pc:%04x / FC:%s FN:%s FP/V:%s FH:%s FZ:%s FS:%s",
             cpu->_registers.af(),
             cpu->_registers.bc(),
             cpu->_registers.de(),
             cpu->_registers.hl(),
             cpu->_special_registers.i,
             cpu->_special_registers.r,
             cpu->_special_registers.ix,
             cpu->_special_registers.iy,
             cpu->_special_registers.sp,
             cpu->_special_registers.pc,
             cpu->_registers.FC_Carry?"true": "false",
             cpu->_registers.FN_Subtract?"true": "false",
             cpu->_registers.FPV_ParityOverflow?"true": "false",
             cpu->_registers.FH_HalfCarry?"true": "false",
             cpu->_registers.FZ_Zero?"true": "false",
             cpu->_registers.FS_Sign?"true": "false"
             );
    write(cpu, buffer);
}

void Log::execute(Cpu* cpu, uint8_t op_code, const char* mnemonic){
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "type:execute\tpc:%04x\topcode:%02x\tmnemonic:%s", cpu->_special_registers.pc - 1, op_code, mnemonic);
    write(cpu, buffer);
}

void Log::general(Cpu* cpu, const char* step){
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "type:general\ttext:%s", step);
    write(cpu, buffer);
}

void Log::target_register(Cpu* cpu, const char* reg_name){
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "type:register\tregister:%s", reg_name);
    write(cpu, buffer);
}

void Log::error(Cpu* cpu, const char* error){
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "type:error\terror:%s", error);
    write(cpu, buffer);
}

void Log::mem_read(Cpu* cpu, uint16_t addr, uint8_t data){
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "type:m2(read)\taddr:%04x\tdata:%02x", addr, data);
    write(cpu, buffer);
}

void Log::mem_write(Cpu* cpu, uint16_t addr, uint8_t data){
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "type:m3(write)\taddr:%04x\tdata:%02x", addr, data);
    write(cpu, buffer);
}

void Log::io_read(Cpu* cpu, uint16_t addr, uint8_t data){
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "type:io(read)\taddr:%04x\tdata:%02x", addr, data);
    write(cpu, buffer);
}

void Log::io_write(Cpu* cpu, uint16_t addr, uint8_t data){
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "type:io(write)\taddr:%04x\tdata:%02x", addr, data);
    write(cpu, buffer);
}
