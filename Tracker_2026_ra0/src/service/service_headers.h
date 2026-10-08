/**
 * \file service_headers.h
 * \brief Aggregates shared service dependencies.
 * \details Includes common C, FSP, timer, UART, GNSS, flash, and telemetry headers.
 * \author Kaushik Ray
 * \date Last Modified: 30 Sep 2026
 * \copyright (C) 2026 Siliconbrane Inc. All rights reserved.
 * \note Proprietary source. Unauthorized use or distribution is prohibited.
 */

#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef SERVICE_SERVICE_HEADERS_H_
#define SERVICE_SERVICE_HEADERS_H_

#include "hal_data.h"
#include "r_rtc_c.h"

#include "service_timer.h"
#include "service_uart.h"
#include "service_gnss.h"
#include "service_dataflash.h"
#include "service_telemetry.h"
#include "service_adc.h"

#endif /* SERVICE_SERVICE_HEADERS_H_ */
