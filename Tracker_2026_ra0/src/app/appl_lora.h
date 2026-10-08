/**
 * \file appl_lora.h
 * \brief Public API for application LoRa radio operations.
 * \details Declares radio initialization, profile selection, and packet APIs.
 * \author Kaushik Ray
 * \date Last Modified: 30 Sep 2026
 * \copyright (C) 2026 Siliconbrane Inc. All rights reserved.
 * \note Proprietary source. Unauthorized use or distribution is prohibited.
 */

#ifndef APP_APPL_LORA_H_
#define APP_APPL_LORA_H_

#include <stdbool.h>
#include <stdint.h>
#include "r_external_irq_api.h"
#include "appl_flood_mesh.h"

#define APPL_LORA_RX_WINDOW_MS 500U

extern volatile bool tx_dynamic_busy;

typedef enum appl_lora_mode
{
    LORA_PARING    = 0x00,
    LORA_NORMAL    = 0x01,
    LORA_LONG      = 0x02,
} appl_lora_mode_type_t;

/* ============================================================================
 * FUNCTION PROTOTYPES
 * ============================================================================ */

/**
 * @brief  Callback function for MCU_LORA_DIO1 external IRQ.
 * @param  p_args Pointer to external IRQ callback arguments.
 */
void lora_dio1_callback(external_irq_callback_args_t *p_args);

/**
 * @brief  Initializes the SX126x radio with settings compatible with SX127x (V1).
 */
void lora_radio_init(void);
void appl_lora_tx_long_range(void);
void appl_lora_tx_mid_range(void);
void appl_lora_tx_short_range(void);

/**
 * @brief  Transmits 20 bytes of data from tx_buffer via LoRa.
 */
void lora_send_20_bytes(void);

/**
 * @brief Queue a 20-byte SX126x transmission without waiting for completion.
 * @param data20 Payload to copy into the transmit queue; NULL is ignored.
 */
void lora_tx_packet_it(const uint8_t *data20);

/** @brief Advance the nonblocking radio state machine; call periodically from the app scheduler. */
void appl_lora_tick(void);

#endif /* APP_APPL_LORA_H_ */

