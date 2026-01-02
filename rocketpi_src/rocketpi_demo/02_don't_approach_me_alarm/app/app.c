#include "app.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "driver_buzzer_test.h"
#include "driver_ir_remote_basic.h"
#include "driver_ir_remote_interface.h"
#include "driver_mg58f18_radar.h"
#include "main.h"

/* 参数相关宏：
 * APP_DISTANCE_*   感应距离阈值范围/步进/默认值
 * APP_DELAY_*      OUT 保持/延迟时间的步进、上下限、默认值（毫秒）
 * APP_BEEP_*       蜂鸣器的频率、占空比、单次鸣响时长与间隔
 * APP_IR_KEY_*     红外遥控键值映射（HX1838/NEC）
 * APP_IR_QUEUE_SIZE 红外按键环形队列长度
 */
#define APP_DISTANCE_MIN            100U    /* 感应距离阈值下限（越小越远） */
#define APP_DISTANCE_MAX            20000U  /* 感应距离阈值上限 */
#define APP_DISTANCE_STEP           1000U   /* 红外按键调整步进 */
#define APP_DISTANCE_INITIAL        100U    /* 上电默认距离阈值 */

#define APP_DELAY_STEP_MS           1000U   /* OUT 延迟/保持时间步进（毫秒） */
#define APP_DELAY_DEFAULT_MS        1000U   /* 默认保持时间（毫秒） */
#define APP_DELAY_MIN_MS            1000U   /* 保持时间下限（毫秒） */
#define APP_DELAY_MAX_MS            (0xFFFFFFUL / 32UL) /* 协议可表示的最大保持时间 */

#define APP_BEEP_FREQ_HZ            BUZZER_TEST_DEFAULT_FREQUENCY_HZ /* 蜂鸣频率 */
#define APP_BEEP_DUTY_PERCENT       BUZZER_TEST_DEFAULT_DUTY_PERCENT /* 蜂鸣占空比 */
#define APP_BEEP_PULSE_MS           120U    /* 单次蜂鸣时长 */
#define APP_BEEP_GAP_MS             80U     /* 多声蜂鸣间隔 */

/* HX1838/NEC 常用键值，可按需要调整 */
#define APP_IR_KEY_DISTANCE_FARTHER 0x45U /* CH-，距离更远（阈值减小） */
#define APP_IR_KEY_DISTANCE_NEARER  0x47U /* CH+，距离更近（阈值增大） */
#define APP_IR_KEY_DELAY_LONGER     0x15U /* VOL+，保持时间+1s */
#define APP_IR_KEY_DELAY_SHORTER    0x07U /* VOL-，保持时间-1s */
#define APP_IR_KEY_SAVE             0x43U /* PLAY/PAUSE，保存到 Flash */

#define APP_IR_QUEUE_SIZE           8U     /* 红外按键环形队列长度 */

typedef enum
{
    APP_BUZZER_SEQ_IDLE = 0,
    APP_BUZZER_SEQ_SINGLE,
    APP_BUZZER_SEQ_TRIPLE
} app_buzzer_sequence_t;

typedef struct
{
    app_buzzer_sequence_t mode;
    uint8_t step;
    uint32_t next_deadline_ms;
} app_buzzer_state_t;

static volatile uint8_t s_ir_queue[APP_IR_QUEUE_SIZE];
static volatile uint8_t s_ir_head = 0U;
static volatile uint8_t s_ir_tail = 0U;
static volatile uint8_t s_ir_overflow = 0U;
static volatile uint8_t s_ir_beep_count = 0U;

static uint16_t s_distance_threshold = APP_DISTANCE_INITIAL;
static uint32_t s_delay_ms = APP_DELAY_DEFAULT_MS;
static bool s_radar_io_state = false;
static bool s_radar_io_valid = false;
static app_buzzer_state_t s_buzzer = {APP_BUZZER_SEQ_IDLE, 0U, 0U};

/* 将红外命令压入环形队列（IRQ 上下文调用） */
static void app_ir_push_command(uint8_t command);
/* 从队列取出一条红外命令（主循环调用） */
static bool app_ir_pop_command(uint8_t *command);
/* 红外接收回调：记录按键并触发一次蜂鸣 */
static void app_ir_receive_callback(ir_remote_t *data);
/* 根据按键码调整雷达参数或保存配置 */
static void app_handle_ir_command(uint8_t command);
/* 下发并更新距离阈值 */
static void app_apply_distance_threshold(uint16_t new_value);
/* 下发并更新输出保持时间 */
static void app_apply_delay_ms(uint32_t new_value);
/* 轮询雷达 OUT 管脚，打印跳变并在上升沿三声蜂鸣 */
static void app_check_radar_io(void);
/* 启动一次单声蜂鸣序列 */
static void app_request_single_beep(void);
/* 启动一次三声蜂鸣序列 */
static void app_request_triple_beep(void);
/* 推进非阻塞蜂鸣状态机 */
static void app_buzzer_advance(void);
/* 强制停止蜂鸣并清空状态机 */
static void app_buzzer_force_stop(void);

