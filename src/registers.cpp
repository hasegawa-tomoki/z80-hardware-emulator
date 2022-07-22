#include "registers.hpp"

void Registers::af(uint16_t value){
    this->a = (uint8_t)(value >> 8);
    this->f((uint8_t)(value & 0xff));
}
uint16_t Registers::af() const{
    return (this->a << 8) + this->f();
}

void Registers::bc(uint16_t value){
    this->b = (uint8_t)(value >> 8);
    this->c = (uint8_t)(value & 0xff);
}
uint16_t Registers::bc() const{
    return (this->b << 8) + this->c;
}

void Registers::de(uint16_t value){
    this->d = (uint8_t)(value >> 8);
    this->e = (uint8_t)(value & 0xff);
}
uint16_t Registers::de() const{
    return (this->d << 8) + this->e;
}

void Registers::hl(uint16_t value){
    this->h = (uint8_t)(value >> 8);
    this->l = (uint8_t)(value & 0xff);
}
uint16_t Registers::hl() const{
    return (this->h << 8) + this->l;
}

void Registers::f(uint8_t value){
    this->FC_Carry =            ((value & 0b00000001) > 0);
    this->FN_Subtract =         ((value & 0b00000010) > 0);
    this->FPV_ParityOverflow =  ((value & 0b00000100) > 0);
    this->F_bit3 =              ((value & 0b00001000) > 0);
    this->FH_HalfCarry =        ((value & 0b00010000) > 0);
    this->F_bit5 =              ((value & 0b00100000) > 0);
    this->FZ_Zero =             ((value & 0b01000000) > 0);
    this->FS_Sign =             ((value & 0b10000000) > 0);
}
uint8_t Registers::f() const {
    uint8_t value = 0;
    if (this->FC_Carry){            value |= 0b00000001; }
    if (this->FN_Subtract){         value |= 0b00000010; }
    if (this->FPV_ParityOverflow){  value |= 0b00000100; }
    if (this->F_bit3){              value |= 0b00001000; }
    if (this->FH_HalfCarry){        value |= 0b00010000; }
    if (this->F_bit5){              value |= 0b00100000; }
    if (this->FZ_Zero){             value |= 0b01000000; }
    if (this->FS_Sign){             value |= 0b10000000; }
    return value;
}

uint8_t Registers::carry_by_val(){
    return ((this->FC_Carry) ? 1 : 0);
}
