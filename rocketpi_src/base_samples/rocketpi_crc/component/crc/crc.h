/*
 * Bare-metal friendly CRC interface extracted from Zephyr.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef COMPONENT_CRC_CRC_H_
#define COMPONENT_CRC_CRC_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __weak
#if (defined(__GNUC__) || defined(__clang__)) && !defined(_WIN32)
#define __weak __attribute__((weak))
#else
#define __weak
#endif
#endif

#define CRC24_PGP_INITIAL_VALUE 0x00B704CEU
#define CRC24_PGP_POLY 0x01864CFBU
#define CRC24_FINAL_VALUE_MASK 0x00FFFFFFU

uint16_t crc16(uint16_t poly, uint16_t seed, const uint8_t *src, size_t len);
uint16_t crc16_reflect(uint16_t poly, uint16_t seed, const uint8_t *src, size_t len);
uint16_t crc16_ccitt(uint16_t seed, const uint8_t *src, size_t len);
uint16_t crc16_itu_t(uint16_t seed, const uint8_t *src, size_t len);

uint32_t crc24_pgp(const uint8_t *data, size_t len);
uint32_t crc24_pgp_update(uint32_t crc, const uint8_t *data, size_t len);
uint32_t crc24q_rtcm3(const uint8_t *data, size_t len);

uint32_t crc32_ieee(const uint8_t *data, size_t len);
uint32_t crc32_ieee_update(uint32_t crc, const uint8_t *data, size_t len);
uint32_t crc32_c(uint32_t crc, const uint8_t *data, size_t len, bool first_pkt, bool last_pkt);
uint32_t crc32_k_4_2_update(uint32_t crc, const uint8_t *data, size_t len);

uint8_t crc4(const uint8_t *src, size_t len, uint8_t polynomial, uint8_t initial_value, bool reversed);
uint8_t crc4_ti(uint8_t seed, const uint8_t *src, size_t len);

uint8_t crc7_be(uint8_t seed, const uint8_t *src, size_t len);

uint8_t crc8_ccitt(uint8_t val, const void *buf, size_t cnt);
uint8_t crc8_rohc(uint8_t val, const void *buf, size_t cnt);
uint8_t crc8(const uint8_t *src, size_t len, uint8_t polynomial, uint8_t initial_value, bool reversed);

#ifdef __cplusplus
}
#endif

#endif /* COMPONENT_CRC_CRC_H_ */
