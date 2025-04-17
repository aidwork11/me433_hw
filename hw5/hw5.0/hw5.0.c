#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <math.h>


#define CS_PIN_0 17
#define SCK_PIN_0 18
#define SPI_TX_PIN_0 19

#define SPI_RX_PIN_1 12
#define CS_PIN_1 13
#define SCK_PIN_1 14
#define SPI_TX_PIN_1 15

#define RAM_READ 0x03
#define RAM_WRITE 0x02



static inline void cs_toggle(bool value, uint cs_pin) {
    asm volatile("nop \n nop \n nop");
    gpio_put(cs_pin, value);
    asm volatile("nop \n nop \n nop");
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

    float angle = data_value * 2.0f * M_PI / 1024.0f;
    uint16_t sin_value = (uint16_t)((sin(angle) + 1.0f) * 512.0f);

    control_byte |= ((sin_value >> 8) & 0x03);

    uint8_t data_byte = sin_value & 0xFF;
    
    // Fill the output array
    data[0] = control_byte;
    data[1] = data_byte;

}


void read_from_RAM(uint8_t* from_data, uint16_t address) {

    uint8_t address_high = (address >> 8) & 0xFF;
    uint8_t address_low = address & 0xFF;
    uint8_t command[3] = {RAM_READ, address_high, address_low};

    cs_toggle(false, CS_PIN_1);
    spi_write_blocking(spi1, command, 3);
    spi_read_blocking(spi1, 0xFF, from_data, 1);
    cs_toggle(true, CS_PIN_1);

    sleep_ms(1);
}

bool write_to_RAM(uint8_t to_data, uint16_t address) {
    uint8_t address_high = (address >> 8) & 0xFF;
    uint8_t address_low = address & 0xFF;
    uint8_t command[4] = {RAM_WRITE, address_high, address_low, to_data};

    cs_toggle(false, CS_PIN_1);
    spi_write_blocking(spi1, command, 4);
    cs_toggle(true, CS_PIN_1);



    uint8_t read_data;
    read_from_RAM(&read_data, address);
    return read_data == to_data;  // 0 means something went wrong
}

void set_RAM_to_byte_mode() {
    uint8_t command[2] = {0x01, 0x00};
    cs_toggle(false, CS_PIN_1);
    spi_write_blocking(spi1, command, 2);
    cs_toggle(true, CS_PIN_1);
}

uint8_t read_RAM_status() {
    uint8_t status;
    uint8_t command = 0x05; 
    
    cs_toggle(false, CS_PIN_1);
    spi_write_blocking(spi1, &command, 1);
    spi_read_blocking(spi1, 0xFF, &status, 1);
    cs_toggle(true, CS_PIN_1);
    
    return status; // 0x00 means ready

}



