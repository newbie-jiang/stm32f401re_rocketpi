/*
 * CRC self-test API.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef COMPONENT_CRC_CRC_TEST_H_
#define COMPONENT_CRC_CRC_TEST_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	const char *name;
	uint32_t expected;
	uint32_t actual;
	bool passed;
} crc_test_result_t;

bool crc_run_all_tests(void);
size_t crc_get_test_count(void);
const crc_test_result_t *crc_get_test_results(void);

#ifdef __cplusplus
}
#endif

#endif /* COMPONENT_CRC_CRC_TEST_H_ */
