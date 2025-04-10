#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#define LED_PIN 16
#define BUTTON_PIN 15
#define ADC_PIN 26

int main() {

    // setup
    stdio_init_all();
    adc_init();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
    adc_gpio_init(ADC_PIN);
    adc_select_input(0);

    // wait for usb connection
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    // turn on led
    gpio_put(LED_PIN, true);

    // wait for button press
    bool button_pressed = false;
    while (button_pressed == false) { // loop until ready
        if (!gpio_get(BUTTON_PIN)) { // (if pin in low)
            button_pressed = true;
        }
    }

    // turn off led
    gpio_put(LED_PIN, false);

    while (1) {
       /* char message[100];
        scanf("%s", message);
        printf("message: %s\r\n",message);
        sleep_ms(50); */

        // prompt for user input
        printf("Enter how many samples to take:\n");
        unsigned int num_samples;
        scanf("%u", &num_samples);

        printf("Taking %u samples...\n", num_samples);

        // read adc
        for (unsigned int i = 0; i < num_samples; i++) {
            uint16_t adc_output = adc_read();
            float voltage = (adc_output * 3.3) / 4095;
            printf("  Voltage: %0.3f\n", voltage);
            fflush(stdout);
            sleep_ms(10);
        }


    } 
}