int main()
{
    // so much set up
    stdio_init_all();
    gpio_init(CS_PIN_0);
    gpio_init(CS_PIN_1);
    gpio_set_dir(CS_PIN_0, GPIO_OUT);
    gpio_set_dir(CS_PIN_1, GPIO_OUT);
    spi_init(spi1, 100000); // the baud, or bits per second
    spi_init(spi0, 1000 * 1000);
    gpio_set_function(SCK_PIN_0, GPIO_FUNC_SPI);
    gpio_set_function(SPI_TX_PIN_0, GPIO_FUNC_SPI);
    gpio_set_function(SCK_PIN_1, GPIO_FUNC_SPI);
    gpio_set_function(SPI_TX_PIN_1, GPIO_FUNC_SPI);
    gpio_set_function(SPI_RX_PIN_1, GPIO_FUNC_SPI);

    set_RAM_to_byte_mode();

    // wait for terminal to connect
    while (!stdio_usb_connected()) {
        printf("Waiting for USB connection...\n");
        sleep_ms(1000);
    }
    while (read_RAM_status() != 0x00) {
        printf("RAM is not ready\n");
        sleep_ms(1000);
    }

    // get two floats from user
    volatile float f1, f2;
    printf("Enter two floats to use:");
    scanf("%f %f", &f1, &f2);
    printf("\nf1: %f, f2: %f\n", f1, f2);

    float operating_freq = 150000000.0f; //150 MHz

    // do the math 1000 times
    uint32_t add_start_time = time_us_32();
    volatile float f_add, f_sub, f_mult, f_div;
    for (int i = 0; i < 1000; i++) {
        f_add = f1+f2;
    }
    uint32_t add_end_time = time_us_32();

    uint32_t sub_start_time = time_us_32();
    for (int i = 0; i < 1000; i++) {
        f_sub = f1-f2;
    }
    uint32_t sub_end_time = time_us_32();

    uint32_t mult_start_time = time_us_32();
    for (int i = 0; i < 1000; i++) {
        f_mult = f1*f2;
    }
    uint32_t mult_end_time = time_us_32();

    uint32_t div_start_time = time_us_32();
    for (int i = 0; i < 1000; i++) {
        f_div = f1/f2;
    }
    uint32_t div_end_time = time_us_32();

    float add_cycles = (float)(add_end_time - add_start_time) * operating_freq / 1000000.0f;
    float sub_cycles = (float)(sub_end_time - sub_start_time) * operating_freq / 1000000.0f;
    float mult_cycles = (float)(mult_end_time - mult_start_time) * operating_freq / 1000000.0f;
    float div_cycles = (float)(div_end_time - div_start_time) * operating_freq / 1000000.0f;

    printf("\nResults: \n%f+%f=%f \n%f-%f=%f \n%f*%f=%f \n%f/%f=%f\n", f1,f2,f_add, f1,f2,f_sub, f1,f2,f_mult, f1,f2,f_div);
    
    printf("ADD: Time taken: %d us. Average time per operation: %.3f us. Cycles per operation: %f\n", 
           add_end_time - add_start_time, 
           (float)(add_end_time - add_start_time) / 1000.0f, 
           add_cycles / 1000);
    printf("SUB: Time taken: %d us. Average time per operation: %.3f us. Cycles per operation: %f\n", 
           sub_end_time - sub_start_time, 
           (float)(sub_end_time - sub_start_time) / 1000.0f, 
           sub_cycles / 1000);
    printf("MULT: Time taken: %d us. Average time per operation: %.3f us. Cycles per operation: %f\n", 
           mult_end_time - mult_start_time, 
           (float)(mult_end_time - mult_start_time) / 1000.0f, 
           mult_cycles / 1000);
    printf("DIV: Time taken: %d us. Average time per operation: %.3f us. Cycles per operation: %f\n", 
           div_end_time - div_start_time, 
           (float)(div_end_time - div_start_time) / 1000.0f, 
           div_cycles / 1000);

    // convert to cycles
    uint32_t cycles_per_operation = (add_end_time - add_start_time) / 1000.0f;

    uint8_t data_to_RAM[2];
    uint8_t data_from_RAM[2];
    uint8_t data_to_DAC[2];
    
    bool success;
    for (int i = 0; i < 2048; i+=2) {
        prepare_sin_data(i, data_to_RAM);
        success = write_to_RAM(data_to_RAM[1], i);
        if (!success) {
            printf("write_to_RAM[%d]: FAILED\n", i);
        }
        success = write_to_RAM(data_to_RAM[0], i+1);
        if (!success) {
            printf("write_to_RAM[%d]: FAILED\n", i+1);
        }
    }

    printf("sin write_to_RAM done\n");


    while (true) {
        printf("Hello, world!\n");
        for (int i = 0; i < 2048; i+=2) {
            read_from_RAM(&(data_from_RAM[1]), (uint16_t)i);
            read_from_RAM(&(data_from_RAM[0]), (uint16_t)(i+1));
            uint16_t data_from_RAM_output = (data_from_RAM[1] << 8) | data_from_RAM[0];
            //printf("data_from_RAM: %x\n", data_from_RAM_output);
            cs_toggle(false, CS_PIN_0);
            spi_write_blocking(spi0, data_from_RAM, 2);
            cs_toggle(true, CS_PIN_0);
            sleep_ms(1024/1000);
        }
    }
}