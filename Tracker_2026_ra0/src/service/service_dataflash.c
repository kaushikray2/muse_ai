/**
 * \file service_dataflash.c
 * \brief Implements data-flash read, write, and verification services.
 * \details Writes data in FSP-sized chunks and verifies stored contents.
 * \author Kaushik Ray
 * \date Last Modified: 30 Sep 2026
 * \copyright (C) 2026 Siliconbrane Inc. All rights reserved.
 * \note Proprietary source. Unauthorized use or distribution is prohibited.
 */


#include "service_headers.h"

//#define DF_BLOCK_ADDR   (0x40100000U)   // Example Data Flash block start
//#define DF_BLOCK_SIZE   (1024U)         // Typical DF block size

static const uint8_t my_data[FLASH_WRITE_TOTAL_BYTES] =
{
    // Fill with your 48 bytes
    0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
    0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,
    0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,
    0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,
    0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,
    0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58
};

/* Use the Flash LP instance from hal_data.c */
//extern flash_lp_instance_t g_flash0;

/* --------------------------------------------------------------------------
 * Write 48 bytes to Data Flash
 * -------------------------------------------------------------------------- */
fsp_err_t flash_write_48_bytes(const uint8_t * data48)
{
    if (data48 == NULL)
    {
        return FSP_ERR_INVALID_POINTER;
    }

    fsp_err_t err;

    /* Erase one DF block */
    err = R_FLASH_LP_Erase(&g_flash0_ctrl, DF_BLOCK_ADDR, 1);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    /* Write in 8-byte chunks */
    for (uint32_t offset = 0; offset < FLASH_WRITE_TOTAL_BYTES; offset += FLASH_WRITE_CHUNK_SIZE)
    {
        err = R_FLASH_LP_Write(&g_flash0_ctrl,
                               (uint32_t)(DF_BLOCK_ADDR + offset),
                               (uint32_t)&data48[offset],
                               FLASH_WRITE_CHUNK_SIZE);

        if (FSP_SUCCESS != err)
        {
            return err;
        }
    }

    /* Verify after write */
    return flash_verify_48_bytes(data48);
}

/* --------------------------------------------------------------------------
 * Read 48 bytes from Data Flash
 * -------------------------------------------------------------------------- */
fsp_err_t flash_read_48_bytes(uint8_t * out48)
{
    if (out48 == NULL)
    {
        return FSP_ERR_INVALID_POINTER;
    }

    /* Data Flash is memory-mapped, so just memcpy */
    for (uint32_t i = 0; i < FLASH_WRITE_TOTAL_BYTES; i++)
    {
        out48[i] = *((volatile uint8_t *)(DF_BLOCK_ADDR + i));
    }

    return FSP_SUCCESS;
}

/* --------------------------------------------------------------------------
 * Verify 48 bytes stored in Data Flash
 * -------------------------------------------------------------------------- */
fsp_err_t flash_verify_48_bytes(const uint8_t * data48)
{
    if (data48 == NULL)
    {
        return FSP_ERR_INVALID_POINTER;
    }

    for (uint32_t i = 0; i < FLASH_WRITE_TOTAL_BYTES; i++)
    {
        uint8_t df_val = *((volatile uint8_t *)(DF_BLOCK_ADDR + i));

        if (df_val != data48[i])
        {
            return FSP_ERR_ABORTED;   /* mismatch */
        }
    }

    return FSP_SUCCESS;
}

uint8_t flash_test(void)
{
    fsp_err_t test_result = flash_write_48_bytes(my_data);


	if(0 !=  test_result )
		test_result = 1;
	else
		test_result = 0;

    return (uint8_t)test_result;
}

