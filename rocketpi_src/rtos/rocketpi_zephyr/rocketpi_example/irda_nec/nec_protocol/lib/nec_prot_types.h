/*
 * Copyright (c) 2022 Jan Privara
 * SPDX-License-Identifier: MIT
 */

#ifndef NEC_PROT_TYPES_H
#define NEC_PROT_TYPES_H

#include <inttypes.h>

/**
 * IR waveform data.
 * Each sample represents a pulse or space duration in microseconds (usec).
 * The even samples are pulses, odd are spaces.
 * The first sample (`samples[0]`) is always a pulse duration.
 */
typedef struct
{
	uint16_t length;    // number of samples
	uint32_t * samples;
} nec_waveform_t;

/**
 * NEC decoded data - address and command
 */
typedef struct
{
    uint16_t address;
    uint16_t command;
} nec_decoded_data_t;

#endif //NEC_PROT_TYPES_H
