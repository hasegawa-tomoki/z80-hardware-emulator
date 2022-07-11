#include <cstdio>
#include "opcode.hpp"
#include "mcycle.hpp"
#include "stdexcept"

OpCode::OpCode() {
    this->_cpu = nullptr;
}
OpCode::OpCode(Cpu* cpu) {
    this->_cpu = cpu;
}

void OpCode::execute(uint8_t opCode){
    switch (opCode){
        case 0x00: // nop
            break;
        case 0x01: // ld bc, nn
            this->_cpu->_registers.c = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_registers.b = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc + 1);
            this->_cpu->_special_registers.pc += 2;
            break;
        case 0x02: { // ld (bc),a
            uint8_t addr = (this->_cpu->_registers.b << 8) + this->_cpu->_registers.c;
            Mcycle::m3(this->_cpu, addr, this->_cpu->_registers.a);
            break;
        }
        case 0x03: // inc bc
            this->_cpu->_registers.bc(this->_cpu->_registers.bc() + 1);
            break;
        case 0x04: // inc r
        case 0x0C:
        case 0x14:
        case 0x1C:
        case 0x24:
        case 0x2C:
        case 0x3C: {
            uint8_t* reg = this->targetRegisterB(-2, opCode);
            this->setFlagsByIncrement(*reg);
            (*reg)++;
            break;
        }
        case 0x05: // dec r
        case 0x0D:
        case 0x15:
        case 0x1D:
        case 0x25:
        case 0x2D:
        case 0x3D: {
            uint8_t* reg = this->targetRegisterB(-1, opCode);
            this->setFlagsByDecrement(*reg);
            (*reg)--;
            break;
        }
        case 0x06: // ld r, n
        case 0x0e:
        case 0x16:
        case 0x1e:
        case 0x26:
        case 0x2e:
        case 0x3e: {
            uint8_t* reg = this->targetRegisterB(0, opCode);
            *reg = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            break;
        }
        case 0x07: { // rlca
            bool carry_bit = ((this->_cpu->_registers.a & 0x80) > 0);
            this->_cpu->_registers.a = (this->_cpu->_registers.a << 1) | (this->_cpu->_registers.a >> 7);
            this->setFlagsByRotate(this->_cpu->_registers.a, carry_bit);
            break;
        }
        case 0x08: { // ex af, af'
            uint16_t af = this->_cpu->_registers.af();
            this->_cpu->_registers.af(this->_cpu->_registers_alternate.af());
            this->_cpu->_registers_alternate.af(af);
            break;
        }
        case 0x09: // add hl, rr
        case 0x19:
        case 0x29:
        case 0x39: {
            uint16_t value;
            switch (opCode){ // NOLINT(hicpp-multiway-paths-covered)
                case 0x09: value = this->_cpu->_registers.bc(); break;
                case 0x19: value = this->_cpu->_registers.de(); break;
                case 0x29: value = this->_cpu->_registers.hl(); break;
                case 0x39: value = this->_cpu->_special_registers.sp; break;
            }
            this->setFlagsByAdd16(this->_cpu->_registers.hl(), value);
            this->_cpu->_registers.hl(this->_cpu->_registers.hl() + value);
            break;
        }
        case 0x0A: // ld a,(bc)
            this->_cpu->_registers.a = Mcycle::m2(this->_cpu, this->_cpu->_registers.bc());
            break;
        case 0x0B: // dec bc
            this->_cpu->_registers.bc(this->_cpu->_registers.bc() - 1);
            break;
        case 0x0F: { // rrca
            bool carry_bit = ((this->_cpu->_registers.a & 1) > 0);
            this->_cpu->_registers.a = (this->_cpu->_registers.a >> 1) + ((this->_cpu->_registers.a & 1) << 7);
            this->setFlagsByRotate(this->_cpu->_registers.a, carry_bit);
            break;
        }
        case 0x10: { // djnz n
            auto diff = (int8_t)Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc += diff;
            break;
        }
        case 0x11: // ld de, nn
            this->_cpu->_registers.e = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_registers.d = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc + 1);
            this->_cpu->_special_registers.pc += 2;
            break;
        case 0x12: // ld (de),a
            Mcycle::m3(this->_cpu, this->_cpu->_registers.de(), this->_cpu->_registers.a);
            break;
        case 0x13: // inc de
            this->_cpu->_registers.de(this->_cpu->_registers.de() + 1);
            break;
        case 0x17: { // rla
            bool carry_bit = ((this->_cpu->_registers.a & 0x80) > 0);
            this->_cpu->_registers.a = (this->_cpu->_registers.a << 1) & 0xff | this->_cpu->_registers.carry_by_val();
            this->setFlagsByRotate(this->_cpu->_registers.a, carry_bit);
            break;
        }
        case 0x18: // jr n
            this->_cpu->_special_registers.pc += (int8_t)(Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc));
            break;
        case 0x1A: // ld a,(de)
            this->_cpu->_registers.a = Mcycle::m2(this->_cpu, this->_cpu->_registers.de());
            break;
        case 0x1B: // dec de
            this->_cpu->_registers.de(this->_cpu->_registers.de() - 1);
            break;
        case 0x1F: { // rra
            bool carry_bit = ((this->_cpu->_registers.a & 1) > 0);
            this->_cpu->_registers.a = (this->_cpu->_registers.a >> 1) | (this->_cpu->_registers.carry_by_val() << 7);
            this->setFlagsByRotate(this->_cpu->_registers.a, carry_bit);
            break;
        }
        case 0x20: // jr nz, n
            if (! this->_cpu->_registers.FZ_Zero){
                this->_cpu->_special_registers.pc += (int8_t)(Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc));
            } else {
                this->_cpu->_special_registers.pc++;
            }
            break;
        case 0x21: // ld hl, nn
            this->_cpu->_registers.hl(
                    Mcycle::m2(this->_cpu, (this->_cpu->_special_registers.pc + 1) << 8) +
                    Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc));
            this->_cpu->_special_registers.pc += 2;
            break;
        case 0x22: { // ld (nn), hl
            uint16_t addr = Mcycle::m2(this->_cpu, (this->_cpu->_special_registers.pc + 1) << 8) +
                            Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            Mcycle::m3(this->_cpu, addr + 1, this->_cpu->_registers.h);
            Mcycle::m3(this->_cpu, addr, this->_cpu->_registers.l);
            this->_cpu->_special_registers.pc += 2;
            break;
        }
        case 0x23: // inc hl
            this->_cpu->_registers.hl(this->_cpu->_registers.hl() + 1);
            break;
        case 0x27: { // daa
            uint8_t a = this->_cpu->_registers.a;
            uint8_t aH = (a & 0b11110000) >> 4;
            uint8_t aL = a & 0b00001111;
            uint8_t h = aH < 9 ? 0 : aH == 9 ? 1 : 2;
            uint8_t l = aL < 10 ? 0 : 1;
            uint8_t addition = 0x00;
            switch ((this->_cpu->_registers.FC_Carry ? 0b010 : 0) | (this->_cpu->_registers.FH_HalfCarry ? 0b001 : 0)) {
                case 0b00:
                    switch (h) {
                        case 0: addition = (0 == l) ? 0x00 : 0x06; break;
                        case 1: addition = (0 == l) ? 0x00 : 0x66; break;
                        case 2: addition = (0 == l) ? 0x60 : 0x66; break;
                        default: throw std::logic_error("Something wrong");
                    }
                    break;
                case 0b01:
                    switch (h) {
                        case 0: addition = 0x06; break;
                        case 1: addition = (0 == l) ? 0x06 : 0x66; break;
                        case 2: addition = 0x66; break;
                        default: throw std::logic_error("Something wrong");
                    }
                    break;
                case 0b10: addition = (0 == l) ? 0x60 : 0x66; break;
                case 0b11: addition = 0x66; break;
            }
            uint8_t addH = (addition & 0b11110000) >> 4;
            uint8_t addL = addition & 0b00001111;
            this->_cpu->_registers.a = a + (this->_cpu->_registers.FN_Subtract ? -addition : addition);
            this->_cpu->_registers.FH_HalfCarry = (9 < aL + addL);
            this->_cpu->_registers.FC_Carry = (9 < aH + addH);
            this->_cpu->_registers.FS_Sign = ((this->_cpu->_registers.a & 0x80) > 0);
            this->_cpu->_registers.FZ_Zero = (this->_cpu->_registers.a == 0);
            this->_cpu->_registers.FPV_ParityOverflow = (OpCode::count1(this->_cpu->_registers.a) % 2 == 0);
            break;
        }
        case 0x28: // jr z, n
            // JR cc, n
            if (this->_cpu->_registers.FZ_Zero){
                this->_cpu->_special_registers.pc += (int8_t)(Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc));
            } else {
                this->_cpu->_special_registers.pc++;
            }
            break;
        case 0x2A: { // ld hl, (nn)
            this->_cpu->_registers.h = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc + 1);
            this->_cpu->_registers.l = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc += 2;
            break;
        }
        case 0x2B: // dec hl
            this->_cpu->_registers.hl(this->_cpu->_registers.hl() - 1);
            break;
        case 0x2F: // cpl
            this->_cpu->_registers.a ^= 0xff;
            this->_cpu->_registers.FN_Subtract = true;
            this->_cpu->_registers.FH_HalfCarry = true;
            break;
        case 0x30: // jr nc, n
            if (!this->_cpu->_registers.FC_Carry){
                this->_cpu->_special_registers.pc += (int8_t)(Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc));
            } else {
                this->_cpu->_special_registers.pc++;
            }
            break;
        case 0x31: // ld sp, nn
            this->_cpu->_special_registers.sp = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc + 1) << 8) + Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc += 2;
            break;
        case 0x32: { // ld (nn), a
            uint16_t addr = Mcycle::m2(this->_cpu, (this->_cpu->_special_registers.pc + 1) << 8) +
                            Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            Mcycle::m3(this->_cpu, addr, this->_cpu->_registers.a);
            this->_cpu->_special_registers.pc += 2;
            break;
        }
        case 0x33: // inc sp
            this->_cpu->_special_registers.sp++;
            break;
        case 0x34: { // inc (hl)
            uint16_t value = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
            this->setFlagsByIncrement(value);
            Mcycle::m3(this->_cpu, this->_cpu->_registers.hl(), value + 1);
            break;
        }
        case 0x35: { // dec (hl)
            uint16_t value = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
            this->setFlagsByDecrement(value);
            Mcycle::m3(this->_cpu, this->_cpu->_registers.hl(), value - 1);
            break;
        }
        case 0x36: // ld (hl), n
            Mcycle::m3(this->_cpu, this->_cpu->_registers.hl(), Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc));
            this->_cpu->_special_registers.pc++;
            break;
        case 0x37: // scf
            this->_cpu->_registers.FC_Carry = true;
            this->_cpu->_registers.FN_Subtract = false;
            this->_cpu->_registers.FH_HalfCarry = false;
            break;
        case 0x38: // jr c, n
            if (this->_cpu->_registers.FC_Carry) {
                this->_cpu->_special_registers.pc += (int8_t)(Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc));
            } else {
                this->_cpu->_special_registers.pc++;
            }
            break;
        case 0x3A: { // ld a, (nn)
            uint16_t addr = Mcycle::m2(this->_cpu, (this->_cpu->_special_registers.pc + 1) << 8) +
                            Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc += 2;
            this->_cpu->_registers.a = Mcycle::m2(this->_cpu, addr);
            break;
        }
        case 0x3B: // dec sp
            this->_cpu->_special_registers.sp--;
            break;
        case 0x3F: { // ccf
            bool saved_carry = this->_cpu->_registers.FC_Carry;
            this->_cpu->_registers.FC_Carry = !this->_cpu->_registers.FC_Carry;
            this->_cpu->_registers.FN_Subtract = false;
            this->_cpu->_registers.FH_HalfCarry = saved_carry;
            break;
        }
        case 0x40: // ld b, r
        case 0x41:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x45:
        case 0x47:
            this->_cpu->_registers.b = *(this->targetRegisterA(0x40, opCode));
            break;
        case 0x46: // ld b,(hl)
            this->_cpu->_registers.b = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
            break;
        case 0x48: // ld c, r
        case 0x49:
        case 0x4a:
        case 0x4b:
        case 0x4c:
        case 0x4d:
        case 0x4f:
            this->_cpu->_registers.c = *(this->targetRegisterA(0x48, opCode));
            break;
        case 0x4E: // ld c,(hl)
            this->_cpu->_registers.c = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
            break;
        case 0x50: // ld d, r
        case 0x51:
        case 0x52:
        case 0x53:
        case 0x54:
        case 0x55:
        case 0x57:
            this->_cpu->_registers.d = *(this->targetRegisterA(0x50, opCode));
            break;
        case 0x56: // ld d,(hl)
            this->_cpu->_registers.d = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
            break;
        case 0x58: // ld e, r
        case 0x59:
        case 0x5A:
        case 0x5B:
        case 0x5C:
        case 0x5D:
        case 0x5F:
            this->_cpu->_registers.e = *(this->targetRegisterA(0x58, opCode));
            break;
        case 0x5E: // ld e,(hl)
            this->_cpu->_registers.e = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
            break;
        case 0x60: // ld h, r
        case 0x61:
        case 0x62:
        case 0x63:
        case 0x64:
        case 0x65:
        case 0x67:
            this->_cpu->_registers.h = *(this->targetRegisterA(0x60, opCode));
            break;
        case 0x66: // ld h,(hl)
            this->_cpu->_registers.h = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
            break;
        case 0x68: // ld h, r
        case 0x69:
        case 0x6A:
        case 0x6B:
        case 0x6C:
        case 0x6D:
        case 0x6F:
            this->_cpu->_registers.h = *(this->targetRegisterA(0x68, opCode));
            break;
        case 0x6E: // ld l,(hl)
            this->_cpu->_registers.l = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
            break;
        case 0x70: // ld (hl), r
        case 0x71:
        case 0x72:
        case 0x73:
        case 0x74:
        case 0x75:
        case 0x77:
            Mcycle::m3(this->_cpu, this->_cpu->_registers.hl(), *(this->targetRegisterA(0x70, opCode)));
            break;
        case 0x76: // halt
            this->_cpu->halt = true;
            break;
        case 0x78: // ld l, r
        case 0x79:
        case 0x7A:
        case 0x7B:
        case 0x7C:
        case 0x7D:
        case 0x7F:
            this->_cpu->_registers.l = *(this->targetRegisterA(0x78, opCode));
            break;
        case 0x7E: // ld a,(hl)
            this->_cpu->_registers.a = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
            break;
        case 0x80: // add a, r
        case 0x81:
        case 0x82:
        case 0x83:
        case 0x84:
        case 0x85:
        case 0x87: {
            uint8_t* reg = this->targetRegisterA(0x80, opCode);
            this->setFlagsByAddition(this->_cpu->_registers.a, *reg);
            this->_cpu->_registers.a += *reg;
            break;
        }
        case 0x86: { // add a, (hl)
            uint16_t value = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
            this->setFlagsByAddition(this->_cpu->_registers.a, value);
            this->_cpu->_registers.a += value;
            break;
        }
        case 0x88: // adc a, r
        case 0x89:
        case 0x8A:
        case 0x8B:
        case 0x8C:
        case 0x8D:
        case 0x8F: {
            uint8_t* reg = this->targetRegisterA(0x88, opCode);
            uint8_t carry = this->_cpu->_registers.carry_by_val();
            this->setFlagsByAddition(this->_cpu->_registers.a, *reg + carry);
            this->_cpu->_registers.a += *reg + carry;
            break;
        }
        case 0x8E: { // adc a, (hl)
            uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
            uint8_t carry = this->_cpu->_registers.carry_by_val();
            this->setFlagsByAddition(this->_cpu->_registers.a, value + carry);
            this->_cpu->_registers.a += value + carry;
            break;
        }
        case 0x90: // sub r
        case 0x91:
        case 0x92:
        case 0x93:
        case 0x94:
        case 0x95:
        case 0x97: {
            uint8_t* reg = this->targetRegisterA(0x90, opCode);
            this->setFlagsBySubtract(this->_cpu->_registers.a, *reg);
            this->_cpu->_registers.a -= *reg;
            break;
        }
        case 0x96: { // sub (hl)
            uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
            this->setFlagsBySubtract(this->_cpu->_registers.a, value);
            this->_cpu->_registers.a -= value;
            break;
        }
        case 0x98: // sbc a, r
        case 0x99:
        case 0x9A:
        case 0x9B:
        case 0x9C:
        case 0x9D:
        case 0x9F: {
            uint8_t* reg = this->targetRegisterA(0x98, opCode);
            uint8_t carry = this->_cpu->_registers.carry_by_val();
            this->setFlagsBySubtract(this->_cpu->_registers.a, *reg + carry);
            this->_cpu->_registers.a -= *reg + carry;
            break;
        }
        case 0x9E: { // sbc a,(hl)
            uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
            uint8_t carry = this->_cpu->_registers.carry_by_val();
            this->setFlagsBySubtract(this->_cpu->_registers.a, value + carry);
            this->_cpu->_registers.a -= value + carry;
            break;
        }
        case 0xA0: // and r
        case 0xA1:
        case 0xA2:
        case 0xA3:
        case 0xA4:
        case 0xA5:
        case 0xA7: {
            uint8_t* reg = this->targetRegisterA(0xa0, opCode);
            this->_cpu->_registers.a &= *reg;
            this->setFlagsByLogical(true);
            break;
        }
        case 0xA6: // and (hl)
            this->_cpu->_registers.a &= Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
            this->setFlagsByLogical(true);
            break;
        case 0xA8: // xor r
        case 0xA9:
        case 0xAA:
        case 0xAB:
        case 0xAC:
        case 0xAD:
        case 0xAF: {
            uint8_t* reg = this->targetRegisterA(0xa0, opCode);
            this->_cpu->_registers.a ^= *reg;
            this->setFlagsByLogical(false);
            break;
        }
        case 0xAE: // xor (hl)
            this->_cpu->_registers.a ^= Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
            this->setFlagsByLogical(false);
            break;
        case 0xB0: // or r
        case 0xB1:
        case 0xB2:
        case 0xB3:
        case 0xB4:
        case 0xB5:
        case 0xB7: {
            uint8_t* reg = this->targetRegisterA(0xb0, opCode);
            this->_cpu->_registers.a |= *reg;
            this->setFlagsByLogical(false);
            break;
        }
        case 0xB6: // or (hl)
            // OR (HL)
            this->_cpu->_registers.a |= Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
            this->setFlagsByLogical(false);
            break;
        case 0xB8: // cp r
        case 0xB9:
        case 0xBA:
        case 0xBB:
        case 0xBC:
        case 0xBD:
        case 0xBF:
            this->setFlagsBySubtract(this->_cpu->_registers.a, *(this->targetRegisterA(0xb8, opCode)));
            break;
        case 0xBE: { // cp (hl)
            uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
            this->setFlagsBySubtract(this->_cpu->_registers.a, value);
            break;
        }
        case 0xC0: // ret nz
            // RET !FZ
            if (!this->_cpu->_registers.FZ_Zero) {
                this->_cpu->_special_registers.pc = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp + 1) << 8) + Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp);
                this->_cpu->_special_registers.sp += 2;
            }
            break;
        case 0xC1: // pop bc
            // POP BC
            this->_cpu->_registers.c = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp);
            this->_cpu->_registers.b = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp + 1);
            this->_cpu->_special_registers.sp += 2;
            break;
        case 0xC2: // jp nz, nn
            // JP !FZ, nn
            if (!this->_cpu->_registers.FZ_Zero) {
                this->_cpu->_special_registers.pc = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc + 1) << 8) + Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            } else {
                this->_cpu->_special_registers.pc += 2;
            }
            break;
        case 0xC3: // jp nn
            // JP nn
            this->_cpu->_special_registers.pc = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc + 1) << 8) + Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            break;
        case 0xC4: // call nz, nn
            // CALL !FZ, nn
            if (!this->_cpu->_registers.FZ_Zero) {
                uint16_t jump_addr = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc + 1) << 8) + Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
                this->_cpu->_special_registers.pc += 2;
                this->_cpu->_special_registers.sp--;
                Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp, this->_cpu->_special_registers.pc >> 8);
                this->_cpu->_special_registers.sp--;
                Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp, this->_cpu->_special_registers.pc & 0xff);
                this->_cpu->_special_registers.pc = jump_addr;
            } else {
                this->_cpu->_special_registers.pc = (this->_cpu->_special_registers.pc + 2) & 0xffff;
            }
            break;
        case 0xC5: // push bc
            this->_cpu->_special_registers.sp--;
            Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp, this->_cpu->_registers.b);
            this->_cpu->_special_registers.sp--;
            Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp, this->_cpu->_registers.c);
            break;
        case 0xC6: { // add a, n
            uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            this->setFlagsByAddition(this->_cpu->_registers.a, value);
            this->_cpu->_registers.a += value;
            break;
        }
        case 0xC7: // rst n (n = 0 - 7)
        case 0xCF:
        case 0xD7:
        case 0xDF:
        case 0xE7:
        case 0xEF:
        case 0xF7:
        case 0xFF:
            this->_cpu->_special_registers.sp--;
            Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp, this->_cpu->_special_registers.pc >> 8);
            this->_cpu->_special_registers.sp--;
            Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp, this->_cpu->_special_registers.pc & 0xff);
            this->_cpu->_special_registers.pc = (uint16_t)(opCode - 0xc7);
            break;
        case 0xC8: // ret z
            if (this->_cpu->_registers.FZ_Zero) {
                this->_cpu->_special_registers.pc = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp + 1) << 8) + Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp);
                this->_cpu->_special_registers.sp += 2;
            }
            break;
        case 0xC9: // ret
            this->_cpu->_special_registers.pc = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp + 1) << 8) + Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp);
            this->_cpu->_special_registers.sp += 2;
            break;
        case 0xCA: // jp z, nn
            if (this->_cpu->_registers.FZ_Zero) {
                this->_cpu->_special_registers.pc = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc + 1) << 8) + Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            } else {
                this->_cpu->_special_registers.pc = (this->_cpu->_special_registers.pc + 2) & 0xFFFF;
            }
            break;
        case 0xCB: { // BITS
            uint8_t opcode = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            executeCb(opcode);
            break;
        }
        case 0xCC: // call z, nn
            if (this->_cpu->_registers.FZ_Zero) {
                uint16_t jump_addr = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc + 1) << 8) + Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
                this->_cpu->_special_registers.pc += 2;
                this->_cpu->_special_registers.sp--;
                Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp, this->_cpu->_special_registers.pc >> 8);
                this->_cpu->_special_registers.sp--;
                Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp, this->_cpu->_special_registers.pc & 0xff);
                this->_cpu->_special_registers.pc = jump_addr;
            } else {
                this->_cpu->_special_registers.pc += 2;
            }
            break;
        case 0xCD: { // call nn
            uint16_t jump_addr = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc + 1) << 8) + Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc += 2;
            this->_cpu->_special_registers.sp--;
            Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp, this->_cpu->_special_registers.pc >> 8);
            this->_cpu->_special_registers.sp--;
            Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp, this->_cpu->_special_registers.pc & 0xff);
            this->_cpu->_special_registers.pc = jump_addr;
            break;
        }
        case 0xCE: { // adc a, n
            uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            uint8_t carry = this->_cpu->_registers.carry_by_val();
            this->setFlagsByAddition(this->_cpu->_registers.a, value + carry);
            this->_cpu->_registers.a += value + carry;
            break;
        }
        case 0xD0: // ret nc
            // RET !FC
            if (!this->_cpu->_registers.FC_Carry) {
                this->_cpu->_special_registers.pc = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp + 1) << 8) + Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp);
                this->_cpu->_special_registers.sp += 2;
            }
            break;
        case 0xD1: // pop de
            // POP DE
            this->_cpu->_registers.e = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp);
            this->_cpu->_registers.d = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp + 1);
            this->_cpu->_special_registers.sp += 2;
            break;
        case 0xD2: // jp nc, nn
            // JP !FC, nn
            if (!this->_cpu->_registers.FC_Carry) {
                this->_cpu->_special_registers.pc = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc + 1) << 8) + Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            } else {
                this->_cpu->_special_registers.pc += 2;
            }
            break;
        case 0xD3: { // out (n),a
            uint8_t port = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc + 1);
            Mcycle::out(this->_cpu, port, this->_cpu->_registers.a);
            this->_cpu->_special_registers.pc += 1;
            break;
        }
        case 0xD4: // call nc, nn
            // CALL !FC, nn
            if (!this->_cpu->_registers.FC_Carry) {
                uint16_t jump_addr = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc + 1) << 8) + Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
                this->_cpu->_special_registers.pc += 2;
                this->_cpu->_special_registers.sp--;
                Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp, this->_cpu->_special_registers.pc >> 8);
                this->_cpu->_special_registers.sp--;
                Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp, this->_cpu->_special_registers.pc & 0xff);
                this->_cpu->_special_registers.pc = jump_addr;
            } else {
                this->_cpu->_special_registers.pc += 2;
            }
            break;
        case 0xD5: // push de
            // PUSH DE
            this->_cpu->_special_registers.sp--;
            Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp, this->_cpu->_registers.d);
            this->_cpu->_special_registers.sp--;
            Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp, this->_cpu->_registers.e);
            break;
        case 0xD6: { // sub n
            uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            this->setFlagsBySubtract(this->_cpu->_registers.a, value);
            this->_cpu->_registers.a -= value;
            break;
        }
        case 0xD8: // ret c
            // RET FC
            if (this->_cpu->_registers.FC_Carry) {
                this->_cpu->_special_registers.pc = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp + 1) << 8) + Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp);
                this->_cpu->_special_registers.sp += 2;
            }
            break;
        case 0xD9: { // exx
            uint16_t temp;
            temp = this->_cpu->_registers.bc();
            this->_cpu->_registers.bc(this->_cpu->_registers_alternate.bc());
            this->_cpu->_registers_alternate.bc(temp);
            temp = this->_cpu->_registers.de();
            this->_cpu->_registers.de(this->_cpu->_registers_alternate.de());
            this->_cpu->_registers_alternate.de(temp);
            temp = this->_cpu->_registers.hl();
            this->_cpu->_registers.hl(this->_cpu->_registers_alternate.hl());
            this->_cpu->_registers_alternate.hl(temp);
            break;
        }
        case 0xDA: // jp c, nn
            // JP FC, nn
            if (this->_cpu->_registers.FC_Carry) {
                this->_cpu->_special_registers.pc = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc + 1) << 8) + Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            } else {
                this->_cpu->_special_registers.pc += 2;
            }
            break;
        case 0xDB: { // in a, (n)
            uint8_t port = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc + 1);
            this->_cpu->_registers.a = Mcycle::in(this->_cpu, port);
            this->_cpu->_special_registers.pc += 1;
            break;
        }
        case 0xDC: // call c, nn
            // CALL FC, nn
            if (this->_cpu->_registers.FC_Carry) {
                uint16_t jump_addr = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc + 1) << 8) + Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
                this->_cpu->_special_registers.pc += 2;
                this->_cpu->_special_registers.sp--;
                Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp, this->_cpu->_special_registers.pc >> 8);
                this->_cpu->_special_registers.sp--;
                Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp, this->_cpu->_special_registers.pc & 0xff);
                this->_cpu->_special_registers.pc = jump_addr;
            } else {
                this->_cpu->_special_registers.pc += 2;
            }
            break;
        case 0xDD: { // IX
            uint8_t opcode = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            executeDd(opcode);
            break;
        }
        case 0xDE: { // sbc a, n
            uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            uint8_t carry = this->_cpu->_registers.carry_by_val();
            this->setFlagsBySubtract(this->_cpu->_registers.a, value + carry);
            this->_cpu->_registers.a -= value + carry;
            break;
        }
        case 0xE0: // ret po
            if (! this->_cpu->_registers.FPV_ParityOverflow){
                this->_cpu->_special_registers.pc = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp + 1) << 8) + Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp);
                this->_cpu->_special_registers.sp += 2;
            }
        case 0xE1: // pop hl
            this->_cpu->_registers.hl((Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp + 1) << 8) + Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp));
            this->_cpu->_special_registers.sp += 2;
            break;
        case 0xE2: // jp po, nn ToDO: FIXIT
            Mcycle::m3(this->_cpu, 0xff00 + this->_cpu->_registers.c, this->_cpu->_registers.a);
            break;
        case 0xE3: { // ex (sp),hl
            uint8_t mem_value = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp);
            uint8_t temp_hl = this->_cpu->_registers.hl();
            this->_cpu->_registers.hl(mem_value);
            Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp, temp_hl & 0xff);
            Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp + 1, temp_hl >> 8);
            break;
        }
        case 0xE4: // call po, nn
            if (! this->_cpu->_registers.FPV_ParityOverflow) {
                uint16_t jump_addr = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc + 1) << 8) + Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
                this->_cpu->_special_registers.pc += 2;
                this->_cpu->_special_registers.sp--;
                Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp, this->_cpu->_special_registers.pc >> 8);
                this->_cpu->_special_registers.sp--;
                Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp, this->_cpu->_special_registers.pc & 0xff);
                this->_cpu->_special_registers.pc = jump_addr;
            } else {
                this->_cpu->_special_registers.pc = (this->_cpu->_special_registers.pc + 2) & 0xffff;
            }
            break;
        case 0xE5: // push hl
            // PUSH HL
            this->_cpu->_special_registers.sp--;
            Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp, this->_cpu->_registers.hl() >> 8);
            this->_cpu->_special_registers.sp--;
            Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp, this->_cpu->_registers.hl() & 0xff);
            break;
        case 0xE6: // and n
            // AND n
            this->_cpu->_registers.a &= Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            this->setFlagsByLogical(true);
            break;
        case 0xE8: // ret pe
            if (this->_cpu->_registers.FPV_ParityOverflow) {
                this->_cpu->_special_registers.pc = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp + 1) << 8) + Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp);
                this->_cpu->_special_registers.sp += 2;
            }
            break;
        case 0xE9: // jp (hl)
            // JP, (HL)
            this->_cpu->_special_registers.pc = this->_cpu->_registers.hl();
            break;
        case 0xEA: // jp pe, nn
            if (!this->_cpu->_registers.FPV_ParityOverflow) {
                this->_cpu->_special_registers.pc = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc + 1) << 8) + Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            } else {
                this->_cpu->_special_registers.pc += 2;
            }
        case 0xEB: { // ex de,hl
            uint16_t de = this->_cpu->_registers.de();
            this->_cpu->_registers.de(this->_cpu->_registers.hl());
            this->_cpu->_registers.hl(de);
            break;
        }
        case 0xEC: // call pe, nn
            if (this->_cpu->_registers.FPV_ParityOverflow) {
                uint16_t jump_addr = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc + 1) << 8) + Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
                this->_cpu->_special_registers.pc += 2;
                this->_cpu->_special_registers.sp--;
                Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp, this->_cpu->_special_registers.pc >> 8);
                this->_cpu->_special_registers.sp--;
                Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp, this->_cpu->_special_registers.pc & 0xff);
                this->_cpu->_special_registers.pc = jump_addr;
            } else {
                this->_cpu->_special_registers.pc = (this->_cpu->_special_registers.pc + 2) & 0xffff;
            }
        case 0xED: { // EXTD
            uint8_t opcode = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            executeEd(opcode);
            break;
        }
        case 0xEE: // xor n
            // XOR n
            this->_cpu->_registers.a ^= Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            this->setFlagsByLogical(false);
            break;
        case 0xF0: // ret p
            if (! this->_cpu->_registers.FN_Subtract){
                this->_cpu->_special_registers.pc = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp + 1) << 8) + Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp);
                this->_cpu->_special_registers.sp += 2;
            }
        case 0xF1: { // pop af
            // POP AF
            uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp);
            this->_cpu->_registers.FZ_Zero = ((value & 0x80) == 0x80);
            this->_cpu->_registers.FN_Subtract = ((value & 0x40) == 0x40);
            this->_cpu->_registers.FH_HalfCarry = ((value & 0x20) == 0x20);
            this->_cpu->_registers.FC_Carry = ((value & 0x10) == 0x10);
            this->_cpu->_registers.a = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp + 1);
            this->_cpu->_special_registers.sp += 2;
            break;
        }
        case 0xF2: // jp p, nn
            if (! this->_cpu->_registers.FN_Subtract){
                this->_cpu->_special_registers.pc = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc + 1) << 8) + Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            } else {
                this->_cpu->_special_registers.pc += 2;
            }
        case 0xF3: // di
            // DI
            this->_cpu->waitingDI = 1;
            break;
        case 0xF4: // call p, nn
            if (! this->_cpu->_registers.FN_Subtract) {
                uint16_t jump_addr = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc + 1) << 8) + Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
                this->_cpu->_special_registers.pc += 2;
                this->_cpu->_special_registers.sp--;
                Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp, this->_cpu->_special_registers.pc >> 8);
                this->_cpu->_special_registers.sp--;
                Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp, this->_cpu->_special_registers.pc & 0xff);
                this->_cpu->_special_registers.pc = jump_addr;
            } else {
                this->_cpu->_special_registers.pc = (this->_cpu->_special_registers.pc + 2) & 0xffff;
            }
            break;
        case 0xF5: // push af
            // PUSH AF
            this->_cpu->_special_registers.sp--;
            Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp, this->_cpu->_registers.a);
            this->_cpu->_special_registers.sp--;
            Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp, this->_cpu->_registers.f());
            break;
        case 0xF6: // or n
            // OR n
            this->_cpu->_registers.a |= Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            this->setFlagsByLogical(false);
            break;
        case 0xF8: // ret m
            if (this->_cpu->_registers.FS_Sign){
                this->_cpu->_special_registers.pc = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp + 1) << 8) + Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp);
                this->_cpu->_special_registers.sp += 2;
            }
            break;
        case 0xF9: // ld sp,hl
            // LD SP, HL
            this->_cpu->_special_registers.sp = this->_cpu->_registers.hl();
            break;
        case 0xFA: // jp m, nn
            if (this->_cpu->_registers.FS_Sign){
                this->_cpu->_special_registers.pc = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc + 1) << 8) + Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            } else {
                this->_cpu->_special_registers.pc += 2;
            }
            break;
        case 0xFB: // ei
            this->_cpu->waitingEI = 2;
            break;
        case 0xFC: // call m, nn
            if (this->_cpu->_registers.FS_Sign){
                uint16_t jump_addr = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc + 1) << 8) + Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
                this->_cpu->_special_registers.pc += 2;
                this->_cpu->_special_registers.sp--;
                Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp, this->_cpu->_special_registers.pc >> 8);
                this->_cpu->_special_registers.sp--;
                Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp, this->_cpu->_special_registers.pc & 0xff);
                this->_cpu->_special_registers.pc = jump_addr;
            } else {
                this->_cpu->_special_registers.pc = (this->_cpu->_special_registers.pc + 2) & 0xffff;
            }
            break;
        case 0xFD: { // IY
            uint8_t opcode = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            executeFd(opcode);
            break;
        }
        case 0xFE: { // cp n
            this->setFlagsBySubtract(this->_cpu->_registers.a, Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc));
            this->_cpu->_special_registers.pc++;
            break;
        }
    }
}

