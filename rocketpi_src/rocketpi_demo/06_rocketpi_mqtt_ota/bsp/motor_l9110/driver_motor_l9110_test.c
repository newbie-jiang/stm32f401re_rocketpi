/**
 * @file driver_motor_l9110_test.c
 * @brief Simple demo sequence that exercises forward, brake, and reverse.
 * 演示正转->加速->刹车->反转->加速->刹车的循环。
 */

#include "driver_motor_l9110_test.h"
#include "driver_motor_l9110.h"
#include "main.h"

typedef enum
{
    MOTOR_L9110_TEST_FORWARD_SOFT = 0,  /* 正转慢速 */
    MOTOR_L9110_TEST_FORWARD_FAST,      /* 正转加速 */
    MOTOR_L9110_TEST_BRAKE_FROM_FWD,    /* 正转后刹车 */
    MOTOR_L9110_TEST_REVERSE_SOFT,      /* 反转慢速 */
    MOTOR_L9110_TEST_REVERSE_FAST,      /* 反转加速 */
    MOTOR_L9110_TEST_BRAKE_FROM_REV     /* 反转后刹车 */
} motor_l9110_test_state_t;

static motor_l9110_test_state_t s_state = MOTOR_L9110_TEST_FORWARD_SOFT;
static uint32_t s_state_tick = 0;
static uint32_t s_state_hold_ms = 0;

/* 切换到新状态
 * @param next_state 目标状态
 * @param hold_ms    该状态保持的时间（毫秒）
 */
static void motor_l9110_test_enter(motor_l9110_test_state_t next_state, uint32_t hold_ms)
{
    /* 进入新状态并记录起始时间 */
    s_state = next_state;
    s_state_hold_ms = hold_ms;
    s_state_tick = HAL_GetTick();

    switch (next_state)
    {
    case MOTOR_L9110_TEST_FORWARD_SOFT:
        motor_l9110_drive(MOTOR_L9110_DIR_FORWARD, 50);
        break;
    case MOTOR_L9110_TEST_FORWARD_FAST:
        motor_l9110_drive(MOTOR_L9110_DIR_FORWARD, 100);
        break;
    case MOTOR_L9110_TEST_BRAKE_FROM_FWD:
        motor_l9110_brake();
        break;
    case MOTOR_L9110_TEST_REVERSE_SOFT:
        motor_l9110_drive(MOTOR_L9110_DIR_REVERSE, 50);
        break;
    case MOTOR_L9110_TEST_REVERSE_FAST:
        motor_l9110_drive(MOTOR_L9110_DIR_REVERSE, 100);
        break;
    case MOTOR_L9110_TEST_BRAKE_FROM_REV:
    default:
        motor_l9110_brake();
        break;
    }
}

void motor_l9110_test_init(void)
{
    /* 初始化驱动后进入初始状态 */
    motor_l9110_init();
    motor_l9110_test_enter(MOTOR_L9110_TEST_FORWARD_SOFT, 2000);
}

void motor_l9110_test_task(void)
{
    uint32_t now = HAL_GetTick();

    /* 未到驻留时间则直接返回（非阻塞） */
    if ((now - s_state_tick) < s_state_hold_ms)
    {
        return;
    }

    switch (s_state)
    {
    case MOTOR_L9110_TEST_FORWARD_SOFT:
        motor_l9110_test_enter(MOTOR_L9110_TEST_FORWARD_FAST, 2000);
        break;
    case MOTOR_L9110_TEST_FORWARD_FAST:
        motor_l9110_test_enter(MOTOR_L9110_TEST_BRAKE_FROM_FWD, 1000);
        break;
    case MOTOR_L9110_TEST_BRAKE_FROM_FWD:
        motor_l9110_test_enter(MOTOR_L9110_TEST_REVERSE_SOFT, 2000);
        break;
    case MOTOR_L9110_TEST_REVERSE_SOFT:
        motor_l9110_test_enter(MOTOR_L9110_TEST_REVERSE_FAST, 2000);
        break;
    case MOTOR_L9110_TEST_REVERSE_FAST:
        motor_l9110_test_enter(MOTOR_L9110_TEST_BRAKE_FROM_REV, 1000);
        break;
    case MOTOR_L9110_TEST_BRAKE_FROM_REV:
    default:
        motor_l9110_test_enter(MOTOR_L9110_TEST_FORWARD_SOFT, 2000);
        break;
    }
}
