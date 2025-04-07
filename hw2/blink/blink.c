#include "pico/stdlib.h"
#include "stdio.h"
#define LED_PIN 16
#define BUTTON_IN_PIN 15
#define LED_DELAY_MS (uint32_t)250

volatile bool red_led_state = false;
volatile uint64_t last_interrupt_time = 0;
volatile unsigned int button_press_count = 0;

void green_set_led(bool led_on) {
    gpio_put(PICO_DEFAULT_LED_PIN, led_on);
}

void red_set_led(bool led_on) {
    gpio_put(LED_PIN, led_on);
}

// update light asynchronously
// if statement is for debouncing - make sure the light only toggles once
void button_interrupt(void) {
    uint64_t time_since_boot = time_us_64();
    if (time_since_boot - last_interrupt_time > 200000) {
        red_led_state = !red_led_state;
        red_set_led(red_led_state);
        last_interrupt_time = time_since_boot;
        button_press_count++;
    }
}

void pico_led_init(void) {

    // green blinking
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    // red blinking
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // button (input)
    gpio_init(BUTTON_IN_PIN);
    gpio_set_dir(BUTTON_IN_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_IN_PIN);

    // button interrupt
    gpio_set_irq_enabled_with_callback(BUTTON_IN_PIN, GPIO_IRQ_EDGE_FALL, true, (void*)&button_interrupt);

}

int main() {
    pico_led_init();
    stdio_init_all();

    bool green_led_state = false;

    while (true) {
        printf("Button Press Count: %i\n", button_press_count);
        green_set_led(green_led_state);
        green_led_state = !green_led_state;
        sleep_ms(LED_DELAY_MS);
    }
    
}
