/* generated HAL source file - do not edit */
#include "hal_data.h"
rtc_c_instance_ctrl_t g_rtc0_ctrl;

/** RTC_C extended configuration */
const rtc_c_extended_cfg g_rtc0_cfg_extend = { .clock_source_div =
		RTC_CLOCK_SOURCE_SUBCLOCK_DIV_BY_1, };

const rtc_cfg_t g_rtc0_cfg = { .p_err_cfg = NULL, .p_callback = g_rtc0_callback,
		.p_context = NULL, .periodic_ipl = (1), .alarm_irq = FSP_INVALID_VECTOR,
#if defined(VECTOR_NUMBER_RTC_ALARM_OR_PERIOD)
    .periodic_irq            = VECTOR_NUMBER_RTC_ALARM_OR_PERIOD,
#else
		.periodic_irq = FSP_INVALID_VECTOR,
#endif
		.p_extend = &g_rtc0_cfg_extend, };
/* Instance structure to use this module. */
const rtc_instance_t g_rtc0 = { .p_ctrl = &g_rtc0_ctrl, .p_cfg = &g_rtc0_cfg,
		.p_api = &g_rtc_on_rtc_c };
adc_d_instance_ctrl_t g_adc0_ctrl;

/* ADC conversion time 9.1875 us, conversion start time: 0.03125us */
const adc_d_extended_cfg_t g_adc0_cfg_extend = { .channel_mode =
		ADC_D_CHANNEL_MODE_SCAN, .operation_voltage =
		ADC_D_VOLTAGE_MODE_NORMAL_1, .conversion_clockdiv = ADC_D_CLOCK_DIV_1,
		.trigger_source = ADC_D_TRIGGER_SOURCE_SOFTWARE, .operation_trigger =
				ADC_D_TRIGGER_MODE_WAIT, .conversion_operation =
				ADC_D_CONVERSION_MODE_ONESHOT, .upper_lower_bound =
				ADC_D_BOUNDARY_IN_RANGE, .negative_vref =
				ADC_D_NEGATIVE_VREF_VSS, .positive_vref =
				ADC_D_POSITIVE_VREF_VCC, .upper_bound_limit = 255,
		.lower_bound_limit = 0, };
const adc_cfg_t g_adc0_cfg = { .unit = 0,
		.mode = (adc_mode_t) 0, // Unused
		.resolution = ADC_RESOLUTION_12_BIT,
		.alignment = (adc_alignment_t) 0, // Unused
		.trigger = (adc_trigger_t) 0, // Unused
		.p_callback = g_adc0_callback, .p_context = NULL, .p_extend =
				&g_adc0_cfg_extend,
#if defined(VECTOR_NUMBER_ADC0_SCAN_END)
    .scan_end_irq        = VECTOR_NUMBER_ADC0_SCAN_END,
#else
		.scan_end_irq = FSP_INVALID_VECTOR,
#endif
		.scan_end_ipl = (2), .scan_end_b_irq = FSP_INVALID_VECTOR,
		.scan_end_b_ipl = BSP_IRQ_DISABLED, };

const adc_d_channel_cfg_t g_adc0_channel_cfg =
		{ .channel_input = ADC_CHANNEL_0 };
/* Instance structure to use this module. */
const adc_instance_t g_adc0 = { .p_ctrl = &g_adc0_ctrl, .p_cfg = &g_adc0_cfg,
		.p_channel_cfg = &g_adc0_channel_cfg, .p_api = &g_adc_on_adc_d };
