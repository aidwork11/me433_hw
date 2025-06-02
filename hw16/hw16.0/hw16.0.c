#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define MOTORPIN 16
#define pin2 17

void pwm_pin_init() {
    gpio_set_function(MOTORPIN, GPIO_FUNC_PWM);
    gpio_set_function(pin2, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(MOTORPIN);
    uint slice_num2 = pwm_gpio_to_slice_num(pin2);
    
    float div = 40;
    pwm_set_clkdiv(slice_num, div);
    pwm_set_clkdiv(slice_num2, div);
    
    uint16_t wrap = 62500;
    pwm_set_wrap(slice_num, wrap);
    pwm_set_wrap(slice_num2, wrap);
    
    pwm_set_enabled(slice_num, true);
    pwm_set_enabled(slice_num2, true);
}

void set_motor_speed(int speed1, int speed2) {
    if (speed1 > 25) speed1 = 25;
    if (speed1 < 0) speed1 = 0;
    if (speed2 > 25) speed2 = 25;
    if (speed2 < 0) speed2 = 0;
    
    // Convert 0-25 range to 0-62500 range
    uint16_t pwm_value1 = (speed1 * 62500) / 25;
    uint16_t pwm_value2 = (speed2 * 62500) / 25;

    pwm_set_gpio_level(MOTORPIN, pwm_value1);
    pwm_set_gpio_level(pin2, pwm_value2);
}

int main()
{
    stdio_init_all();

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    pwm_pin_init();
    int controller;
    int i = 0;
    int j = 0;

    printf("Enter w or s to control the motor\n");

    while (true) {
        controller = getchar();
        if (controller == 'w') {
            i++;
        }
        else if (controller == 's') {
            i--;
        }
        else if (controller == 'q') {
            j++;
        }
        else if (controller == 'a') {
            j--;
        }
        set_motor_speed(i, j);
        printf("%d %d\n", i, j);
    }
}