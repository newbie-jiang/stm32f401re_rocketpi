/*
 * Copyright (c) 2022 Jan Privara
 * SPDX-License-Identifier: MIT
 */

#include "nec_prot.h"

//------------------------------------------------------------------------------
static inline int in_range(uint32_t val, uint32_t min, uint32_t max)
{
    if (val >= min && val <= max)
        return 1;
    return 0;
}

//------------------------------------------------------------------------------
int nec_decode_raw_bits(const nec_waveform_t * waveform, uint32_t * raw_bits)
{
    int sample_idx = 0;

    *raw_bits = 0;

    // check there is enough samples
    if (waveform->length < NEC_SAMPLES)
        return 0; 

    // check leading pulse is within range
    if (!in_range(waveform->samples[sample_idx++], 
                  NEC_LEAD_PULSE_US_MIN, NEC_LEAD_PULSE_US_MAX))
        return 0;

    // check leading space is within range
    if (!in_range(waveform->samples[sample_idx++],
                  NEC_LEAD_SPACE_US_MIN, NEC_LEAD_SPACE_US_MAX))
        return 0;

    for (int i = 0; i < NEC_RAW_BITS; i++) {
        uint32_t pulse = waveform->samples[sample_idx++];
        uint32_t space = waveform->samples[sample_idx++];

        // check the data pulse is within range
        if (!in_range(pulse, NEC_PULSE_US_MIN, NEC_PULSE_US_MAX))
            return 0;

        // decode raw bit 0 or 1 based on the space duration
        if (in_range(space, NEC_SPACE_1_US_MIN, NEC_SPACE_1_US_MAX))
            *raw_bits |= (1 << i);
        else if (!in_range(space, NEC_SPACE_0_US_MIN, NEC_SPACE_0_US_MAX))
            return 0;
    }

    // check the trailing pulse is within range
    if (!in_range(waveform->samples[sample_idx++],
                  NEC_PULSE_US_MIN, NEC_PULSE_US_MAX))
        return 0;

    return 1;
}

//------------------------------------------------------------------------------
int nec_decode(const nec_waveform_t * waveform, nec_decoded_data_t * res)
{
    uint32_t raw_bits_buf = 0;

    // decode raw bits
    if (!nec_decode_raw_bits(waveform, &raw_bits_buf))
        return 0;

    uint8_t addr     = (raw_bits_buf)       & 0xff;
    uint8_t addr_inv = (raw_bits_buf >> 8)  & 0xff;
    uint8_t cmd      = (raw_bits_buf >> 16) & 0xff;
    uint8_t cmd_inv  = (raw_bits_buf >> 24) & 0xff;

    // check address equals to inverted address
    if (addr != (~addr_inv & 0xff))
        return 0;

    // check command equals to inverted command
    if (cmd != (~cmd_inv & 0xff))
        return 0;

    // set the result
    res->address = addr;
    res->command = cmd;

    return NEC_SAMPLES;
}

//------------------------------------------------------------------------------
void nec_encode(uint8_t address, uint8_t command, nec_waveform_t * waveform)
{
    uint8_t data_byte[4];

    int sample_idx = 0;

    waveform->samples[sample_idx++] = NEC_LEAD_PULSE_US;
    waveform->samples[sample_idx++] = NEC_LEAD_SPACE_US;

    data_byte[0] = address;
    data_byte[1] = ~address;
    data_byte[2] = command;
    data_byte[3] = ~command;

    for (int b = 0; b < 4; b++) {
        for (int i = 0; i < 8; i++) {
            int data_bit = (data_byte[b] >> i) & 0x1;

            waveform->samples[sample_idx++] = NEC_PULSE_US;

            if (data_bit)
                waveform->samples[sample_idx++] = NEC_SPACE_1_US;
            else
                waveform->samples[sample_idx++] = NEC_SPACE_0_US;
        }
    }

    waveform->samples[sample_idx] = NEC_PULSE_US;

    waveform->length = NEC_SAMPLES;
}
