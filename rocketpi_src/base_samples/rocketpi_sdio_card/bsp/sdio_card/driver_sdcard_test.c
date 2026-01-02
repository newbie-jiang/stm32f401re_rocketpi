#include "driver_sdcard_test.h"

#include <stdio.h>

#define SDCARD_TEST_BUFFER_BYTES           (4U * 1024U)
#define SDCARD_TEST_TIMEOUT_MS             5000U
#define SDCARD_TEST_PROGRESS_STEP_BLOCKS   65536U

/**
 * 设置为 1 使用 HAL SDIO DMA 接口，默认为阻塞方式。
 * DMA 模式也会通过 sdcard_test_wait_ready() 等待完成。
 */
#ifndef SDCARD_TEST_USE_DMA
#define SDCARD_TEST_USE_DMA                1U
#endif

static const char *sdcard_test_card_type_to_string(uint32_t type);
static const char *sdcard_test_card_version_to_string(uint32_t version);
static uint8_t sdcard_test_wait_ready(uint32_t timeout_ms);
static void sdcard_test_fill_pattern(uint8_t *buffer, uint32_t length, uint32_t seed);
static uint8_t sdcard_test_verify_pattern(const uint8_t *buffer, uint32_t length, uint32_t seed);
static void sdcard_test_print_info(const HAL_SD_CardInfoTypeDef *info);
static uint8_t sdcard_test_prepare_card(HAL_SD_CardInfoTypeDef *info);
static uint8_t sdcard_test_process_range(const HAL_SD_CardInfoTypeDef *info, uint32_t block_addr, uint32_t block_count);
static HAL_StatusTypeDef sdcard_test_write_blocks(uint8_t *buffer, uint32_t block_addr, uint32_t block_count);
static HAL_StatusTypeDef sdcard_test_read_blocks(uint8_t *buffer, uint32_t block_addr, uint32_t block_count);

/* 将卡类型枚举转换为可读字符串 */
static const char *sdcard_test_card_type_to_string(uint32_t type)
{
    switch (type)
    {
        case CARD_SDSC:
            return "SDSC";
        case CARD_SDHC_SDXC:
            return "SDHC/SDXC";
        case CARD_SECURED:
            return "Secured";
        default:
            return "Unknown";
    }
}

/* 将卡版本枚举转换为可读字符串 */
static const char *sdcard_test_card_version_to_string(uint32_t version)
{
    switch (version)
    {
        case CARD_V1_X:
            return "1.x";
        case CARD_V2_X:
            return "2.x+";
        default:
            return "Unknown";
    }
}

/* 轮询等待 SD 卡进入传输状态，检测错误与超时 */
static uint8_t sdcard_test_wait_ready(uint32_t timeout_ms)
{
    const uint32_t start = HAL_GetTick();
    HAL_SD_CardStateTypeDef state;

    do
    {
        state = HAL_SD_GetCardState(&hsd);
        if (state == HAL_SD_CARD_TRANSFER)
        {
            return 0U;
        }
        if (state == HAL_SD_CARD_ERROR)
        {
            printf("sdcard: card reports error state\r\n");

            return 1U;
        }
        HAL_Delay(1U);
    }
    while ((HAL_GetTick() - start) < timeout_ms);

    printf("sdcard: wait ready timeout, last state=0x%08lX\r\n", (unsigned long)state);

    return 1U;
}

/* 写指定数量的连续块，按配置选择 DMA 或阻塞模式 */
static HAL_StatusTypeDef sdcard_test_write_blocks(uint8_t *buffer, uint32_t block_addr, uint32_t block_count)
{
#if SDCARD_TEST_USE_DMA
    return HAL_SD_WriteBlocks_DMA(&hsd, buffer, block_addr, block_count);
#else
    return HAL_SD_WriteBlocks(&hsd, buffer, block_addr, block_count, HAL_MAX_DELAY);
#endif
}

/* 读指定数量的连续块，按配置选择 DMA 或阻塞模式 */
static HAL_StatusTypeDef sdcard_test_read_blocks(uint8_t *buffer, uint32_t block_addr, uint32_t block_count)
{
#if SDCARD_TEST_USE_DMA
    return HAL_SD_ReadBlocks_DMA(&hsd, buffer, block_addr, block_count);
#else
    return HAL_SD_ReadBlocks(&hsd, buffer, block_addr, block_count, HAL_MAX_DELAY);
#endif
}

