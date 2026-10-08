/**
 * \file service_uart.c
 * \brief Implements the UART receive ring buffer.
 * \details Buffers ISR-received bytes for foreground service processing.
 * \author Kaushik Ray
 * \date Last Modified: 30 Sep 2026
 * \copyright (C) 2026 Siliconbrane Inc. All rights reserved.
 * \note Proprietary source. Unauthorized use or distribution is prohibited.
 */

#include "service_headers.h"

/* Static internal buffer and volatile pointers for ISR safety */
//static volatile uint8_t uart_ring_buffer[SERVICE_UART_RING_BUFFER_SIZE];
uint8_t uart_ring_buffer[SERVICE_UART_RING_BUFFER_SIZE];
static volatile uint8_t uart_head = 0; /* Index for incoming data (written by ISR) */
static volatile uint8_t uart_tail = 0; /* Index for outgoing data (read by application) */

/**
 * \brief Initializes the UART service by resetting buffer pointers.
 */
void service_uart_init(void)
{
    uart_head = 0;
    uart_tail = 0;
}

/**
 * \brief Interrupt Service Routine helper to push data into the ring buffer.
 * * \param data The byte received from the UART hardware.
 * * \details Logic: Calculates the next head position. If the next position
 * equals the tail, the buffer is full and the data is discarded to
 * prevent overwriting unread data.
 */
void service_uart_rx_isr(uint8_t data)
{
    uint8_t next_head = (uint8_t)(uart_head + 1u);

    /* Check for Buffer Overrun (Full condition) */
    if (next_head != uart_tail)
    {
        uart_ring_buffer[uart_head] = data;
        uart_head = next_head;   /* Implicit wrap-around at 255 + 1 -> 0 */
    }
    /* Buffer full: Newest byte is dropped to preserve existing buffer integrity */
}

/**
 * \brief Returns the number of bytes currently stored in the buffer.
 * * \return size_t Number of unread bytes [0-255].
 * * \details Logic: Subtraction of unsigned 8-bit integers handles the wrap-around
 * automatically (e.g., if head is 2 and tail is 254, the result is 4).
 */
size_t service_uart_available(void)
{
    return (uint8_t)(uart_head - uart_tail);
}

/**
 * \brief Reads a single byte from the buffer.
 * * \return uint8_t The oldest byte in the buffer, or 0 if empty.
 * * \details Logic: Checks if data is available (head != tail). If so, retrieves
 * the byte at the tail and increments the tail index.
 */
uint8_t service_uart_read_byte(void)
{
    uint8_t data = 0;

    /* Check for Empty condition */
    if (uart_head != uart_tail)
    {
        data = uart_ring_buffer[uart_tail];
        uart_tail = (uint8_t)(uart_tail + 1u);  /* Implicit wrap-around */
    }

    return data;
}

/**
 * \brief Flushes the buffer by resetting the pointers.
 */
void service_uart_clear(void)
{
    /* Note: In some high-speed systems, you might want to disable
       interrupts here to ensure an atomic clear. */
    uart_head = uart_tail = 0;
}

