/**
 * \file service_timer.c
 * \brief Implements system tick tracking and elapsed-time queries.
 * \details Converts the 0.5 ms hardware timer callback into scheduler and millisecond time.
 * \author Kaushik Ray
 * \date Last Modified: 30 Sep 2026
 * \copyright (C) 2026 Siliconbrane Inc. All rights reserved.
 * \note Proprietary source. Unauthorized use or distribution is prohibited.
 */


#include "service_headers.h"


static volatile uint8_t timer_0_5_ms_tick;
static volatile uint32_t system_timer; /* Counts 0.5 ms ticks. */

/**
 * \brief Initialize the service timer module
 *
 * \details Initializes the hardware timer layer which enables the system timer.
 *          Must be called during system initialization before any timer operations.
 */
void service_timer_ini(void)
{
    timer_0_5_ms_tick = 0u;
    system_timer = 0u;
}

void service_init_system_timer(void)
{
    service_timer_ini();
}

void service_system_timer_isr(void)
{
    SysTick_Handler();
}

/**
 * \brief System tick interrupt handler called every 0.5 milliseconds
 *
 * \details Handles the periodic system timer interrupt. Sets the 0.5ms tick flag and
 *          increments the system timer counter. This interrupt is called automatically
 *          by the hardware timer every 0.5ms.
 */
void SysTick_Handler(void)
{
  timer_0_5_ms_tick = 1u;
  system_timer++;
}

/**
 * \brief Check if 0.5 millisecond tick has occurred
 *
 * \details Checks if the 0.5ms timer tick has elapsed and automatically clears the flag.
 *          Should be called in the main scheduler loop to trigger 0.5ms tasks.
 *
 * \return Returns 1 if 0.5ms tick has occurred, 0 otherwise. Flag is cleared after read.
 */
uint8_t service_timer_0_5_ms_due(void)
{
  uint8_t result = 0u;
  if (1u == timer_0_5_ms_tick)
  {
    result = 1u;
    timer_0_5_ms_tick = 0u;
  }
  return result;
}

/**
 * \brief Get the current system time in milliseconds
 *
 * \details Returns the current system time value converted to milliseconds.
 *          The internal system timer increments every 0.5ms, so this function
 *          right-shifts by 1 bit to convert to 1ms resolution.
 *
 * \return Current system time in milliseconds
 */
uint32_t service_timer_get_system_time_1ms(void)
{
  return (system_timer >> 1u);
}

/**
 * \brief Calculate elapsed time from a given start point
 *
 * \details Computes the time elapsed in milliseconds from a recorded start time
 *          to the current system time. Handles timer overflow by comparing the
 *          current time with the start time and calculating the elapsed duration.
 *
 * \param[in] system_timer_start Previously recorded system time reference point
 * \return Elapsed time in milliseconds since the start point
 */
uint32_t system_timer_time_lapsed(uint32_t system_timer_start)
{
  uint32_t system_timer_now, result;
  system_timer_now = (system_timer >> 1u);
  if (system_timer_now >= system_timer_start)
  {
    result = system_timer_now - system_timer_start;
  }
  else
  {
    result = 0x7FFFFFFFu - system_timer_start + system_timer_now;
  }
  return result;
}
