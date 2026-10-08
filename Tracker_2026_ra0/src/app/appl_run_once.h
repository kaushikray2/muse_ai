/**
 * \file appl_run_once.h
 * \brief Public declarations for one-shot application utilities.
 * \details Exposes the device unique-ID copy helper.
 * \author Kaushik Ray
 * \date Last Modified: 30 Sep 2026
 * \copyright (C) 2026 Siliconbrane Inc. All rights reserved.
 * \note Proprietary source. Unauthorized use or distribution is prohibited.
 */

#ifndef APP_APPL_RUN_ONCE_H_
#define APP_APPL_RUN_ONCE_H_

#include <stdint.h>

void appl_run_once_read_unique_id(uint8_t *uid_buffer_16bytes);

#endif /* APP_APPL_RUN_ONCE_H_ */
