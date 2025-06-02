#include <stdio.h>
#include "pico/stdlib.h"
#include "cam.h"

#define MOTORPIN_LEFT 16
#define MOTORPIN_RIGHT 17

uint8_t scan_camera(cameraPixel_t pixel_left, cameraPixel_t pixel_right){
    uint8_t pixel_left_avg = (pixel_left.r + pixel_left.g + pixel_left.b) / 3;
    uint8_t pixel_right_avg = (pixel_right.r + pixel_right.g + pixel_right.b) / 3;

    uint8_t threshold = 255/8;
    if (pixel_left_avg < threshold && pixel_right_avg < threshold){
        return 0;
    }
    else if (pixel_left_avg > threshold && pixel_right_avg < threshold){
        return 1;
    }
    else if (pixel_left_avg < threshold && pixel_right_avg > threshold){
        return 2;
    }
    else{
        return 3;
    }
}

int main()
{
    stdio_init_all();



    gpio_init(MOTORPIN_LEFT);
    gpio_init(MOTORPIN_RIGHT);
    gpio_set_dir(MOTORPIN_LEFT, GPIO_OUT);
    gpio_set_dir(MOTORPIN_RIGHT, GPIO_OUT);
    gpio_put(MOTORPIN_LEFT, 0);
    gpio_put(MOTORPIN_RIGHT, 0);




    init_camera_pins();

 
    while (true) {
        // uncomment these and printImage() when testing with python 
        //char m[10];
        //scanf("%s",m);

        setSaveImage(1);
        while(getSaveImage()==1){}
        convertImage();
        int com = findLine(IMAGESIZEY/2); // calculate the position of the center of the ine
        setPixel(IMAGESIZEY/2,com,0,255,0); // draw the center so you can see it in python
       // printImage();


        uint8_t offset = 6;
        cameraPixel_t pixel_left = getPixel((IMAGESIZEY/2)-offset,(IMAGESIZEX/2));
        cameraPixel_t pixel_right = getPixel((IMAGESIZEY/2)+offset,(IMAGESIZEX/2));

        printf("%d %d %d    %d %d %d\r\n",pixel_left.r,pixel_left.g,pixel_left.b,pixel_right.r,pixel_right.g,pixel_right.b);

        uint8_t scan_result = scan_camera(pixel_left, pixel_right);

       // printf("%d\r\n",scan_result);

        if (scan_result == 0){
            gpio_put(MOTORPIN_LEFT, 1);
            gpio_put(MOTORPIN_RIGHT, 1);
        }
        else if (scan_result == 1){
            gpio_put(MOTORPIN_LEFT, 1);
            gpio_put(MOTORPIN_RIGHT, 0);
        }
        else if (scan_result == 2){
            gpio_put(MOTORPIN_LEFT, 0);
            gpio_put(MOTORPIN_RIGHT, 1);
        }           
        else {
            gpio_put(MOTORPIN_LEFT, 0);
            gpio_put(MOTORPIN_RIGHT, 0);
        }

        sleep_ms(50); // Add delay between frames
    }
}
