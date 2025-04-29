#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/pio.h"


#define servo_pwm_pin 16

void servo_pwm_init(float div, uint16_t wrap) {
    gpio_set_function(servo_pwm_pin, GPIO_FUNC_PWM); // Set the LED Pin to be PWM
    uint slice_num = pwm_gpio_to_slice_num(servo_pwm_pin); // Get PWM slice number
    pwm_set_clkdiv(slice_num, div); // divider
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true); // turn on the PWM // set the duty cycle to 10%
}

void servo_pwm_set_angle(float angle) {
    // 0 degrees: 0.5ms, 180 degrees: 2.5ms
    uint16_t pulse_width_us = 500 + (angle / 180.0) * (2000);
    uint16_t pulse_width_ticks = pulse_width_us / 0.8;
    pwm_set_gpio_level(servo_pwm_pin, pulse_width_ticks);
}

int main()
{
    stdio_init_all();
    servo_pwm_init(100, 30000);
    float angle = 0;
    bool direction = true;
    while (true) {
        if (angle > 180 || angle < 0) {
            direction = !direction;
        }
        direction ? (angle += 1) : (angle -= 1);
        servo_pwm_set_angle(angle);
        sleep_ms(2000/180);
    }
}
