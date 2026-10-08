#ifndef TEST_DATA_FLASH_H_
#define TEST_DATA_FLASH_H_

#include "hal_data.h"

typedef enum
{
	TEST_DATA_FLASH_RESULT_PENDING = 0U,
	TEST_DATA_FLASH_RESULT_PASS,
	TEST_DATA_FLASH_RESULT_FAIL
} test_data_flash_result_t;

extern volatile test_data_flash_result_t test_data_flash_pass_fail;

void test_data_flash_init(void);
void test_data_flash_tick(void);
void test_data_flash_flash_callback(flash_callback_args_t * p_args);
fsp_err_t test_data_flash_result_get(void);
uint8_t test_data_flash_is_complete(void);

#endif /* TEST_DATA_FLASH_H_ */