void OpCode::executeCb(uint8_t opCode) {
    if ((opCode >> 6) == 0b01){
        uint8_t bit = ((opCode & 0b0011100) >> 3);
        uint8_t reg_idx = (opCode & 0b00000111);
        if (reg_idx == 0b110){
            // bit b, (hl)
            uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
            this->_cpu->_registers.FZ_Zero = ((value & (1 << bit)) == 0);
        } else {
            // bit b, r
            uint8_t* reg = this->targetRegisterA(0, reg_idx);
            this->_cpu->_registers.FZ_Zero = ((*reg & (1 << bit)) == 0);
        }
        this->_cpu->_registers.FN_Subtract = false;
        this->_cpu->_registers.FH_HalfCarry = true;
        return;
    }
    if ((opCode >> 6) == 0b10){
        uint8_t bit = ((opCode & 0b0011100) >> 3);
        uint8_t reg_idx = (opCode & 0b00000111);
        if (reg_idx == 0b110){
            // res b, (hl)
            uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
            value &= -(value & (1 << bit));
            Mcycle::m3(this->_cpu, this->_cpu->_registers.hl(), value);
        } else {
            // res b, r
            uint8_t* reg = this->targetRegisterA(0, reg_idx);
            *reg &= -(*reg & (1 << bit));
        }
        return;
    }
    if ((opCode >> 6) == 0b11){
        uint8_t bit = ((opCode & 0b0011100) >> 3);
        uint8_t reg_idx = (opCode & 0b00000111);
        if (reg_idx == 0b110){
            // set b, (hl)
            uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
            value |= (value & (1 << bit));
            Mcycle::m3(this->_cpu, this->_cpu->_registers.hl(), value);
        } else {
            // set b, r
            uint8_t* reg = this->targetRegisterA(0, reg_idx);
            *reg |= (*reg & (1 << bit));
        }
        return;
    }

    switch (opCode){
        case 0x00: // rlc r
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x05:
        case 0x07: {
            uint8_t* reg = this->targetRegisterA(0x00, opCode);
            bool carry_bit = ((*reg & 0x80) > 0);
            *reg = (*reg << 1) | (*reg >> 7);
            this->setFlagsByRotate(*reg, carry_bit);
            break;
        }
        case 0x06: { // rlc (hl)
            uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
            bool carry_bit = ((value & 0x80) > 0);
            value = (value << 1) | (value >> 7);
            this->setFlagsByRotate(value, carry_bit);
            Mcycle::m3(this->_cpu, this->_cpu->_registers.hl(), value);
            break;
        }
        case 0x08: // rrc r
        case 0x09:
        case 0x0A:
        case 0x0B:
        case 0x0C:
        case 0x0D:
        case 0x0F: {
            uint8_t *reg = this->targetRegisterA(0x00, opCode);
            bool carry_bit = ((*reg & 1) > 0);
            *reg = (*reg >> 1) + ((*reg & 1) << 7);
            this->setFlagsByRotate(*reg, carry_bit);
            break;
        }
        case 0x0E: { // rrc (hl)
            uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
            bool carry_bit = ((value & 1) > 0);
            value = (value >> 1) + ((value & 1) << 7);
            this->setFlagsByRotate(value, carry_bit);
            Mcycle::m3(this->_cpu, this->_cpu->_registers.hl(), value);
            break;
        }
        case 0x10: // rl r
        case 0x11:
        case 0x12:
        case 0x13:
        case 0x14:
        case 0x15:
        case 0x17: {
            uint8_t* reg = this->targetRegisterA(0x10, opCode);
            bool carry_bit = ((*reg & 0x80) > 0);
            *reg = (*reg << 1) & 0xff | this->_cpu->_registers.carry_by_val();
            this->setFlagsByRotate(*reg, carry_bit);
            break;
        }
        case 0x16: { // rl (hl)
            uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
            bool carry_bit = ((value & 0x80) > 0);
            value = (value << 1) & 0xff | this->_cpu->_registers.carry_by_val();
            this->setFlagsByRotate(value, carry_bit);
            Mcycle::m3(this->_cpu, this->_cpu->_registers.hl(), value);
            break;
        }
        case 0x18: // rr r
        case 0x19:
        case 0x1A:
        case 0x1B:
        case 0x1C:
        case 0x1D:
        case 0x1F: {
            uint8_t* reg = this->targetRegisterA(0x18, opCode);
            bool carry_bit = ((*reg & 1) > 0);
            *reg = (*reg >> 1) | (this->_cpu->_registers.carry_by_val() << 7);
            this->setFlagsByRotate(*reg, carry_bit);
            break;
        }
        case 0x1E: { // rr (hl)
            uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
            bool carry_bit = ((value & 1) > 0);
            value = (value >> 1) | (this->_cpu->_registers.carry_by_val() << 7);
            this->setFlagsByRotate(value, carry_bit);
            Mcycle::m3(this->_cpu, this->_cpu->_registers.hl(), value);
            break;
        }
        case 0x20: // sla r
        case 0x21:
        case 0x22:
        case 0x23:
        case 0x24:
        case 0x25:
        case 0x27: {
            uint8_t* reg = this->targetRegisterA(0x20, opCode);
            bool carry_bit = ((*reg & 0x80) > 0);
            *reg = (*reg << 1) & 0xff;
            this->setFlagsByRotate(*reg, carry_bit);
            break;
        }
        case 0x26: { // sla (hl)
            uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
            bool carry_bit = ((value & 0x80) > 0);
            value = (value << 1) & 0xff;
            this->setFlagsByRotate(value, carry_bit);
            Mcycle::m3(this->_cpu, this->_cpu->_registers.hl(), value);
            break;
        }
        case 0x28: // sra r
        case 0x29:
        case 0x2A:
        case 0x2B:
        case 0x2C:
        case 0x2D:
        case 0x2F: {
            uint8_t* reg = this->targetRegisterA(0x28, opCode);
            bool carry_bit = ((*reg & 0x01) > 0);
            *reg = (*reg & 0x80) | (*reg >> 1);
            this->setFlagsByRotate(*reg, carry_bit);
            break;
        }
        case 0x2E: { // sra (hl)
            uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
            bool carry_bit = ((value & 0x01) > 0);
            value = (value & 0x80) | (value >> 1);
            this->setFlagsByRotate(value, carry_bit);
            Mcycle::m3(this->_cpu, this->_cpu->_registers.hl(), value);
            break;
        }
        case 0x38: // srl r
        case 0x39:
        case 0x3A:
        case 0x3B:
        case 0x3C:
        case 0x3D:
        case 0x3F: {
            uint8_t* reg = this->targetRegisterA(0x28, opCode);
            bool carry_bit = ((*reg & 0x01) > 0);
            *reg >>= 1;
            this->setFlagsByRotate(*reg, carry_bit);
            break;
        }
        case 0x3E: { // srl (hl)
            uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
            bool carry_bit = ((value & 0x01) > 0);
            value >>= 1;
            this->setFlagsByRotate(value, carry_bit);
            Mcycle::m3(this->_cpu, this->_cpu->_registers.hl(), value);
            break;
        }
        default:
            char error[100];
            sprintf(error, "Invalid op code: CB %02x", opCode);
            throw std::runtime_error(error);
    }
}

