#ifndef MCP23S17_HPP
#define MCP23S17_HPP
#include "pigpio.h"

class Mcp23s17 {
public:
    int _pi;
    int _spi;

    int _device_id = 0;
    uint8_t _gpio_a = 0;
    uint8_t _gpio_b = 0;
    uint8_t _io_dir_a = 0;
    uint8_t _io_dir_b = 0;
    uint8_t _gppu_a = 0;
    uint8_t _gppu_b = 0;

    static const int GPIO_IORESET = 12;
    static const int PULLUP_ENABLED = 0;
    static const int PULLUP_DISABLED = 1;
    static const int DIR_INPUT = 0;
    static const int DIR_OUTPUT = 1;
    static const int LEVEL_LOW = 0;
    static const int LEVEL_HIGH = 1;

    static const uint8_t MCP23S17_IODIRA = 0x00;
    static const uint8_t MCP23S17_IODIRB = 0x01;
    static const uint8_t MCP23S17_IPOLA = 0x02;
    static const uint8_t MCP23S17_IPOLB = 0x03;
    static const uint8_t MCP23S17_GPIOA = 0x12;
    static const uint8_t MCP23S17_GPIOB = 0x13;
    static const uint8_t MCP23S17_OLATA = 0x14;
    static const uint8_t MCP23S17_OLATB = 0x15;
    static const uint8_t MCP23S17_IOCON = 0x0A;
    static const uint8_t MCP23S17_GPPUA = 0x0C;
    static const uint8_t MCP23S17_GPPUB = 0x0D;

    static const uint8_t IOCON_UNUSED = 0x01;
    static const uint8_t IOCON_INTPOL = 0x02;
    static const uint8_t IOCON_ODR = 0x04;
    static const uint8_t IOCON_HAEN = 0x08;
    static const uint8_t IOCON_DISSLW = 0x10;
    static const uint8_t IOCON_SEQOP = 0x20;
    static const uint8_t IOCON_MIRROR = 0x40;
    static const uint8_t IOCON_BANK_MODE = 0x80;

    static const uint8_t IOCON_INIT = 0x28;

    static const uint8_t MCP23S17_CMD_WRITE = 0x40;
    static const uint8_t MCP23S17_CMD_READ = 0x41;

    Mcp23s17();
    Mcp23s17(int pi);
    int init();

    void set_direction(uint8_t device_id, int pin, int direction);
    void set_direction_8(uint8_t device_id, int gpio, int direction);
    void set_pull_up_mode(uint8_t device_id, int pin, int mode);
    int digital_read(uint8_t device_id, int pin);
    void digital_write(uint8_t device_id, int pin, int level);
    void write_gpio(uint8_t device_id, uint16_t data);
    void write_gpio8(uint8_t device_id, int gpio, uint8_t data);
    uint8_t read_gpio8(uint8_t device_id, int gpio);
private:
    void reset() const;
    void write_register(uint8_t device_id, uint8_t reg, uint8_t value) const;
    uint8_t read_register(uint8_t device_id, uint8_t reg) const;
    void cs_on();
    void cs_off();
};

#endif