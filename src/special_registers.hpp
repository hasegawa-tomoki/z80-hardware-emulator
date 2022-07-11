#ifndef Z80EMU_SPECIAL_REGISTERS_HPP
#define Z80EMU_SPECIAL_REGISTERS_HPP
#include <cstdint>

class SpecialRegisters {
public:
    uint8_t i = 0;
    uint8_t r = 0;
    uint16_t sp = 0;
    uint16_t pc = 0;
    uint16_t ix = 0;
    uint16_t iy = 0;
};


#endif //Z80EMU_SPECIAL_REGISTERS_HPP
