/**
 * \file appl_lora.c
 * \brief Controls SX126x radio configuration and packet transfers.
 * \details Implements LoRa profiles, transmit/receive operations, and DIO1 handling.
 * \author Kaushik Ray
 * \date Last Modified: 30 Sep 2026
 * \copyright (C) 2026 Siliconbrane Inc. All rights reserved.
 * \note Proprietary source. Unauthorized use or distribution is prohibited.
 */

#include "hal_data.h"
#include "appl_lora.h"
#include "sx126x.h"
#include "service_timer.h"
#include <string.h>

/* ============================================================================
 * LORA CONFIGURATION MACROS
 * ============================================================================ */
#define TX_TIMEOUT_VALUE        3000        // 3 seconds (in ms)
#define TX_SWITCH_SETTLE_TIME_MS 1u
#define LORA_TX_QUEUE_LENGTH    4U

volatile bool tx_dynamic_busy = false;

typedef enum
{
    LORA_STATE_IDLE,
    LORA_STATE_TX_SETTLE,
    LORA_STATE_TX_WAIT,
    LORA_STATE_RX_WINDOW
} lora_state_t;

static volatile bool lora_irq_pending;
static lora_state_t lora_state;
static uint32_t tx_state_started_at_ms;
static uint32_t rx_window_started_at_ms;
static uint8_t tx_queue[LORA_TX_QUEUE_LENGTH][APPL_LORA_PACKET_SIZE];
static uint8_t tx_queue_head;
static uint8_t tx_queue_tail;
static uint8_t tx_queue_count;

/* Buffer configurations: Fixed string initialization warning */
uint8_t tx_buffer[20] = {'H','e','l','l','o',' ','L','o','R','a',' ','D','a','t','a','!','!','!','!','!'}; // Exact 20 Bytes

/* ============================================================================
 * RF SWITCH CONTROL HELPER FUNCTIONS
 * ============================================================================ */
static void rf_switch_set_tx(void)
{
    R_IOPORT_PinWrite(&g_ioport_ctrl, MCU_LORA_RXEN, BSP_IO_LEVEL_LOW);
    R_IOPORT_PinWrite(&g_ioport_ctrl, MCU_LORA_TXEN, BSP_IO_LEVEL_HIGH);
}

static void rf_switch_set_rx(void)
{
    R_IOPORT_PinWrite(&g_ioport_ctrl, MCU_LORA_TXEN, BSP_IO_LEVEL_LOW);
    R_IOPORT_PinWrite(&g_ioport_ctrl, MCU_LORA_RXEN, BSP_IO_LEVEL_HIGH);
}

static void rf_switch_set_idle(void)
{
    R_IOPORT_PinWrite(&g_ioport_ctrl, MCU_LORA_TXEN, BSP_IO_LEVEL_LOW);
    R_IOPORT_PinWrite(&g_ioport_ctrl, MCU_LORA_RXEN, BSP_IO_LEVEL_LOW);
}

static uint32_t lora_elapsed_ms(uint32_t start_time)
{
    return service_timer_get_system_time_1ms() - start_time;
}

/* ============================================================================
 * INTERRUPT CALLBACK (Uncommented to allow TX/RX completions to exit loops)
 * ============================================================================ */
void lora_dio1_callback(external_irq_callback_args_t *p_args)
{
    (void) p_args;
    lora_irq_pending = true;
}

//void sau_spi_callback(spi_callback_args_t * p_args)
//{
//    if (SPI_EVENT_TRANSFER_COMPLETE == p_args->event)
//    {
//    	g_lora_tx_done = true;
//    }
//}



/* ============================================================================
 * LORA INITIALIZATION (V2 SX126x configured to match V1 SX127x settings)
 * ============================================================================ */
