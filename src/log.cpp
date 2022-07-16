#include "log.hpp"
#include "cpu.hpp"
#include <string>
#include <fstream>

void Log::write(Cpu* cpu, char* string){
    static std::ofstream stream("log.txt");
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "tick:%010d\t%s", cpu->tick, string);
    //printf("%s\n", buffer);

    stream << buffer << std::endl;
    stream.flush();
}

void Log::dump(Cpu* cpu){
    char buffer[400];
    snprintf(buffer, sizeof(buffer), "opcode:%02x", cpu->executing);
    write(cpu, buffer);
}