void OpCode::executeDd(uint8_t opCode){
    switch (opCode){
        case 0x09: // add ix, rr
        case 0x19:
        case 0x29:
        case 0x39: {
            uint16_t value;
            switch (opCode){ // NOLINT(hicpp-multiway-paths-covered)
                case 0x09: value = this->_cpu->_registers.bc(); break;
                case 0x19: value = this->_cpu->_registers.de(); break;
                case 0x29: value = this->_cpu->_special_registers.ix; break;
                case 0x39: value = this->_cpu->_special_registers.sp; break;
            }
            this->setFlagsByAdd16(this->_cpu->_special_registers.ix, value);
            this->_cpu->_special_registers.ix += value;
            break;
        }
        case 0x21: { // ld ix, nn
            uint16_t data = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc + 1) << 8) +
                    Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc += 2;
            this->_cpu->_special_registers.ix = data;
            break;
        }
        case 0x22: { // ld (nn), ix
            uint16_t addr = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc + 1) << 8) +
                            Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc += 2;
            Mcycle::m3(this->_cpu, addr + 1, this->_cpu->_special_registers.ix >> 8);
            Mcycle::m3(this->_cpu, addr, this->_cpu->_special_registers.ix & 0xff);
            break;
        }
        case 0x23: // inc ix
            this->_cpu->_special_registers.ix++;
            break;
        case 0x2A: { // ld ix, (nn)
            uint16_t addr = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc + 1) << 8) +
                            Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.ix =
                    (Mcycle::m2(this->_cpu, addr + 1) << 8) +
                    Mcycle::m2(this->_cpu, addr);
            break;
        }
        case 0x2B: // dec ix
            this->_cpu->_special_registers.ix--;
            break;
        case 0x34: { // inc (ix + d)
            auto d = (int8_t)Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            uint16_t addr = this->_cpu->_special_registers.ix + d;
            uint16_t data = Mcycle::m2(this->_cpu, addr);
            this->setFlagsByIncrement(data);
            Mcycle::m3(this->_cpu, addr, data + 1);
            break;
        }
        case 0x35: { // dec (ix + d)
            auto d = (int8_t)Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            uint16_t addr = this->_cpu->_special_registers.ix + d;
            uint16_t data = Mcycle::m2(this->_cpu, addr);
            this->setFlagsByDecrement(data);
            Mcycle::m3(this->_cpu, addr, data - 1);
            break;
        }
        case 0x36: { // ld (ix + d), n
            auto d = (int8_t)Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            uint8_t data = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            Mcycle::m3(this->_cpu, this->_cpu->_special_registers.ix + d, data);
            break;
        }
        case 0x46: // ld r, (ix + d)
        case 0x4E:
        case 0x56:
        case 0x5E:
        case 0x66:
        case 0x6E:
        case 0x7E: {
            auto d = (int8_t)Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            uint8_t* targetRegister = this->targetRegisterB(0x40, opCode);
            *targetRegister = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.ix + d);
            break;
        }
        case 0x70: // ld (ix + d), r
        case 0x71:
        case 0x72:
        case 0x73:
        case 0x74:
        case 0x75:
        case 0x77: {
            auto d = (int8_t)Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            uint8_t* targetRegister = this->targetRegisterA(0x70, opCode);
            Mcycle::m3(this->_cpu, this->_cpu->_special_registers.ix + d, *targetRegister);
            break;
        }
        case 0x86: { // add a, (ix + d)
            auto d = (int8_t)Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.ix + d);
            this->setFlagsByAddition(this->_cpu->_registers.a, value);
            this->_cpu->_registers.a += value;
            break;
        }
        case 0x8E: { // adc a, (ix + d)
            auto d = (int8_t)Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.ix + d);
            uint8_t carry = this->_cpu->_registers.carry_by_val();
            this->setFlagsByAddition(this->_cpu->_registers.a, value + carry);
            this->_cpu->_registers.a += value + carry;
            break;
        }
        case 0x96: { // sub (ix + d)
            auto d = (int8_t)Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.ix + d);
            this->setFlagsBySubtract(this->_cpu->_registers.a, value);
            this->_cpu->_registers.a -= value;
            break;
        }
        case 0x9E: { // sbc a, (ix + d)
            auto d = (int8_t)Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.ix + d);
            this->_cpu->_registers.FH_HalfCarry = ((this->_cpu->_registers.a & 0xf) - (value & 0xf) - this->_cpu->_registers.carry_by_val() < 0);
            this->_cpu->_registers.FC_Carry = (this->_cpu->_registers.a - (value + this->_cpu->_registers.carry_by_val()));
            this->_cpu->_registers.a -= (value + this->_cpu->_registers.carry_by_val());
            this->_cpu->_registers.FZ_Zero = (this->_cpu->_registers.a == 0);
            this->_cpu->_registers.FN_Subtract = true;
            break;
        }
        case 0xA6: { // and (ix + d)
            auto d = (int8_t)Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            this->_cpu->_registers.a &= Mcycle::m2(this->_cpu, this->_cpu->_special_registers.ix + d);
            this->setFlagsByLogical(true);
            break;
        }
        case 0xAE: { // xor (ix + d)
            auto d = (int8_t)Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            this->_cpu->_registers.a ^= Mcycle::m2(this->_cpu, this->_cpu->_special_registers.ix + d);
            this->setFlagsByLogical(false);
            break;
        }
        case 0xB6: { // or (ix + d)
            auto d = (int8_t)Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            this->_cpu->_registers.a |= Mcycle::m2(this->_cpu, this->_cpu->_special_registers.ix + d);
            this->setFlagsByLogical(false);
            break;
        }
        case 0xBE: { // cp (ix + d)
            auto d = (int8_t)Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.ix + d);
            this->setFlagsBySubtract(this->_cpu->_registers.a, value);
            break;
        }
        case 0xCB: {
            this->executeXxCb(this->_cpu->_special_registers.ix);
            break;
        }
        case 0xE1: // pop ix
            this->_cpu->_special_registers.ix =
                    (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp + 1) << 8) +
                    Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp);
            this->_cpu->_special_registers.sp += 2;
            break;
        case 0xE3: { // ex (sp), ix
            uint8_t temp_ix = this->_cpu->_special_registers.ix;
            this->_cpu->_special_registers.ix = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp);
            Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp, temp_ix & 0xff);
            Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp + 1, temp_ix >> 8);
            break;
        }
        case 0xE5: // push ix
            this->_cpu->_special_registers.sp--;
            Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp, this->_cpu->_special_registers.ix >> 8);
            this->_cpu->_special_registers.sp--;
            Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp, this->_cpu->_special_registers.ix & 0xff);
            break;
        case 0xE9: // jp (ix)
            this->_cpu->_special_registers.pc =
                    (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.ix + 1) << 8) +
                    Mcycle::m2(this->_cpu, this->_cpu->_special_registers.ix);
            break;
        case 0xF9: // ld sp, ix
            this->_cpu->_special_registers.sp = this->_cpu->_special_registers.ix;
            break;
        default: {
            char error[100];
            sprintf(error, "Invalid op code: DD %02x", opCode);
            throw std::runtime_error(error);
        }
    }
}

