#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <math.h>
#define CS_PIN 17
#define SCK_PIN 18
#define SPI_TX_PIN 19

static inline void cs_toggle(bool value) {
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(CS_PIN, value);
    asm volatile("nop \n nop \n nop"); // FIXME
}

void prepare_triangle_data(uint16_t data_value, uint8_t data[2]) {
    // Ensure data value doesn't exceed 10 bits (0-1023)
    data_value &= 0x3FF;
    
    // Build the control byte (first byte)
    uint8_t control_byte = 0;
    
   // control_byte |= (1 << 5);  // GA bit = 1 (gain = 1x)
    control_byte |= (1 << 4);  // SHDN bit = 1 (active mode)
    
    // Add the 2 most significant bits of data to the control byte
    control_byte |= ((data_value >> 8) & 0x03);
    
    // Second byte contains the 8 least significant bits of data
    uint8_t data_byte = data_value & 0xFF;

    uint16_t data_output = control_byte << 8 | data_byte;
    printf("data_output: %d\n", data_output);
    
    // Fill the output array
    data[0] = control_byte;
    data[1] = data_byte;
}

void prepare_sin_data(uint16_t data_value, uint8_t data[2]) {
    uint8_t control_byte = 0;

    // settings
    control_byte |= (1 << 7);
    control_byte |= (1 << 6);
    control_byte |= (1 << 5);
    control_byte |= (1 << 4); 

    data_value *= 2;
    data_value %= 1024;

    float angle = data_value * 2.0f * 3.314f / 1024.0f;
    uint16_t sin_value = (uint16_t)((sin(angle) + 1.0f) * 512.0f);

    control_byte |= ((sin_value >> 8) & 0x03);

    uint8_t data_byte = sin_value & 0xFF;

    uint16_t data_output = control_byte << 8 | data_byte;
    printf("data_output: %d\n", data_output);
    
    // Fill the output array
    data[0] = control_byte;
    data[1] = data_byte;

}



int main()
{
    stdio_init_all();
    gpio_init(CS_PIN);
    gpio_set_dir(CS_PIN, GPIO_OUT);

    spi_init(spi_default, 1000 * 1000); // the baud, or bits per second
    gpio_set_function(SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI_TX_PIN, GPIO_FUNC_SPI);

    uint8_t triangle_data[2];
    uint8_t sin_data[2];

    while (true) {

        for (int i = 0; i < 1024; i++) {
            prepare_triangle_data(i, triangle_data);
            prepare_sin_data(i, sin_data);
            cs_toggle(false);
            spi_write_blocking(spi_default, triangle_data, sizeof(triangle_data));
            cs_toggle(true);
            cs_toggle(false);
            spi_write_blocking(spi_default, sin_data, sizeof(sin_data));
            cs_toggle(true);
            sleep_ms(1024/1000);
        }
    }
}
