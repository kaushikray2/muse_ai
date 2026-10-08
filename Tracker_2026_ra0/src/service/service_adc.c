
#include "service_headers.h"

/* 
 * Configuration Constants 
 * Adjust these based on your specific hardware (e.g. 12-bit ADC = 4095)
 */
#define ADC_NUM_SAMPLES      5
#define ADC_MAX_VAL          4095 
#define VREF_MV              3300 // 3.3V reference in millivolts

/* 
 * Voltage Divider Multiplier 
 * If measuring a battery > 3.3V, you likely have a voltage divider.
 * e.g., If using a 1/2 divider, this would be 2. If measuring directly, set to 1.
 */
#define VOLTAGE_DIVIDER_MULT 1

typedef enum {
    BATT_STATE_INIT = 0,
    BATT_STATE_START_SCAN,
    BATT_STATE_WAIT_SCAN,
    BATT_STATE_CALCULATE
} batt_state_t;

static batt_state_t g_batt_state   = BATT_STATE_INIT;
static uint32_t     g_adc_sum      = 0;
static uint8_t      g_sample_count = 0;
static adc_cfg_t g_adc_runtime_cfg;
static adc_d_extended_cfg_t g_adc_runtime_extend;

fsp_err_t service_adc_open(void)
{
    g_adc_runtime_cfg = g_adc0_cfg;
    g_adc_runtime_extend = *(adc_d_extended_cfg_t const *) g_adc0_cfg.p_extend;
    g_adc_runtime_cfg.p_extend = &g_adc_runtime_extend;

    return R_ADC_D_Open(&g_adc0_ctrl, &g_adc_runtime_cfg);
}

fsp_err_t service_adc_scan_cfg(void)
{
    return R_ADC_D_ScanCfg(&g_adc0_ctrl, &g_adc0_channel_cfg);
}

fsp_err_t service_adc_scan_start(void)
{
    return R_ADC_D_ScanStart(&g_adc0_ctrl);
}

fsp_err_t service_adc_read(adc_channel_t channel, uint16_t * p_data)
{
    return R_ADC_D_Read(&g_adc0_ctrl, channel, p_data);
}

fsp_err_t service_adc_close(void)
{
    return R_ADC_D_Close(&g_adc0_ctrl);
}

/* Final calculated battery voltage in mV */
uint32_t g_battery_mv = 0; 

/* Volatile flag set by the callback (acts as your ISR flag) */
volatile bool g_adc_scan_complete = false;

/* 
 * FSP ADC Callback 
 * Configure this function name in your FSP configuration under the ADC module interrupts.
 */
void g_adc0_callback(adc_callback_args_t * p_args)
{
    if ((NULL != p_args) && (ADC_EVENT_SCAN_COMPLETE == p_args->event))
    {
        g_adc_scan_complete = true;
    }
}

/*
 * Call this function every 5ms.
 * It will take 25ms total (5 ticks) to gather all 5 samples, 
 * which provides great natural filtering against high-frequency noise.
 */
void service_adc_process_battery_voltage_5ms(void)
{
    uint16_t current_adc_val = 0;

    switch (g_batt_state)
    {
        case BATT_STATE_INIT:
        {
            if (FSP_SUCCESS == service_adc_open())
            {
                if (FSP_SUCCESS == service_adc_scan_cfg())
                {
                    g_batt_state = BATT_STATE_START_SCAN;
                }
                else
                {
                    (void) service_adc_close();
                }
            }
            break;
        }

        case BATT_STATE_START_SCAN:
            g_adc_scan_complete = false;
            if (FSP_SUCCESS == service_adc_scan_start())
            {
                g_batt_state = BATT_STATE_WAIT_SCAN;
            }
            break;

        case BATT_STATE_WAIT_SCAN:
            if (g_adc_scan_complete)
            {
                /* Read the result (Replace ADC_CHANNEL_0 with your configured battery channel) */
                if (FSP_SUCCESS == service_adc_read(ADC_CHANNEL_0, &current_adc_val))
                {
                    g_adc_sum += current_adc_val;
                    g_sample_count++;

                    if (g_sample_count >= ADC_NUM_SAMPLES)
                    {
                        g_batt_state = BATT_STATE_CALCULATE;
                    }
                    else
                    {
                        g_batt_state = BATT_STATE_START_SCAN;
                    }
                }
            }
            /* If not complete, just wait for the next 5ms tick */
            break;

        case BATT_STATE_CALCULATE:
        {
            /* 1. Average the readings */
            uint32_t average_adc = g_adc_sum / ADC_NUM_SAMPLES;

            /* 2. Calculate millivolts using purely integer math to avoid floats.
             * V_batt = (ADC_AVG * VREF_MV) / ADC_MAX
             * Max intermediate value: 4095 * 3300 = 13,513,500 (Fits perfectly in a 32-bit uint) 
             */
            g_battery_mv = ((average_adc * VREF_MV) / ADC_MAX_VAL) * VOLTAGE_DIVIDER_MULT;

            /* Reset accumulators for the next measurement cycle */
            g_adc_sum      = 0;
            g_sample_count = 0;

            g_batt_state = BATT_STATE_START_SCAN;
            break;
        }
        
        default:
            g_batt_state = BATT_STATE_INIT;
            break;
    }
}
