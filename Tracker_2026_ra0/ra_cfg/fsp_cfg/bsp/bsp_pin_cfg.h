/* generated configuration header file - do not edit */
#ifndef BSP_PIN_CFG_H_
#define BSP_PIN_CFG_H_
#include "r_ioport.h"

/* Common macro for FSP header files. There is also a corresponding FSP_FOOTER macro at the end of this file. */
FSP_HEADER

#define MCU_LORA_RXEN (BSP_IO_PORT_00_PIN_14)
#define MCU_LORA_NSS (BSP_IO_PORT_00_PIN_15)
#define MCU_LORA_SI (BSP_IO_PORT_01_PIN_00)
#define MCU_LORA_SO (BSP_IO_PORT_01_PIN_01)
#define MCU_LORA_SCK (BSP_IO_PORT_01_PIN_02)
#define MCU_LED (BSP_IO_PORT_01_PIN_03)
#define MCU_GNSS_TXD (BSP_IO_PORT_01_PIN_09)
#define MCU_GNSS_RXD (BSP_IO_PORT_01_PIN_10)
#define MCU_LORA_RST (BSP_IO_PORT_01_PIN_12)
#define MCU_LORA_TXEN (BSP_IO_PORT_02_PIN_01)
#define MCU_RST (BSP_IO_PORT_02_PIN_06)
#define MCU_LORA_DIO1 (BSP_IO_PORT_02_PIN_07)
#define MCU_LORA_BUSY (BSP_IO_PORT_02_PIN_08)

#define PIN_SCK00 (BSP_IO_PORT_01_PIN_02)
#define CFG_SCK00 ((uint32_t) IOPORT_CFG_PERIPHERAL_PIN | (uint32_t) IOPORT_PERIPHERAL_SAU3_OUT)

#define PIN_SO00 (BSP_IO_PORT_01_PIN_01)
#define CFG_SO00 ((uint32_t) IOPORT_CFG_PERIPHERAL_PIN | (uint32_t) IOPORT_PERIPHERAL_SAU3_OUT)

extern const ioport_cfg_t g_bsp_pin_cfg; /* R7FA0E2094CFJ.pincfg */

void BSP_PinConfigSecurityInit();

/* Common macro for FSP header files. There is also a corresponding FSP_HEADER macro at the top of this file. */
FSP_FOOTER
#endif /* BSP_PIN_CFG_H_ */
