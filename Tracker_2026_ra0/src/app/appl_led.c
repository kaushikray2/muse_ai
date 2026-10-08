/*
 * appl_led.c
 *
 *  Created on: Oct 7, 2026
 *      Author: kaushik
 */

#include "appl_headers.h"

/* GPIO state structure for LED control */
static struct {
  uint16_t led_blink_counter; /* Counter for LED 1Hz blinking (0-99, total
                                 1000ms per cycle) */
} gpio_state;

void service_gpio_blink_led_1hz(void) {

  static bsp_io_level_t level = BSP_IO_LEVEL_LOW;
  /* Increment counter every 5ms */
  gpio_state.led_blink_counter++;

  /* Check if counter has reached the limit for a full 1 second cycle (100 * 5ms
   * = 500ms per state) */
  if (gpio_state.led_blink_counter >= 100u) {
    gpio_state.led_blink_counter = 0u;
  }

  /* Toggle LED state every 500ms */
  if (gpio_state.led_blink_counter == 0u) {
    /* Toggle LED state */
    R_IOPORT_PinWrite(&g_ioport_ctrl, MCU_LED, level);
    level = (level == BSP_IO_LEVEL_LOW) ? BSP_IO_LEVEL_HIGH : BSP_IO_LEVEL_LOW;
  }
}
