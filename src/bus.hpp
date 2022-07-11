#ifndef Z80EMU_BUS_HPP
#define Z80EMU_BUS_HPP

#include "../libs/mcp23s17/mcp23s17.hpp"

class Bus {
public:
    Mcp23s17 _io;

    explicit Bus();
    explicit Bus(Mcp23s17 io);

    void tick();

    void setAddress(uint16_t addr);
    uint8_t read();
    void write(uint8_t data);
};

#endif //Z80EMU_BUS_HPP
