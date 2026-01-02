/*
 * Copyright (c) 2022 Jan Privara
 * SPDX-License-Identifier: MIT
 */

#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include "nec_prot.h"
}

TEST_GROUP(NEC_Encode)
{

};

TEST(NEC_Encode, All_Combinations)
{
    uint32_t waveform_samples[NEC_SAMPLES];

    nec_waveform_t waveform = {
        .samples = waveform_samples
    };

    nec_decoded_data_t dec_res;

    for (int addr = 0; addr < 256; addr++) {
        for (int cmd = 0; cmd < 256; cmd++) {
            nec_encode(addr, cmd, &waveform);

            nec_decode(&waveform, &dec_res);

            CHECK(dec_res.address == addr);
            CHECK(dec_res.command == cmd);
        }
    }
}
