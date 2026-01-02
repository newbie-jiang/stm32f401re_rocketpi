/**
 * @file driver_mg58f18_radar_test.h
 * @brief MG58F18 雷达驱动的冒烟测试入口。
 */

#ifndef DRIVER_MG58F18_RADAR_TEST_H
#define DRIVER_MG58F18_RADAR_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

/** 执行一轮完整的协议交互测试并打印结果。 */
void mg58f18_radar_test_run(void);
/** 持续轮询并打印串口数据与 OUT 引脚变化。 */
void mg58f18_radar_test_poll(void);

#ifdef __cplusplus
}
#endif

#endif /* DRIVER_MG58F18_RADAR_TEST_H */