#include "r_sau_spi_cfg.h"
sau_spi_instance_ctrl_t g_spi0_ctrl;
#if SAU_SPI_CFG_DTC_SUPPORT_ENABLE
transfer_info_t RA_NOT_DEFINED_info[2] =
{
    { .transfer_settings_word_b.dest_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED,
      .transfer_settings_word_b.repeat_area = TRANSFER_REPEAT_AREA_DESTINATION,
      .transfer_settings_word_b.irq = TRANSFER_IRQ_END,
      .transfer_settings_word_b.chain_mode = TRANSFER_CHAIN_MODE_EACH,
      .transfer_settings_word_b.src_addr_mode = TRANSFER_ADDR_MODE_FIXED,
      .transfer_settings_word_b.size = TRANSFER_SIZE_1_BYTE,
      .transfer_settings_word_b.mode = TRANSFER_MODE_NORMAL,
      .p_dest = (void*) NULL,
      .p_src = (void const*) NULL,
      .num_blocks = 0,
      .length = 0, },
    { .transfer_settings_word_b.dest_addr_mode = TRANSFER_ADDR_MODE_FIXED,
      .transfer_settings_word_b.repeat_area = TRANSFER_REPEAT_AREA_DESTINATION,
      .transfer_settings_word_b.irq = TRANSFER_IRQ_END,
      .transfer_settings_word_b.chain_mode = TRANSFER_CHAIN_MODE_DISABLED,
      .transfer_settings_word_b.src_addr_mode = TRANSFER_ADDR_MODE_FIXED,
      .transfer_settings_word_b.size = TRANSFER_SIZE_1_BYTE,
      .transfer_settings_word_b.mode = TRANSFER_MODE_NORMAL,
      .p_dest = (void*) NULL,
      .p_src = (void const*) NULL,
      .num_blocks = 0,
      .length = 0, }
};
const transfer_cfg_t RA_NOT_DEFINED_cfg_sau_spi =
{
  .p_info              = RA_NOT_DEFINED_info,
  .p_extend = &RA_NOT_DEFINED_cfg_extend, };

/* Instance structure to use this module. */
const transfer_instance_t RA_NOT_DEFINED_sau_spi =
{
    .p_ctrl        = &RA_NOT_DEFINED_ctrl,
    .p_cfg         = &RA_NOT_DEFINED_cfg_sau_spi,
    .p_api         = &g_transfer_on_dtc
};

#endif
/** SPI extended configuration */
const sau_spi_extended_cfg_t g_spi0_cfg_extend = { .clk_div = {
/* Actual calculated bitrate: 100000 */
.stclk = 79, .operation_clock = SAU_SPI_OPERATION_CLOCK_CK0, }, .transfer_mode =
		SAU_SPI_TRANSFER_MODE_SINGLE, .data_phase =
		SAU_SPI_DATA_PHASE_HALF_CYCLE_START, .clock_phase =
		SAU_SPI_CLOCK_PHASE_REVERSE, .sau_unit = 0,
#if defined(PIN_SCK00)
    .sck_pin_settings.pin = PIN_SCK00,
#else
		.sck_pin_settings.pin = (bsp_io_port_pin_t) UINT16_MAX,
#endif
#if defined(CFG_SCK00)
    .sck_pin_settings.cfg = CFG_SCK00,
#else
		.sck_pin_settings.cfg = (uint32_t) IOPORT_CFG_PORT_DIRECTION_INPUT,
#endif
#if defined(PIN_SO00)
    .so_pin_settings.pin = PIN_SO00,
#else
		.so_pin_settings.pin = (bsp_io_port_pin_t) UINT16_MAX,
#endif
#if defined(CFG_SO00)
    .so_pin_settings.cfg = CFG_SO00,
#else
		.so_pin_settings.cfg = (uint32_t) IOPORT_CFG_PORT_DIRECTION_INPUT,
#endif
		};

const spi_cfg_t g_spi0_cfg = { .channel = 0, .operating_mode = SPI_MODE_MASTER,
		.bit_order = SPI_BIT_ORDER_MSB_FIRST, .p_callback = sau_spi_callback,
		.p_context = NULL,
#if defined(VECTOR_NUMBER_SAU0_SPI_TXRXI00)
    .tei_irq         = VECTOR_NUMBER_SAU0_SPI_TXRXI00,
#else
		.tei_irq = FSP_INVALID_VECTOR,
#endif
#define RA_NOT_DEFINED (1)
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
		.p_transfer_tx = NULL,
#else
    .p_transfer_tx   = &RA_NOT_DEFINED_sau_spi,
#endif
#undef RA_NOT_DEFINED
		.tei_ipl = (2), .p_extend = &g_spi0_cfg_extend, };
/* Instance structure to use this module. */
const spi_instance_t g_spi0 = { .p_ctrl = &g_spi0_ctrl, .p_cfg = &g_spi0_cfg,
		.p_api = &g_spi_on_sau };
flash_lp_instance_ctrl_t g_flash0_ctrl;
const flash_cfg_t g_flash0_cfg = { .data_flash_bgo = true, .p_callback =
		g_flash_callback, .p_context = NULL, .ipl = (3),
#if defined(VECTOR_NUMBER_FCU_FRDYI)
    .irq                 = VECTOR_NUMBER_FCU_FRDYI,
#else
		.irq = FSP_INVALID_VECTOR,
#endif
		};
