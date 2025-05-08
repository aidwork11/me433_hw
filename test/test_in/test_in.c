#include <stdio.h>
#include "pico/stdlib.h"


int main()
{
    stdio_init_all();

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    gpio_init(16);
    gpio_set_dir(16, GPIO_IN);
    gpio_pull_up(16);

    while (true) {
        printf("Hello, world!\n");
        bool state = gpio_get(16);
        printf("State: %d\n", state);
        sleep_ms(1000);
    }
}
