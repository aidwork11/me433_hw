#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"

// config registers
#define CONFIG 0x1A
#define GYRO_CONFIG 0x1B
#define ACCEL_CONFIG 0x1C
#define PWR_MGMT_1 0x6B
#define PWR_MGMT_2 0x6C
// sensor data registers:
#define ACCEL_XOUT_H 0x3B
#define ACCEL_XOUT_L 0x3C
#define ACCEL_YOUT_H 0x3D
#define ACCEL_YOUT_L 0x3E
#define ACCEL_ZOUT_H 0x3F
#define ACCEL_ZOUT_L 0x40
#define TEMP_OUT_H   0x41
#define TEMP_OUT_L   0x42
#define GYRO_XOUT_H  0x43
#define GYRO_XOUT_L  0x44
#define GYRO_YOUT_H  0x45
#define GYRO_YOUT_L  0x46
#define GYRO_ZOUT_H  0x47
#define GYRO_ZOUT_L  0x48
#define WHO_AM_I     0x75

#define MPU6050_ADDR 0x68

struct MPU6050_data {
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z; 
};

struct MPU6050_data read_mpu6050() {
    struct MPU6050_data data;
    uint8_t raw_read_data[14];
    uint8_t reg_addr = ACCEL_XOUT_H;
    i2c_write_blocking(i2c0, MPU6050_ADDR, &reg_addr, 1, true);
    i2c_read_blocking(i2c0, MPU6050_ADDR, raw_read_data, 14, false);

    data.accel_x = (raw_read_data[0] << 8) | raw_read_data[1];
    data.accel_y = (raw_read_data[2] << 8) | raw_read_data[3];
    data.accel_z = (raw_read_data[4] << 8) | raw_read_data[5];
    data.gyro_x = (raw_read_data[6] << 8) | raw_read_data[7];
    data.gyro_y = (raw_read_data[8] << 8) | raw_read_data[9];
    data.gyro_z = (raw_read_data[10] << 8) | raw_read_data[11];

    // unit conversions
    data.accel_x *= 0.000061 * 4;
    data.accel_y *= 0.000061 * 4;
    data.accel_z *= 0.000061 * 4;
    data.gyro_x *= 0.007630;
    data.gyro_y *= 0.007630;
    data.gyro_z *= 0.007630;
    return data;
}

int main()
{
    i2c_init(i2c0, 100 * 1000);
    i2c_init(i2c1, 100 * 1000);
    
    // Set i2c pins
    gpio_set_function(16, GPIO_FUNC_I2C);
    gpio_set_function(17, GPIO_FUNC_I2C);
    gpio_set_function(18, GPIO_FUNC_I2C);
    gpio_set_function(19, GPIO_FUNC_I2C);
    gpio_pull_up(18);
    gpio_pull_up(19);
    
    // initialize stdio
    stdio_init_all();
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    printf("Starting SSD1306 setup...\n");
    ssd1306_setup();
    ssd1306_clear();
    ssd1306_update();
    printf("SSD1306 setup complete!\n");

    // wake up MPU6050
    uint8_t wake_up[2] = {PWR_MGMT_1, 0x00};
    i2c_write_blocking(i2c0, MPU6050_ADDR, wake_up, 2, false);
    sleep_ms(100);

    // WHO AM I?
    uint8_t who_am_i_response = 0;
    uint8_t who_am_i_reg = WHO_AM_I;
    i2c_write_blocking(i2c0, MPU6050_ADDR, &who_am_i_reg, 1, false);
    i2c_read_blocking(i2c0, MPU6050_ADDR, &who_am_i_response, 1, false);
    printf("WHO_AM_I: 0x%x\n", who_am_i_response);

    // turn on accelerometer
    uint8_t accel_config_reg = ACCEL_CONFIG;
    uint8_t accel_config_data = 0x00;
    uint8_t turn_on_accel[2] = {accel_config_reg, accel_config_data};
    i2c_write_blocking(i2c0, MPU6050_ADDR, turn_on_accel, 2, false);

    // turn on gyroscope
    uint8_t gyro_config_reg = GYRO_CONFIG;
    uint8_t gyro_config_data = 0b00011000;
    uint8_t turn_on_gyro[2] = {gyro_config_reg, gyro_config_data};
    i2c_write_blocking(i2c0, MPU6050_ADDR, turn_on_gyro, 2, false);

    unsigned char x = 64;
    unsigned char y = 16;

    unsigned int color = 1;

    while (true) {
        struct MPU6050_data data;
        data = read_mpu6050();
        printf("Accel X: %d, Accel Y: %d, Accel Z: %d\n", data.accel_x, data.accel_y, data.accel_z);
        printf("Gyro X: %d, Gyro Y: %d, Gyro Z: %d\n", data.gyro_x, data.gyro_y, data.gyro_z);
        printf("--------------------------------\n");
        sleep_ms(200);

        ssd1306_drawPixel(x, y, 1);
        ssd1306_update();
        x+=data.accel_x;
        y-=data.accel_y; // added this because of my chip orientation. could be +=
        color++;
    }
}
