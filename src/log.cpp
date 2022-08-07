#include "log.hpp"
#include "cpu.hpp"
#include <string>
#include <fstream>
#include "config.hpp"

void Log::write(char* string){
#ifdef Z80EMU_ENABLE_LOG
    static std::ofstream stream("log.txt");
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "%s", string);

    stream << buffer << std::endl;
//    printf("%s\n", buffer);
#endif //Z80EMU_ENABLE_LOG
}

void Log::dump_registers(Cpu *cpu) {
    char buffer[400];
    snprintf(buffer, sizeof(buffer),
             "type:registers\t\t\taf:%04x bc:%04x de:%04x hl:%04x i:%02x r:%02x ix:%04x iy:%04x sp:%04x pc:%04x / FC:%s FN:%s FP/V:%s FH:%s FZ:%s FS:%s",
             cpu->registers.af(),
             cpu->registers.bc(),
             cpu->registers.de(),
             cpu->registers.hl(),
             cpu->special_registers.i,
             cpu->special_registers.r,
             cpu->special_registers.ix,
             cpu->special_registers.iy,
             cpu->special_registers.sp,
             cpu->special_registers.pc,
             cpu->registers.FC_Carry ? "true" : "false",
             cpu->registers.FN_Subtract ? "true" : "false",
             cpu->registers.FPV_ParityOverflow ? "true" : "false",
             cpu->registers.FH_HalfCarry ? "true" : "false",
             cpu->registers.FZ_Zero ? "true" : "false",
             cpu->registers.FS_Sign ? "true" : "false"
             );
    write(buffer);
}

void Log::execute(Cpu* cpu, uint8_t op_code, const char* mnemonic){
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "type:execute\tpc:%04x %02x: %s", cpu->special_registers.pc - 1, op_code, mnemonic);
    write(buffer);
}

void Log::general(Cpu* cpu, const char* step){
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "type:general\t\t\ttext:%s", step);
    write(buffer);
}

void Log::target_register(Cpu* cpu, const char* reg_name){
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "type:register\t\t\tregister:%s", reg_name);
    write(buffer);
}

void Log::error(Cpu* cpu, const char* error){
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "type:error\t\t\terror:%s", error);
    write(buffer);
}

void Log::mem_read(Cpu* cpu, uint16_t addr, uint8_t data){
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "type:m2(read)\t\t\taddr:%04x\tdata:%02x", addr, data);
    write(buffer);
}

void Log::mem_write(Cpu* cpu, uint16_t addr, uint8_t data){
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "type:m3(write)\t\t\taddr:%04x\tdata:%02x", addr, data);
    write(buffer);
}

void Log::io_read(Cpu* cpu, uint16_t addr, uint8_t data){
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "type:io(read)\t\t\taddr:%04x\tdata:%02x", addr, data);
    write(buffer);
}

void Log::io_write(Cpu* cpu, uint16_t addr, uint8_t data){
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "type:io(write)\t\t\taddr:%04x\tdata:%02x", addr, data);
    write(buffer);
}

void Log::bus(Bus* bus, const char* step){
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "type:bus\t\t\ttext:%s", step);
    write(buffer);
}
