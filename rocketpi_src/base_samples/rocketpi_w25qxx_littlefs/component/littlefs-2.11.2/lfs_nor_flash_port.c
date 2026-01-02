#include "lfs_nor_flash_port.h"

#include "driver_w25qxx_interface.h"

#include <stdbool.h>

#define LFS_NOR_FLASH_PAGE_SIZE          (256U)
#define LFS_NOR_FLASH_READ_SIZE          (16U)
#define LFS_NOR_FLASH_PROG_SIZE          (LFS_NOR_FLASH_PAGE_SIZE)
#define LFS_NOR_FLASH_CACHE_SIZE         (LFS_NOR_FLASH_PAGE_SIZE)
#define LFS_NOR_FLASH_LOOKAHEAD_SIZE     (16U)

static w25qxx_handle_t g_w25qxx_handle;
static bool g_w25qxx_ready = false;

static int lfs_nor_flash_read(const struct lfs_config *cfg, lfs_block_t block,
                              lfs_off_t off, void *buffer, lfs_size_t size);
static int lfs_nor_flash_prog(const struct lfs_config *cfg, lfs_block_t block,
                              lfs_off_t off, const void *buffer, lfs_size_t size);
static int lfs_nor_flash_erase(const struct lfs_config *cfg, lfs_block_t block);
static int lfs_nor_flash_sync(const struct lfs_config *cfg);

static bool lfs_nor_flash_range_valid(lfs_block_t block, lfs_off_t off, lfs_size_t size)
{
    if (block >= LFS_NOR_FLASH_BLOCK_COUNT) {
        return false;
    }

    if (off + size > LFS_NOR_FLASH_BLOCK_SIZE) {
        return false;
    }

    return true;
}

static int lfs_nor_flash_hw_init(void)
{
    if (g_w25qxx_ready) {
        return LFS_ERR_OK;
    }

    DRIVER_W25QXX_LINK_INIT(&g_w25qxx_handle, w25qxx_handle_t);
    DRIVER_W25QXX_LINK_SPI_QSPI_INIT(&g_w25qxx_handle, w25qxx_interface_spi_qspi_init);
    DRIVER_W25QXX_LINK_SPI_QSPI_DEINIT(&g_w25qxx_handle, w25qxx_interface_spi_qspi_deinit);
    DRIVER_W25QXX_LINK_SPI_QSPI_WRITE_READ(&g_w25qxx_handle, w25qxx_interface_spi_qspi_write_read);
    DRIVER_W25QXX_LINK_DELAY_MS(&g_w25qxx_handle, w25qxx_interface_delay_ms);
    DRIVER_W25QXX_LINK_DELAY_US(&g_w25qxx_handle, w25qxx_interface_delay_us);
    DRIVER_W25QXX_LINK_DEBUG_PRINT(&g_w25qxx_handle, w25qxx_interface_debug_print);

    uint8_t res = w25qxx_set_type(&g_w25qxx_handle, W25Q64);
    if (res != 0U) {
        return LFS_ERR_IO;
    }

    res = w25qxx_set_interface(&g_w25qxx_handle, W25QXX_INTERFACE_SPI);
    if (res != 0U) {
        return LFS_ERR_IO;
    }

    res = w25qxx_set_dual_quad_spi(&g_w25qxx_handle, W25QXX_BOOL_FALSE);
    if (res != 0U) {
        return LFS_ERR_IO;
    }

    res = w25qxx_init(&g_w25qxx_handle);
    if (res != 0U) {
        return LFS_ERR_IO;
    }

    g_w25qxx_ready = true;
    return LFS_ERR_OK;
}

static void lfs_nor_flash_hw_deinit(void)
{
    if (!g_w25qxx_ready) {
        return;
    }

    (void)w25qxx_deinit(&g_w25qxx_handle);
    g_w25qxx_ready = false;
}

