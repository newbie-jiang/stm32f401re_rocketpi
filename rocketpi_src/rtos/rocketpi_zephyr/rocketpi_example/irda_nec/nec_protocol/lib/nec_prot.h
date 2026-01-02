/*
 * Copyright (c) 2022 Jan Privara
 * SPDX-License-Identifier: MIT
 */

#ifndef NEC_PROT_H
#define NEC_PROT_H

#include "nec_prot_types.h"

#define NEC_RAW_BITS 32

#define NEC_SAMPLES (2 + (NEC_RAW_BITS * 2) + 1)  // LEADING PULSE and SPACE +
                                                  // 32 DATA BITS +
                                                  // TRAILING PULSE

// Protocol timing
#define NEC_PULSE_US 562

#define NEC_LEAD_PULSE_US (16 * NEC_PULSE_US) // 9000 us
#define NEC_LEAD_SPACE_US (8 * NEC_PULSE_US)  // 4500 us

#define NEC_SPACE_1_US (3 * NEC_PULSE_US)     // 1687 us
#define NEC_SPACE_0_US (NEC_PULSE_US)         // 562 us

// Timing tolerances
#define NEC_PULSE_TOLERANCE (NEC_PULSE_US * 50 / 100)
#define NEC_SPACE_TOLERANCE (NEC_PULSE_US * 60 / 100)

#define NEC_PULSE_US_MIN (NEC_PULSE_US - NEC_PULSE_TOLERANCE)
#define NEC_PULSE_US_MAX (NEC_PULSE_US + NEC_PULSE_TOLERANCE)

#define NEC_LEAD_PULSE_US_MIN (NEC_LEAD_PULSE_US - NEC_PULSE_TOLERANCE)
#define NEC_LEAD_PULSE_US_MAX (NEC_LEAD_PULSE_US + NEC_PULSE_TOLERANCE)

#define NEC_LEAD_SPACE_US_MIN (NEC_LEAD_SPACE_US - NEC_SPACE_TOLERANCE)
#define NEC_LEAD_SPACE_US_MAX (NEC_LEAD_SPACE_US + NEC_SPACE_TOLERANCE)

#define NEC_SPACE_0_US_MIN (NEC_SPACE_0_US - NEC_SPACE_TOLERANCE)
#define NEC_SPACE_0_US_MAX (NEC_SPACE_0_US + NEC_SPACE_TOLERANCE)

#define NEC_SPACE_1_US_MIN (NEC_SPACE_1_US - NEC_SPACE_TOLERANCE)
#define NEC_SPACE_1_US_MAX (NEC_SPACE_1_US + NEC_SPACE_TOLERANCE)

/**
 * @brief Decode NEC
 *
 * Decode NEC message from sequence of pulse/space durations (waveform).
 *
 * @param waveform Pointer to the waveform data structure.
 * @param res      Pointer to the result structure.
 *
 * @return Number of samples "consumed" during the decoding, 0 on decoding
 *         failure.
 */
int nec_decode(const nec_waveform_t * waveform, nec_decoded_data_t * res);

/**
 * @brief Encode NEC
 *
 * Encode NEC command.
 *
 * @param address  Remote devide address.
 * @param command  Command.
 * @param waveform Pointer to the waveform data structure. The size of the 
                   samples array in the target structure must be at least 67.
 *
 */
void nec_encode(uint8_t address, uint8_t command, nec_waveform_t * waveform);

#endif // NEC_PROT_H
