#include "gtest/gtest.h"
#include "../src/cpu.hpp"

class OpCodeTest : public ::testing::Test {

protected:
    void SetUp() override
    {
        cpu = new Cpu(0);
        auto *opCode = new OpCode(cpu);
        cpu->_opCode = *opCode;

        std::array<uint8_t, 8844> memory{
                0x00, 0x00, 0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,
        };
        cpu->virtual_memory = memory;
        cpu->enable_virtual_memory = true;

#undef Z80EMU_ENABLE_LOG
    }

    Cpu *cpu;
};

// ld r, r'
TEST_F(OpCodeTest, Test_LD_R_R){
    // Base: 0b01000000
    //           -R1-R2 : LD R1, R2
    // 0b000: B, 0b001: C, 0b010: D, 0b011: E, 0b100: H, 0b101: L, 0b110: XX, 0b111: A

    // LD A, R
    cpu->_registers.a = 0x00;
    cpu->_registers.b = 0x11;
    cpu->_registers.c = 0x22;
    cpu->_registers.d = 0x33;
    cpu->_registers.e = 0x44;
    cpu->_registers.h = 0x55;
    cpu->_registers.l = 0x66;
    // LD A, B
    cpu->_opCode.execute(0b01111000);
    EXPECT_EQ(cpu->_registers.a, 0x11);
    // LD A, C
    cpu->_opCode.execute(0b01111001);
    EXPECT_EQ(cpu->_registers.a, 0x22);
    // LD A, D
    cpu->_opCode.execute(0b01111010);
    EXPECT_EQ(cpu->_registers.a, 0x33);
    // LD A, E
    cpu->_opCode.execute(0b01111011);
    EXPECT_EQ(cpu->_registers.a, 0x44);
    // LD A, H
    cpu->_opCode.execute(0b01111100);
    EXPECT_EQ(cpu->_registers.a, 0x55);
    // LD A, L
    cpu->_opCode.execute(0b01111101);
    EXPECT_EQ(cpu->_registers.a, 0x66);
    // LD A, A
    cpu->_opCode.execute(0b01111111);
    EXPECT_EQ(cpu->_registers.a, 0x66);

    // LD B, R
    cpu->_registers.a = 0x00;
    cpu->_registers.b = 0x11;
    cpu->_registers.c = 0x22;
    cpu->_registers.d = 0x33;
    cpu->_registers.e = 0x44;
    cpu->_registers.h = 0x55;
    cpu->_registers.l = 0x66;
    // LD B, B
    cpu->_opCode.execute(0b01000000);
    EXPECT_EQ(cpu->_registers.b, 0x11);
    // LD B, C
    cpu->_opCode.execute(0b01000001);
    EXPECT_EQ(cpu->_registers.b, 0x22);
    // LD B, D
    cpu->_opCode.execute(0b01000010);
    EXPECT_EQ(cpu->_registers.b, 0x33);
    // LD B, E
    cpu->_opCode.execute(0b01000011);
    EXPECT_EQ(cpu->_registers.b, 0x44);
    // LD B, H
    cpu->_opCode.execute(0b01000100);
    EXPECT_EQ(cpu->_registers.b, 0x55);
    // LD B, L
    cpu->_opCode.execute(0b01000101);
    EXPECT_EQ(cpu->_registers.b, 0x66);
    // LD B, A
    cpu->_opCode.execute(0b01000111);
    EXPECT_EQ(cpu->_registers.b, 0x00);
}

// 0x01: ld bc, nn
TEST_F(OpCodeTest, Test_LD_BC_NN) {
    std::array<uint8_t, 8844> memory{
            0x01, 0x11, 0x22, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,
    };
    cpu->virtual_memory = memory;

    cpu->_registers.bc(0x0000);
    cpu->_special_registers.pc = 0x0001;
    cpu->_opCode.execute(0x01);
    EXPECT_EQ(cpu->_registers.bc(), 0x2211);
}

