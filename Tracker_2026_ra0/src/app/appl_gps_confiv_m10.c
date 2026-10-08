#include "appl_gps_confiv_m10.h"
#include "hal_data.h" // Provides access to g_uart0_ctrl and FSP APIs

// UBX Sync characters
#define UBX_SYNC1 0xB5
#define UBX_SYNC2 0x62

/* 
 * Static buffer required because R_SAU_UART_Write is non-blocking.
 * If this were a local stack variable, it would go out of scope 
 * and corrupt the data before the SAU TX interrupt finishes sending it.
 */
static uint8_t ubx_tx_buffer[32]; 

/* 
 * Dummy byte for wake-up. Must also be static for the non-blocking UART.
 */
static uint8_t ubx_wake_byte = 0xFF;

/**
 * Generic function to send a UBX command using Renesas SAU UART.
 * Automatically calculates and appends the 16-bit Fletcher checksum.
 */
static fsp_err_t ubx_send_message(uint8_t msg_class, uint8_t msg_id, const uint8_t *payload, uint8_t payload_len) {
    uint8_t ck_a = 0;
    uint8_t ck_b = 0;
    uint8_t i;

    // 1. Construct Header
    ubx_tx_buffer[0] = UBX_SYNC1;
    ubx_tx_buffer[1] = UBX_SYNC2;
    ubx_tx_buffer[2] = msg_class;
    ubx_tx_buffer[3] = msg_id;
    ubx_tx_buffer[4] = payload_len;        // Length LSB
    ubx_tx_buffer[5] = 0x00;               // Length MSB

    // 2. Add Payload
    for (i = 0; i < payload_len; i++) {
        ubx_tx_buffer[6 + i] = payload[i];
    }

    // 3. Calculate Checksum over Header and Payload
    for (i = 2; i < (6 + payload_len); i++) {
        ck_a += ubx_tx_buffer[i];
        ck_b += ck_a;
    }

    // 4. Append Checksum
    ubx_tx_buffer[6 + payload_len] = ck_a;
    ubx_tx_buffer[7 + payload_len] = ck_b;

    uint32_t total_length = 8 + payload_len;

    // 5. Transmit data sequentially using FSP API
    // Note: Change &g_uart0_ctrl if your FSP UART instance is named differently.
    return R_SAU_UART_Write(&g_uart0_ctrl, ubx_tx_buffer, total_length);
}

/**
 * Commands the M10Q to enter Software Backup Mode.
 */
void UBLOX_EnterSoftwareBackup(void) {
    /* 
     * UBX-RXM-PMREQ (v0) payload length: 8 bytes
     * 
     * Bytes 0-3: Duration in ms (0x00000000 = Infinite sleep)
     * Bytes 4-7: Flags
     *            Bit 1 = Backup mode (0x02)
     *            Bit 2 = Force flag (0x04) 
     *            Flags = 0x02 | 0x04 = 0x06
     */
    uint8_t pmreq_payload[8] = {
        0x00, 0x00, 0x00, 0x00, // Duration: Infinite
        0x06, 0x00, 0x00, 0x00  // Flags: Backup + Force
    };

    // Send UBX-RXM-PMREQ (Class: 0x02, ID: 0x41)
    ubx_send_message(0x02, 0x41, pmreq_payload, 8);
}

/**
 * Wakes the M10Q up from Software Backup Mode.
 */
void UBLOX_WakeUp(void) {
    // Sending any dummy character over UART RX will wake the M10Q 
    // from software backup mode.
    // Note: Change &g_uart0_ctrl if your FSP UART instance is named differently.
    R_SAU_UART_Write(&g_uart0_ctrl, &ubx_wake_byte, 1);
}
