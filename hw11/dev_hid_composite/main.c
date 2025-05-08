/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"


#include "bsp/board_api.h"
#include "tusb.h"
#include <math.h>
#include "usb_descriptors.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void led_blinking_task(void);
void hid_task(bool up_state, bool right_state, bool down_state, bool left_state, bool state);

/*------------- MAIN -------------*/
int main(void)
{
 // board_init();
  stdio_init_all();



  gpio_init(16);
  gpio_set_dir(16, GPIO_IN);
  gpio_pull_up(16);

  gpio_init(17);
  gpio_set_dir(17, GPIO_IN);
  gpio_pull_up(17);

  gpio_init(18);
  gpio_set_dir(18, GPIO_IN);
  gpio_pull_up(18);

  gpio_init(14);
  gpio_set_dir(14, GPIO_IN);
  gpio_pull_up(14);

  gpio_init(13);
  gpio_set_dir(13, GPIO_IN);
  gpio_pull_up(13);

  gpio_init(15);
  gpio_set_dir(15, GPIO_OUT);



  // init device stack on configured roothub port
  tud_init(BOARD_TUD_RHPORT);

  if (board_init_after_tusb) {
    board_init_after_tusb();
  }

  bool state = false;
  while (1)
  {
    tud_task(); // tinyusb device task
    led_blinking_task();

    bool right_state = !gpio_get(16);
    bool up_state = !gpio_get(17);
    bool down_state = !gpio_get(18);
    bool left_state = !gpio_get(14);
    bool click_state = !gpio_get(13);

    if (click_state) {
      state = !state;
      sleep_ms(50);
    }

    if (state) {
      gpio_put(15, true);
    } else {
      gpio_put(15, false);
    }

    hid_task(up_state, right_state, down_state, left_state, state);
  }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_NOT_MOUNTED;
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

static void send_hid_report(bool up_state, bool right_state, bool down_state, bool left_state, bool state)
{
  // skip if hid is not ready yet
  if ( !tud_hid_ready() ) return;

  int8_t vertical_delta;
  int8_t horizontal_delta;
  int8_t button_state;

  if (up_state & down_state) {
    vertical_delta = 0;
  } else if (up_state) {
    vertical_delta = -5;
  } else if (down_state) {
    vertical_delta = 5;
  } else {
    vertical_delta = 0;
  }

  if (right_state & left_state) {
    horizontal_delta = 0;
  } else if (right_state) {
    horizontal_delta = 5;
  } else if (left_state) {
    horizontal_delta = -5;
  } else {
    horizontal_delta = 0;
  }


  if (state) {
    horizontal_delta = sin(rand()) * 20;
    vertical_delta = cos(rand()) * 20;
  }



  // no button, right + down, no scroll, no pan
  tud_hid_mouse_report(REPORT_ID_MOUSE, button_state, horizontal_delta, vertical_delta, 0, 0);
}

// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
void hid_task(bool up_state, bool right_state, bool down_state, bool left_state, bool state)
{
  // Poll every 10ms
  const uint32_t interval_ms = 10;
  static uint32_t start_ms = 0;

  if ( board_millis() - start_ms < interval_ms) return; // not enough time
  start_ms += interval_ms;

  uint32_t const btn = board_button_read();

  // Remote wakeup
  if ( tud_suspended() && btn )
  {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    tud_remote_wakeup();
  }else
  {
    // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
    send_hid_report(up_state, right_state, down_state, left_state, state);
  }
}


// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  (void) instance;

  if (report_type == HID_REPORT_TYPE_OUTPUT)
  {
    // Set keyboard LED e.g Capslock, Numlock etc...
    if (report_id == REPORT_ID_KEYBOARD)
    {
      // bufsize should be (at least) 1
      if ( bufsize < 1 ) return;

      uint8_t const kbd_leds = buffer[0];

      if (kbd_leds & KEYBOARD_LED_CAPSLOCK)
      {
        // Capslock On: disable blink, turn led on
        blink_interval_ms = 0;
        board_led_write(true);
      }else
      {
        // Caplocks Off: back to normal blink
        board_led_write(false);
        blink_interval_ms = BLINK_MOUNTED;
      }
    }
  }
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // blink is disabled
  if (!blink_interval_ms) return;

  // Blink every interval ms
  if ( board_millis() - start_ms < blink_interval_ms) return; // not enough time
  start_ms += blink_interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
}
