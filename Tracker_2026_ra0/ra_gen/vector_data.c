/* generated vector source file - do not edit */
#include "bsp_api.h"
/* Do not build these data structures if no interrupts are currently allocated because IAR will have build errors. */
#if VECTOR_DATA_IRQ_COUNT > 0
        BSP_DONT_REMOVE const fsp_vector_t g_vector_table[BSP_ICU_VECTOR_NUM_ENTRIES] BSP_PLACE_IN_SECTION(BSP_SECTION_APPLICATION_VECTORS) =
        {
                        [4] = r_icu_isr, /* ICU IRQ2 (External pin interrupt 2) */
            [11] = fcu_frdyi_isr, /* FCU FRDYI (Flash ready interrupt) */
            [12] = sau_uart_txi_isr, /* SAU1 UART TXI2 (SAU UART TX 2/I2C 20/SPI 20) */
            [13] = sau_uart_rxi_isr, /* SAU1 UART RXI2 (SAU UART RX 2/I2C 21/SPI 21) */
            [18] = sau_spi_txrxi_isr, /* SAU0 SPI TXRXI00 (SAU UART TX 0/I2C 00/SPI 00) */
            [19] = tau_tmi_isr, /* TAU0 TMI00 (End of timer channel 00 count or capture) */
            [31] = adc_d_scan_end_isr, /* ADC0 SCAN END (End of A/D scanning operation) */
            [32] = rtc_c_alarm_prd_or_alm_isr, /* RTC ALARM OR PERIOD (Alarm or Periodic interrupt) */
        };
        #if BSP_FEATURE_ICU_HAS_IELSR
        const bsp_interrupt_event_t g_interrupt_event_link_select[BSP_ICU_VECTOR_NUM_ENTRIES] =
        {
            [4] = BSP_PRV_VECT_ENUM(EVENT_ICU_IRQ2,GROUP4), /* ICU IRQ2 (External pin interrupt 2) */
            [11] = BSP_PRV_VECT_ENUM(EVENT_FCU_FRDYI,GROUP3), /* FCU FRDYI (Flash ready interrupt) */
            [12] = BSP_PRV_VECT_ENUM(EVENT_SAU1_UART_TXI2,GROUP4), /* SAU1 UART TXI2 (SAU UART TX 2/I2C 20/SPI 20) */
            [13] = BSP_PRV_VECT_ENUM(EVENT_SAU1_UART_RXI2,GROUP5), /* SAU1 UART RXI2 (SAU UART RX 2/I2C 21/SPI 21) */
            [18] = BSP_PRV_VECT_ENUM(EVENT_SAU0_SPI_TXRXI00,GROUP2), /* SAU0 SPI TXRXI00 (SAU UART TX 0/I2C 00/SPI 00) */
            [19] = BSP_PRV_VECT_ENUM(EVENT_TAU0_TMI00,GROUP3), /* TAU0 TMI00 (End of timer channel 00 count or capture) */
            [31] = BSP_PRV_VECT_ENUM(EVENT_ADC0_SCAN_END,GROUP7), /* ADC0 SCAN END (End of A/D scanning operation) */
            [32] = BSP_PRV_VECT_ENUM(EVENT_RTC_ALARM_OR_PERIOD,FIXED), /* RTC ALARM OR PERIOD (Alarm or Periodic interrupt) */
        };
        #endif
        #endif
