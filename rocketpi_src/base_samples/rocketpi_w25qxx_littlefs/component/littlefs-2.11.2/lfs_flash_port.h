/**
 * @file lfs_flash_port.h
 * @brief STM32F401 internal Flash-backed block device glue for littlefs.
 */

#ifndef LFS_FLASH_PORT_H
#define LFS_FLASH_PORT_H

#include "lfs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LFS_FLASH_PORT_START_ADDR   (0x08020000UL)  /* Sector 5 base */
#define LFS_FLASH_PORT_END_ADDR     (0x08080000UL)  /* One past sector 7 */
#define LFS_FLASH_PORT_BLOCK_SIZE   (0x20000UL)     /* 128 KiB sectors */
#define LFS_FLASH_PORT_BLOCK_COUNT  (3U)

extern const struct lfs_config lfs_flash_port_cfg;

int lfs_flash_port_format(lfs_t *lfs);
int lfs_flash_port_mount(lfs_t *lfs);
int lfs_flash_port_unmount(lfs_t *lfs);

#ifdef __cplusplus
}
#endif

#endif /* LFS_FLASH_PORT_H */
