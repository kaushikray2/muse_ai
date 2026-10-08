/**
 * \file appl_main.c
 * \brief Application entry point and periodic task scheduler.
 * \details Initializes application services and dispatches scheduled tasks.
 * \author Kaushik Ray
 * \date Last Modified: 30 Sep 2026
 * \copyright (C) 2026 Siliconbrane Inc. All rights reserved.
 * \note Proprietary source. Unauthorized use or distribution is prohibited.
 */

#include "appl_headers.h"
#include "test_scheduler.h"

/* Local Function Prototypes */
void app_main_ini(void);
void init_rtc(rtc_ctrl_t *p_ctrl, rtc_cfg_t const *p_cfg);

// volatile uint32_t system_events = 0; // Bitmask for hardware flags

// Event Flag Definitions
// #define EVENT_RTC_WAKEUP (1 << 0)
// #define EVENT_GNSS_READY (1 << 1)
// #define EVENT_LORA_DIO1 (1 << 2)

/**
 * \brief Main application entry point with task scheduler
 *
 * \details Initializes the system and runs the main scheduler loop. The
 * scheduler operates on a 0.5ms tick basis, executing 1ms and 5ms tasks
 * according to the scheduler counter. Loops indefinitely executing periodic
 * tasks. Returns only on system reset.
 *
 * \return Does not return in normal operation
 */

int app_main(void) {
  static uint8_t scheduler_counter = 0u;
  static uint8_t scheduler_100ms_counter = 0u;

  /* 1 - Init the peripherals */
  app_main_ini();

  /* 2 - Read the Data flash and update the system Flags */

  /* endless loop */
  for (;;) {
    if (1u == service_timer_0_5_ms_due()) {
      scheduler_counter++;

      /* execute 0.5ms scheduler */
      // service_voltage_tick();

      /* execute 1ms scheduler */
      if (0 == scheduler_counter % 2u) /* 1ms */
      {
        // service_watchdog_tick(); /* call this function every 1ms to ensure
        //  the correct windowed trigger for WDT */
      }

      /* execute 5ms scheduler */
      switch (scheduler_counter % 10u) {
      case 0u:
        service_gpio_blink_led_1hz(); /* Blink LED on port 6 pin 7 at 1Hz
                                       */
        //service_gps_tick();
        break;
      case 1u:
        //service_telemetry_tick(); /* Telemetry manages internal 60s interval(called every 5ms) */

        break;
      case 2u:
        //appl_lora_tick();
        break;
      case 3u:
        //service_adc_process_battery_voltage_5ms();
        break;
      case 4u: {
              static uint32_t sleep_30s_ticks = 0u;

              /* Case 4 runs every 5ms, so 6000 ticks equal 30 seconds awake */
              sleep_30s_ticks++;
              if (sleep_30s_ticks >= 6000u) {
                sleep_30s_ticks = 0u;

                /* 1. Stop the 0.5ms scheduler timer & disable its IRQ line */
                R_TAU_Stop(&g_timer0_ctrl);
                R_BSP_IrqDisable(g_timer0_cfg.cycle_end_irq);
                R_BSP_IrqClearPending(g_timer0_cfg.cycle_end_irq);

                /* 2. Disable external IRQs */
                R_ICU_ExternalIrqDisable(&g_external_irq0_ctrl);

                /* 3. Compute wake time = now + 60s (absolute RTC alarm).
                   No seconds reset: the alarm matches on sec+min+hour, so it
                   fires exactly 60s from now regardless of the current
                   seconds value. */
                rtc_time_t current_time;
                R_RTC_C_CalendarTimeGet(&g_rtc0_ctrl, &current_time);

                uint8_t wake_sec  = current_time.tm_sec;
                uint8_t wake_min  = current_time.tm_min + 1u; /* +60s */
                uint8_t wake_hour = current_time.tm_hour;
                if (wake_min >= 60u) {
                  wake_min -= 60u;
                  wake_hour++;
                  if (wake_hour >= 24u) {
                    wake_hour = 0u;
                  }
                }

                rtc_alarm_time_t alarm_time;
                alarm_time.time.tm_sec  = wake_sec;
                alarm_time.time.tm_min  = wake_min;
                alarm_time.time.tm_hour = wake_hour;
                alarm_time.time.tm_mday = current_time.tm_mday;
                alarm_time.time.tm_mon  = current_time.tm_mon;
                alarm_time.time.tm_year = current_time.tm_year;
                alarm_time.time.tm_wday = current_time.tm_wday;
                alarm_time.sec_match  = true;
                alarm_time.min_match  = true;
                alarm_time.hour_match = true;
                alarm_time.mday_match = false;
                alarm_time.mon_match  = false;
                alarm_time.year_match = false;
                alarm_time.enb        = true;
                R_RTC_C_CalendarAlarmSet(&g_rtc0_ctrl, &alarm_time);

                /* 4. Clear any pending alarm IRQ & enable its NVIC line */
                R_BSP_IrqClearPending(g_rtc0_cfg.alarm_irq);
                R_BSP_IrqEnable(g_rtc0_cfg.alarm_irq);

                /* 5. Sleep until the alarm fires (~60s) */
                __DSB();
                __WFI();
                __ISB();

                /* 6. Wake: shut off the alarm IRQ, then RESTART the
                   scheduler timer (it was stopped in step 1). */
                R_BSP_IrqDisable(g_rtc0_cfg.alarm_irq);
                R_BSP_IrqClearPending(g_rtc0_cfg.alarm_irq);
                R_TAU_Start(&g_timer0_ctrl);
                R_BSP_IrqClearPending(g_timer0_cfg.cycle_end_irq);
                R_BSP_IrqEnable(g_timer0_cfg.cycle_end_irq);
              }
            } break;
      case 5u:
        break;
      case 6u:
        break;
      case 7u:
        break;
      case 8u:
        break;
      case 9u:
        break;
      default:
        break;
      }

      if (scheduler_counter >= 200u) /* 100ms */
      {
        scheduler_counter = 0u;
        scheduler_100ms_counter++;

        if (scheduler_100ms_counter >= 10u) /* 1 second */
        {
          scheduler_100ms_counter = 0u;
          test_scheduler_tick();
        }
      }
    } /* end 0.5ms Task Cycle */
    else 
    {
      //__WFI();
    }
  } /* end for */
  return 0;
} /* end main */