/* 生成与种子相关的线性递增数据，用于写卡测试 */
static void sdcard_test_fill_pattern(uint8_t *buffer, uint32_t length, uint32_t seed)
{
    for (uint32_t i = 0U; i < length; ++i)
    {
        buffer[i] = (uint8_t)((i + seed) & 0xFFU);
    }
}

/* 校验读回数据是否与预期模式一致 */
static uint8_t sdcard_test_verify_pattern(const uint8_t *buffer, uint32_t length, uint32_t seed)
{
    for (uint32_t i = 0U; i < length; ++i)
    {
        const uint8_t expected = (uint8_t)((i + seed) & 0xFFU);

        if (buffer[i] != expected)
        {
            printf("sdcard: data mismatch at byte %lu (expected=0x%02X, read=0x%02X)\r\n",
                   (unsigned long)i,
                   expected,
                   buffer[i]);

            return 1U;
        }
    }

    return 0U;
}

/* 打印卡类型、容量、逻辑块等关键信息 */
static void sdcard_test_print_info(const HAL_SD_CardInfoTypeDef *info)
{
    const uint64_t capacity_bytes = (uint64_t)info->LogBlockNbr * (uint64_t)info->LogBlockSize;
    const uint32_t capacity_mb = (uint32_t)(capacity_bytes / (1024ULL * 1024ULL));

    printf("sdcard: type=%s, version=%s, class=%lu\r\n",
           sdcard_test_card_type_to_string(info->CardType),
           sdcard_test_card_version_to_string(info->CardVersion),
           (unsigned long)info->Class);
    printf("sdcard: RCA=%lu, logical blocks=%lu, logical block size=%lu bytes\r\n",
           (unsigned long)info->RelCardAdd,
           (unsigned long)info->LogBlockNbr,
           (unsigned long)info->LogBlockSize);
    printf("sdcard: capacity=%lu MB\r\n", (unsigned long)capacity_mb);
}

/* 初始化 SD 卡、配置 1bit 总线并获取卡信息 */
static uint8_t sdcard_test_prepare_card(HAL_SD_CardInfoTypeDef *info)
{
    HAL_StatusTypeDef status;

    status = HAL_SD_InitCard(&hsd);
    if (status != HAL_OK)
    {
        printf("sdcard: HAL_SD_InitCard failed (err=0x%08lX)\r\n", (unsigned long)hsd.ErrorCode);

        return 1U;
    }

    status = HAL_SD_ConfigWideBusOperation(&hsd, SDIO_BUS_WIDE_1B);
    if (status != HAL_OK)
    {
        printf("sdcard: HAL_SD_ConfigWideBusOperation failed (err=0x%08lX)\r\n", (unsigned long)hsd.ErrorCode);

        return 1U;
    }

    status = HAL_SD_GetCardInfo(&hsd, info);
    if (status != HAL_OK)
    {
        printf("sdcard: HAL_SD_GetCardInfo failed (err=0x%08lX)\r\n", (unsigned long)hsd.ErrorCode);

        return 1U;
    }

    return 0U;
}