/* Instance structure to use this module. */
const flash_instance_t g_flash0 = { .p_ctrl = &g_flash0_ctrl, .p_cfg =
		&g_flash0_cfg, .p_api = &g_flash_on_flash_lp };
dtc_instance_ctrl_t g_transfer1_ctrl;

#if (BSP_CFG_DCACHE_ENABLED) && (1 == 1)
const transfer_info_t g_transfer1_user_config_info =
{
    .transfer_settings_word_b.dest_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED,
    .transfer_settings_word_b.repeat_area    = TRANSFER_REPEAT_AREA_DESTINATION,
    .transfer_settings_word_b.irq            = TRANSFER_IRQ_END,
    .transfer_settings_word_b.chain_mode     = TRANSFER_CHAIN_MODE_DISABLED,
    .transfer_settings_word_b.src_addr_mode  = TRANSFER_ADDR_MODE_FIXED,
    .transfer_settings_word_b.size           = TRANSFER_SIZE_1_BYTE,
    .transfer_settings_word_b.mode           = TRANSFER_MODE_NORMAL,
    .p_dest                                  = (void *) NULL,
    .p_src                                   = (void const *) NULL,
    .num_blocks                              = (uint16_t) 0,
    .length                                  = (uint16_t) 0,
};
#endif

#if BSP_CFG_DCACHE_ENABLED
    #if (1 > 0)
    transfer_info_t g_transfer1_info_fsp_nocache[1] DTC_TRANSFER_INFO_ALIGNMENT;
    #else
    /* User must call api::reconfigure before enable DTC transfer. */
    #endif
#else
#if (1 == 1)
transfer_info_t g_transfer1_info DTC_TRANSFER_INFO_ALIGNMENT =
		{ .transfer_settings_word_b.dest_addr_mode =
				TRANSFER_ADDR_MODE_INCREMENTED,
				.transfer_settings_word_b.repeat_area =
						TRANSFER_REPEAT_AREA_DESTINATION,
				.transfer_settings_word_b.irq = TRANSFER_IRQ_END,
				.transfer_settings_word_b.chain_mode =
						TRANSFER_CHAIN_MODE_DISABLED,
				.transfer_settings_word_b.src_addr_mode =
						TRANSFER_ADDR_MODE_FIXED,
				.transfer_settings_word_b.size = TRANSFER_SIZE_1_BYTE,
				.transfer_settings_word_b.mode = TRANSFER_MODE_NORMAL, .p_dest =
						(void*) NULL, .p_src = (void const*) NULL, .num_blocks =
						(uint16_t) 0, .length = (uint16_t) 0, };
#elif (1 > 1)
    /* User is responsible to initialize the array. */
    transfer_info_t g_transfer1_info[1] DTC_TRANSFER_INFO_ALIGNMENT;
    #else
    /* User must call api::reconfigure before enable DTC transfer. */
    #endif
#endif

const dtc_extended_cfg_t g_transfer1_cfg_extend = { .activation_source =
		VECTOR_NUMBER_SAU1_UART_RXI2,

#if BSP_CFG_DCACHE_ENABLED
    #if (1 == 1)
        .p_user_config_info =  &g_transfer1_user_config_info,
    #else
        .p_user_config_info = NULL,
    #endif
#else
		/* p_user_config_info not present. */
#endif
		};

const transfer_cfg_t g_transfer1_cfg = {
#if BSP_CFG_DCACHE_ENABLED
    #if (1 > 0)
        .p_info              = g_transfer1_info_fsp_nocache,
    #else
        .p_info = NULL,
    #endif
#else
#if (1 == 1)
		.p_info = &g_transfer1_info,
#elif (1 > 1)
        .p_info              = g_transfer1_info,
    #else
        .p_info = NULL,
    #endif
#endif
		.p_extend = &g_transfer1_cfg_extend, };

/* Instance structure to use this module. */
const transfer_instance_t g_transfer1 = { .p_ctrl = &g_transfer1_ctrl, .p_cfg =
		&g_transfer1_cfg, .p_api = &g_transfer_on_dtc };
dtc_instance_ctrl_t g_transfer0_ctrl;

