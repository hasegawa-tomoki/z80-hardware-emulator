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

    void ixh(uint8_t value);
    [[nodiscard]] uint8_t ixh() const;
    void ixl(uint8_t value);
    [[nodiscard]] uint8_t ixl() const;
    void iyh(uint8_t value);
    [[nodiscard]] uint8_t iyh() const;
    void iyl(uint8_t value);
    [[nodiscard]] uint8_t iyl() const;
};


#endif //Z80EMU_SPECIAL_REGISTERS_HPP