void lora_radio_init(void)
{
	    /* 1. Hardware Reset & Wakeup */
	    sx126x_reset(NULL);
	    sx126x_wakeup(NULL);

	    /* 2. Set Standby Mode (STDBY_RC) */
	    sx126x_set_standby(NULL, SX126X_STANDBY_CFG_RC);

	    /* 3. Configure DIO3 to supply power to the TCXO (Critical for E22)
	     * Voltage: 3.3V, Delay: 320 steps (320 * 15.625 us = 5 ms)
	     */
	    sx126x_set_dio3_as_tcxo_ctrl(NULL, SX126X_TCXO_CTRL_3_3V, 320);

	    /* 4. Enable internal DC-DC converter (Crucial for power stability) */
	    sx126x_set_reg_mode(NULL, SX126X_REG_MODE_DCDC);

	    /* 5. Set Packet Type to LoRa */
	    sx126x_set_pkt_type(NULL, SX126X_PKT_TYPE_LORA);

	    /* 6. Frequency Configuration */
	    sx126x_set_rf_freq(NULL, 915000000UL); // 915 MHz

	    /* 7. Calibrate RF Image for 900MHz band (0xE1 to 0xE9) */
	    sx126x_cal_img(NULL, 0xE1, 0xE9);

	    /* 8. Enable Internal RF Switch (DIO2) */
	    sx126x_set_dio2_as_rf_sw_ctrl(NULL, true);

	    /* Configure PA for SX1262
	     * Required by standard C drivers to properly stage the output power
	     */
	    sx126x_pa_cfg_params_t pa_cfg = {
	        .pa_duty_cycle = 0x04,
	        .hp_max        = 0x07,
	        .device_sel    = 0x00, // SX1262
	        .pa_lut        = 0x01
	    };
	    sx126x_set_pa_cfg(NULL, &pa_cfg);

	    /* 9. Set TX Power (10 dBm) and Ramp Time (40us) */
	    sx126x_set_tx_params(NULL, 22, SX126X_RAMP_40_US);  /* Kaushik: Controls the power of tx 0 = 0db can be changed up to 22db */

	    /* 10. Buffer Base Addresses (TX and RX overlapping at 0x00) */
	    sx126x_set_buffer_base_address(NULL, 0x00, 0x00);

	    /* 11. Modulation Parameters (SF7, BW 125kHz, CR 4/5) */
	    sx126x_mod_params_lora_t mod_params = {
	        .sf   = SX126X_LORA_SF7,
	        .bw   = SX126X_LORA_BW_125,
	        .cr   = SX126X_LORA_CR_4_5,
	        .ldro = 0x00 // Low Data Rate Optimization OFF
	    };
	    sx126x_set_lora_mod_params(NULL, &mod_params);

	    /* 12. Packet Parameters (20 Bytes Payload, 8 Preamble) */
	    sx126x_pkt_params_lora_t pkt_params = {
	        .preamble_len_in_symb = 8,
	        .header_type          = SX126X_LORA_PKT_EXPLICIT,
	        .pld_len_in_bytes     = 20,
	        .crc_is_on            = true,
	        .invert_iq_is_on      = false
	    };
	    sx126x_set_lora_pkt_params(NULL, &pkt_params);

	    /* Set LoRa Sync Word (Match RFM95 / SX127x default 0x12) */
	    sx126x_set_lora_sync_word(NULL, 0x1424);

	    /* 13. Configure Interrupts (Enable TxDone IRQ on DIO1) */
	    sx126x_set_dio_irq_params(NULL,
	                              SX126X_IRQ_TX_DONE, // Global IRQ mask (Enable TxDone)
	                              SX126X_IRQ_TX_DONE, // DIO1 Mask (Route TxDone to DIO1 pin)
	                              SX126X_IRQ_NONE,    // DIO2 Mask
	                              SX126X_IRQ_NONE);   // DIO3 Mask
}

void appl_lora_tx_long_range(void)
{
    sx126x_mod_params_lora_t mod_params = {
        .sf   = SX126X_LORA_SF12,
        .bw   = SX126X_LORA_BW_125,
        .cr   = SX126X_LORA_CR_4_8,
        .ldro = 1u
    };

    if ((LORA_STATE_IDLE != lora_state) || (0U != tx_queue_count))
    {
        return;
    }

    sx126x_set_standby(NULL, SX126X_STANDBY_CFG_RC);
    sx126x_set_lora_mod_params(NULL, &mod_params);
}

void appl_lora_tx_mid_range(void)
{
    sx126x_mod_params_lora_t mod_params = {
        .sf   = SX126X_LORA_SF7,
        .bw   = SX126X_LORA_BW_125,
        .cr   = SX126X_LORA_CR_4_5,
        .ldro = 0u
    };

    if ((LORA_STATE_IDLE != lora_state) || (0U != tx_queue_count))
    {
        return;
    }

    sx126x_set_standby(NULL, SX126X_STANDBY_CFG_RC);
    sx126x_set_lora_mod_params(NULL, &mod_params);
}

void appl_lora_tx_short_range(void)
{
    sx126x_mod_params_lora_t mod_params = {
        .sf   = SX126X_LORA_SF5,
        .bw   = SX126X_LORA_BW_500,
        .cr   = SX126X_LORA_CR_4_5,
        .ldro = 0u
    };

    if ((LORA_STATE_IDLE != lora_state) || (0U != tx_queue_count))
    {
        return;
    }

    sx126x_set_standby(NULL, SX126X_STANDBY_CFG_RC);
    sx126x_set_lora_mod_params(NULL, &mod_params);
}

