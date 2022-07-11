#ifndef Z80EMU_REGISTERS_HPP
#define Z80EMU_REGISTERS_HPP
#include <cstdint>

class Registers
{
public:
    uint8_t a = 0;
    uint8_t b = 0;
    uint8_t c = 0;
    uint8_t d = 0;
    uint8_t e = 0;
    uint8_t h = 0;
    uint8_t l = 0;
//    uint16_t ix = 0;
//    uint16_t iy = 0;

    // C: Carry
    bool FC_Carry = false;
    // N: Add/Subtract
    bool FN_Subtract = false;
    // P/V: Parity/Overflow Flag
    bool FPV_ParityOverflow = false;
    // H: Half Carry Flag
    bool FH_HalfCarry = false;
    // Z: Zero Flag
    bool FZ_Zero = false;
    // S: Sign Flag
    bool FS_Sign = false;

    void f(uint8_t value);
    [[nodiscard]] uint8_t f() const;

    void af(uint16_t value);
    [[nodiscard]] uint16_t af() const;
    void bc(uint16_t value);
    [[nodiscard]] uint16_t bc() const;
    void de(uint16_t value);
    [[nodiscard]] uint16_t de() const;
    void hl(uint16_t value);
    [[nodiscard]] uint16_t hl() const;

    uint8_t carry_by_val();

//    void setFlagC();
//    void resetFlagC();
//    void setFlagN();
//    void resetFlagN();
//    void setFlagPV();
//    void resetFlagPV();
//    void setFlagH();
//    void resetFlagH();
//    void setFlagZ();
//    void resetFlagZ();
//    void setFlagS();
//    void resetFlagS();
};

#endif //Z80EMU_REGISTERS_HPP