/**
 * \brief Trigger a CPU reset via watch-dog timer
 *
 * \details Initiates a microcontroller reset by starting the watch-dog timer
 * with a very short timeout. Used for emergency system resets. Function blocks
 *          indefinitely waiting for watch-dog to reset the chip.
 */
void app_main_cpu_reset(void) {
  // service_watchdog_trigger_reset();
  while (1) {
    __WFI(); // Wait for reset
  }
}

/**
 * \brief Initialize application layer
 *
 * \details Performs application layer initialization. Placeholder for
 * application-specific setup routines. Can be extended with additional
 * initialization as needed.
 */
void app_main_ini(void) {
  /* Initialize IOPORT driver */
  R_IOPORT_Open(&g_ioport_ctrl, &g_bsp_pin_cfg);

  /* Initialize ELC event routing before starting peripherals that may use it.
   */
//  if (FSP_SUCCESS == R_ELC_Open(&g_elc_ctrl, &g_elc_cfg)) {
//    (void)R_ELC_Enable(&g_elc_ctrl);
//  }

  /* Initialize and start TAU timer */
  R_TAU_Open(&g_timer0_ctrl, &g_timer0_cfg);
  R_TAU_Start(&g_timer0_ctrl);

  //R_SAU_UART_Open(&g_uart0_ctrl, &g_uart0_cfg);
  //R_FLASH_LP_Open(&g_flash0_ctrl, &g_flash0_cfg);

  //R_SAU_SPI_Open(&g_spi0_ctrl, &g_spi0_cfg);

  /* Open the external IRQ module to apply your FSP settings */
  //R_ICU_ExternalIrqOpen(&g_external_irq0_ctrl, &g_external_irq0_cfg);

  /* Initialize the RTC once, but leave its IRQs disabled until case 4
     arms the 60-second alarm before WFI. */
  init_rtc(&g_rtc0_ctrl, &g_rtc0_cfg);

  service_timer_ini(); /* Reset and initialize the application timer state
                          before launching the scheduler. */
  test_scheduler_init();
  //appl_flood_mesh_init();
  //service_telemetry_init();
  //lora_radio_init();
  // appl_gps_confiv_m10_init();

  //UBLOX_WakeUp();

  /* Enable the interrupt after the radio has been configured. */
  //R_ICU_ExternalIrqEnable(&g_external_irq0_ctrl);
}

/* Initialize the RTC counter without enabling its periodic wake-up IRQ. */
void init_rtc(rtc_ctrl_t *p_ctrl, rtc_cfg_t const *p_cfg) {
  rtc_time_t initial_time = {0};
  initial_time.tm_sec = 0;
  initial_time.tm_min = 0;
  initial_time.tm_hour = 12;
  initial_time.tm_mday = 1;
  initial_time.tm_mon = 0;            // January (0-11)
  initial_time.tm_year = 2026 - 1900; // Years since 1900
  initial_time.tm_wday = 4;           // Thursday (0-6)

  R_RTC_C_Open(p_ctrl, p_cfg);
  R_RTC_C_CalendarTimeSet(p_ctrl, &initial_time);

  /* R_RTC_C_Open enables the RTC IRQs. Disable them here so the first
     wake-up is armed only when case 4 sets the 60-second alarm before WFI. */
  R_BSP_IrqDisable(p_cfg->periodic_irq);
  R_BSP_IrqDisable(p_cfg->alarm_irq);
}