// 0x02: ld (bc), a
TEST_F(OpCodeTest, Test_LD_bc_A) {
    EXPECT_EQ(cpu->virtual_memory[0x0000], 0x00);
    cpu->_registers.bc(0x0000);
    cpu->_registers.a = 0xff;
    cpu->_opCode.execute(0x02);
    EXPECT_EQ(cpu->virtual_memory[0x0000], 0xff);
}

// 0x03: inc bc
// 0x13: inc de
// 0x23: inc hl
// 0x33: inc sp
TEST_F(OpCodeTest, Test_INC_RR) {
    // 0x03: inc bc
    cpu->_registers.FZ_Zero = false;
    cpu->_registers.FC_Carry = false;

    cpu->_registers.bc(0xffff);
    cpu->_opCode.execute(0x03);
    EXPECT_EQ(cpu->_registers.bc(), 0x0000);

    EXPECT_EQ(cpu->_registers.FZ_Zero, false);
    EXPECT_EQ(cpu->_registers.FC_Carry, false);

    // 0x13: inc de
    cpu->_registers.FZ_Zero = false;
    cpu->_registers.FC_Carry = false;

    cpu->_registers.de(0xffff);
    cpu->_opCode.execute(0x13);
    EXPECT_EQ(cpu->_registers.de(), 0x0000);

    EXPECT_EQ(cpu->_registers.FZ_Zero, false);
    EXPECT_EQ(cpu->_registers.FC_Carry, false);

    // 0x23: inc hl
    cpu->_registers.FZ_Zero = false;
    cpu->_registers.FC_Carry = false;

    cpu->_registers.hl(0xffff);
    cpu->_opCode.execute(0x23);
    EXPECT_EQ(cpu->_registers.hl(), 0x0000);

    EXPECT_EQ(cpu->_registers.FZ_Zero, false);
    EXPECT_EQ(cpu->_registers.FC_Carry, false);

    // 0x33: inc sp
    cpu->_registers.FZ_Zero = false;
    cpu->_registers.FC_Carry = false;

    cpu->_special_registers.sp = 0xffff;
    cpu->_opCode.execute(0x33);
    EXPECT_EQ(cpu->_special_registers.sp, 0x0000);

    EXPECT_EQ(cpu->_registers.FZ_Zero, false);
    EXPECT_EQ(cpu->_registers.FC_Carry, false);
}

