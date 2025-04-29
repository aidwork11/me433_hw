/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"
#include "hardware/pwm.h"


#define NUM_PIXELS 4

#ifdef PICO_DEFAULT_WS2812_PIN
#define WS2812_PIN PICO_DEFAULT_WS2812_PIN
#else
// default to pin 2 if the board doesn't have a default WS2812 pin defined
#define WS2812_PIN 15
#endif

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

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} wsColor; 


static inline void put_pixel(PIO pio, uint sm, uint32_t pixel_grb) {
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            (uint32_t) (b);
}

void pattern_snakes(PIO pio, uint sm, uint len, wsColor *pixels) {
    for (uint i = 0; i < len; ++i) {
        put_pixel(pio, sm, urgb_u32(pixels[i].r, pixels[i].g, pixels[i].b));
    }
}


// adapted from https://forum.arduino.cc/index.php?topic=8498.0
// hue is a number from 0 to 360 that describes a color on the color wheel
// sat is the saturation level, from 0 to 1, where 1 is full color and 0 is gray
// brightness sets the maximum brightness, from 0 to 1
wsColor HSBtoRGB(float hue, float sat, float brightness) {
    float red = 0.0;
    float green = 0.0;
    float blue = 0.0;

    if (sat == 0.0) {
        red = brightness;
        green = brightness;
        blue = brightness;
    } else {
        if (hue == 360.0) {
            hue = 0;
        }

        int slice = hue / 60.0;
        float hue_frac = (hue / 60.0) - slice;

        float aa = brightness * (1.0 - sat);
        float bb = brightness * (1.0 - sat * hue_frac);
        float cc = brightness * (1.0 - sat * (1.0 - hue_frac));

        switch (slice) {
            case 0:
                red = brightness;
                green = cc;
                blue = aa;
                break;
            case 1:
                red = bb;
                green = brightness;
                blue = aa;
                break;
            case 2:
                red = aa;
                green = brightness;
                blue = cc;
                break;
            case 3:
                red = aa;
                green = bb;
                blue = brightness;
                break;
            case 4:
                red = cc;
                green = aa;
                blue = brightness;
                break;
            case 5:
                red = brightness;
                green = aa;
                blue = bb;
                break;
            default:
                red = 0.0;
                green = 0.0;
                blue = 0.0;
                break;
        }
    }

    unsigned char ired = red * 255.0;
    unsigned char igreen = green * 255.0;
    unsigned char iblue = blue * 255.0;

    wsColor c;
    c.r = ired;
    c.g = igreen;
    c.b = iblue;
    return c;
}



int main() {
    //set_sys_clock_48();
    stdio_init_all();
    servo_pwm_init(100, 30000);

    printf("WS2812 Smoke Test, using pin %d\n", WS2812_PIN);

    // todo get free sm
    PIO pio;
    uint sm;
    uint offset;

    // This will find a free pio and state machine for our program and load it for us
    // We use pio_claim_free_sm_and_add_program_for_gpio_range (for_gpio_range variant)
    // so we will get a PIO instance suitable for addressing gpios >= 32 if needed and supported by the hardware
    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_program, &pio, &sm, &offset, WS2812_PIN, 1, true);
    hard_assert(success);

    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, false);

    wsColor pixels[NUM_PIXELS];

    float angle = 0;
    bool direction = true;


    while (1) {
        for (int i = 0; i <= 330; ++i) {
            wsColor c = HSBtoRGB(i, 1.0, 1.0);
            wsColor c2 = HSBtoRGB(i + 10, 1.0, 1.0);
            wsColor c3 = HSBtoRGB(i + 20, 1.0, 1.0);
            wsColor c4 = HSBtoRGB(i + 30, 1.0, 1.0);
            pixels[0] = c;
            pixels[1] = c2;
            pixels[2] = c3;
            pixels[3] = c4;
            pattern_snakes(pio, sm, NUM_PIXELS, pixels);
            sleep_ms(10);

            if (angle > 180 || angle < 0) {
                direction = !direction;
            }
            direction ? (angle += 1) : (angle -= 1);
            servo_pwm_set_angle(angle);
            sleep_ms(2000/348);
        }
    }

    // This will free resources and unload our program
    pio_remove_program_and_unclaim_sm(&ws2812_program, pio, sm, offset);
}
