/**
 * @file driver_motor_l9110.h
 * @brief Simple PWM-based driver for the L9110 dual-channel motor bridge.
 *
 * TIM4 CH1 -> INB, TIM4 CH2 -> INA (10 kHz PWM as configured in CubeMX).
 * 使用 TIM4 通道 1/2 分别驱动 INB/INA，10 kHz PWM 刹车式调速。
 */

#ifndef DRIVER_MOTOR_L9110_H
#define DRIVER_MOTOR_L9110_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "tim.h"

typedef enum
{
    MOTOR_L9110_DIR_BRAKE = 0,
    MOTOR_L9110_DIR_FORWARD,
    MOTOR_L9110_DIR_REVERSE
} motor_l9110_direction_t;

/* 初始化 PWM 输出并默认刹车 */
void motor_l9110_init(void);

/* 指定方向和占空比（0-100%），低电平阶段等效刹车
 * @param direction 运动方向（正/反/刹车）
 * @param duty_percent 占空比 0-100%，数值越大越快
 */
void motor_l9110_drive(motor_l9110_direction_t direction, uint8_t duty_percent);

/* 使用有符号占空比：正为正转，负为反转
 * @param duty_percent -100~100，占空比正负代表方向
 */
void motor_l9110_drive_signed(int8_t duty_percent);

/* 立即刹车（双低） */
void motor_l9110_brake(void);

#ifdef __cplusplus
}
#endif

#endif /* DRIVER_MOTOR_L9110_H */
