#include "sx126x_hal.h"
#include "hal_data.h" // Renesas FSP standard header
#include "sx126x.h"

/* ============================================================================
 * USER CONFIGURATION MACROS
 * ============================================================================ */
#define SPI_CTRL g_spi0_ctrl

/* Dynamic wait timeout loop counter (~1 second limit) */
#define SPI_TIMEOUT_COUNT (1000000UL)

/* State machine flags */
//volatile bool g_lora_tx_done = false;
//volatile bool g_lora_rx_done = false;

/* ============================================================================
 * SPI SYNCHRONIZATION & HELPER FUNCTIONS
 * ============================================================================ */
volatile bool g_spi_transfer_complete = false;

void sau_spi_callback(spi_callback_args_t * p_args)
{
    if (SPI_EVENT_TRANSFER_COMPLETE == p_args->event)
    {
        g_spi_transfer_complete = true;
    }
}

//void lora_dio1_callback(external_irq_callback_args_t *p_args)
//{
//    (void)p_args;
//
//    sx126x_irq_mask_t irq_status;
//    sx126x_get_irq_status(NULL, &irq_status);
//    sx126x_clear_irq_status(NULL, irq_status);
//
//    if (irq_status & SX126X_IRQ_TX_DONE)
//    {
//        g_lora_tx_done = true;
//    }
//
//    if (irq_status & SX126X_IRQ_RX_DONE)
//    {
//        g_lora_rx_done = true;
//    }
//}

static inline sx126x_hal_status_t wait_for_spi_complete(void)
{
    uint32_t timeout = SPI_TIMEOUT_COUNT;

    while (!g_spi_transfer_complete && (timeout > 0))
    {
        timeout--;
    }

    if (timeout == 0)
    {
        /* SPI Transfer timed out */
        return SX126X_HAL_STATUS_ERROR;
    }

    g_spi_transfer_complete = false;
    return SX126X_HAL_STATUS_OK;
}

static inline void sx126x_hal_nss_low(void)
{
    R_IOPORT_PinWrite(&g_ioport_ctrl, MCU_LORA_NSS, BSP_IO_LEVEL_LOW);
}

static inline void sx126x_hal_nss_high(void)
{
    R_IOPORT_PinWrite(&g_ioport_ctrl, MCU_LORA_NSS, BSP_IO_LEVEL_HIGH);
}

static inline void sx126x_hal_wait_on_busy(void)
{
    bsp_io_level_t busy_state;

    do {
        R_IOPORT_PinRead(&g_ioport_ctrl, MCU_LORA_BUSY, &busy_state);
    } while (busy_state == BSP_IO_LEVEL_HIGH);
}

/* ============================================================================
 * SX126X HAL IMPLEMENTATIONS
 * ============================================================================ */

sx126x_hal_status_t sx126x_hal_reset(const void* context)
{
    (void)context;

    /* Pull RESET low to trigger reset */
    R_IOPORT_PinWrite(&g_ioport_ctrl, MCU_LORA_RST, BSP_IO_LEVEL_LOW);
    R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);

    /* Pull RESET high to resume normal operation */
    R_IOPORT_PinWrite(&g_ioport_ctrl, MCU_LORA_RST, BSP_IO_LEVEL_HIGH);
    R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);

    return SX126X_HAL_STATUS_OK;
}

sx126x_hal_status_t sx126x_hal_wakeup(const void* context)
{
    (void)context;

    /* Matching original STM32 toggling sequence */
    sx126x_hal_nss_low();
    R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
    sx126x_hal_nss_high();

    sx126x_hal_wait_on_busy();

    return SX126X_HAL_STATUS_OK;
}

sx126x_hal_status_t sx126x_hal_write(const void* context,
                                     const uint8_t* command, const uint16_t command_length,
                                     const uint8_t* data, const uint16_t data_length)
{
    (void)context;

    sx126x_hal_wait_on_busy();
    sx126x_hal_nss_low();

    /* Write Command */
    g_spi_transfer_complete = false;
    if (R_SAU_SPI_Write(&SPI_CTRL, command, command_length, SPI_BIT_WIDTH_8_BITS) != FSP_SUCCESS)
    {
        sx126x_hal_nss_high();
        return SX126X_HAL_STATUS_ERROR;
    }
    if (wait_for_spi_complete() != SX126X_HAL_STATUS_OK)
    {
        sx126x_hal_nss_high();
        return SX126X_HAL_STATUS_ERROR;
    }

    /* Write Payload Data */
    if (data != NULL && data_length > 0)
    {
        g_spi_transfer_complete = false;
        if (R_SAU_SPI_Write(&SPI_CTRL, data, data_length, SPI_BIT_WIDTH_8_BITS) != FSP_SUCCESS)
        {
            sx126x_hal_nss_high();
            return SX126X_HAL_STATUS_ERROR;
        }
        if (wait_for_spi_complete() != SX126X_HAL_STATUS_OK)
        {
            sx126x_hal_nss_high();
            return SX126X_HAL_STATUS_ERROR;
        }
    }

    sx126x_hal_nss_high();
    return SX126X_HAL_STATUS_OK;
}

sx126x_hal_status_t sx126x_hal_read(const void* context,
                                    const uint8_t* command, const uint16_t command_length,
                                    uint8_t* data, const uint16_t data_length)
{
    (void)context;

    sx126x_hal_wait_on_busy();
    sx126x_hal_nss_low();

    /* Write Command */
    g_spi_transfer_complete = false;
    if (R_SAU_SPI_Write(&SPI_CTRL, command, command_length, SPI_BIT_WIDTH_8_BITS) != FSP_SUCCESS)
    {
        sx126x_hal_nss_high();
        return SX126X_HAL_STATUS_ERROR;
    }
    if (wait_for_spi_complete() != SX126X_HAL_STATUS_OK)
    {
        sx126x_hal_nss_high();
        return SX126X_HAL_STATUS_ERROR;
    }

    /* Read Payload Data */
    if (data != NULL && data_length > 0)
    {
        g_spi_transfer_complete = false;
        if (R_SAU_SPI_Read(&SPI_CTRL, data, data_length, SPI_BIT_WIDTH_8_BITS) != FSP_SUCCESS)
        {
            sx126x_hal_nss_high();
            return SX126X_HAL_STATUS_ERROR;
        }
        if (wait_for_spi_complete() != SX126X_HAL_STATUS_OK)
        {
            sx126x_hal_nss_high();
            return SX126X_HAL_STATUS_ERROR;
        }
    }

    sx126x_hal_nss_high();
    return SX126X_HAL_STATUS_OK;
}

/* ============================================================================
 * RF SWITCH CONTROL (RESTORED FROM STM32 SOURCE)
 * ============================================================================ */

sx126x_hal_status_t sx126x_rf_switch_tx(void)
{
    R_IOPORT_PinWrite(&g_ioport_ctrl, MCU_LORA_RXEN, BSP_IO_LEVEL_LOW);
    R_IOPORT_PinWrite(&g_ioport_ctrl, MCU_LORA_TXEN, BSP_IO_LEVEL_HIGH);

    return SX126X_HAL_STATUS_OK;
}

sx126x_hal_status_t sx126x_rf_switch_rx(void)
{
    R_IOPORT_PinWrite(&g_ioport_ctrl, MCU_LORA_TXEN, BSP_IO_LEVEL_LOW);
    R_IOPORT_PinWrite(&g_ioport_ctrl, MCU_LORA_RXEN, BSP_IO_LEVEL_HIGH);

    return SX126X_HAL_STATUS_OK;
}