#if (BSP_CFG_DCACHE_ENABLED) && (1 == 1)
const transfer_info_t g_transfer0_user_config_info =
{
    .transfer_settings_word_b.dest_addr_mode = TRANSFER_ADDR_MODE_FIXED,
    .transfer_settings_word_b.repeat_area    = TRANSFER_REPEAT_AREA_SOURCE,
    .transfer_settings_word_b.irq            = TRANSFER_IRQ_END,
    .transfer_settings_word_b.chain_mode     = TRANSFER_CHAIN_MODE_DISABLED,
    .transfer_settings_word_b.src_addr_mode  = TRANSFER_ADDR_MODE_INCREMENTED,
    .transfer_settings_word_b.size           = TRANSFER_SIZE_1_BYTE,
    .transfer_settings_word_b.mode           = TRANSFER_MODE_NORMAL,
    .p_dest                                  = (void *) NULL,
    .p_src                                   = (void const *) NULL,
    .num_blocks                              = (uint16_t) 0,
    .length                                  = (uint16_t) 0,
};
#endif

#if BSP_CFG_DCACHE_ENABLED
    #if (1 > 0)
    transfer_info_t g_transfer0_info_fsp_nocache[1] DTC_TRANSFER_INFO_ALIGNMENT;
    #else
    /* User must call api::reconfigure before enable DTC transfer. */
    #endif
#else
#if (1 == 1)
transfer_info_t g_transfer0_info DTC_TRANSFER_INFO_ALIGNMENT =
		{ .transfer_settings_word_b.dest_addr_mode = TRANSFER_ADDR_MODE_FIXED,
				.transfer_settings_word_b.repeat_area =
						TRANSFER_REPEAT_AREA_SOURCE,
				.transfer_settings_word_b.irq = TRANSFER_IRQ_END,
				.transfer_settings_word_b.chain_mode =
						TRANSFER_CHAIN_MODE_DISABLED,
				.transfer_settings_word_b.src_addr_mode =
						TRANSFER_ADDR_MODE_INCREMENTED,
				.transfer_settings_word_b.size = TRANSFER_SIZE_1_BYTE,
				.transfer_settings_word_b.mode = TRANSFER_MODE_NORMAL, .p_dest =
						(void*) NULL, .p_src = (void const*) NULL, .num_blocks =
						(uint16_t) 0, .length = (uint16_t) 0, };
#elif (1 > 1)
    /* User is responsible to initialize the array. */
    transfer_info_t g_transfer0_info[1] DTC_TRANSFER_INFO_ALIGNMENT;
    #else
    /* User must call api::reconfigure before enable DTC transfer. */
    #endif
#endif

const dtc_extended_cfg_t g_transfer0_cfg_extend = { .activation_source =
		VECTOR_NUMBER_SAU1_UART_TXI2,

#if BSP_CFG_DCACHE_ENABLED
    #if (1 == 1)
        .p_user_config_info =  &g_transfer0_user_config_info,
    #else
        .p_user_config_info = NULL,
    #endif
#else
		/* p_user_config_info not present. */
#endif
		};

const transfer_cfg_t g_transfer0_cfg = {
#if BSP_CFG_DCACHE_ENABLED
    #if (1 > 0)
        .p_info              = g_transfer0_info_fsp_nocache,
    #else
        .p_info = NULL,
    #endif
#else
#if (1 == 1)
		.p_info = &g_transfer0_info,
#elif (1 > 1)
        .p_info              = g_transfer0_info,
    #else
        .p_info = NULL,
    #endif
#endif
		.p_extend = &g_transfer0_cfg_extend, };

/* Instance structure to use this module. */
const transfer_instance_t g_transfer0 = { .p_ctrl = &g_transfer0_ctrl, .p_cfg =
		&g_transfer0_cfg, .p_api = &g_transfer_on_dtc };
sau_uart_instance_ctrl_t g_uart0_ctrl;

sau_uart_baudrate_setting_t g_uart0_baud_setting = {
/* Actual calculated bitrate: 115942 */
.stclk = 68, .prs = 1, .operation_clock = SAU_UART_OPERATION_CLOCK_CK0, };

