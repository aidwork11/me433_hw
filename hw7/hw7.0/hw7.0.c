#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "font.h"
#include "hardware/adc.h"
#include <stdlib.h>

#define I2C_SDA_PIN 4
#define I2C_SCL_PIN 5

void draw_char(unsigned char x, unsigned char y, unsigned char c) {
    c -= 0x20;
    for (unsigned char i = 0; i < 5; i++) {
        const char row = ASCII[c][i];
        for (unsigned char j = 0; j < 8; j++) {
            ssd1306_drawPixel(x + i, y + j, (row >> j) & 1);
        }
    }
}

void draw_string(unsigned char x, unsigned char y, unsigned char* str) {
    unsigned char input = x;
    for (unsigned char i = 0; str[i] != '\0'; i++) {
        if (input + 6 >= 128) {
            input = 0;
            y += 8;
        }
        draw_char(input, y, str[i]);
        input += 6;
    }
}

int main()
{
    stdio_init_all();

    while(!stdio_usb_connected()) {
        sleep_ms(100);
    }

    i2c_init(i2c_default, 400 * 1000);
    adc_init();
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);


    printf("Starting setup...\n");
    ssd1306_setup();
    ssd1306_clear();
    ssd1306_update();
    printf("Setup complete!\n");

    unsigned char x = 0;
    unsigned char y = 0;

    while (true) {
        unsigned int t = to_us_since_boot(get_absolute_time());  

        printf("Hello, world!\n");
        uint16_t adc_output = adc_read();
        float voltage = (adc_output * 3.3) / 4095;
        char* voltage_str = (char*)malloc(20);
        sprintf(voltage_str, "Voltage: %0.3fV", voltage);
        draw_string(x, y, voltage_str);
        ssd1306_update();

        unsigned int t2 = to_us_since_boot(get_absolute_time()); 
        float fps = 1000000.0 / (t2 - t);
        char* fps_str = (char*)malloc(20);
        sprintf(fps_str, "FPS: %0.3f", fps);
        draw_string(x, 24, fps_str);
        ssd1306_update();
    }
}
