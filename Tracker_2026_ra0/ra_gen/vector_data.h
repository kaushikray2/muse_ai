/* generated vector header file - do not edit */
#ifndef VECTOR_DATA_H
#define VECTOR_DATA_H
#ifdef __cplusplus
        extern "C" {
        #endif
/* Number of interrupts allocated */
#ifndef VECTOR_DATA_IRQ_COUNT
#define VECTOR_DATA_IRQ_COUNT    (8)
#endif
/* ISR prototypes */
void r_icu_isr(void);
void fcu_frdyi_isr(void);
void sau_uart_txi_isr(void);
void sau_uart_rxi_isr(void);
void sau_spi_txrxi_isr(void);
void tau_tmi_isr(void);
void adc_d_scan_end_isr(void);
void rtc_c_alarm_prd_or_alm_isr(void);

/* Vector table allocations */
#define VECTOR_NUMBER_ICU_IRQ2 ((IRQn_Type) 4) /* ICU IRQ2 (External pin interrupt 2) */
#define ICU_IRQ2_IRQn          ((IRQn_Type) 4) /* ICU IRQ2 (External pin interrupt 2) */
#define VECTOR_NUMBER_FCU_FRDYI ((IRQn_Type) 11) /* FCU FRDYI (Flash ready interrupt) */
#define FCU_FRDYI_IRQn          ((IRQn_Type) 11) /* FCU FRDYI (Flash ready interrupt) */
#define VECTOR_NUMBER_SAU1_UART_TXI2 ((IRQn_Type) 12) /* SAU1 UART TXI2 (SAU UART TX 2/I2C 20/SPI 20) */
#define SAU1_UART_TXI2_IRQn          ((IRQn_Type) 12) /* SAU1 UART TXI2 (SAU UART TX 2/I2C 20/SPI 20) */
#define VECTOR_NUMBER_SAU1_UART_RXI2 ((IRQn_Type) 13) /* SAU1 UART RXI2 (SAU UART RX 2/I2C 21/SPI 21) */
#define SAU1_UART_RXI2_IRQn          ((IRQn_Type) 13) /* SAU1 UART RXI2 (SAU UART RX 2/I2C 21/SPI 21) */
#define VECTOR_NUMBER_SAU0_SPI_TXRXI00 ((IRQn_Type) 18) /* SAU0 SPI TXRXI00 (SAU UART TX 0/I2C 00/SPI 00) */
#define SAU0_SPI_TXRXI00_IRQn          ((IRQn_Type) 18) /* SAU0 SPI TXRXI00 (SAU UART TX 0/I2C 00/SPI 00) */
#define VECTOR_NUMBER_TAU0_TMI00 ((IRQn_Type) 19) /* TAU0 TMI00 (End of timer channel 00 count or capture) */
#define TAU0_TMI00_IRQn          ((IRQn_Type) 19) /* TAU0 TMI00 (End of timer channel 00 count or capture) */
#define VECTOR_NUMBER_ADC0_SCAN_END ((IRQn_Type) 31) /* ADC0 SCAN END (End of A/D scanning operation) */
#define ADC0_SCAN_END_IRQn          ((IRQn_Type) 31) /* ADC0 SCAN END (End of A/D scanning operation) */
#define VECTOR_NUMBER_RTC_ALARM_OR_PERIOD ((IRQn_Type) 32) /* RTC ALARM OR PERIOD (Alarm or Periodic interrupt) */
#define RTC_ALARM_OR_PERIOD_IRQn          ((IRQn_Type) 32) /* RTC ALARM OR PERIOD (Alarm or Periodic interrupt) */
/* The number of entries required for the ICU vector table. */
#define BSP_ICU_VECTOR_NUM_ENTRIES (33)

#ifdef __cplusplus
        }
        #endif
#endif /* VECTOR_DATA_H */
