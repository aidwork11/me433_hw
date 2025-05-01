#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/adc.h"

#define FLAG_VALUE 0xDEADBEEF

void core1_entry() {

  //  printf("Hello, core 1!\n");

    multicore_fifo_push_blocking(FLAG_VALUE); // write back to core0

   // printf("Hello, core 1 back!\n");

    uint8_t ack = multicore_fifo_pop_blocking(); // read from core0

    while(1) {

        uint8_t g = multicore_fifo_pop_blocking(); // read from core0
        uint32_t response = 0xF0; // default response

    //  printf("Hello, core 1 back 2!\n");
        if (g == 0 ) {
            uint16_t result = adc_read();
            response = (uint32_t)result;}
        else if (g == 1) {
            gpio_put(15, true);
            response = 0xF2;}
        else if (g == 2) {
            gpio_put(15, false);
            response = 0xF3;}

        multicore_fifo_push_blocking(response);

    }



    while (1)
        tight_loop_contents(); // do nothing forever
}

int main()
{
    stdio_init_all();

    while (!stdio_usb_connected()) {
        printf("Waiting for USB connection...\n");
        sleep_ms(1000);
    }

    adc_init();
    gpio_init(15);
    gpio_set_dir(15, GPIO_OUT);
    multicore_launch_core1(core1_entry);



   uint32_t flag = multicore_fifo_pop_blocking();
   if (flag != FLAG_VALUE) {
    printf("Hmm, that's not right on core 0!\n");
   } else {
    printf("Its all gone well on core 0!");
   }

   multicore_fifo_push_blocking(FLAG_VALUE);

   uint8_t user_input;

    while (true) {
        printf("Enter 0 to read ADC, 1 to turn on LED, 2 to turn off LED: ");
        scanf("%d", &user_input);
        printf("%d\n", user_input);

        multicore_fifo_push_blocking(user_input);

        uint32_t response = multicore_fifo_pop_blocking();

        if (response == 0xF0) {
            printf("Invalid input\n");
        }
        else if (response == 0xF2) {
            printf("LED turned ON\n");
        }
        else if (response == 0xF3) {
            printf("LED turned OFF\n");
        }
        else {
            printf("ADC result: %d\n", response);
        }


        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