/* 将红外命令压入环形队列（满则丢弃并置溢出标志） */
static void app_ir_push_command(uint8_t command)
{
    /* Simple ring buffer for IR commands coming from IRQ. */
    const uint8_t next = (uint8_t)((s_ir_head + 1U) % APP_IR_QUEUE_SIZE);
    if (next == s_ir_tail)
    {
        s_ir_overflow = 1U;
        return;
    }

    s_ir_queue[s_ir_head] = command;
    s_ir_head = next;
}

/* 从环形队列读取一条命令，若为空返回 false */
static bool app_ir_pop_command(uint8_t *command)
{
    if (s_ir_tail == s_ir_head)
    {
        return false;
    }

    *command = s_ir_queue[s_ir_tail];
    s_ir_tail = (uint8_t)((s_ir_tail + 1U) % APP_IR_QUEUE_SIZE);
    return true;
}

/* 红外驱动的接收回调，记录按键并叠加提示音计数 */
static void app_ir_receive_callback(ir_remote_t *data)
{
    if (data == NULL)
    {
        return;
    }

    if ((data->status == IR_REMOTE_STATUS_OK) || (data->status == IR_REMOTE_STATUS_REPEAT))
    {
        if (s_ir_beep_count < 5U)
        {
            s_ir_beep_count++;
        }
        /* 不管按下什么键，都打印键值和状态便于调试 */
        printf("[APP][IR] key=0x%02X status=%s\r\n",
               data->command,
               (data->status == IR_REMOTE_STATUS_REPEAT) ? "REPEAT" : "OK");
        app_ir_push_command(data->command);
    }
}

/* 启动蜂鸣器输出一个音符 */
static void app_buzzer_start_tone(void)
{
    (void)buzzer_test_start(APP_BEEP_FREQ_HZ, APP_BEEP_DUTY_PERCENT);
}

/* 停止蜂鸣器输出 */
static void app_buzzer_stop_tone(void)
{
    (void)buzzer_test_stop();
}

/* 清空蜂鸣状态机并确保停止输出 */
static void app_buzzer_force_stop(void)
{
    app_buzzer_stop_tone();
    s_buzzer.mode = APP_BUZZER_SEQ_IDLE;
    s_buzzer.step = 0U;
    s_buzzer.next_deadline_ms = 0U;
}

static void app_request_single_beep(void)
{
    /* Non-blocking single beep. */
    app_buzzer_force_stop();
    s_buzzer.mode = APP_BUZZER_SEQ_SINGLE;
    s_buzzer.step = 0U;
    s_buzzer.next_deadline_ms = HAL_GetTick();
    app_buzzer_advance();
}

/* 请求三声短促蜂鸣（非阻塞） */
static void app_request_triple_beep(void)
{
    /* Non-blocking triple beep. */
    app_buzzer_force_stop();
    s_buzzer.mode = APP_BUZZER_SEQ_TRIPLE;
    s_buzzer.step = 0U;
    s_buzzer.next_deadline_ms = HAL_GetTick();
    app_buzzer_advance();
}

/* 根据当前状态推进蜂鸣序列，使用 HAL_GetTick 定时 */
static void app_buzzer_advance(void)
{
    const uint32_t now = HAL_GetTick();

    switch (s_buzzer.mode)
    {
        case APP_BUZZER_SEQ_SINGLE:
            if (s_buzzer.step == 0U)
            {
                app_buzzer_start_tone();
                s_buzzer.next_deadline_ms = now + APP_BEEP_PULSE_MS;
                s_buzzer.step = 1U;
            }
            else
            {
                app_buzzer_force_stop();
            }
            break;

        case APP_BUZZER_SEQ_TRIPLE:
            switch (s_buzzer.step)
            {
                case 0U:
                    app_buzzer_start_tone();
                    s_buzzer.next_deadline_ms = now + APP_BEEP_PULSE_MS;
                    s_buzzer.step = 1U;
                    break;
                case 1U:
                    app_buzzer_stop_tone();
                    s_buzzer.next_deadline_ms = now + APP_BEEP_GAP_MS;
                    s_buzzer.step = 2U;
                    break;
                case 2U:
                    app_buzzer_start_tone();
                    s_buzzer.next_deadline_ms = now + APP_BEEP_PULSE_MS;
                    s_buzzer.step = 3U;
                    break;
                case 3U:
                    app_buzzer_stop_tone();
                    s_buzzer.next_deadline_ms = now + APP_BEEP_GAP_MS;
                    s_buzzer.step = 4U;
                    break;
                case 4U:
                    app_buzzer_start_tone();
                    s_buzzer.next_deadline_ms = now + APP_BEEP_PULSE_MS;
                    s_buzzer.step = 5U;
                    break;
                default:
                    app_buzzer_force_stop();
                    break;
            }
            break;

        default:
            app_buzzer_force_stop();
            break;
    }
}

