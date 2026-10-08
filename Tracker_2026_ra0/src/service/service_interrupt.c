/**
 * \file service_interrupt.c
 * \brief FSP callback handlers for application peripherals.
 * \details Forwards timer ticks and UART receive bytes to their service layers.
 * \author Kaushik Ray
 * \date Last Modified: 30 Sep 2026
 * \copyright (C) 2026 Siliconbrane Inc. All rights reserved.
 * \note Proprietary source. Unauthorized use or distribution is prohibited.
 */
#include "service_headers.h"
#include "../test/test_data_flash.h"


void timer0_callback(timer_callback_args_t *p_args) /* System Timer */
{
	(void)p_args;   // Prevent unused‑parameter warning
	/* interrupt timer goes here */
	SysTick_Handler();
}

// 1. Declare a volatile flag so the main loop and ISR can safely share state
volatile bool g_uart_tx_completed = false;

void g_uart0_callback(uart_callback_args_t *p_args) /* GPS UART RX and TX ISR */
{
    if (p_args->event == UART_EVENT_TX_COMPLETE)
    {
        /* Transmission done */
        g_uart_tx_completed = true; // Tell the main loop the data has left the MCU
    }
    else if (p_args->event == UART_EVENT_RX_CHAR)
    {
    	uint8_t data = (uint8_t)p_args->data;
        /* Handle received byte */
        service_uart_rx_isr(data);
    }
}

void g_flash_callback(flash_callback_args_t * p_args)
{
    if (p_args == NULL)
    {
        return;
    }

    switch (p_args->event)
    {
        case FLASH_EVENT_ERASE_COMPLETE:
            /* Erase finished */
            break;

        case FLASH_EVENT_WRITE_COMPLETE:
            /* Write finished */
            break;

        default:
            break;
    }

    test_data_flash_flash_callback(p_args);
}

/* 1. Define the callback function to handle the interrupt */
//void g_rtc0_callback(rtc_callback_args_t * p_args)
//{
//    if (p_args == NULL)
//    {
//        return;
//    }
//
//    if (RTC_EVENT_PERIODIC_IRQ == p_args->event)
//    {
//        /* The RTC IRQ is intentionally disabled here. Case 4 re-enables and
//           reconfigures it immediately before the next WFI entry. */
//        R_BSP_IrqDisable(g_rtc0_cfg.periodic_irq);
//        R_TAU_Start(&g_timer0_ctrl);
//        //R_SAU_UART_Open(&g_uart0_ctrl, &g_uart0_cfg);
//    }
//}

void g_rtc0_callback(rtc_callback_args_t * p_args)
{
    if (p_args == NULL)
    {
        return;
    }

    if (RTC_EVENT_PERIODIC_IRQ == p_args->event)
    {
        /* 1. Disable RTC NVIC line so it doesn't trigger repeatedly while awake */
        R_BSP_IrqDisable(g_rtc0_cfg.periodic_irq);
        R_BSP_IrqClearPending(g_rtc0_cfg.periodic_irq);

        /* 2. Restart TAU peripheral timer for 0.5ms ticks */
        R_TAU_Start(&g_timer0_ctrl);

        /* 3. Reset application tick service state */
        service_timer_ini();
    }
}