void OpCode::executeXxCb(uint16_t idx){
    auto d = (int8_t)Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
    this->_cpu->_special_registers.pc++;
    uint8_t ex = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
    this->_cpu->_special_registers.pc++;
    uint8_t value = Mcycle::m2(this->_cpu, idx + d);
    if ((ex & 0b11000111) == 0b01000110){
        // bit b, (ix + d) // bit b, (iy + d)
        uint8_t bit = ((ex & 0b0011100) >> 3);
        this->_cpu->_registers.FZ_Zero = ((value & (1 << bit)) == 0);
    } else if ((ex & 0b11000111) == 0b10000110){
        // res b, (ix + d) // res b, (iy + d)
        uint8_t bit = ((ex & 0b0011100) >> 3);
        value &= -(value & (1 << bit));
        Mcycle::m3(this->_cpu, idx + d, value);
    } else if ((ex & 0b11000111) == 0b11000110){
        // set b, (ix + d) // set b, (iy + d)
        uint8_t bit = ((ex & 0b0011100) >> 3);
        value |= (value & (1 << bit));
        Mcycle::m3(this->_cpu, idx + d, value);
    } else {
        bool carry_bit;
        switch (ex){
            case 0x06: // rlc (ix + d) // rlc (iy + d)
                carry_bit = ((value & 0x80) > 0);
                value = (value << 1) & 0xff | (value >> 7);
                break;
            case 0x16: // rl (ix + d) // rl (iy + d)
                carry_bit = ((value & 0x80) > 0);
                value = (value << 1) & 0xff | this->_cpu->_registers.carry_by_val();
                break;
            case 0x26: // sla (ix + d) // sla (iy + d)
                carry_bit = ((value & 0x80) > 0);
                value = (value << 1) & 0xff;
                break;
            case 0x0E: // rrc (ix + d) // rrc (iy + d)
                carry_bit = ((value & 1) > 0);
                value = (value >> 1) | ((value & 1) << 7);
                break;
            case 0x1E: // rr (ix + d) // rr (iy + d)
                carry_bit = ((value & 1) > 0);
                value = (value >> 1) | (this->_cpu->_registers.carry_by_val() << 7);
                break;
            case 0x2E: // sra (ix + d) // sra (iy + d)
                carry_bit = ((value & 0x01) > 0);
                value = (value & 0x80) | (value >> 1);
                break;
            case 0x3E: // srl (ix + d) // srl (iy + d)
                carry_bit = ((value & 0x01) > 0);
                value >>= 1;
                break;
            default:
                char error[100];
                sprintf(error, "Invalid target register code: XX CB %02x %02x", d, ex);
                throw std::runtime_error(error);
        }
        this->setFlagsByRotate(value, carry_bit);
        Mcycle::m3(this->_cpu, idx + d, value);
    }
}

