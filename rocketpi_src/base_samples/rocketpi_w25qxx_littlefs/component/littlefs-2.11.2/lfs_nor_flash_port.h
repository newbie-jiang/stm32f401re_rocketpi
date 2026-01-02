/**
 * @file lfs_nor_flash_port.h
 * @brief W25Qxx SPI NOR flash-backed block device glue for littlefs.
 */

#ifndef LFS_NOR_FLASH_PORT_H
#define LFS_NOR_FLASH_PORT_H

#include "lfs.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef LFS_NOR_FLASH_TOTAL_SIZE
/* Default to W25Q64 (8 MiB) if caller does not override. */
#define LFS_NOR_FLASH_TOTAL_SIZE   (8UL * 1024UL * 1024UL)
#endif

#ifndef LFS_NOR_FLASH_BLOCK_SIZE
#define LFS_NOR_FLASH_BLOCK_SIZE   (4UL * 1024UL) /* Use 4 KiB erase blocks */
#endif

#define LFS_NOR_FLASH_BLOCK_COUNT  (LFS_NOR_FLASH_TOTAL_SIZE / LFS_NOR_FLASH_BLOCK_SIZE)

extern const struct lfs_config lfs_nor_flash_port_cfg;

int lfs_nor_flash_port_format(lfs_t *lfs);
int lfs_nor_flash_port_mount(lfs_t *lfs);
int lfs_nor_flash_port_unmount(lfs_t *lfs);

#ifdef __cplusplus
}
#endif

#endif /* LFS_NOR_FLASH_PORT_H */
