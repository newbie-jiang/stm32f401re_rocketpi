/**
 * Copyright (c) 2025
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @file      driver_sdcard_test.h
 * @brief     driver sdcard test header file
 * @version   1.0.0
 * @date      2025-10-19
 */

#ifndef DRIVER_SDCARD_TEST_H
#define DRIVER_SDCARD_TEST_H

#include "sdio.h"

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @brief default block address used by sdcard_test_run()
 */
#define SDCARD_TEST_DEFAULT_BLOCK_ADDR   0U

/**
 * @brief default block count used by sdcard_test_run()
 * @note  set to 0 to test the whole card from the start block
 */
#define SDCARD_TEST_DEFAULT_BLOCK_COUNT  0U

/**
 * @brief     print card information retrieved from HAL_SD_GetCardInfo()
 * @return    status code
 *            - 0 success
 *            - 1 operation failed
 */
uint8_t sdcard_test_card_info(void);

/**
 * @brief     perform blocking read/write verification on the SD card
 * @param[in] block_addr start block address
 * @param[in] block_count number of blocks to transfer, 0 means test to the end of the card
 * @return    status code
 *            - 0 success
 *            - 1 read/write failed
 */
uint8_t sdcard_test_block_read_write(uint32_t block_addr, uint32_t block_count);

/**
 * @brief     run the default SD card test sequence (info + read/write)
 * @param[in] block_addr start block address
 * @param[in] block_count number of blocks to transfer, 0 means test to the end of the card
 * @return    status code
 *            - 0 success
 *            - 1 test failed
 */
uint8_t sdcard_test_run(uint32_t block_addr, uint32_t block_count);

#ifdef __cplusplus
}
#endif

#endif
