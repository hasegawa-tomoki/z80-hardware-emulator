#include "mcp23s17.hpp"
#include <cstdio>
#include <pigpiod_if2.h>
#include <unistd.h>

Mcp23s17::Mcp23s17()
{
}

Mcp23s17::Mcp23s17(int pi)
{
    this->_pi = pi;
    this->_spi = -1;
}

int Mcp23s17::init()
{
    set_mode(this->_pi, GPIO_IORESET, PI_OUTPUT);

    int spi_cs = 0;
    int spi_baud = 10 * 1000 * 1000;
    int flag = 0;
    int spi0 = spi_open(this->_pi, spi_cs, spi_baud, flag);
    if (spi0 < 0){
        printf("Failed to spiOpen();\n");
        return 1;
    }
    this->_spi = spi0;

    this->reset();
    usleep(2000);

    for (int device_id = 0; device_id <= 1; device_id++){
        this->write_register(device_id, MCP23S17_IOCON, IOCON_INIT);

        for (int idx = 0; idx < 16; idx++){
            this->set_direction(device_id, idx, DIR_OUTPUT);
        }
//        for (int idx = 0; idx < 16; idx++){
//            this->set_pull_up_mode(device_id, idx, PULLUP_ENABLED);
//        }
    }
    return 0;
}

void Mcp23s17::reset() const
{
    gpio_write(this->_pi, GPIO_IORESET, 0);
    usleep(2);
    gpio_write(this->_pi, GPIO_IORESET, 1);
    usleep(2);
}

void Mcp23s17::write_register(uint8_t device_id, uint8_t reg, uint8_t value) const
{
    auto command = (uint8_t)(MCP23S17_CMD_WRITE | (device_id <<1));
    char data[] = {command, reg, value};

    spi_write(this->_pi, this->_spi, data, 3);
}

uint8_t Mcp23s17::read_register(uint8_t device_id, uint8_t reg) const
{
    auto command = (uint8_t)(MCP23S17_CMD_READ | (device_id << 1));

    char data[] = {command, reg, 0};
    char buffer[3];

    spi_xfer(this->_pi, this->_spi, data, buffer, 3);
    return buffer[2];
}

void Mcp23s17::set_direction_8(uint8_t device_id, int gpio, int direction) {
    if (direction != DIR_INPUT && direction != DIR_OUTPUT){
        return;
    }

    uint8_t data;
    if (direction == DIR_OUTPUT){
        data = 0;
    } else {
        data = 0xff;
    }

    uint8_t reg;
    if (gpio == MCP23S17_GPIOA){
        reg = MCP23S17_IODIRA;
    } else {
        reg = MCP23S17_IODIRB;
    }


    this->write_register(device_id, reg, data);

    if (gpio == MCP23S17_GPIOA){
        this->_io_dir_a = data;
    } else {
        this->_io_dir_b = data;
    }
}

void Mcp23s17::set_direction(uint8_t device_id, int pin, int direction)
{
    if (pin >= 16){
        return;
    }
    if (direction != DIR_INPUT && direction != DIR_OUTPUT){
        return;
    }

    uint8_t reg;
    uint8_t data;
    int no_shifts;

    if (pin < 8){
        reg = MCP23S17_IODIRA;
        data = this->_io_dir_a;
        no_shifts = pin;
    } else {
        reg = MCP23S17_IODIRB;
        data = this->_io_dir_b;
        no_shifts = pin & 0x07;
    }

    if (direction == DIR_INPUT){
        data |= (1 << no_shifts);
    } else {
        data &= (~(1 << no_shifts));
    }

    this->write_register(device_id, reg, data);

    if (pin < 8){
        this->_io_dir_a = data;
    } else {
        this->_io_dir_b = data;
    }
}

void Mcp23s17::set_pull_up_mode(uint8_t device_id, int pin, int mode)
{
    if (pin >= 16){
        return;
    }
    if (mode != PULLUP_ENABLED && mode != PULLUP_DISABLED){
        return;
    }

    uint8_t reg;
    uint8_t data;
    int no_shifts;

    if (pin < 8){
        reg = MCP23S17_GPPUA;
        data = this->_gppu_a;
        no_shifts = pin;
    } else {
        reg = MCP23S17_GPPUB;
        data = this->_gppu_b;
        no_shifts = pin & 0x07;
    }

    if (mode == PULLUP_ENABLED){
        data |= (1 << no_shifts);
    } else {
        data &= (~(1 << no_shifts));
    }

    this->write_register(device_id, reg, data);

    if (pin < 8){
        this->_gppu_a = data;
    } else {
        this->_gppu_b = data;
    }
}

int Mcp23s17::digital_read(uint8_t device_id, int pin)
{
    if (pin >= 16){
        return 0;
    }

    if (pin < 8){
        this->_gpio_a = this->read_register(device_id, MCP23S17_GPIOA);
        if ((this->_gpio_a & (1 << pin)) != 0){
            return LEVEL_HIGH;
        } else {
            return LEVEL_LOW;
        }
    } else {
        this->_gpio_b = this->read_register(device_id, MCP23S17_GPIOB);
        pin &= 0x07;
        if ((this->_gpio_b & (1 << pin)) != 0){
            return LEVEL_HIGH;
        } else {
            return LEVEL_LOW;
        }
    }
}

void Mcp23s17::digital_write(uint8_t device_id, int pin, int level)
{
    if (pin >= 16){
        return;
    }
    if (level != LEVEL_HIGH && level != LEVEL_LOW){
        return;
    }

    uint8_t reg;
    uint8_t data;
    int noshifts;

    if (pin < 8){
        reg = MCP23S17_GPIOA;
        data = this->_gpio_a;
        noshifts = pin;
    } else {
        reg = MCP23S17_GPIOB;
        data = this->_gpio_b;
        noshifts = pin & 0x07;
    }

    if (level == LEVEL_HIGH){
        data |= (1 << noshifts);
    } else {
        data &= (~(1 << noshifts));
    }

    this->write_register(device_id, reg, data);

    if (pin < 8){
        this->_gpio_a = data;
    } else {
        this->_gpio_b = data;
    }
}

void Mcp23s17::write_gpio(uint8_t device_id, uint16_t data)
{
    this->_gpio_a = (data & 0xff);
    this->_gpio_b = (data >> 8);
    this->write_register(device_id, MCP23S17_GPIOA, this->_gpio_a);
    this->write_register(device_id, MCP23S17_GPIOB, this->_gpio_b);
}

void Mcp23s17::write_gpio8(uint8_t device_id, int gpio, uint8_t data)
{
    if (gpio == MCP23S17_GPIOA){
        this->_gpio_a = data;
    } else {
        this->_gpio_b = data;
    }
    this->write_register(device_id, gpio, data);
    usleep(1);
}

uint8_t Mcp23s17::read_gpio8(uint8_t device_id, int gpio)
{
    uint8_t data;
    if (gpio == MCP23S17_GPIOA){
        data = this->read_register(device_id, MCP23S17_GPIOA);
        this->_gpio_a = data;
    } else {
        data = this->read_register(device_id, MCP23S17_GPIOB);
        this->_gpio_b = data;
    }
    return data;
}