// 0x04: inc b
// 0x0C: inc c
// 0x14: inc d
// 0x1C: inc e
// 0x24: inc h
// 0x2C: inc l
// 0x3C: inc a
TEST_F(OpCodeTest, Test_INC_R) {
    // 0x04: inc b
    cpu->_registers.FS_Sign = true;
    cpu->_registers.FZ_Zero = true;
    cpu->_registers.FH_HalfCarry = true;
    cpu->_registers.FPV_ParityOverflow = true;
    cpu->_registers.FN_Subtract = true;
    cpu->_registers.FC_Carry = false;

    cpu->_registers.b = 0x10;
    cpu->_opCode.execute(0x04);
    EXPECT_EQ(cpu->_registers.b, 0x11);

    EXPECT_EQ(cpu->_registers.FS_Sign, false);
    EXPECT_EQ(cpu->_registers.FZ_Zero, false);
    EXPECT_EQ(cpu->_registers.FH_HalfCarry, false);
    EXPECT_EQ(cpu->_registers.FPV_ParityOverflow, false);
    EXPECT_EQ(cpu->_registers.FN_Subtract, false);
    EXPECT_EQ(cpu->_registers.FC_Carry, false);

    cpu->_registers.FS_Sign = true;
    cpu->_registers.FZ_Zero = false;
    cpu->_registers.FH_HalfCarry = false;
    cpu->_registers.FPV_ParityOverflow = true;
    cpu->_registers.FN_Subtract = true;
    cpu->_registers.FC_Carry = false;

    cpu->_registers.b = 0xff;
    cpu->_opCode.execute(0x04);
    EXPECT_EQ(cpu->_registers.b, 0x00);

    EXPECT_EQ(cpu->_registers.FS_Sign, false);
    EXPECT_EQ(cpu->_registers.FZ_Zero, true);
    EXPECT_EQ(cpu->_registers.FH_HalfCarry, true);
    EXPECT_EQ(cpu->_registers.FPV_ParityOverflow, false);
    EXPECT_EQ(cpu->_registers.FN_Subtract, false);
    EXPECT_EQ(cpu->_registers.FC_Carry, false);

    cpu->_registers.FPV_ParityOverflow = false;
    cpu->_registers.b = 0x7f;
    cpu->_opCode.execute(0x04);
    EXPECT_EQ(cpu->_registers.b, 0x80);
    EXPECT_EQ(cpu->_registers.FPV_ParityOverflow, true);

    // 0x0C: inc c
    cpu->_registers.c = 0xff;
    cpu->_opCode.execute(0x0c);
    EXPECT_EQ(cpu->_registers.c, 0x00);

    // 0x14: inc d
    cpu->_registers.d = 0xff;
    cpu->_opCode.execute(0x14);
    EXPECT_EQ(cpu->_registers.d, 0x00);

    // 0x1C: inc e
    cpu->_registers.e = 0xff;
    cpu->_opCode.execute(0x1c);
    EXPECT_EQ(cpu->_registers.e, 0x00);

    // 0x24: inc h
    cpu->_registers.h = 0xff;
    cpu->_opCode.execute(0x24);
    EXPECT_EQ(cpu->_registers.h, 0x00);

    // 0x2C: inc l
    cpu->_registers.l = 0xff;
    cpu->_opCode.execute(0x2c);
    EXPECT_EQ(cpu->_registers.l, 0x00);

    // 0x3C: inc a
    cpu->_registers.a = 0xff;
    cpu->_opCode.execute(0x3c);
    EXPECT_EQ(cpu->_registers.a, 0x00);
}

// 0x05: dec b
// 0x0D: dec c
// 0x15: dec d
// 0x1D: dec e
// 0x25: dec h
// 0x2D: dec l
// 0x3D: dec a
TEST_F(OpCodeTest, Test_DEC_R) {
    // 0x05: dec b
    cpu->_registers.FS_Sign = true;
    cpu->_registers.FZ_Zero = true;
    cpu->_registers.FH_HalfCarry = true;
    cpu->_registers.FPV_ParityOverflow = true;
    cpu->_registers.FN_Subtract = false;
    cpu->_registers.FC_Carry = false;

    cpu->_registers.b = 0x11;
    cpu->_opCode.execute(0x05);
    EXPECT_EQ(cpu->_registers.b, 0x10);

    EXPECT_EQ(cpu->_registers.FS_Sign, false);
    EXPECT_EQ(cpu->_registers.FZ_Zero, false);
    EXPECT_EQ(cpu->_registers.FH_HalfCarry, false);
    EXPECT_EQ(cpu->_registers.FPV_ParityOverflow, false);
    EXPECT_EQ(cpu->_registers.FN_Subtract, true);
    EXPECT_EQ(cpu->_registers.FC_Carry, false);

    cpu->_registers.FS_Sign = false;
    cpu->_registers.FZ_Zero = true;
    cpu->_registers.FH_HalfCarry = false;
    cpu->_registers.FPV_ParityOverflow = true;
    cpu->_registers.FN_Subtract = false;
    cpu->_registers.FC_Carry = false;

    cpu->_registers.b = 0x00;
    cpu->_opCode.execute(0x05);
    EXPECT_EQ(cpu->_registers.b, 0xff);

    EXPECT_EQ(cpu->_registers.FS_Sign, true);
    EXPECT_EQ(cpu->_registers.FZ_Zero, false);
    EXPECT_EQ(cpu->_registers.FH_HalfCarry, true);
    EXPECT_EQ(cpu->_registers.FPV_ParityOverflow, false);
    EXPECT_EQ(cpu->_registers.FN_Subtract, true);
    EXPECT_EQ(cpu->_registers.FC_Carry, false);

    cpu->_registers.FPV_ParityOverflow = false;
    cpu->_registers.b = 0x80;
    cpu->_opCode.execute(0x05);
    EXPECT_EQ(cpu->_registers.b, 0x7f);
    EXPECT_EQ(cpu->_registers.FPV_ParityOverflow, true);

    // 0x0D: dec c
    cpu->_registers.c = 0x11;
    cpu->_opCode.execute(0x0d);
    EXPECT_EQ(cpu->_registers.c, 0x10);
    // 0x15: dec d
    cpu->_registers.d = 0x11;
    cpu->_opCode.execute(0x15);
    EXPECT_EQ(cpu->_registers.d, 0x10);
    // 0x1D: dec e
    cpu->_registers.e = 0x11;
    cpu->_opCode.execute(0x1d);
    EXPECT_EQ(cpu->_registers.e, 0x10);
    // 0x25: dec h
    cpu->_registers.h = 0x11;
    cpu->_opCode.execute(0x25);
    EXPECT_EQ(cpu->_registers.h, 0x10);
    // 0x2D: dec l
    cpu->_registers.l = 0x11;
    cpu->_opCode.execute(0x2d);
    EXPECT_EQ(cpu->_registers.l, 0x10);
    // 0x3D: dec a
    cpu->_registers.a = 0x11;
    cpu->_opCode.execute(0x3d);
    EXPECT_EQ(cpu->_registers.a, 0x10);
}



