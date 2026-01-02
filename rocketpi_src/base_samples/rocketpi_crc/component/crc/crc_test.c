/*
 * Simple bare-metal friendly CRC self-tests.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "crc_test.h"

#include "crc.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

typedef uint32_t (*crc_test_func_t)(void);

typedef struct {
	const char *name;
	crc_test_func_t dut;
	crc_test_func_t reference_override;
	uint8_t width;
	uint32_t polynomial;
	uint32_t seed;
	uint32_t xor_out;
	bool lsb_variant;
} crc_test_case_t;

static const uint8_t kTestPayload[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
static const size_t kTestPayloadSize = ARRAY_SIZE(kTestPayload);

/* 根据CRC位宽生成对应的掩码，用于裁剪中间结果 */
static uint32_t crc_mask(uint8_t width)
{
	if (width >= 32U) {
		return 0xFFFFFFFFU;
	}

	return (uint32_t)((1ULL << width) - 1ULL);
}

/* 按MSB先行方式计算参考CRC结果，用于非反射CRC对比 */
static uint32_t crc_reference_msb(const uint8_t *data, size_t len, uint8_t width,
				  uint32_t polynomial, uint32_t seed, uint32_t xor_out)
{
	if (width == 0U) {
		return 0U;
	}

	const uint64_t mask = (width >= 32U) ? 0xFFFFFFFFULL : ((1ULL << width) - 1ULL);
	const uint64_t topbit = 1ULL << (width - 1U);
	const uint64_t poly = ((uint64_t)polynomial) & mask;
	const uint64_t xor_masked = ((uint64_t)xor_out) & mask;
	uint64_t crc = ((uint64_t)seed) & mask;

	for (size_t i = 0; i < len; ++i) {
		uint8_t cur = data[i];

		for (uint8_t bit = 0U; bit < 8U; ++bit) {
			if ((cur & 0x80U) != 0U) {
				crc ^= topbit;
			}

			cur = (uint8_t)((cur << 1U) & 0xFFU);

			if ((crc & topbit) != 0U) {
				crc = ((crc << 1U) & mask) ^ poly;
			} else {
				crc = (crc << 1U) & mask;
			}
		}
	}

	crc ^= xor_masked;
	return (uint32_t)(crc & mask);
}

/* 按LSB先行方式计算参考CRC结果，用于反射CRC对比 */
static uint32_t crc_reference_lsb(const uint8_t *data, size_t len, uint8_t width,
				  uint32_t polynomial, uint32_t seed, uint32_t xor_out)
{
	if (width == 0U) {
		return 0U;
	}

	const uint64_t mask = (width >= 32U) ? 0xFFFFFFFFULL : ((1ULL << width) - 1ULL);
	const uint64_t poly = ((uint64_t)polynomial) & mask;
	const uint64_t xor_masked = ((uint64_t)xor_out) & mask;
	uint64_t crc = ((uint64_t)seed) & mask;

	for (size_t i = 0; i < len; ++i) {
		crc ^= data[i];

		for (uint8_t bit = 0U; bit < 8U; ++bit) {
			if ((crc & 0x01ULL) != 0U) {
				crc = (crc >> 1U) ^ poly;
			} else {
				crc >>= 1U;
			}

			crc &= mask;
		}
	}

	crc ^= xor_masked;
	return (uint32_t)(crc & mask);
}

/* Device-under-test helpers */
/* 计算单次CRC32 IEEE基准值 */
static uint32_t dut_crc32_ieee(void)
{
	return crc32_ieee(kTestPayload, kTestPayloadSize);
}

/* 分块调用CRC32 IEEE更新接口验证持续性 */
static uint32_t dut_crc32_ieee_chunked(void)
{
	const size_t split = 4U;
	uint32_t crc = crc32_ieee_update(0U, kTestPayload, split);

	return crc32_ieee_update(crc, &kTestPayload[split], kTestPayloadSize - split);
}

/* 验证CRC32C Castagnoli软件实现 */
static uint32_t dut_crc32_c(void)
{
	return crc32_c(0U, kTestPayload, kTestPayloadSize, true, true);
}

/* 验证CRC32K/4.2查表实现 */
static uint32_t dut_crc32_k(void)
{
	return crc32_k_4_2_update(0U, kTestPayload, kTestPayloadSize);
}

/* 验证PGP CRC24一次性计算 */
static uint32_t dut_crc24_pgp(void)
{
	return crc24_pgp(kTestPayload, kTestPayloadSize) & CRC24_FINAL_VALUE_MASK;
}

