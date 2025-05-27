#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define MOTORPIN 16
#define pin2 17

void pwm_pin_init() {
    gpio_set_function(MOTORPIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(MOTORPIN);
    
    float div = 40;
    pwm_set_clkdiv(slice_num, div);
    
    uint16_t wrap = 62500;
    pwm_set_wrap(slice_num, wrap);
    
    pwm_set_enabled(slice_num, true);
}

void set_motor_speed(int speed) {
    if (speed > 25) speed = 25;
    if (speed < 0) speed = 0;
    
    // Convert 0-25 range to 0-62500 range
    uint16_t pwm_value = (speed * 62500) / 25;
    pwm_set_gpio_level(MOTORPIN, pwm_value);
}

int main()
{
    stdio_init_all();

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    gpio_init(pin2);
    gpio_set_dir(pin2, GPIO_OUT);
    gpio_put(pin2, 0);

    pwm_pin_init();
    int controller;
    int i = 0;

    printf("Enter w or s to control the motor\n");

    while (true) {
        controller = getchar();
        if (controller == 'w') {
            i++;
        }
        else if (controller == 's') {
            i--;
        }
        set_motor_speed(i);
        printf("%d\n", i);
    }
}