static bool lora_tx_queue_push(const uint8_t * packet)
{
    if (LORA_TX_QUEUE_LENGTH == tx_queue_count)
    {
        return false;
    }

    memcpy(tx_queue[tx_queue_tail], packet, APPL_LORA_PACKET_SIZE);
    tx_queue_tail = (uint8_t)((tx_queue_tail + 1U) % LORA_TX_QUEUE_LENGTH);
    tx_queue_count++;
    return true;
}

static void lora_tx_queue_pop(void)
{
    tx_queue_head = (uint8_t)((tx_queue_head + 1U) % LORA_TX_QUEUE_LENGTH);
    tx_queue_count--;
}

static void lora_return_to_idle(void)
{
    sx126x_set_standby(NULL, SX126X_STANDBY_CFG_RC);
    sx126x_clear_irq_status(NULL, SX126X_IRQ_ALL);
    rf_switch_set_idle();
    lora_irq_pending = false;
    tx_dynamic_busy = false;
    lora_state = LORA_STATE_IDLE;
}

static bool lora_enter_rx_mode(void)
{
    static const sx126x_pkt_params_lora_t pkt_params = {
        .preamble_len_in_symb = 8U,
        .header_type          = SX126X_LORA_PKT_EXPLICIT,
        .pld_len_in_bytes     = APPL_LORA_PACKET_SIZE,
        .crc_is_on            = true,
        .invert_iq_is_on      = false
    };
    const sx126x_irq_mask_t rx_irq_mask = SX126X_IRQ_RX_DONE | SX126X_IRQ_TIMEOUT |
                                         SX126X_IRQ_CRC_ERROR | SX126X_IRQ_HEADER_ERROR;

    if ((SX126X_STATUS_OK != sx126x_set_standby(NULL, SX126X_STANDBY_CFG_RC)) ||
        (SX126X_STATUS_OK != sx126x_clear_irq_status(NULL, SX126X_IRQ_ALL)) ||
        (SX126X_STATUS_OK != sx126x_set_lora_pkt_params(NULL, &pkt_params)) ||
        (SX126X_STATUS_OK != sx126x_set_dio_irq_params(NULL, rx_irq_mask, rx_irq_mask,
                                                       SX126X_IRQ_NONE, SX126X_IRQ_NONE)))
    {
        return false;
    }

    lora_irq_pending = false;
    rf_switch_set_rx();
    if (SX126X_STATUS_OK != sx126x_set_rx_with_timeout_in_rtc_step(NULL, SX126X_RX_CONTINUOUS))
    {
        rf_switch_set_idle();
        return false;
    }

    return true;
}

static void lora_start_rx_window(void)
{
    if (lora_enter_rx_mode())
    {
        rx_window_started_at_ms = service_timer_get_system_time_1ms();
        lora_state = LORA_STATE_RX_WINDOW;
    }
    else
    {
        lora_return_to_idle();
    }
}

static bool lora_start_next_tx(uint32_t now_ms)
{
    static const sx126x_pkt_params_lora_t pkt_params = {
        .preamble_len_in_symb = 8U,
        .header_type          = SX126X_LORA_PKT_EXPLICIT,
        .pld_len_in_bytes     = APPL_LORA_PACKET_SIZE,
        .crc_is_on            = true,
        .invert_iq_is_on      = false
    };
    const sx126x_irq_mask_t tx_irq_mask = SX126X_IRQ_TX_DONE | SX126X_IRQ_TIMEOUT;
    const uint8_t * packet;

    if (0U == tx_queue_count)
    {
        return false;
    }

    packet = tx_queue[tx_queue_head];
    lora_irq_pending = false;
    tx_dynamic_busy = true;

    if ((SX126X_STATUS_OK != sx126x_set_standby(NULL, SX126X_STANDBY_CFG_RC)) ||
        (SX126X_STATUS_OK != sx126x_clear_irq_status(NULL, SX126X_IRQ_ALL)) ||
        (SX126X_STATUS_OK != sx126x_set_lora_pkt_params(NULL, &pkt_params)) ||
        (SX126X_STATUS_OK != sx126x_write_buffer(NULL, 0x00U, packet, APPL_LORA_PACKET_SIZE)) ||
        (SX126X_STATUS_OK != sx126x_set_dio_irq_params(NULL, tx_irq_mask, tx_irq_mask,
                                                       SX126X_IRQ_NONE, SX126X_IRQ_NONE)))
    {
        lora_tx_queue_pop();
        lora_return_to_idle();
        return false;
    }

    lora_tx_queue_pop();
    rf_switch_set_tx();
    tx_state_started_at_ms = now_ms;
    lora_state = LORA_STATE_TX_SETTLE;
    return true;
}