void OpCode::executeEd(uint8_t opCode){
    switch (opCode){
        case 0x40: // in r, (c)
        case 0x48:
        case 0x50:
        case 0x58:
        case 0x60:
        case 0x68:
        case 0x78: {
            uint8_t* reg = this->targetRegisterB(0x3A, opCode);
            uint8_t value = Mcycle::in(this->_cpu, *reg);
            this->_cpu->_registers.FN_Subtract = false;
            this->_cpu->_registers.FH_HalfCarry = false;
            this->_cpu->_registers.FZ_Zero = (value == 0);
            this->_cpu->_registers.FS_Sign = ((value & 0x80) > 0);
            this->_cpu->_registers.FPV_ParityOverflow = (OpCode::count1(value) % 2 == 0);
            break;
        }
        case 0x41: // out (c), r
        case 0x49:
        case 0x51:
        case 0x59:
        case 0x61:
        case 0x69:
        case 0x79: {
            uint8_t* reg = this->targetRegisterB(0x3b, opCode);
            this->_cpu->_special_registers.pc++;
            Mcycle::out(this->_cpu, this->_cpu->_registers.c, *reg);
            break;
        }
        case 0x42: // sbc hl, bc
        case 0x52:
        case 0x62:
        case 0x72: {
            uint16_t value;
            switch (opCode){ // NOLINT(hicpp-multiway-paths-covered)
                case 0x42: value = this->_cpu->_registers.bc(); break;
                case 0x52: value = this->_cpu->_registers.de(); break;
                case 0x62: value = this->_cpu->_registers.hl(); break;
                case 0x72: value = this->_cpu->_special_registers.sp; break;
            }
            uint16_t carry = this->_cpu->_registers.carry_by_val();
            this->setFlagsBySbc16(this->_cpu->_registers.hl(), value + carry);
        }
        case 0x43: { // ld (nn), bc
            uint16_t addr = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc + 1) << 8) +
                            Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc += 2;
            Mcycle::m3(this->_cpu, addr + 1, this->_cpu->_registers.b);
            Mcycle::m3(this->_cpu, addr, this->_cpu->_registers.c);
            break;
        }
        case 0x44: { // neg
            uint8_t a = this->_cpu->_registers.a;
            this->_cpu->_registers.a = 0;
            this->setFlagsBySubtract(this->_cpu->_registers.a, a);
            this->_cpu->_registers.a -= a;
            break;
        }
        case 0x45: // retn
            this->_cpu->_special_registers.pc = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp + 1) << 8) + Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp);
            this->_cpu->_special_registers.sp += 2;
            this->_cpu->iff1 = this->_cpu->iff2;
            break;
        case 0x46: // im 0
            this->_cpu->interrupt_mode = 0;
            break;
        case 0x47: // ld i, a
            this->_cpu->_special_registers.i = this->_cpu->_registers.a;
            break;
        case 0x4A: // adc hl, bc
        case 0x5A:
        case 0x6A:
        case 0x7A: {
            uint16_t value;
            switch (opCode){ // NOLINT(hicpp-multiway-paths-covered)
                case 0x4A: value = this->_cpu->_registers.bc(); break;
                case 0x5A: value = this->_cpu->_registers.de(); break;
                case 0x6A: value = this->_cpu->_registers.hl(); break;
                case 0x7A: value = this->_cpu->_special_registers.sp; break;
            }
            uint16_t carry = this->_cpu->_registers.carry_by_val();
            this->setFlagsByAdc16(this->_cpu->_registers.hl(), value + carry);
            this->_cpu->_registers.hl(this->_cpu->_registers.hl() + value + carry);
            break;
        }
        case 0x4B: { // ld bc, (nn)
            uint16_t addr = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc + 1) << 8) +
                            Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc += 2;
            this->_cpu->_registers.b = Mcycle::m2(this->_cpu, addr + 1);
            this->_cpu->_registers.c = Mcycle::m2(this->_cpu, addr);
            break;
        }
        case 0x4D: // reti
            this->_cpu->_special_registers.pc = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp + 1) << 8) + Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp);
            this->_cpu->_special_registers.sp += 2;
            break;
        case 0x4F: // ld r, a
            this->_cpu->_special_registers.r = this->_cpu->_registers.a;
            break;
        case 0x53: { // ld (nn), de
            uint16_t addr = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc + 1) << 8) +
                            Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc += 2;
            Mcycle::m3(this->_cpu, addr + 1, this->_cpu->_registers.d);
            Mcycle::m3(this->_cpu, addr, this->_cpu->_registers.e);
            break;
        }
        case 0x56: // im 1
            this->_cpu->interrupt_mode = 1;
            break;
        case 0x57: // ld a, i
            this->_cpu->_registers.a = this->_cpu->_special_registers.i;
            break;
        case 0x5B: { // ld de, (nn)
            uint16_t addr = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc + 1) << 8) +
                            Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc += 2;
            this->_cpu->_registers.d = Mcycle::m2(this->_cpu, addr + 1);
            this->_cpu->_registers.e = Mcycle::m2(this->_cpu, addr);
            break;
        }
        case 0x5E: // im 2
            this->_cpu->interrupt_mode = 2;
            break;
        case 0x5F: // ld a, r
            this->_cpu->_registers.a = this->_cpu->_special_registers.r;
            break;
        case 0x67: { // rrd
            unsigned char beforeN = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
            unsigned char nH = (beforeN & 0b11110000) >> 4;
            unsigned char nL = beforeN & 0b00001111;
            unsigned char aH = (this->_cpu->_registers.a & 0b11110000) >> 4;
            unsigned char aL = this->_cpu->_registers.a & 0b00001111;
            unsigned char beforeA = this->_cpu->_registers.a;
            unsigned char afterA = (aH << 4) | nL;
            unsigned char afterN = (aL << 4) | nH;
            this->_cpu->_registers.a = afterA;
            Mcycle::m3(this->_cpu, this->_cpu->_registers.hl(), afterN);
            this->_cpu->_registers.FS_Sign = ((this->_cpu->_registers.a & 0x80) > 0);
            this->_cpu->_registers.FZ_Zero = (this->_cpu->_registers.a == 0);
            this->_cpu->_registers.FH_HalfCarry = false;
            this->_cpu->_registers.FPV_ParityOverflow = (OpCode::count1(this->_cpu->_registers.a) % 2 == 0);
            this->_cpu->_registers.FN_Subtract = false;
            break;
        }
        case 0x6F: { // rld
            uint8_t beforeN = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
            uint8_t nH = (beforeN & 0b11110000) >> 4;
            uint8_t nL = beforeN & 0b00001111;
            uint8_t aH = (this->_cpu->_registers.a & 0b11110000) >> 4;
            uint8_t aL = this->_cpu->_registers.a & 0b00001111;
            uint8_t beforeA = this->_cpu->_registers.a;
            uint8_t afterA = (aH << 4) | nH;
            uint8_t afterN = (nL << 4) | aL;
            this->_cpu->_registers.a = afterA;
            Mcycle::m3(this->_cpu, this->_cpu->_registers.hl(), afterN);
            this->_cpu->_registers.FS_Sign = ((this->_cpu->_registers.a & 0x80) > 0);
            this->_cpu->_registers.FZ_Zero = (this->_cpu->_registers.a == 0);
            this->_cpu->_registers.FH_HalfCarry = false;
            this->_cpu->_registers.FPV_ParityOverflow = (OpCode::count1(this->_cpu->_registers.a) % 2 == 0);
            this->_cpu->_registers.FN_Subtract = false;
            break;
        }
        case 0x73: { // ld (nn), sp
            uint16_t addr = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc + 1) << 8) +
                            Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc += 2;
            Mcycle::m3(this->_cpu, addr + 1, this->_cpu->_special_registers.sp >> 8);
            Mcycle::m3(this->_cpu, addr, this->_cpu->_special_registers.sp & 0xff);
            break;
        }
        case 0x7B: { // ld sp, (nn)
            uint16_t addr = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc + 1) << 8) +
                            Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc += 2;
            this->_cpu->_special_registers.sp =
                    (Mcycle::m2(this->_cpu, addr + 1) << 8) +
                    Mcycle::m2(this->_cpu, addr);
            break;
        }
        case 0xA0: { // ldi
            uint8_t data = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
            Mcycle::m3(this->_cpu, this->_cpu->_registers.de(), data);
            this->_cpu->_registers.hl(this->_cpu->_registers.hl() + 1);
            this->_cpu->_registers.de(this->_cpu->_registers.de() + 1);
            this->_cpu->_registers.bc(this->_cpu->_registers.bc() - 1);
            this->_cpu->_registers.FPV_ParityOverflow = (this->_cpu->_registers.bc() != 0);
            this->_cpu->_registers.FN_Subtract = false;
            this->_cpu->_registers.FH_HalfCarry = false;
            break;
        }
        case 0xA1: { // cpi
            uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
            this->setFlagsBySubtract(this->_cpu->_registers.a, value, false);
            this->_cpu->_registers.hl(this->_cpu->_registers.hl() + 1);
            this->_cpu->_registers.bc(this->_cpu->_registers.bc() - 1);
            this->_cpu->_registers.FPV_ParityOverflow = (this->_cpu->_registers.bc() != 0);
            break;
        }
        case 0xA2: { // ini
            uint8_t value = Mcycle::in(this->_cpu, this->_cpu->_registers.c);
            Mcycle::m3(this->_cpu, this->_cpu->_registers.hl(), value);
            this->_cpu->_registers.b--;
            this->_cpu->_registers.hl(this->_cpu->_registers.hl() + 1);
            this->_cpu->_registers.FN_Subtract = true;
            this->_cpu->_registers.FZ_Zero = (this->_cpu->_registers.b == 0);
            break;
        }
        case 0xA3: { // outi
            uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
            Mcycle::out(this->_cpu, this->_cpu->_registers.c, value);
            this->_cpu->_registers.b--;
            this->_cpu->_registers.hl(this->_cpu->_registers.hl() + 1);
            this->_cpu->_registers.FN_Subtract = true;
            this->_cpu->_registers.FZ_Zero = (this->_cpu->_registers.b == 0);
            break;
        }
        case 0xA8: { // ldd
            uint8_t data = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
            Mcycle::m3(this->_cpu, this->_cpu->_registers.de(), data);
            this->_cpu->_registers.hl(this->_cpu->_registers.hl() - 1);
            this->_cpu->_registers.de(this->_cpu->_registers.de() - 1);
            this->_cpu->_registers.bc(this->_cpu->_registers.bc() - 1);
            this->_cpu->_registers.FPV_ParityOverflow = (this->_cpu->_registers.bc() != 0);
            this->_cpu->_registers.FN_Subtract = false;
            this->_cpu->_registers.FH_HalfCarry = false;
            break;
        }
        case 0xA9: { // cpd
            uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
            this->setFlagsBySubtract(this->_cpu->_registers.a, value, false);
            this->_cpu->_registers.hl(this->_cpu->_registers.hl() - 1);
            this->_cpu->_registers.bc(this->_cpu->_registers.bc() - 1);
            this->_cpu->_registers.FPV_ParityOverflow = (this->_cpu->_registers.bc() != 0);
            break;
        }
        case 0xAA: { // ind
            uint8_t value = Mcycle::in(this->_cpu, this->_cpu->_registers.c);
            Mcycle::m3(this->_cpu, this->_cpu->_registers.hl(), value);
            this->_cpu->_registers.b--;
            this->_cpu->_registers.hl(this->_cpu->_registers.hl() - 1);
            this->_cpu->_registers.FN_Subtract = true;
            this->_cpu->_registers.FZ_Zero = (this->_cpu->_registers.b == 0);
            break;
        }
        case 0xAB: { // outd
            uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
            Mcycle::out(this->_cpu, this->_cpu->_registers.c, value);
            this->_cpu->_registers.b--;
            this->_cpu->_registers.hl(this->_cpu->_registers.hl() - 1);
            this->_cpu->_registers.FN_Subtract = true;
            this->_cpu->_registers.FZ_Zero = (this->_cpu->_registers.b == 0);
            break;
        }
        case 0xB0: { // ldir
            do {
                uint8_t data = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
                Mcycle::m3(this->_cpu, this->_cpu->_registers.de(), data);
                this->_cpu->_registers.hl(this->_cpu->_registers.hl() + 1);
                this->_cpu->_registers.de(this->_cpu->_registers.de() + 1);
                this->_cpu->_registers.bc(this->_cpu->_registers.bc() - 1);
            } while(this->_cpu->_registers.bc() > 0);
            this->_cpu->_registers.FPV_ParityOverflow = false;
            this->_cpu->_registers.FN_Subtract = false;
            this->_cpu->_registers.FH_HalfCarry = false;
            break;
        }
        case 0xB1: { // cpir
            do {
                uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
                this->setFlagsBySubtract(this->_cpu->_registers.a, value, false);
                this->_cpu->_registers.hl(this->_cpu->_registers.hl() + 1);
                this->_cpu->_registers.bc(this->_cpu->_registers.bc() - 1);
                this->_cpu->_registers.FPV_ParityOverflow = (this->_cpu->_registers.bc() != 0);
            } while(this->_cpu->_registers.bc() > 0);
            break;
        }
        case 0xB2: { // inir
            do {
                uint8_t value = Mcycle::in(this->_cpu, this->_cpu->_registers.c);
                Mcycle::m3(this->_cpu, this->_cpu->_registers.hl(), value);
                this->_cpu->_registers.b--;
                this->_cpu->_registers.hl(this->_cpu->_registers.hl() + 1);
            } while(this->_cpu->_registers.b > 0);
            this->_cpu->_registers.FN_Subtract = true;
            this->_cpu->_registers.FZ_Zero = true;
            break;
        }
        case 0xB3: { // otir
            do {
                uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
                Mcycle::out(this->_cpu, this->_cpu->_registers.c, value);
                this->_cpu->_registers.b--;
                this->_cpu->_registers.hl(this->_cpu->_registers.hl() + 1);
            } while(this->_cpu->_registers.b > 0);
            this->_cpu->_registers.FN_Subtract = true;
            this->_cpu->_registers.FZ_Zero = true;
            break;
        }
        case 0xB8: { // lddr
            do {
                uint8_t data = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
                Mcycle::m3(this->_cpu, this->_cpu->_registers.de(), data);
                this->_cpu->_registers.hl(this->_cpu->_registers.hl() - 1);
                this->_cpu->_registers.de(this->_cpu->_registers.de() - 1);
                this->_cpu->_registers.bc(this->_cpu->_registers.bc() - 1);
            } while(this->_cpu->_registers.bc() > 0);
            this->_cpu->_registers.FPV_ParityOverflow = false;
            this->_cpu->_registers.FN_Subtract = false;
            this->_cpu->_registers.FH_HalfCarry = false;
            break;
        }
        case 0xB9: { // cpdr
            do {
                uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
                this->setFlagsBySubtract(this->_cpu->_registers.a, value, false);
                this->_cpu->_registers.hl(this->_cpu->_registers.hl() - 1);
                this->_cpu->_registers.bc(this->_cpu->_registers.bc() - 1);
                this->_cpu->_registers.FPV_ParityOverflow = (this->_cpu->_registers.bc() != 0);
            } while(this->_cpu->_registers.bc() > 0);
            break;
        }
        case 0xBA: { // indr
            do {
                uint8_t value = Mcycle::in(this->_cpu, this->_cpu->_registers.c);
                Mcycle::m3(this->_cpu, this->_cpu->_registers.hl(), value);
                this->_cpu->_registers.b--;
                this->_cpu->_registers.hl(this->_cpu->_registers.hl() - 1);
            } while(this->_cpu->_registers.b > 0);
            this->_cpu->_registers.FN_Subtract = true;
            this->_cpu->_registers.FZ_Zero = true;
            break;
        }
        case 0xBB: { // otdr
            do {
                uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_registers.hl());
                Mcycle::out(this->_cpu, this->_cpu->_registers.c, value);
                this->_cpu->_registers.b--;
                this->_cpu->_registers.hl(this->_cpu->_registers.hl() - 1);
            } while(this->_cpu->_registers.b > 0);
            this->_cpu->_registers.FN_Subtract = true;
            this->_cpu->_registers.FZ_Zero = true;
            break;
        }
        default: {
            char error[100];
            sprintf(error, "Invalid op code: ED %02x", opCode);
            throw std::runtime_error(error);
        }
    }
}

