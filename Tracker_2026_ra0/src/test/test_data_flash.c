#include "test_data_flash.h"

#include "../service/service_dataflash.h"

typedef enum
{
	TEST_DATA_FLASH_ERASE_START,
	TEST_DATA_FLASH_ERASE_WAIT,
	TEST_DATA_FLASH_WRITE_START,
	TEST_DATA_FLASH_WRITE_WAIT,
	TEST_DATA_FLASH_DONE
} test_data_flash_state_t;

static uint8_t test_data[FLASH_WRITE_TOTAL_BYTES] =
{
	0x01U, 0x02U, 0x03U, 0x04U, 0x05U, 0x06U, 0x07U, 0x08U,
	0x11U, 0x12U, 0x13U, 0x14U, 0x15U, 0x16U, 0x17U, 0x18U,
	0x21U, 0x22U, 0x23U, 0x24U, 0x25U, 0x26U, 0x27U, 0x28U,
	0x31U, 0x32U, 0x33U, 0x34U, 0x35U, 0x36U, 0x37U, 0x38U,
	0x41U, 0x42U, 0x43U, 0x44U, 0x45U, 0x46U, 0x47U, 0x48U,
	0x51U, 0x52U, 0x53U, 0x54U, 0x55U, 0x56U, 0x57U, 0x58U
};

static volatile test_data_flash_state_t test_state;
static volatile flash_event_t completed_event;
static volatile uint8_t callback_pending;
static volatile fsp_err_t test_result = FSP_ERR_IN_USE;
volatile test_data_flash_result_t test_data_flash_pass_fail = TEST_DATA_FLASH_RESULT_PENDING;

static void test_data_flash_finish(fsp_err_t result)
{
	test_result = result;
	test_state = TEST_DATA_FLASH_DONE;
}

static fsp_err_t test_data_flash_verify(void)
{
	for (uint32_t index = 0U; index < FLASH_WRITE_TOTAL_BYTES; index++)
	{
		uint8_t actual = *((volatile uint8_t *)(DF_BLOCK_ADDR + index));
		if (actual != test_data[index])
		{
			return FSP_ERR_ABORTED;
		}
	}

	return FSP_SUCCESS;
}

void test_data_flash_init(void)
{
	callback_pending = 0U;
	test_result = FSP_ERR_IN_USE;
	test_data_flash_pass_fail = TEST_DATA_FLASH_RESULT_PENDING;
	test_state = TEST_DATA_FLASH_ERASE_START;
}

void test_data_flash_tick(void)
{
	fsp_err_t err;

	switch (test_state)
	{
		case TEST_DATA_FLASH_ERASE_START:
			callback_pending = 0U;
			test_state = TEST_DATA_FLASH_ERASE_WAIT;
			err = R_FLASH_LP_Erase(&g_flash0_ctrl, DF_BLOCK_ADDR, 1U);
			if (FSP_SUCCESS != err)
			{
				test_data_flash_finish(err);
			}
			break;

		case TEST_DATA_FLASH_ERASE_WAIT:
			if (0U != callback_pending)
			{
				callback_pending = 0U;
				if (FLASH_EVENT_ERASE_COMPLETE == completed_event)
				{
					test_state = TEST_DATA_FLASH_WRITE_START;
				}
				else
				{
					test_data_flash_finish(FSP_ERR_ABORTED);
				}
			}
			break;

		case TEST_DATA_FLASH_WRITE_START:
			callback_pending = 0U;
			test_state = TEST_DATA_FLASH_WRITE_WAIT;
			err = R_FLASH_LP_Write(&g_flash0_ctrl,
								   (uint32_t)test_data,
								   DF_BLOCK_ADDR,
								   FLASH_WRITE_TOTAL_BYTES);
			if (FSP_SUCCESS != err)
			{
				test_data_flash_finish(err);
			}
			break;

		case TEST_DATA_FLASH_WRITE_WAIT:
			if (0U != callback_pending)
			{
				callback_pending = 0U;
				if (FLASH_EVENT_WRITE_COMPLETE == completed_event)
				{
					test_data_flash_finish(test_data_flash_verify());
				}
				else
				{
					test_data_flash_finish(FSP_ERR_ABORTED);
				}
			}
			break;

		case TEST_DATA_FLASH_DONE:
			test_data_flash_pass_fail = (FSP_SUCCESS == test_result) ?
					TEST_DATA_FLASH_RESULT_PASS : TEST_DATA_FLASH_RESULT_FAIL;
			break;

		default:
			break;
	}
}

void test_data_flash_flash_callback(flash_callback_args_t * p_args)
{
	if ((NULL != p_args) &&
		((TEST_DATA_FLASH_ERASE_WAIT == test_state) ||
		 (TEST_DATA_FLASH_WRITE_WAIT == test_state)))
	{
		completed_event = p_args->event;
		callback_pending = 1U;
	}
}

fsp_err_t test_data_flash_result_get(void)
{
	return test_result;
}

uint8_t test_data_flash_is_complete(void)
{
	return (uint8_t)(TEST_DATA_FLASH_DONE == test_state);
}
