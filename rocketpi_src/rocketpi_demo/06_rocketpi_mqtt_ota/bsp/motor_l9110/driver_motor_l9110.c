/**
 * @file driver_motor_l9110.c
 * @brief PWM control helpers for the L9110 motor bridge.
 *
 * Recommended PWM (brake style):
 * - Forward: INB low (TIM4 CH1), INA = PWM (TIM4 CH2)
 * - Reverse: INA low, INB = PWM
 * - Brake:   INA = 0, INB = 0
 * 正转 INB=0、INA=PWM；反转 INA=0、INB=PWM；刹车双低。
 */

#include "driver_motor_l9110.h"

#define MOTOR_L9110_TIMER       (&htim4)
#define MOTOR_L9110_CHANNEL_IB  TIM_CHANNEL_1
#define MOTOR_L9110_CHANNEL_IA  TIM_CHANNEL_2

/* 将 0-100% 占空比换算为 CCR 脉宽 */
static uint32_t motor_l9110_calc_pulse(uint8_t duty_percent)
{
    uint32_t period = MOTOR_L9110_TIMER->Init.Period;
    uint32_t pulse = (period + 1U) * duty_percent / 100U;

    if (pulse > period)
    {
        pulse = period;
    }

    return pulse;
}

/* 同步更新两路，避免瞬间交叉导通 */
static void motor_l9110_apply(uint32_t ia_pulse, uint32_t ib_pulse)
{
    __HAL_TIM_SET_COMPARE(MOTOR_L9110_TIMER, MOTOR_L9110_CHANNEL_IA, ia_pulse);
    __HAL_TIM_SET_COMPARE(MOTOR_L9110_TIMER, MOTOR_L9110_CHANNEL_IB, ib_pulse);
}

/* 初始化 PWM 输出并默认刹车，防止上电误转 */
void motor_l9110_init(void)
{
    HAL_TIM_PWM_Start(MOTOR_L9110_TIMER, MOTOR_L9110_CHANNEL_IA);
    HAL_TIM_PWM_Start(MOTOR_L9110_TIMER, MOTOR_L9110_CHANNEL_IB);
    motor_l9110_brake();
}

/* 方向 + 占空比控制（0-100%），低电平阶段为刹车
 * @param direction 正转/反转/刹车
 * @param duty_percent 占空比，数值越大转速越高
 */
void motor_l9110_drive(motor_l9110_direction_t direction, uint8_t duty_percent)
{
    if (duty_percent > 100U)
    {
        duty_percent = 100U;
    }

    uint32_t pulse = motor_l9110_calc_pulse(duty_percent);

    switch (direction)
    {
    case MOTOR_L9110_DIR_FORWARD:
        /* INA = PWM, INB = 0 */
        motor_l9110_apply(pulse, 0U);
        break;
    case MOTOR_L9110_DIR_REVERSE:
        /* INA = 0, INB = PWM */
        motor_l9110_apply(0U, pulse);
        break;
    case MOTOR_L9110_DIR_BRAKE:
    default:
        motor_l9110_apply(0U, 0U);
        break;
    }
}

/* 有符号占空比控制：正为正转，负为反转
 * @param duty_percent -100~100，占空比正负代表方向
 */
void motor_l9110_drive_signed(int8_t duty_percent)
{
    int16_t duty = duty_percent;

    if (duty > 100)
    {
        duty = 100;
    }
    else if (duty < -100)
    {
        duty = -100;
    }

    if (duty >= 0)
    {
        motor_l9110_drive(MOTOR_L9110_DIR_FORWARD, (uint8_t)duty);
    }
    else
    {
        motor_l9110_drive(MOTOR_L9110_DIR_REVERSE, (uint8_t)(-duty));
    }
}

/* 双低刹车，快速停转 */
void motor_l9110_brake(void)
{
    motor_l9110_apply(0U, 0U);
}