void OpCode::executeFd(uint8_t opCode){
    switch (opCode){
        case 0x09: // add iy, rr
        case 0x19:
        case 0x29:
        case 0x39: {
            uint16_t value;
            switch (opCode){ // NOLINT(hicpp-multiway-paths-covered)
                case 0x09: value = this->_cpu->_registers.bc(); break;
                case 0x19: value = this->_cpu->_registers.de(); break;
                case 0x29: value = this->_cpu->_special_registers.iy; break;
                case 0x39: value = this->_cpu->_special_registers.sp; break;
            }
            this->setFlagsByAdd16(this->_cpu->_special_registers.iy, value);
            this->_cpu->_special_registers.iy += value;
            break;
        }
        case 0x21: { // ld ix, nn
            uint16_t data = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc + 1) << 8) +
                            Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc += 2;
            this->_cpu->_special_registers.iy = data;
            break;
        }
        case 0x22: { // ld (nn), iy
            uint16_t addr = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc + 1) << 8) +
                            Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc += 2;
            Mcycle::m3(this->_cpu, addr + 1, this->_cpu->_special_registers.iy >> 8);
            Mcycle::m3(this->_cpu, addr, this->_cpu->_special_registers.iy & 0xff);
            break;
        }
        case 0x23: // inc iy
            this->_cpu->_special_registers.iy++;
            break;
        case 0x2A: { // ld iy, (nn)
            uint16_t addr = (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc + 1) << 8) +
                            Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc += 2;
            this->_cpu->_special_registers.iy =
                    (Mcycle::m2(this->_cpu, addr + 1) << 8) +
                    Mcycle::m2(this->_cpu, addr);
            break;
        }
        case 0x2B: // dec iy
            this->_cpu->_special_registers.iy--;
            break;
        case 0x34: { // inc (iy + d)
            auto d = (int8_t)Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            uint16_t addr = this->_cpu->_special_registers.iy + d;
            uint16_t data = Mcycle::m2(this->_cpu, addr);
            this->setFlagsByIncrement(data);
            Mcycle::m3(this->_cpu, addr, data + 1);
            break;
        }
        case 0x35: { // dec (iy + d)
            auto d = (int8_t)Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            uint16_t addr = this->_cpu->_special_registers.iy + d;
            uint16_t data = Mcycle::m2(this->_cpu, addr);
            this->setFlagsByDecrement(data);
            Mcycle::m3(this->_cpu, addr, data - 1);
            break;
        }
        case 0x36: { // ld (iy + d), n
            auto d = (int8_t)Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            uint8_t data = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            Mcycle::m3(this->_cpu, this->_cpu->_special_registers.iy + d, data);
            break;
        }
        case 0x46: // ld r, (iy + d)
        case 0x4E:
        case 0x56:
        case 0x5E:
        case 0x66:
        case 0x6E:
        case 0x7E: {
            auto d = (int8_t)Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            uint8_t* targetRegister = this->targetRegisterB(0x40, opCode);
            *targetRegister = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.iy + d);
            break;
        }
        case 0x70: // ld (iy + d), r
        case 0x71:
        case 0x72:
        case 0x73:
        case 0x74:
        case 0x75:
        case 0x77: {
            auto d = (int8_t)Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            uint8_t* targetRegister = this->targetRegisterA(0x70, opCode);
            Mcycle::m3(this->_cpu, this->_cpu->_special_registers.iy + d, *targetRegister);
            break;
        }
        case 0x86: { // add a, (iy + d)
            auto d = (int8_t)Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.iy + d);
            this->setFlagsByAddition(this->_cpu->_registers.a, value);
            this->_cpu->_registers.a += value;
            break;
        }
        case 0x8E: { // adc a, (iy + d)
            auto d = (int8_t)Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.iy + d);
            uint8_t carry = this->_cpu->_registers.carry_by_val();
            this->setFlagsByAddition(this->_cpu->_registers.a, value + carry);
            this->_cpu->_registers.a += value + carry;
            break;
        }
        case 0x96: { // sub (iy + d)
            auto d = (int8_t)Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.iy + d);
            this->setFlagsBySubtract(this->_cpu->_registers.a, value);
            this->_cpu->_registers.a -= value;
            break;
        }
        case 0x9E: { // sbc a, (iy + d)
            auto d = (int8_t)Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.iy + d);
            this->_cpu->_registers.FH_HalfCarry = ((this->_cpu->_registers.a & 0xf) - (value & 0xf) - this->_cpu->_registers.carry_by_val() < 0);
            this->_cpu->_registers.FC_Carry = (this->_cpu->_registers.a - (value + this->_cpu->_registers.carry_by_val()));
            this->_cpu->_registers.a -= (value + this->_cpu->_registers.carry_by_val());
            this->_cpu->_registers.FZ_Zero = (this->_cpu->_registers.a == 0);
            this->_cpu->_registers.FN_Subtract = true;
            break;
        }
        case 0xA6: { // and (iy + d)
            auto d = (int8_t)Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            this->_cpu->_registers.a &= Mcycle::m2(this->_cpu, this->_cpu->_special_registers.iy + d);
            this->setFlagsByLogical(true);
            break;
        }
        case 0xAE: { // xor (iy + d)
            auto d = (int8_t)Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            this->_cpu->_registers.a ^= Mcycle::m2(this->_cpu, this->_cpu->_special_registers.iy + d);
            this->setFlagsByLogical(false);
            break;
        }
        case 0xB6: { // or (iy + d)
            auto d = (int8_t)Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            this->_cpu->_registers.a |= Mcycle::m2(this->_cpu, this->_cpu->_special_registers.iy + d);
            this->setFlagsByLogical(false);
            break;
        }
        case 0xBE: { // cp (iy + d)
            auto d = (int8_t)Mcycle::m2(this->_cpu, this->_cpu->_special_registers.pc);
            this->_cpu->_special_registers.pc++;
            uint8_t value = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.iy + d);
            this->setFlagsBySubtract(this->_cpu->_registers.a, value);
            break;
        }
        case 0xCB: {
            this->executeXxCb(this->_cpu->_special_registers.iy);
            break;
        }
        case 0xE1: // pop iy
            this->_cpu->_special_registers.iy =
                    (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp + 1) << 8) +
                    Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp);
            this->_cpu->_special_registers.sp += 2;
            break;
        case 0xE3: { // ex (sp), iy
            uint8_t temp_iy = this->_cpu->_special_registers.iy;
            this->_cpu->_special_registers.iy = Mcycle::m2(this->_cpu, this->_cpu->_special_registers.sp);
            Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp, temp_iy & 0xff);
            Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp + 1, temp_iy >> 8);
            break;
        }
        case 0xE5: // push iy
            this->_cpu->_special_registers.sp--;
            Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp, this->_cpu->_special_registers.iy >> 8);
            this->_cpu->_special_registers.sp--;
            Mcycle::m3(this->_cpu, this->_cpu->_special_registers.sp, this->_cpu->_special_registers.iy & 0xff);
            break;
        case 0xE9: // jp (iy)
            this->_cpu->_special_registers.pc =
                    (Mcycle::m2(this->_cpu, this->_cpu->_special_registers.iy + 1) << 8) +
                    Mcycle::m2(this->_cpu, this->_cpu->_special_registers.iy);
            break;
        case 0xF9: // ld sp, iy
            this->_cpu->_special_registers.sp = this->_cpu->_special_registers.iy;
            break;
        default: {
            char error[100];
            sprintf(error, "Invalid op code: FD %02x", opCode);
            throw std::runtime_error(error);
        }
    }
}

