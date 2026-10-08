/**
 * \file appl_tracker_sm.c
 * \brief High-level application state machine for PawPrint Tracker.
 */
#include "appl_tracker_sm.h"

extern volatile bool g_system_sleep_requested; // Sourced from app_main.c

typedef enum {
    TRACKER_STATE_INIT = 0,
    TRACKER_STATE_WAIT_GNSS,
    TRACKER_STATE_READ_BATTERY,
    TRACKER_STATE_LORA_TX,
    TRACKER_STATE_LORA_WAIT,
    TRACKER_STATE_GO_SLEEP,
    TRACKER_STATE_IDLE
} tracker_state_t;

static tracker_state_t current_state = TRACKER_STATE_INIT;

/* ---------------------------------------------------------
   Reset the state machine when the RTC wakes the device
--------------------------------------------------------- */
void appl_tracker_sm_reset(void)
{
    current_state = TRACKER_STATE_INIT;
}

/* ---------------------------------------------------------
   Tick Function - Called every 5ms by the scheduler
--------------------------------------------------------- */
void appl_tracker_sm_tick(void)
{
    switch (current_state)
    {
        case TRACKER_STATE_INIT:
            // Turn on the MOSFET powering the LC76G main VCC
            // service_gnss_power_on();

            // Re-init the GNSS software parser to clear old data
            service_gps_init();

            current_state = TRACKER_STATE_WAIT_GNSS;
            break;

        case TRACKER_STATE_WAIT_GNSS:
            /*
             * service_gps_tick() is processing UART data in another 5ms slot.
             * We just sit here and check if it found a good fix yet.
             */
            if (service_gps_is_valid())
            {
                const gps_data_t* p_gps = service_gps_get_data();

                // Wait until HDOP drops to 2.0 or better (20 in x10 scaling)
                if (p_gps->hdop_x10 > 0 && p_gps->hdop_x10 <= 20)
                {
                    current_state = TRACKER_STATE_READ_BATTERY;
                }
            }
            break;

        case TRACKER_STATE_READ_BATTERY:
            /*
             * Trigger the ADC to check the battery voltage.
             * If your ADC is polled or takes time, you might wait here.
             * Assuming service_adc_trigger_read() is non-blocking.
             */
            // service_adc_trigger_read();

            current_state = TRACKER_STATE_LORA_TX;
            break;

        case TRACKER_STATE_LORA_TX:
            /*
             * Pack the GPS coordinates, HDOP, and Battery into your 21-byte array
             * and push it to the Ebyte LoRa module via SPI.
             */
            // uint8_t payload[21];
            // build_telemetry_payload(payload);
            // appl_lora_send_packet(payload, sizeof(payload));

            current_state = TRACKER_STATE_LORA_WAIT;
            break;

        case TRACKER_STATE_LORA_WAIT:
            /*
             * Wait for the LoRa module to finish transmitting and completing
             * its RX window. Check whatever flag your LoRa driver sets.
             */
            // if (appl_lora_is_sequence_complete())
            // {
                 current_state = TRACKER_STATE_GO_SLEEP;
            // }
            break;

        case TRACKER_STATE_GO_SLEEP:
            /*
             * Signal to app_main.c that all business logic is done and
             * the system is ready to kill the timers and sleep.
             */
            g_system_sleep_requested = true;

            // Move to an IDLE state so we don't spam the sleep request
            current_state = TRACKER_STATE_IDLE;
            break;

        case TRACKER_STATE_IDLE:
            // Do nothing. Waiting for app_main to halt the timer.
            break;

        default:
            current_state = TRACKER_STATE_INIT;
            break;
    }
}