/* 处理一条红外按键，调整阈值/延迟或保存参数 */
static void app_handle_ir_command(uint8_t command)
{
    switch (command)
    {
        case APP_IR_KEY_DISTANCE_FARTHER:
        {
            uint32_t candidate = (s_distance_threshold > APP_DISTANCE_STEP)
                                     ? (uint32_t)s_distance_threshold - APP_DISTANCE_STEP
                                     : APP_DISTANCE_MIN;
            if (candidate < APP_DISTANCE_MIN)
            {
                candidate = APP_DISTANCE_MIN;
            }
            app_apply_distance_threshold((uint16_t)candidate);
            break;
        }
        case APP_IR_KEY_DISTANCE_NEARER:
        {
            uint32_t candidate = (uint32_t)s_distance_threshold + APP_DISTANCE_STEP;
            if (candidate > APP_DISTANCE_MAX)
            {
                candidate = APP_DISTANCE_MAX;
            }
            app_apply_distance_threshold((uint16_t)candidate);
            break;
        }
        case APP_IR_KEY_DELAY_LONGER:
        {
            uint32_t candidate = s_delay_ms + APP_DELAY_STEP_MS;
            if (candidate > APP_DELAY_MAX_MS)
            {
                candidate = APP_DELAY_MAX_MS;
            }
            app_apply_delay_ms(candidate);
            break;
        }
        case APP_IR_KEY_DELAY_SHORTER:
        {
            uint32_t candidate = (s_delay_ms > (APP_DELAY_MIN_MS + APP_DELAY_STEP_MS))
                                     ? (s_delay_ms - APP_DELAY_STEP_MS)
                                     : APP_DELAY_MIN_MS;
            app_apply_delay_ms(candidate);
            break;
        }
        case APP_IR_KEY_SAVE:
        {
            const mg58f18_radar_status_t status = mg58f18_radar_save_settings();
            printf("[APP][RADAR] save settings: %s\r\n", mg58f18_radar_status_string(status));
            HAL_Delay(150U);

            uint16_t dist = 0U;
            uint32_t hold_ms = 0U;
            const mg58f18_radar_status_t dist_status = mg58f18_radar_get_distance_threshold(&dist);
            const mg58f18_radar_status_t hold_status = mg58f18_radar_get_delay_ms(&hold_ms);
            if (dist_status == MG58F18_RADAR_STATUS_OK)
            {
                s_distance_threshold = dist;
            }
            if (hold_status == MG58F18_RADAR_STATUS_OK)
            {
                s_delay_ms = hold_ms;
            }
            printf("[APP][RADAR] after save: distance=%u hold=%lu ms (dist:%s hold:%s)\r\n",
                   (unsigned int)s_distance_threshold,
                   (unsigned long)s_delay_ms,
                   mg58f18_radar_status_string(dist_status),
                   mg58f18_radar_status_string(hold_status));
            break;
        }
        default:
            break;
    }
}

/* 设置并缓存距离阈值（包含合法化） */
static void app_apply_distance_threshold(uint16_t new_value)
{
    if (new_value < APP_DISTANCE_MIN)
    {
        new_value = APP_DISTANCE_MIN;
    }
    if (new_value > APP_DISTANCE_MAX)
    {
        new_value = APP_DISTANCE_MAX;
    }

    const mg58f18_radar_status_t status = mg58f18_radar_set_distance_threshold(new_value);
    printf("[APP][RADAR] set distance %u -> %s\r\n", (unsigned int)new_value, mg58f18_radar_status_string(status));
    if (status == MG58F18_RADAR_STATUS_OK)
    {
        s_distance_threshold = new_value;
    }
}