/** UART extended configuration for UARTonSAU HAL driver */
const sau_uart_extended_cfg_t g_uart0_cfg_extend = { .sequence =
		SAU_UART_DATA_SEQUENCE_LSB, .signal_level =
		SAU_UART_SIGNAL_LEVEL_STANDARD, .p_baudrate = &g_uart0_baud_setting, };

/** UART interface configuration */
const uart_cfg_t g_uart0_cfg = { .channel = 2, .data_bits = UART_DATA_BITS_8,
		.parity = UART_PARITY_OFF, .stop_bits = UART_STOP_BITS_1, .p_callback =
				g_uart0_callback, .rxi_ipl = (2), .txi_ipl = (2), .eri_ipl =
				(BSP_IRQ_DISABLED),
#if defined(VECTOR_NUMBER_SAU1_UART_RXI2)
                .rxi_irq         = VECTOR_NUMBER_SAU1_UART_RXI2,
#else
		.rxi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SAU1_UART_TXI2)
                .txi_irq         = VECTOR_NUMBER_SAU1_UART_TXI2,
#else
		.txi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SAU1_UART_ERRI2)
                .eri_irq         = VECTOR_NUMBER_SAU1_UART_ERRI2,
#else
		.eri_irq = FSP_INVALID_VECTOR,
#endif
		.p_context = NULL, .p_extend = &g_uart0_cfg_extend,

#define RA_NOT_DEFINED (1)
#if (RA_NOT_DEFINED == g_transfer0)
                .p_transfer_tx   = NULL,
#else
		.p_transfer_tx = &g_transfer0,
#endif
#if (RA_NOT_DEFINED == g_transfer1)
                .p_transfer_rx   = NULL,
#else
		.p_transfer_rx = &g_transfer1,
#endif
#undef RA_NOT_DEFINED
		};

/* Instance structure to use this module. */
const uart_instance_t g_uart0 = { .p_ctrl = &g_uart0_ctrl,
		.p_cfg = &g_uart0_cfg, .p_api = &g_uart_on_sau };
tau_instance_ctrl_t g_timer0_ctrl;
const tau_extended_cfg_t g_timer0_extend = { .opirq =
		TAU_INTERRUPT_OPIRQ_BIT_SET, .tau_func = TAU_FUNCTION_INTERVAL,
		.bit_mode = TAU_BIT_MODE_16BIT, .initial_output =
				TAU_PIN_OUTPUT_CFG_DISABLED, .input_source =
				TAU_INPUT_SOURCE_NONE, .tau_filter =
				TAU_INPUT_NOISE_FILTER_DISABLE, .trigger_edge =
				TAU_TRIGGER_EDGE_RISING, .operation_clock = TAU_OPERATION_CK00,
		/* Not used for 16-bit or lower 8-bit mode */
		.period_higher_8bit_counts = (uint16_t) 0x100,
		.higher_8bit_cycle_end_ipl = (BSP_IRQ_DISABLED),
#if defined(VECTOR_NUMBER_TAU0_TMI00H)
    .higher_8bit_cycle_end_irq       = VECTOR_NUMBER_TAU0_TMI00H,
#else
		.higher_8bit_cycle_end_irq = FSP_INVALID_VECTOR,
#endif
		};
const timer_cfg_t g_timer0_cfg =
		{ .mode = (timer_mode_t) 0,
				/* Actual Period: 0.0005000000 seconds. */
				/* Minimum Period ~ Maximum Period: 0.0000000625 ~ 0.00204800 seconds. */.period_counts =
						(uint32_t) 0x3e80, .duty_cycle_counts = 0, .source_div =
						(timer_source_div_t) BSP_CFG_TAU_CK00, .channel = 0,
				.p_callback = timer0_callback,
				/** If NULL then do not add & */
#if defined(NULL)
    .p_context           = NULL,
#else
				.p_context = (void*) &NULL,
#endif
				.p_extend = &g_timer0_extend, .cycle_end_ipl = (2),
#if defined(VECTOR_NUMBER_TAU0_TMI00)
    .cycle_end_irq       = VECTOR_NUMBER_TAU0_TMI00,
#else
				.cycle_end_irq = FSP_INVALID_VECTOR,
#endif
		};
/* Instance structure to use this module. */
const timer_instance_t g_timer0 = { .p_ctrl = &g_timer0_ctrl, .p_cfg =
		&g_timer0_cfg, .p_api = &g_timer_on_tau };
void g_hal_init(void) {
	g_common_init();
}