uint8_t* OpCode::targetRegisterA(uint8_t offset, uint8_t opCode) const{
    switch (opCode - offset){
        case 0: return &(this->_cpu->_registers.b);
        case 1: return &(this->_cpu->_registers.c);
        case 2: return &(this->_cpu->_registers.d);
        case 3: return &(this->_cpu->_registers.e);
        case 4: return &(this->_cpu->_registers.h);
        case 5: return &(this->_cpu->_registers.l);
        case 7: return &(this->_cpu->_registers.a);
        default:
            char error[100];
            sprintf(error, "Invalid target register code: %02x", opCode);
            throw std::runtime_error(error);
    }
}

uint8_t* OpCode::targetRegisterB(int8_t offset, uint8_t opCode) const {
    switch(opCode - offset){
        case 0x06: return &(this->_cpu->_registers.b);
        case 0x0E: return &(this->_cpu->_registers.c);
        case 0x16: return &(this->_cpu->_registers.d);
        case 0x1E: return &(this->_cpu->_registers.e);
        case 0x26: return &(this->_cpu->_registers.h);
        case 0x2E: return &(this->_cpu->_registers.l);
        case 0x3E: return &(this->_cpu->_registers.a);
        default:
            char error[100];
            sprintf(error, "Invalid target register code: %02x", opCode);
            throw std::runtime_error(error);
    }
}