/* 在给定块范围内进行写入、读取和数据比对，带进度输出 */
static uint8_t sdcard_test_process_range(const HAL_SD_CardInfoTypeDef *info, uint32_t block_addr, uint32_t block_count)
{
    const uint32_t block_size = info->LogBlockSize;
    uint8_t buffer[SDCARD_TEST_BUFFER_BYTES];
    uint32_t remaining_blocks;
    uint32_t processed_blocks = 0U;
    uint32_t total_blocks;
    uint32_t max_chunk_blocks;
    uint32_t last_report = 0U;

    if (block_size == 0U)
    {
        printf("sdcard: invalid block size\r\n");

        return 1U;
    }

    max_chunk_blocks = SDCARD_TEST_BUFFER_BYTES / block_size;
    if (max_chunk_blocks == 0U)
    {
        printf("sdcard: configured buffer %lu bytes is smaller than a block (%lu bytes)\r\n",
               (unsigned long)SDCARD_TEST_BUFFER_BYTES,
               (unsigned long)block_size);

        return 1U;
    }

    if (block_addr >= info->LogBlockNbr)
    {
        printf("sdcard: start block %lu is outside the card (max %lu)\r\n",
               (unsigned long)block_addr,
               (unsigned long)(info->LogBlockNbr - 1U));

        return 1U;
    }

    if ((block_count == 0U) || (block_addr + block_count > info->LogBlockNbr))
    {
        remaining_blocks = info->LogBlockNbr - block_addr;
    }
    else
    {
        remaining_blocks = block_count;
    }

    if (remaining_blocks == 0U)
    {
        printf("sdcard: nothing to test\r\n");

        return 1U;
    }

    total_blocks = remaining_blocks;

    printf("sdcard: verifying %lu blocks from address %lu (chunk=%lu blocks)\r\n",
           (unsigned long)total_blocks,
           (unsigned long)block_addr,
           (unsigned long)max_chunk_blocks);

    while (remaining_blocks > 0U)
    {
        const uint32_t chunk_blocks = (remaining_blocks < max_chunk_blocks) ? remaining_blocks : max_chunk_blocks;
        const uint32_t chunk_bytes = chunk_blocks * block_size;
        const uint32_t current_block = block_addr + processed_blocks;
        HAL_StatusTypeDef status;

        sdcard_test_fill_pattern(buffer, chunk_bytes, current_block);

        status = sdcard_test_write_blocks(buffer, current_block, chunk_blocks);
        if (status != HAL_OK)
        {
            printf("sdcard: write failed at block %lu (err=0x%08lX)\r\n",
                   (unsigned long)current_block,
                   (unsigned long)hsd.ErrorCode);

            return 1U;
        }

        if (sdcard_test_wait_ready(SDCARD_TEST_TIMEOUT_MS) != 0U)
        {
            printf("sdcard: card not ready after write at block %lu\r\n", (unsigned long)current_block);

            return 1U;
        }

        status = sdcard_test_read_blocks(buffer, current_block, chunk_blocks);
        if (status != HAL_OK)
        {
            printf("sdcard: read failed at block %lu (err=0x%08lX)\r\n",
                   (unsigned long)current_block,
                   (unsigned long)hsd.ErrorCode);

            return 1U;
        }

        if (sdcard_test_wait_ready(SDCARD_TEST_TIMEOUT_MS) != 0U)
        {
            printf("sdcard: card not ready after read at block %lu\r\n", (unsigned long)current_block);

            return 1U;
        }

        if (sdcard_test_verify_pattern(buffer, chunk_bytes, current_block) != 0U)
        {
            return 1U;
        }

        processed_blocks += chunk_blocks;
        remaining_blocks -= chunk_blocks;

        if ((processed_blocks - last_report) >= SDCARD_TEST_PROGRESS_STEP_BLOCKS || remaining_blocks == 0U)
        {
            printf("sdcard: progress %lu/%lu blocks\r\n",
                   (unsigned long)processed_blocks,
                   (unsigned long)total_blocks);
            last_report = processed_blocks;
        }
    }

    return 0U;
}

/* 仅查询并打印卡信息的测试入口 */
uint8_t sdcard_test_card_info(void)
{
    HAL_SD_CardInfoTypeDef info;

    printf("sdcard: querying card information...\r\n");
    if (sdcard_test_prepare_card(&info) != 0U)
    {
        return 1U;
    }

    sdcard_test_print_info(&info);

    return 0U;
}

/* 从指定起始块开始做一次读写校验 */
uint8_t sdcard_test_block_read_write(uint32_t block_addr, uint32_t block_count)
{
    HAL_SD_CardInfoTypeDef info;

    if (sdcard_test_prepare_card(&info) != 0U)
    {
        return 1U;
    }

    if (sdcard_test_process_range(&info, block_addr, block_count) != 0U)
    {
        return 1U;
    }

    printf("sdcard: block read/write test completed\r\n");

    return 0U;
}

/* 完整的测试流程：初始化、打印信息并遍历测试块区间 */
uint8_t sdcard_test_run(uint32_t block_addr, uint32_t block_count)
{
    HAL_SD_CardInfoTypeDef info;

    printf("sdcard: starting full test sequence\r\n");
    if (sdcard_test_prepare_card(&info) != 0U)
    {
        return 1U;
    }

    sdcard_test_print_info(&info);

    if (sdcard_test_process_range(&info, block_addr, block_count) != 0U)
    {
        return 1U;
    }

    printf("sdcard: test completed successfully\r\n");

    return 0U;
}
