
#include "lfs_port.h"

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_flash_ex.h"

#include <stdbool.h>
#include <string.h>

#define LFS_PORT_PROG_SIZE          (4U)      /* Word programming */
#define LFS_PORT_READ_SIZE          (16U)     /* 128-bit reads */
#define LFS_PORT_CACHE_SIZE         (256U)
#define LFS_PORT_LOOKAHEAD_SIZE     (16U)

#define LFS_PORT_FIRST_SECTOR       FLASH_SECTOR_6

#if defined(FLASH_FLAG_RDERR)
#define LFS_PORT_FLASH_ERROR_FLAGS (FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |     \
                                    FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR |    \
                                    FLASH_FLAG_PGSERR | FLASH_FLAG_RDERR)
#else
#define LFS_PORT_FLASH_ERROR_FLAGS (FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |     \
                                    FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR |    \
                                    FLASH_FLAG_PGSERR)
#endif


static int lfs_port_read(const struct lfs_config *cfg, lfs_block_t block,
                         lfs_off_t off, void *buffer, lfs_size_t size);
static int lfs_port_prog(const struct lfs_config *cfg, lfs_block_t block,
                         lfs_off_t off, const void *buffer, lfs_size_t size);
static int lfs_port_erase(const struct lfs_config *cfg, lfs_block_t block);
static int lfs_port_sync(const struct lfs_config *cfg);
static void lfs_port_clear_flash_flags(void);

const struct lfs_config lfs_port_cfg = {
    .context = NULL,
    .read = lfs_port_read,
    .prog = lfs_port_prog,
    .erase = lfs_port_erase,
    .sync = lfs_port_sync,
    .read_size = LFS_PORT_READ_SIZE,
    .prog_size = LFS_PORT_PROG_SIZE,
    .block_size = LFS_PORT_FLASH_BLOCK_SIZE,
    .block_count = LFS_PORT_FLASH_BLOCK_COUNT,
    .cache_size = LFS_PORT_CACHE_SIZE,
    .lookahead_size = LFS_PORT_LOOKAHEAD_SIZE,
    .block_cycles = 100,
};

static inline uint32_t lfs_port_block_address(lfs_block_t block)
{
    return LFS_PORT_FLASH_START_ADDR + (block * LFS_PORT_FLASH_BLOCK_SIZE);
}

static bool lfs_port_range_valid(lfs_block_t block, lfs_off_t off, lfs_size_t size)
{
    if (block >= LFS_PORT_FLASH_BLOCK_COUNT) {
        return false;
    }

    if (off + size > LFS_PORT_FLASH_BLOCK_SIZE) {
        return false;
    }

    const uint32_t start = lfs_port_block_address(block) + off;
    const uint32_t end = start + size;

    return (start >= LFS_PORT_FLASH_START_ADDR) &&
           (end <= LFS_PORT_FLASH_END_ADDR);
}

static void lfs_port_clear_flash_flags(void)
{
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | LFS_PORT_FLASH_ERROR_FLAGS);
}

static int lfs_port_read(const struct lfs_config *cfg, lfs_block_t block,
                         lfs_off_t off, void *buffer, lfs_size_t size)
{
    (void)cfg;

    if (!lfs_port_range_valid(block, off, size)) {
        return LFS_ERR_CORRUPT;
    }

    const void *src = (const void *)(lfs_port_block_address(block) + off);
    memcpy(buffer, src, size);
    return LFS_ERR_OK;
}

static HAL_StatusTypeDef lfs_port_flash_program(uint32_t address,
                                                const uint8_t *data,
                                                size_t size)
{
    HAL_StatusTypeDef status = HAL_OK;

    while ((address % sizeof(uint32_t)) && size && (status == HAL_OK)) {
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, address, *data);
        address += 1U;
        data += 1U;
        size -= 1U;
    }

    while ((size >= sizeof(uint32_t)) && (status == HAL_OK)) {
        uint32_t word32;
        memcpy(&word32, data, sizeof(word32));
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, word32);
        address += sizeof(uint32_t);
        data += sizeof(uint32_t);
        size -= sizeof(uint32_t);
    }

    while ((size >= sizeof(uint16_t)) && (status == HAL_OK)) {
        uint16_t halfword;
        memcpy(&halfword, data, sizeof(halfword));
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, address, halfword);
        address += sizeof(uint16_t);
        data += sizeof(uint16_t);
        size -= sizeof(uint16_t);
    }

    while (size && (status == HAL_OK)) {
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, address, *data);
        address += 1U;
        data += 1U;
        size -= 1U;
    }

    return status;
}

static int lfs_port_prog(const struct lfs_config *cfg, lfs_block_t block,
                         lfs_off_t off, const void *buffer, lfs_size_t size)
{
    (void)cfg;

    if (!lfs_port_range_valid(block, off, size)) {
        return LFS_ERR_CORRUPT;
    }

    uint32_t address = lfs_port_block_address(block) + off;

    HAL_FLASH_Unlock();
    lfs_port_clear_flash_flags();
    HAL_StatusTypeDef status = lfs_port_flash_program(address,
                                                      (const uint8_t *)buffer,
                                                      size);
    HAL_FLASH_Lock();

    if (status != HAL_OK) {
        return LFS_ERR_IO;
    }

    return LFS_ERR_OK;
}

static int lfs_port_erase(const struct lfs_config *cfg, lfs_block_t block)
{
    (void)cfg;

    if (block >= LFS_PORT_FLASH_BLOCK_COUNT) {
        return LFS_ERR_CORRUPT;
    }

    FLASH_EraseInitTypeDef erase = {
        .TypeErase = FLASH_TYPEERASE_SECTORS,
        .Banks = FLASH_BANK_1,
        .VoltageRange = FLASH_VOLTAGE_RANGE_3,
        .Sector = LFS_PORT_FIRST_SECTOR + block,
        .NbSectors = 1,
    };

    uint32_t sector_error = 0U;

    HAL_FLASH_Unlock();
    lfs_port_clear_flash_flags();
    HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&erase, &sector_error);
    HAL_FLASH_Lock();

    if ((status != HAL_OK) || (sector_error != 0xFFFFFFFFU)) {
        return LFS_ERR_IO;
    }

    return LFS_ERR_OK;
}

static int lfs_port_sync(const struct lfs_config *cfg)
{
    (void)cfg;
    return LFS_ERR_OK;
}

int lfs_port_format(lfs_t *lfs)
{
    return lfs_format(lfs, &lfs_port_cfg);
}

int lfs_port_mount(lfs_t *lfs)
{
    return lfs_mount(lfs, &lfs_port_cfg);
}

int lfs_port_unmount(lfs_t *lfs)
{
    return lfs_unmount(lfs);
}
