/* generated HAL header file - do not edit */
#ifndef HAL_DATA_H_
#define HAL_DATA_H_
#include <stdint.h>
#include "bsp_api.h"
#include "common_data.h"
#include "r_rtc_c.h"
#include "r_rtc_api.h"
#include "r_adc_d.h"
#include "r_adc_api.h"
#include "r_sau_spi.h"
#include "r_spi_api.h"
#include "r_flash_lp.h"
#include "r_flash_api.h"
#include "r_dtc.h"
#include "r_transfer_api.h"
#include "r_sau_uart.h"
#include "r_uart_api.h"
#include "r_tau.h"
#include "r_timer_api.h"
FSP_HEADER
/* RTC Instance. */
extern const rtc_instance_t g_rtc0;

/** Access the RTC instance using these structures when calling API functions directly (::p_api is not used). */
extern rtc_c_instance_ctrl_t g_rtc0_ctrl;
extern const rtc_cfg_t g_rtc0_cfg;
extern const rtc_c_extended_cfg g_rtc0_cfg_extend;

#ifndef g_rtc0_callback
void g_rtc0_callback(rtc_callback_args_t *p_args);
#endif
/** ADC on ADC Instance. */
extern const adc_instance_t g_adc0;

/** Access the ADC instance using these structures when calling API functions directly (::p_api is not used). */
extern adc_d_instance_ctrl_t g_adc0_ctrl;
extern const adc_cfg_t g_adc0_cfg;
extern const adc_d_channel_cfg_t g_adc0_channel_cfg;

#ifndef g_adc0_callback
void g_adc0_callback(adc_callback_args_t *p_args);
#endif
/** SPI on SAU Instance. */
extern const spi_instance_t g_spi0;

/** Access the SAU_SPI instance using these structures when calling API functions directly (::p_api is not used). */
extern sau_spi_instance_ctrl_t g_spi0_ctrl;
extern const spi_cfg_t g_spi0_cfg;

/** Called by the driver when a transfer has completed or an error has occurred (Must be implemented by the user). */
#ifndef sau_spi_callback
void sau_spi_callback(spi_callback_args_t *p_args);
#endif
/* Flash on Flash LP Instance. */
extern const flash_instance_t g_flash0;

/** Access the Flash LP instance using these structures when calling API functions directly (::p_api is not used). */
extern flash_lp_instance_ctrl_t g_flash0_ctrl;
extern const flash_cfg_t g_flash0_cfg;

#ifndef g_flash_callback
void g_flash_callback(flash_callback_args_t *p_args);
#endif
/* Transfer on DTC Instance. */
extern const transfer_instance_t g_transfer1;

/** Access the DTC instance using these structures when calling API functions directly (::p_api is not used). */
extern dtc_instance_ctrl_t g_transfer1_ctrl;
extern const transfer_cfg_t g_transfer1_cfg;
/* Transfer on DTC Instance. */
extern const transfer_instance_t g_transfer0;

/** Access the DTC instance using these structures when calling API functions directly (::p_api is not used). */
extern dtc_instance_ctrl_t g_transfer0_ctrl;
extern const transfer_cfg_t g_transfer0_cfg;
/** UART on SAU Instance. */
extern const uart_instance_t g_uart0;

/** Access the UART instance using these structures when calling API functions directly (::p_api is not used). */
extern sau_uart_instance_ctrl_t g_uart0_ctrl;
extern const uart_cfg_t g_uart0_cfg;
extern const sau_uart_extended_cfg_t g_uart0_cfg_extend;

#ifndef g_uart0_callback
void g_uart0_callback(uart_callback_args_t *p_args);
#endif
/** TAU Timer Instance */
extern const timer_instance_t g_timer0;

/** Access the TAU instance using these structures when calling API functions directly (::p_api is not used). */
extern tau_instance_ctrl_t g_timer0_ctrl;
extern const timer_cfg_t g_timer0_cfg;

#ifndef timer0_callback
void timer0_callback(timer_callback_args_t *p_args);
#endif
void hal_entry(void);
void g_hal_init(void);
FSP_FOOTER
#endif /* HAL_DATA_H_ */
