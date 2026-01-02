/*
 * Copyright (c) 2022 Jan Privara
 * SPDX-License-Identifier: MIT
 */

#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <string.h> // memcpy

extern "C"
{
#include "nec_prot.h"
}

TEST_GROUP(NEC_Raw_Bits)
{
    static const int n_samples = 67;

    uint32_t waveform_nec0[n_samples] = {
        9000,4500, // leading pulse and space
        562,  562, // address[0]
        562,  562, // address[1]
        562,  562, // address[2]
        562,  562, // address[3]
        562,  562, // address[4]
        562,  562, // address[5]
        562,  562, // address[6]
        562,  562, // address[7]
        562, 1687, // inverted address[0]
        562, 1687, // inverted address[1]
        562, 1687, // inverted address[2]
        562, 1687, // inverted address[3]
        562, 1687, // inverted address[4]
        562, 1687, // inverted address[5]
        562, 1687, // inverted address[6]
        562, 1687, // inverted address[7]
        562,  562, // command[0]
        562,  562, // command[1]
        562,  562, // command[2]
        562,  562, // command[3]
        562,  562, // command[4]
        562,  562, // command[5]
        562,  562, // command[6]
        562,  562, // command[7]
        562, 1687, // inverted command[0]
        562, 1687, // inverted command[1]
        562, 1687, // inverted command[2]
        562, 1687, // inverted command[3]
        562, 1687, // inverted command[4]
        562, 1687, // inverted command[5]
        562, 1687, // inverted command[6]
        562, 1687, // inverted command[7]
        487        // trailing pulse
    };

    uint32_t waveform_nec1[n_samples] = {
        9000,4500, // leading pulse and space
        562, 1687, // address[0]
        562, 1687, // address[1]
        562, 1687, // address[2]
        562, 1687, // address[3]
        562, 1687, // address[4]
        562, 1687, // address[5]
        562, 1687, // address[6]
        562, 1687, // address[7]
        562,  562, // inverted address[0]
        562,  562, // inverted address[1]
        562,  562, // inverted address[2]
        562,  562, // inverted address[3]
        562,  562, // inverted address[4]
        562,  562, // inverted address[5]
        562,  562, // inverted address[6]
        562,  562, // inverted address[7]
        562, 1687, // command[0]
        562, 1687, // command[1]
        562, 1687, // command[2]
        562, 1687, // command[3]
        562, 1687, // command[4]
        562, 1687, // command[5]
        562, 1687, // command[6]
        562, 1687, // command[7]
        562,  562, // inverted command[0]
        562,  562, // inverted command[1]
        562,  562, // inverted command[2]
        562,  562, // inverted command[3]
        562,  562, // inverted command[4]
        562,  562, // inverted command[5]
        562,  562, // inverted command[6]
        562,  562, // inverted command[7]
        487        // trailing pulse
    };
};

TEST(NEC_Raw_Bits, Ones)
{
    nec_waveform_t waveform = {
        .length = n_samples,
        .samples = waveform_nec0
    };

    nec_decoded_data_t dec_res;
    nec_decode(&waveform, &dec_res);

    CHECK(dec_res.address == 0);
    CHECK(dec_res.command == 0);
}

TEST(NEC_Raw_Bits, Zeros)
{
    nec_waveform_t waveform = {
        .length = n_samples,
        .samples = waveform_nec1
    };

    nec_decoded_data_t dec_res;
    nec_decode(&waveform, &dec_res);

    CHECK(dec_res.address == 0xff);
    CHECK(dec_res.command == 0xff);
}

TEST(NEC_Raw_Bits, All_Combinations)
{
    uint32_t waveform_samples[n_samples];

    nec_waveform_t waveform = {
        .length = n_samples,
        .samples = waveform_samples
    };

    uint32_t * addr_lsb = &waveform_samples[2];
    uint32_t * inv_addr_lsb = addr_lsb+(2*8);
    uint32_t * cmd_lsb = inv_addr_lsb+(2*8);
    uint32_t * inv_cmd_lsb = cmd_lsb+(2*8);

    nec_decoded_data_t dec_res;

    for (int addr = 0; addr < 256; addr++) {
        for (int cmd = 0; cmd < 256; cmd++) {
            memcpy(waveform_samples, waveform_nec0, sizeof(waveform_nec0));

            for (int b = 0; b < 8; b++) {
                if (addr & (1 << b)) {
                    addr_lsb[(b*2)+1] = 1687;
                    inv_addr_lsb[(b*2)+1] = 562;
                }
            }

            for (int b = 0; b < 8; b++) {
                if (cmd & (1 << b)) {
                    cmd_lsb[(b*2)+1] = 1687;
                    inv_cmd_lsb[(b*2)+1] = 562;
                }
            }

            nec_decode(&waveform, &dec_res);

            CHECK(dec_res.address == addr);
            CHECK(dec_res.command == cmd);
        }
    }
}
