#ifndef SERVICE_ADC_H_
#define SERVICE_ADC_H_

#include "service_headers.h"


/* 
 * Globally accessible battery voltage in millivolts.
 * Updated every 25ms (5 samples * 5ms tick).
 */
extern uint32_t g_battery_mv;

/**
 * @brief State machine tick for battery monitoring. 
 *        Must be called periodically every 5ms.
 */
fsp_err_t service_adc_open(void);
fsp_err_t service_adc_scan_cfg(void);
fsp_err_t service_adc_scan_start(void);
fsp_err_t service_adc_read(adc_channel_t channel, uint16_t * p_data);
fsp_err_t service_adc_close(void);

void service_adc_process_battery_voltage_5ms(void);

/**
 * @brief ADC callback to be triggered by the FSP interrupt.
 *        Ensure this function name matches the callback name 
 *        defined in your e2 studio / FSP configuration.
 * 
 * @param p_args FSP ADC callback arguments
 */
void g_adc0_callback(adc_callback_args_t * p_args);

#endif /* SERVICE_ADC_H_ */