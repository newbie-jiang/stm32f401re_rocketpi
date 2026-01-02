/*
 * Copyright (c) 2022 Jan Privara
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IR_TYPES_H
#define IR_TYPES_H

#include <inttypes.h>

typedef struct {
	int length;
	uint32_t *buf;
} ir_raw_bit_buf_t;

typedef struct {
	int32_t pulse;
	int32_t space;
} ir_tim_adj_t;

#endif /* IR_TYPES_H */