uint8_t OpCode::count1(uint8_t data){
    uint8_t count = 0;
    for (int i = 0; i < 8; i++){
        if ((data & (1 << i)) > 0){
            count++;
        }
    }
    return count;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantParameter"
void OpCode::setFlagsByAddition(uint8_t before, uint8_t addition, bool set_carry) const {
    int result = ((int)before) + addition;
    int carry = before ^ addition ^ result;
    auto final_result = (uint8_t)result;
    this->_cpu->_registers.FN_Subtract = false;
    this->_cpu->_registers.FZ_Zero = (final_result == 0);
    this->_cpu->_registers.FS_Sign = ((final_result & 0x80) > 0);
    this->_cpu->_registers.FH_HalfCarry = ((carry & 0x10) != 0);
    this->_cpu->_registers.FPV_ParityOverflow = ((((carry << 1) ^ carry) & 0x100) != 0);
    if (set_carry){
        this->_cpu->_registers.FC_Carry = ((carry & 0x100) != 0);
    }
}
#pragma clang diagnostic pop

void OpCode::setFlagsBySubtract(uint8_t before, uint8_t subtract, bool set_carry) const {
    int result = ((int)before) - subtract;
    int carry = before ^ subtract ^ result;
    auto final_result = (uint8_t)result;
    this->_cpu->_registers.FN_Subtract = true;
    this->_cpu->_registers.FZ_Zero = (final_result == 0);
    this->_cpu->_registers.FS_Sign = ((final_result & 0x80) > 0);
    this->_cpu->_registers.FH_HalfCarry = ((carry & 0x10) != 0);
    this->_cpu->_registers.FPV_ParityOverflow = ((((carry << 1) ^ carry) & 0x100) != 0);
    if (set_carry){
        this->_cpu->_registers.FC_Carry = ((carry & 0x100) != 0);
    }
}

void OpCode::setFlagsByIncrement(uint8_t before) const{
    uint8_t final_result = before + 1;
    this->_cpu->_registers.FN_Subtract = false;
    this->_cpu->_registers.FZ_Zero = (final_result == 0);
    this->_cpu->_registers.FS_Sign = ((final_result & 0x80) > 0);
    this->_cpu->_registers.FH_HalfCarry = ((final_result & 0x0f) == 0);
    this->_cpu->_registers.FPV_ParityOverflow = (final_result == 0x80);
}

void OpCode::setFlagsByDecrement(uint8_t before) const{
    unsigned char final_result = before - 1;
    this->_cpu->_registers.FN_Subtract = true;
    this->_cpu->_registers.FZ_Zero = (final_result == 0);
    this->_cpu->_registers.FS_Sign = ((final_result & 0x80) > 0);
    this->_cpu->_registers.FH_HalfCarry = ((final_result & 0x0f) == 0x0f);
    this->_cpu->_registers.FPV_ParityOverflow = (final_result == 0x7f);
}

void OpCode::setFlagsBySbc16(uint16_t before, uint16_t subtract) const{
    int result = before - subtract;
    int carry = before ^ subtract ^ result;
    auto final_result = (uint16_t)result;
    this->_cpu->_registers.FN_Subtract = true;
    this->_cpu->_registers.FC_Carry = ((carry & 0x10000) != 0);
    this->_cpu->_registers.FH_HalfCarry = ((carry & 0x1000) != 0);
    this->_cpu->_registers.FS_Sign = ((final_result & 0x8000) > 0);
    this->_cpu->_registers.FZ_Zero = (final_result == 0);
    this->_cpu->_registers.FPV_ParityOverflow = ((((carry << 1) ^ carry) & 0x10000) != 0);
}

void OpCode::setFlagsByLogical(bool h){
    this->_cpu->_registers.FS_Sign = ((this->_cpu->_registers.a & 0x80) > 0);
    this->_cpu->_registers.FZ_Zero = (this->_cpu->_registers.a == 0);
    this->_cpu->_registers.FH_HalfCarry = (h);
    this->_cpu->_registers.FPV_ParityOverflow = (this->count1(this->_cpu->_registers.a) % 2 == 0);
    this->_cpu->_registers.FN_Subtract = false;
    this->_cpu->_registers.FC_Carry = false;
}

void OpCode::setFlagsByAdd16(uint16_t before, uint16_t addition) const{
    int result = before + addition;
    int carry = before ^ addition ^ result;
    this->_cpu->_registers.FN_Subtract = false;
    this->_cpu->_registers.FC_Carry = ((carry & 0x10000) != 0);
    this->_cpu->_registers.FH_HalfCarry = ((carry & 0x1000) != 0);
}

void OpCode::setFlagsByAdc16(uint16_t before, uint16_t addition) const{
    int result = before + addition;
    int carry = before ^ addition ^ result;
    auto final_result = (uint16_t)result;
    this->_cpu->_registers.FN_Subtract = false;
    this->_cpu->_registers.FC_Carry = ((carry & 0x10000) != 0);
    this->_cpu->_registers.FH_HalfCarry = ((carry & 0x1000) != 0);
    this->_cpu->_registers.FS_Sign = ((final_result & 0x8000) > 0);
    this->_cpu->_registers.FZ_Zero = (0 == final_result);
    this->_cpu->_registers.FPV_ParityOverflow = ((((carry << 1) ^ carry) & 0x10000) != 0);
}

void OpCode::setFlagsByRotate(uint8_t n, bool carry) const {
    this->_cpu->_registers.FC_Carry = carry;
    this->_cpu->_registers.FH_HalfCarry = false;
    this->_cpu->_registers.FN_Subtract = false;
    this->_cpu->_registers.FS_Sign = ((n & 0x80) > 0);
    this->_cpu->_registers.FZ_Zero = (0 == n);
    this->_cpu->_registers.FPV_ParityOverflow = (this->count1(n) % 2 == 0);
}