/* 设置并缓存输出保持时间（包含合法化） */
static void app_apply_delay_ms(uint32_t new_value)
{
    if (new_value > APP_DELAY_MAX_MS)
    {
        new_value = APP_DELAY_MAX_MS;
    }
    if (new_value < APP_DELAY_MIN_MS)
    {
        new_value = APP_DELAY_MIN_MS;
    }

    const mg58f18_radar_status_t status = mg58f18_radar_set_delay_ms(new_value);
    printf("[APP][RADAR] set hold %lu ms -> %s\r\n", (unsigned long)new_value, mg58f18_radar_status_string(status));
    if (status == MG58F18_RADAR_STATUS_OK)
    {
        s_delay_ms = new_value;
    }
}

/* 轮询雷达 OUT 引脚，打印跳变并在上升沿请求三声蜂鸣 */
static void app_check_radar_io(void)
{
    bool current_state = false;
    const mg58f18_radar_status_t status = mg58f18_radar_read_io(&current_state);
    if (status != MG58F18_RADAR_STATUS_OK)
    {
        return;
    }

    if (!s_radar_io_valid)
    {
        s_radar_io_state = current_state;
        s_radar_io_valid = true;
        printf("[APP][RADAR] initial OUT=%s\r\n", current_state ? "HIGH" : "LOW");
        return;
    }

    if (current_state != s_radar_io_state)
    {
        printf("[APP][RADAR] OUT %s -> %s\r\n",
               s_radar_io_state ? "HIGH" : "LOW",
               current_state ? "HIGH" : "LOW");
        if ((!s_radar_io_state) && current_state)
        {
            app_request_triple_beep();
        }
        s_radar_io_state = current_state;
    }
}

/* 初始化 IR、雷达和蜂鸣器状态，写入初始阈值 */
void app_init(void)
{
    app_buzzer_force_stop();
    printf("\r\n[APP] init\r\n");

    if (ir_remote_interface_timer_init() != 0U)
    {
        printf("[APP][IR] timer init failed\r\n");
    }
    else if (ir_remote_basic_init(app_ir_receive_callback) != 0U)
    {
        printf("[APP][IR] init failed\r\n");
    }
    else
    {
        printf("[APP][IR] ready\r\n");
    }

    mg58f18_radar_status_t radar_status = mg58f18_radar_init();
  if (radar_status != MG58F18_RADAR_STATUS_OK)
  {
    printf("[APP][RADAR] init failed: %s\r\n", mg58f18_radar_status_string(radar_status));
    return;
  }

  HAL_Delay(200U);
  app_apply_distance_threshold(APP_DISTANCE_INITIAL);

    uint32_t delay_ms = 0U;
    radar_status = mg58f18_radar_get_delay_ms(&delay_ms);
    if (radar_status == MG58F18_RADAR_STATUS_OK)
    {
        s_delay_ms = (delay_ms < APP_DELAY_MIN_MS) ? APP_DELAY_MIN_MS : delay_ms;
        if (delay_ms < APP_DELAY_MIN_MS)
        {
            app_apply_delay_ms(APP_DELAY_MIN_MS);
        }
        printf("[APP][RADAR] current hold %lu ms\r\n", (unsigned long)s_delay_ms);
    }
    else
    {
        s_delay_ms = APP_DELAY_DEFAULT_MS;
        app_apply_delay_ms(s_delay_ms);
    }

    const mg58f18_radar_status_t io_status = mg58f18_radar_read_io(&s_radar_io_state);
    s_radar_io_valid = (io_status == MG58F18_RADAR_STATUS_OK);
}

/* 主循环周期调用：驱动蜂鸣状态机、处理红外按键、轮询雷达 OUT */
void app_poll(void)
{
    /* Run lightweight tasks often from main loop. */
    if ((s_ir_beep_count > 0U) && (s_buzzer.mode == APP_BUZZER_SEQ_IDLE))
    {
        s_ir_beep_count--;
        app_request_single_beep();
    }

    if ((s_buzzer.mode != APP_BUZZER_SEQ_IDLE) &&
        ((int32_t)(HAL_GetTick() - s_buzzer.next_deadline_ms) >= 0))
    {
        app_buzzer_advance();
    }

    uint8_t command = 0U;
    while (app_ir_pop_command(&command))
    {
        app_handle_ir_command(command);
    }
    if (s_ir_overflow != 0U)
    {
        printf("[APP][IR] queue overflow, dropping keys\r\n");
        s_ir_overflow = 0U;
    }

    app_check_radar_io();
}

/* EXTI 回调转发：IR 引脚产生中断时交给驱动处理 */
void app_on_exti(uint16_t gpio_pin)
{
    if (gpio_pin == IRDA_IO_Pin)
    {
        (void)ir_remote_basic_irq_handler();
    }
}
