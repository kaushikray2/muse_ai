/**
 * \file service_telemetry.h
 * \brief Public telemetry service API.
 * \details Declares initialization, periodic tick, and message-ID access functions.
 * \author Kaushik Ray
 * \date Last Modified: 30 Sep 2026
 * \copyright (C) 2026 Siliconbrane Inc. All rights reserved.
 * \note Proprietary source. Unauthorized use or distribution is prohibited.
 */

#ifndef SERVICE_TELEMETRY_H
#define SERVICE_TELEMETRY_H

#include <stdint.h>
#include <stdbool.h>


/* ========================================================================
   PUBLIC API
   ======================================================================== */

/**
 * \brief Initialize the telemetry service
 *
 * \details Initializes message ID counter and other internal state.
 *          Must be called during system initialization.
 */
void service_telemetry_init(void);

/**
 * \brief Telemetry tick function - call every 5ms from scheduler slot 2
 *
 * \details Manages 60-second interval timer. When 60 seconds elapse,
 *          fetches current GPS data, packs telemetry packet, and transmits
 *          via LORA radio if GPS fix is valid.
 *
 * \note Only transmits when GPS has valid fix (service_gps_is_valid() returns true)
 */
void service_telemetry_tick(void);

/**
 * \brief Get the current message ID counter value
 *
 * \return Current message ID (useful for debugging/logging)
 */
uint16_t service_telemetry_get_message_id(void);

#endif /* SERVICE_TELEMETRY_H */