/*
TEST_F(OpCodeTest, Test_) {

}
protected:
    virtual void SetUp()
    {
        gregDate = new GregorianDate();
        julDate = new JulianDate();
    }

    virtual void TearDown() {
        delete gregDate;
        delete julDate;
    }

    GregorianDate * gregDate;
    JulianDate * julDate;
};

 TEST_F(DateConverterFixture, FirstDayEver){ // 1/1/1 -> 1/3/1

    gregDate->SetMonth(1);
    gregDate->SetDay(1);
    gregDate->SetYear(1);

    int absGregDate = gregDate->getAbsoluteDate();
    julDate->CalcJulianDate(absGregDate);

    EXPECT_EQ(julDate->getMonth(),1);
    EXPECT_EQ(julDate->getDay(),3);
    EXPECT_EQ(julDate->getYear(),1);

}

TEST_F(DateConverterFixture, PlusOneDiff){ // 3/1/100 -> 3/2/100

    gregDate->SetMonth(3);
    gregDate->SetDay(1);
    gregDate->SetYear(100);

    int absGregDate = gregDate->getAbsoluteDate();
    julDate->CalcJulianDate(absGregDate);

    EXPECT_EQ(julDate->getMonth(),3);
    EXPECT_EQ(julDate->getDay(),2);
    EXPECT_EQ(julDate->getYear(),100);
}

TEST_F(DateConverterFixture, MinusOne_Leap){ // 3/1/300 -> 2/29/300

    gregDate->SetMonth(3);
    gregDate->SetDay(1);
    gregDate->SetYear(300);

    int absGregDate = gregDate->getAbsoluteDate();
    julDate->CalcJulianDate(absGregDate);

    EXPECT_EQ(julDate->getMonth(),2);
    EXPECT_EQ(julDate->getDay(),29);
    EXPECT_EQ(julDate->getYear(),300);
}


TEST_F(DateConverterFixture, PlusFour_Leap){ // 3/1/900 -> 2/25/900

    gregDate->SetMonth(3);
    gregDate->SetDay(1);
    gregDate->SetYear(900);

    int absGregDate = gregDate->getAbsoluteDate();
    julDate->CalcJulianDate(absGregDate);

    EXPECT_EQ(julDate->getMonth(),2);
    EXPECT_EQ(julDate->getDay(),25);
    EXPECT_EQ(julDate->getYear(),900);
}
 */