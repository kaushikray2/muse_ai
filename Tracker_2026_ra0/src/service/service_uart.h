/**
 * \file service_uart.h
 * \brief Public UART ring-buffer API.
 * \details Declares initialization, ISR enqueue, byte read, and queue status functions.
 * \author Kaushik Ray
 * \date Last Modified: 30 Sep 2026
 * \copyright (C) 2026 Siliconbrane Inc. All rights reserved.
 * \note Proprietary source. Unauthorized use or distribution is prohibited.
 */
#ifndef SERVICE_SERVICE_UART_H_
#define SERVICE_SERVICE_UART_H_

#include <stddef.h>
#include <stdint.h>

#define SERVICE_UART_RING_BUFFER_SIZE 256

void service_uart_init(void);
void service_uart_rx_isr(uint8_t data);
size_t service_uart_available(void);
uint8_t service_uart_read_byte(void);
void service_uart_clear(void);

#endif /* SERVICE_SERVICE_UART_H_ */
