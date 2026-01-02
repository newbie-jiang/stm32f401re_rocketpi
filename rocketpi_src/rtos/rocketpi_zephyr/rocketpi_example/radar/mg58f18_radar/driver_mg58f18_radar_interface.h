/**
 * @file driver_mg58f18_radar_interface.h
 * @brief MG58F18 雷达驱动的硬件适配层。
 *
 * 提供协议层与 UART/DMA 的桥接函数，应用请直接调用 driver_mg58f18_radar.h
 * 中的 API，避免直接依赖底层实现细节。
 */

#ifndef DRIVER_MG58F18_RADAR_INTERFACE_H
#define DRIVER_MG58F18_RADAR_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "driver_mg58f18_radar.h"

/**
 * @brief 初始化 UART/DMA 等硬件资源并准备接收。
 */
mg58f18_radar_status_t mg58f18_radar_interface_hw_init(void);
/**
 * @brief 通过 DMA 发送一帧数据（带超时等待）。
 */
mg58f18_radar_status_t mg58f18_radar_interface_hw_send(const uint8_t *data,
                                                       size_t length,
                                                       uint32_t timeout_ms);
/**
 * @brief 获取毫秒滴答，用于等待/超时判断。
 */
uint32_t mg58f18_radar_interface_hw_get_tick(void);
/**
 * @brief 重新启动 DMA 空闲接收（出错后调用）。
 */
void mg58f18_radar_interface_hw_restart_rx(void);

/**
 * @brief 读取雷达 OUT 引脚电平。
 *
 * @return GPIO 为高电平返回 true，否则返回 false。
 */
bool mg58f18_radar_interface_hw_read_io(void);

#ifdef __cplusplus
}
#endif

#endif /* DRIVER_MG58F18_RADAR_INTERFACE_H */