static sx126x_irq_mask_t lora_take_irq_status(void)
{
    sx126x_irq_mask_t irq_status = SX126X_IRQ_NONE;

    lora_irq_pending = false;
    if (SX126X_STATUS_OK != sx126x_get_irq_status(NULL, &irq_status))
    {
        return SX126X_IRQ_NONE;
    }

    if (SX126X_IRQ_NONE != irq_status)
    {
        sx126x_clear_irq_status(NULL, irq_status);
    }
    return irq_status;
}

static bool lora_receive_and_queue_relay(void)
{
    uint8_t packet[APPL_LORA_PACKET_SIZE];
    sx126x_rx_buffer_status_t rx_status;
    sx126x_pkt_status_lora_t packet_status;

    if ((SX126X_STATUS_OK != sx126x_get_rx_buffer_status(NULL, &rx_status)) ||
        (APPL_LORA_PACKET_SIZE != rx_status.pld_len_in_bytes) ||
        (SX126X_STATUS_OK != sx126x_get_lora_pkt_status(NULL, &packet_status)) ||
        (SX126X_STATUS_OK != sx126x_read_buffer(NULL, rx_status.buffer_start_pointer,
                                                packet, APPL_LORA_PACKET_SIZE)))
    {
        return false;
    }

    return appl_flood_mesh_process_received(packet, packet_status.rssi_pkt_in_dbm) &&
           lora_tx_queue_push(packet);
}

void lora_send_20_bytes(void)
{
    lora_tx_packet_it(tx_buffer);
}

void lora_tx_packet_it(const uint8_t *data20)
{
    if (NULL == data20)
    {
        return;
    }

    if (lora_tx_queue_push(data20))
    {
        appl_flood_mesh_remember_transmitted(data20);
    }
}

void appl_lora_tick(void)
{
    uint32_t now_ms = service_timer_get_system_time_1ms();

    switch (lora_state)
    {
        case LORA_STATE_TX_SETTLE:
            if (lora_elapsed_ms(tx_state_started_at_ms) >= TX_SWITCH_SETTLE_TIME_MS)
            {
                if (SX126X_STATUS_OK == sx126x_set_tx(NULL, TX_TIMEOUT_VALUE))
                {
                    tx_state_started_at_ms = now_ms;
                    lora_state = LORA_STATE_TX_WAIT;
                }
                else
                {
                    lora_return_to_idle();
                }
            }
            break;

        case LORA_STATE_TX_WAIT:
        {
            sx126x_irq_mask_t irq_status = SX126X_IRQ_NONE;
            if (lora_irq_pending)
            {
                irq_status = lora_take_irq_status();
            }

            if (irq_status & SX126X_IRQ_TX_DONE)
            {
                tx_dynamic_busy = false;
                lora_start_rx_window();
            }
            else if ((irq_status & SX126X_IRQ_TIMEOUT) ||
                     (lora_elapsed_ms(tx_state_started_at_ms) >= TX_TIMEOUT_VALUE))
            {
                lora_return_to_idle();
            }
            break;
        }

        case LORA_STATE_RX_WINDOW:
        {
            sx126x_irq_mask_t irq_status = SX126X_IRQ_NONE;
            if (lora_irq_pending)
            {
                irq_status = lora_take_irq_status();
                if ((irq_status & SX126X_IRQ_RX_DONE) &&
                    !(irq_status & (SX126X_IRQ_CRC_ERROR | SX126X_IRQ_HEADER_ERROR)))
                {
                    lora_receive_and_queue_relay();
                }
            }

            if ((lora_elapsed_ms(rx_window_started_at_ms) >= APPL_LORA_RX_WINDOW_MS) ||
                (0U != tx_queue_count))
            {
                lora_return_to_idle();
            }
            else if (irq_status & (SX126X_IRQ_RX_DONE | SX126X_IRQ_TIMEOUT |
                                   SX126X_IRQ_CRC_ERROR | SX126X_IRQ_HEADER_ERROR))
            {
                if (!lora_enter_rx_mode())
                {
                    lora_return_to_idle();
                }
            }
            break;
        }

        case LORA_STATE_IDLE:
        default:
            break;
    }

    if ((LORA_STATE_IDLE == lora_state) && (0U != tx_queue_count))
    {
        lora_start_next_tx(now_ms);
    }
}
