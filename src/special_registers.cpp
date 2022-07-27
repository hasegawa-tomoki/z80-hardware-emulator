#include "special_registers.hpp"

void SpecialRegisters::ixh(uint8_t value){
    this->ix = (value << 8) | (this->ix & 0xff);
}
uint8_t SpecialRegisters::ixh() const {
    return this->ix >> 8;
}
void SpecialRegisters::ixl(uint8_t value){
    this->ix = (this->ix & 0xff00) | value;
}
uint8_t SpecialRegisters::ixl() const {
    return this->ix & 0xff;
}
void SpecialRegisters::iyh(uint8_t value){
    this->iy = (value << 8) | (this->iy & 0xff);
}
uint8_t SpecialRegisters::iyh() const {
    return this->iy >> 8;
}
void SpecialRegisters::iyl(uint8_t value){
    this->iy = (this->iy & 0xff00) | value;
}
uint8_t SpecialRegisters::iyl() const {
    return this->iy & 0xff;
}
