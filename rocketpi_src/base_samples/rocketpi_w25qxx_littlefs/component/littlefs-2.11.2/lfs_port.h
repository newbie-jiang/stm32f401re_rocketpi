
/**
 * @file lfs_port.h
 * @brief Compatibility shim kept for existing includes. Prefer lfs_flash_port.h.
 */

#ifndef LFS_PORT_H
#define LFS_PORT_H

#include "lfs_flash_port.h"

#ifdef __cplusplus
extern "C" {
#endif

#define lfs_port_cfg lfs_flash_port_cfg

static inline int lfs_port_format(lfs_t *lfs)
{
    return lfs_flash_port_format(lfs);
}

static inline int lfs_port_mount(lfs_t *lfs)
{
    return lfs_flash_port_mount(lfs);
}

static inline int lfs_port_unmount(lfs_t *lfs)
{
    return lfs_flash_port_unmount(lfs);
}

#ifdef __cplusplus
}
#endif

#endif /* LFS_PORT_H */
