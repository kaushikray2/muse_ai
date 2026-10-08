/**
 * \file service_timer.h
 * \brief Public system timer service API.
 * \details Declares scheduler tick, system time, and elapsed-time functions.
 * \author Kaushik Ray
 * \date Last Modified: 30 Sep 2026
 * \copyright (C) 2026 Siliconbrane Inc. All rights reserved.
 * \note Proprietary source. Unauthorized use or distribution is prohibited.
 */

#ifndef SERVICE_SERVICE_TIMER_H_
#define SERVICE_SERVICE_TIMER_H_

#include <stdint.h>

void service_timer_ini(void); /* initialize lin timer */

uint8_t service_timer_0_5_ms_due(void); /* check if 0.5ms is due, will automatically clear */

void service_init_system_timer(void);
uint32_t service_timer_get_system_time_1ms(void); /* return system timer note: in 1 ms */
void service_system_timer_isr(void);
void SysTick_Handler(void);

#endif /* SERVICE_SERVICE_TIMER_H_ */
