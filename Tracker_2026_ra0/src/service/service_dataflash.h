/**
 * \file service_dataflash.h
 * \brief Public API and layout constants for data-flash operations.
 * \details Declares fixed-size data-flash read, write, and verification helpers.
 * \author Kaushik Ray
 * \date Last Modified: 30 Sep 2026
 * \copyright (C) 2026 Siliconbrane Inc. All rights reserved.
 * \note Proprietary source. Unauthorized use or distribution is prohibited.
 */

#ifndef FLASH_48_WRITE_H
#define FLASH_48_WRITE_H

#include "hal_data.h"
#include <stdint.h>


/* --------------------------------------------------------------------------
 * Configuration
 * -------------------------------------------------------------------------- */

/* Start address of the Data Flash block you want to use */
#ifndef DF_BLOCK_ADDR
#define DF_BLOCK_ADDR       (0x40100000U)   /* Example DF block start */
#endif

/* Size of the DF block (device dependent) */
#ifndef DF_BLOCK_SIZE
#define DF_BLOCK_SIZE       (1024U)
#endif

/* Total bytes to write/read */
#define FLASH_WRITE_TOTAL_BYTES     (48U)

/* Flash LP minimum write size (Data Flash = 8 bytes) */
#define FLASH_WRITE_CHUNK_SIZE      (8U)

/* --------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------- */

/**
 * @brief Write exactly 48 bytes to Data Flash.
 *
 * This function:
 *   1. Erases one DF block
 *   2. Writes 48 bytes in 8-byte chunks
 *   3. Verifies the write
 *
 * @param data48  Pointer to a 48-byte buffer in RAM
 * @return FSP_SUCCESS on success, or an FSP_ERR_xxx code on failure
 */
fsp_err_t flash_write_48_bytes(const uint8_t * data48);

/**
 * @brief Read 48 bytes from Data Flash into a RAM buffer.
 *
 * @param out48  Pointer to a 48-byte buffer in RAM
 * @return FSP_SUCCESS on success
 */
fsp_err_t flash_read_48_bytes(uint8_t * out48);

/**
 * @brief Verify 48 bytes stored in Data Flash.
 *
 * @param data48  Pointer to expected 48-byte buffer
 * @return FSP_SUCCESS if all bytes match, FSP_ERR_NOT_OPEN or FSP_ERR_ABORTED otherwise
 */
fsp_err_t flash_verify_48_bytes(const uint8_t * data48);

uint8_t flash_test(void);


#endif /* FLASH_48_WRITE_H */