/* 分块调用PGP CRC24以测试上下文保持 */
static uint32_t dut_crc24_pgp_chunked(void)
{
	const size_t split = 3U;
	uint32_t crc = crc24_pgp_update(CRC24_PGP_INITIAL_VALUE, kTestPayload, split);

	crc = crc24_pgp_update(crc, &kTestPayload[split], kTestPayloadSize - split);

	return crc & CRC24_FINAL_VALUE_MASK;
}

/* 验证RTCM3使用的CRC24Q */
static uint32_t dut_crc24q(void)
{
	return crc24q_rtcm3(kTestPayload, kTestPayloadSize) & CRC24_FINAL_VALUE_MASK;
}

/* 验证CRC16多项式0x1021实现 */
static uint32_t dut_crc16(void)
{
	return crc16(0x1021U, 0xFFFFU, kTestPayload, kTestPayloadSize);
}

/* 验证CRC16 CCITT非反射实现 */
static uint32_t dut_crc16_ccitt(void)
{
	return crc16_ccitt(0xFFFFU, kTestPayload, kTestPayloadSize);
}

/* 验证CRC16反射算法实现 */
static uint32_t dut_crc16_reflect(void)
{
	return crc16_reflect(0xA001U, 0xFFFFU, kTestPayload, kTestPayloadSize);
}

/* 验证ITU-T CRC16实现 */
static uint32_t dut_crc16_itu_t(void)
{
	return crc16_itu_t(0x0000U, kTestPayload, kTestPayloadSize);
}

/* 验证CRC8 CCITT实现 */
static uint32_t dut_crc8_ccitt(void)
{
	return crc8_ccitt(0x00U, kTestPayload, kTestPayloadSize);
}

/* 验证ROHC CRC8实现 */
static uint32_t dut_crc8_rohc(void)
{
	return crc8_rohc(0xFFU, kTestPayload, kTestPayloadSize);
}

/* 验证可配置的CRC8 MSB模式 */
static uint32_t dut_crc8_msb(void)
{
	return crc8(kTestPayload, kTestPayloadSize, 0x07U, 0x00U, false);
}

/* 验证可配置的CRC8 LSB模式 */
static uint32_t dut_crc8_lsb(void)
{
	return crc8(kTestPayload, kTestPayloadSize, 0x07U, 0x00U, true);
}

/* 验证CRC4 MSB模式 */
static uint32_t dut_crc4_msb(void)
{
	return crc4(kTestPayload, kTestPayloadSize, 0x03U, 0x00U, false) & 0xFU;
}

/* 验证CRC4 LSB模式 */
static uint32_t dut_crc4_lsb(void)
{
	return crc4(kTestPayload, kTestPayloadSize, 0x03U, 0x00U, true) & 0xFU;
}

/* 验证TI特定的CRC4算法 */
static uint32_t dut_crc4_ti(void)
{
	return crc4_ti(0x00U, kTestPayload, kTestPayloadSize) & 0xFU;
}

/* 验证CRC7大端实现 */
static uint32_t dut_crc7(void)
{
	return crc7_be(0x00U, kTestPayload, kTestPayloadSize) & 0x7FU;
}

/* 手写CRC4 LSB参考实现，用于校验API */
static uint32_t ref_crc4_reversed(void)
{
	uint8_t crc = 0x00U;

	for (size_t i = 0; i < kTestPayloadSize; ++i) {
		const uint8_t byte = kTestPayload[i];

		for (uint8_t nibble = 0U; nibble < 2U; ++nibble) {
			const uint8_t shift = (1U - nibble) * 4U;
			const uint8_t part = (byte >> shift) & 0x0FU;

			crc ^= part;

			for (uint8_t bit = 0U; bit < 4U; ++bit) {
				if ((crc & 0x01U) != 0U) {
					crc = (uint8_t)((crc >> 1U) ^ 0x03U);
				} else {
					crc >>= 1U;
				}
			}
		}
	}

	return (uint32_t)(crc & 0x0FU);
}

/* 手写CRC7参考实现，用于校验API */
static uint32_t ref_crc7(void)
{
	uint8_t seed = 0x00U;

	for (size_t i = 0; i < kTestPayloadSize; ++i) {
		uint8_t e = seed ^ kTestPayload[i];
		uint8_t f = e ^ (e >> 4U) ^ (e >> 7U);

		seed = (uint8_t)(((f << 1U) ^ (f << 4U)) & 0xFFU);
	}

	return (uint32_t)(seed & 0x7FU);
}