const struct lfs_config lfs_nor_flash_port_cfg = {
    .context = NULL,
    .read = lfs_nor_flash_read,
    .prog = lfs_nor_flash_prog,
    .erase = lfs_nor_flash_erase,
    .sync = lfs_nor_flash_sync,
    .read_size = LFS_NOR_FLASH_READ_SIZE,
    .prog_size = LFS_NOR_FLASH_PROG_SIZE,
    .block_size = LFS_NOR_FLASH_BLOCK_SIZE,
    .block_count = LFS_NOR_FLASH_BLOCK_COUNT,
    .cache_size = LFS_NOR_FLASH_CACHE_SIZE,
    .lookahead_size = LFS_NOR_FLASH_LOOKAHEAD_SIZE,
    .block_cycles = 500,
};

static int lfs_nor_flash_read(const struct lfs_config *cfg, lfs_block_t block,
                              lfs_off_t off, void *buffer, lfs_size_t size)
{
    (void)cfg;

    if (!lfs_nor_flash_range_valid(block, off, size)) {
        return LFS_ERR_CORRUPT;
    }

    if (!g_w25qxx_ready && lfs_nor_flash_hw_init() != LFS_ERR_OK) {
        return LFS_ERR_IO;
    }

    uint32_t address = (uint32_t)(block * LFS_NOR_FLASH_BLOCK_SIZE + off);

    if (w25qxx_read(&g_w25qxx_handle, address, buffer, size) != 0U) {
        return LFS_ERR_IO;
    }

    return LFS_ERR_OK;
}

static int lfs_nor_flash_prog(const struct lfs_config *cfg, lfs_block_t block,
                              lfs_off_t off, const void *buffer, lfs_size_t size)
{
    (void)cfg;

    if (!lfs_nor_flash_range_valid(block, off, size)) {
        return LFS_ERR_CORRUPT;
    }

    if (!g_w25qxx_ready && lfs_nor_flash_hw_init() != LFS_ERR_OK) {
        return LFS_ERR_IO;
    }

    uint32_t address = (uint32_t)(block * LFS_NOR_FLASH_BLOCK_SIZE + off);
    const uint8_t *data = (const uint8_t *)buffer;

    while (size > 0U) {
        uint32_t page_offset = address % LFS_NOR_FLASH_PAGE_SIZE;
        uint32_t chunk = LFS_NOR_FLASH_PAGE_SIZE - page_offset;
        if (chunk > size) {
            chunk = size;
        }

        if (w25qxx_page_program(&g_w25qxx_handle,
                                address,
                                (uint8_t *)data,
                                (uint16_t)chunk) != 0U) {
            return LFS_ERR_IO;
        }

        address += chunk;
        data += chunk;
        size -= chunk;
    }

    return LFS_ERR_OK;
}

static int lfs_nor_flash_erase(const struct lfs_config *cfg, lfs_block_t block)
{
    (void)cfg;

    if (block >= LFS_NOR_FLASH_BLOCK_COUNT) {
        return LFS_ERR_CORRUPT;
    }

    if (!g_w25qxx_ready && lfs_nor_flash_hw_init() != LFS_ERR_OK) {
        return LFS_ERR_IO;
    }

    uint32_t address = (uint32_t)(block * LFS_NOR_FLASH_BLOCK_SIZE);

    if (w25qxx_sector_erase_4k(&g_w25qxx_handle, address) != 0U) {
        return LFS_ERR_IO;
    }

    return LFS_ERR_OK;
}

static int lfs_nor_flash_sync(const struct lfs_config *cfg)
{
    (void)cfg;
    return LFS_ERR_OK;
}

int lfs_nor_flash_port_format(lfs_t *lfs)
{
    int err = lfs_nor_flash_hw_init();
    if (err != LFS_ERR_OK) {
        return err;
    }

    return lfs_format(lfs, &lfs_nor_flash_port_cfg);
}

int lfs_nor_flash_port_mount(lfs_t *lfs)
{
    int err = lfs_nor_flash_hw_init();
    if (err != LFS_ERR_OK) {
        return err;
    }

    err = lfs_mount(lfs, &lfs_nor_flash_port_cfg);
    if (err != LFS_ERR_OK) {
        lfs_nor_flash_hw_deinit();
    }

    return err;
}

int lfs_nor_flash_port_unmount(lfs_t *lfs)
{
    int err = lfs_unmount(lfs);
    lfs_nor_flash_hw_deinit();
    return err;
}
