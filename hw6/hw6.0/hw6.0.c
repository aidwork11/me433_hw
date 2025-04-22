#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define I2C_SDA_PIN 4
#define I2C_SCL_PIN 5

uint8_t read_pins(uint8_t reg_addr) {
    uint8_t data;
    i2c_write_blocking(i2c_default, 0x20, &reg_addr, 1, true);
    i2c_read_blocking(i2c_default, 0x20, &data, 1, false);
    return data;
}


int main()
{

    stdio_init_all();

    while(!stdio_usb_connected()) {
        sleep_ms(100);
    }

    i2c_init(i2c_default, 400 * 1000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    uint8_t iodir = read_pins(0x00); // IODIR register: should be 0xFF at reset
    printf("IODIR = 0x%02x\n", iodir);  // should print 0xFF on power-up

    uint8_t addr_read = 0x41;
    uint8_t addr_write = 0x40;
    uint8_t addr = 0x20;
    uint8_t io_dir = 0x00;
    uint8_t gpio_reg = 0x09;
    uint8_t gppu_reg = 0x06;

    uint8_t set_GP0_output[2] = {io_dir, 0x01};
    uint8_t set_GP0_high[2] = {gpio_reg, 0xF0};
    uint8_t set_GP0_low[2] = {gpio_reg, 0x00};
    uint8_t set_GP0_pullup[2] = {gppu_reg, 0x01};

    int write1 = i2c_write_blocking(i2c_default, addr, set_GP0_output, 2, false);
    int write2 = i2c_write_blocking(i2c_default, addr, set_GP0_pullup, 2, false);

    while (true) {

        uint8_t data = read_pins(0x09);

        if (data == 0x00) {
            i2c_write_blocking(i2c_default, addr, set_GP0_high, 2, false);
        }
        else {
            i2c_write_blocking(i2c_default, addr, set_GP0_low, 2, false);
        }

    }
}
