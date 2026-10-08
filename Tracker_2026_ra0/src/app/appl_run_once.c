/**
 * \file appl_run_once.c
 * \brief One-shot application utilities.
 * \details Copies the device unique ID into a caller-provided byte buffer.
 * \author Kaushik Ray
 * \date Last Modified: 30 Sep 2026
 * \copyright (C) 2026 Siliconbrane Inc. All rights reserved.
 * \note Proprietary source. Unauthorized use or distribution is prohibited.
 */

#include "appl_run_once.h"

#include "hal_data.h"

void appl_run_once_read_unique_id(uint8_t * uid_buffer_16bytes)
{
    /* Get pointer to the 128-bit Unique ID structure provided by the BSP */
    bsp_unique_id_t const * p_uid = R_BSP_UniqueIdGet();

    /* Option 1: Copy as a 16-byte raw array */
    uint32_t * p_dest = (uint32_t *)uid_buffer_16bytes;
    p_dest[0] = p_uid->unique_id_words[0];
    p_dest[1] = p_uid->unique_id_words[1];
    p_dest[2] = p_uid->unique_id_words[2];
    p_dest[3] = p_uid->unique_id_words[3];
}


//******************************************GPS Config********************************************************

extern sau_uart_instance_ctrl_t g_uart0_ctrl;

extern volatile bool g_uart_tx_completed;

void send_gnss_command(const char *cmd)
{
    g_uart_tx_completed = false;

    // Calculate string length
    uint32_t len = 0;
    while(cmd[len] != '\0') {
        len++;
    }

    // Fire the non-blocking FSP transmit function
    R_SAU_UART_Write(&g_uart0_ctrl, (uint8_t *)cmd, len);

    // Enter deep sleep while waiting for the hardware to transmit
    while(!g_uart_tx_completed)
    {
        __WFI();
    }
}

void configure_lc76g_factory_settings(void)
{
    /*
     * STEP 1: Change Baud Rate to 19200
     * Assumes the RA0E2 and LC76G are currently matching at the default start speed.
     * Note: The correct XOR checksum for "PAIR864,0,0,19200" is 26.
     */
    send_gnss_command("$PAIR864,0,0,19200*26\r\n");

    // Wait for the LC76G to switch its internal UART divider
    R_BSP_SoftwareDelay(50, BSP_DELAY_UNITS_MILLISECONDS);

    /* Switch the RA0E2 UART to match the new 19200 speed */
    sau_uart_baudrate_setting_t baud_setting;
    if (FSP_SUCCESS == R_SAU_UART_BaudCalculate(&g_uart0_ctrl, 19200, &baud_setting))
    {
        R_SAU_UART_BaudSet(&g_uart0_ctrl, &baud_setting);
    }

    /*
     * STEP 2: Mute Unnecessary NMEA Sentences
     * Sent at the new 19200 baud rate to save CPU wake time.
     */
    send_gnss_command("$PAIR062,1,0*3F\r\n"); // Disable GLL
    send_gnss_command("$PAIR062,2,0*3C\r\n"); // Disable GSA
    send_gnss_command("$PAIR062,3,0*3D\r\n"); // Disable GSV
    send_gnss_command("$PAIR062,5,0*3B\r\n"); // Disable VTG

    /*
     * STEP 3: Change Navigation Mode to Pedestrian
     * Removes aggressive vehicle smoothing for accurate dog tracking.
     */
    send_gnss_command("$PAIR080,1*2F\r\n");

    /*
     * STEP 4: Ensure SBAS is Enabled
     * Maximizes HDOP accuracy under heavy tree cover.
     */
    send_gnss_command("$PAIR410,1*22\r\n");

    /*
     * STEP 5: Save Settings to Internal Flash
     * Locks in 19200 baud, muted strings, and Pedestrian mode even if the 1Ah battery drops to 0V.
     */
    send_gnss_command("$PAIR513*3D\r\n");

    // Wait for the internal Quectel flash write to complete before doing anything else
    R_BSP_SoftwareDelay(100, BSP_DELAY_UNITS_MILLISECONDS);
}