static const crc_test_case_t kTestCases[] = {
	{"crc32_ieee", dut_crc32_ieee, NULL, 32U, 0xEDB88320U, 0xFFFFFFFFU, 0xFFFFFFFFU, true},
	{"crc32_ieee_stream", dut_crc32_ieee_chunked, NULL, 32U, 0xEDB88320U, 0xFFFFFFFFU, 0xFFFFFFFFU,
	 true},
	{"crc32_c", dut_crc32_c, NULL, 32U, 0x82F63B78U, 0xFFFFFFFFU, 0xFFFFFFFFU, true},
	{"crc32_k_4_2", dut_crc32_k, NULL, 32U, 0x93A409EBU, 0x00000000U, 0x00000000U, false},
	{"crc24_pgp", dut_crc24_pgp, NULL, 24U, CRC24_PGP_POLY, CRC24_PGP_INITIAL_VALUE, 0x000000U, false},
	{"crc24_pgp_stream", dut_crc24_pgp_chunked, NULL, 24U, CRC24_PGP_POLY, CRC24_PGP_INITIAL_VALUE,
	 0x000000U, false},
	{"crc24q_rtcm3", dut_crc24q, NULL, 24U, 0x01864CFBU, 0x000000U, 0x000000U, false},
	{"crc16", dut_crc16, NULL, 16U, 0x1021U, 0xFFFFU, 0x0000U, false},
	{"crc16_ccitt", dut_crc16_ccitt, NULL, 16U, 0x8408U, 0xFFFFU, 0x0000U, true},
	{"crc16_reflect", dut_crc16_reflect, NULL, 16U, 0xA001U, 0xFFFFU, 0x0000U, true},
	{"crc16_itu_t", dut_crc16_itu_t, NULL, 16U, 0x1021U, 0x0000U, 0x0000U, false},
	{"crc8_ccitt", dut_crc8_ccitt, NULL, 8U, 0x07U, 0x00U, 0x00U, false},
	{"crc8_rohc", dut_crc8_rohc, NULL, 8U, 0xE0U, 0xFFU, 0x00U, true},
	{"crc8_msb_api", dut_crc8_msb, NULL, 8U, 0x07U, 0x00U, 0x00U, false},
	{"crc8_lsb_api", dut_crc8_lsb, NULL, 8U, 0x07U, 0x00U, 0x00U, true},
	{"crc4", dut_crc4_msb, NULL, 4U, 0x03U, 0x00U, 0x00U, false},
	{"crc4_reversed", dut_crc4_lsb, ref_crc4_reversed, 4U, 0x03U, 0x00U, 0x00U, true},
	{"crc4_ti", dut_crc4_ti, NULL, 4U, 0x03U, 0x00U, 0x00U, false},
	{"crc7_be", dut_crc7, ref_crc7, 7U, 0x09U, 0x00U, 0x00U, false},
};

static crc_test_result_t g_results[ARRAY_SIZE(kTestCases)];
static bool g_tests_run;

/* 根据测试用例属性选择合适的参考实现 */
static uint32_t run_reference(const crc_test_case_t *tc)
{
	if (tc->reference_override != NULL) {
		return tc->reference_override();
	}

	if (tc->lsb_variant) {
		return crc_reference_lsb(kTestPayload, kTestPayloadSize, tc->width, tc->polynomial,
					 tc->seed, tc->xor_out);
	}

	return crc_reference_msb(kTestPayload, kTestPayloadSize, tc->width, tc->polynomial,
				 tc->seed, tc->xor_out);
}

/* 运行全部CRC测试并记录结果，返回总体是否通过 */
bool crc_run_all_tests(void)
{
	bool all_pass = true;

	for (size_t i = 0; i < ARRAY_SIZE(kTestCases); ++i) {
		const crc_test_case_t *tc = &kTestCases[i];
		const uint32_t expected = run_reference(tc) & crc_mask(tc->width);
		const uint32_t actual = tc->dut() & crc_mask(tc->width);

		g_results[i].name = tc->name;
		g_results[i].expected = expected;
		g_results[i].actual = actual;
		g_results[i].passed = (expected == actual);

		if (!g_results[i].passed) {
			all_pass = false;
		}
	}

	g_tests_run = true;
	return all_pass;
}

/* 返回当前注册的CRC测试用例数量 */
size_t crc_get_test_count(void)
{
	return ARRAY_SIZE(kTestCases);
}

/* 返回最近一次执行的CRC测试结果数组 */
const crc_test_result_t *crc_get_test_results(void)
{
	if (!g_tests_run) {
		(void)crc_run_all_tests();
	}

	return g_results;
}
