#include "unit_test.h"
#include "driver_ir_remote_interface.h"

#define IRDA_QUEUE_LENGTH    8U
#define IRDA_FEEDBACK_FREQUENCY_HZ      2800U
#define IRDA_FEEDBACK_DUTY_PERCENT      40U
#define IRDA_FEEDBACK_DURATION_MS       120U
#define IRDA_FEEDBACK_MIN_INTERVAL_MS   200U

static ir_remote_handle_t s_ir_handle;
static osMessageQueueId_t s_ir_queue;
static volatile uint8_t s_ir_ready = 0U;
static volatile uint8_t s_ir_last_command = 0U;
static volatile uint8_t s_ir_command_pending = 0U;
static uint32_t s_ir_last_feedback_tick = 0U;

static uint32_t s_ir_last_ok_tick = 0U;

static bool irda_should_accept_ok_frame(void)
{
    uint32_t now = HAL_GetTick();
    uint32_t elapsed = now - s_ir_last_ok_tick;
    if (elapsed < IRDA_FEEDBACK_MIN_INTERVAL_MS)
    {
        return false;
    }

    s_ir_last_ok_tick = now;
    return true;
}

static void irda_request_feedback_beep(void)
{
    uint32_t now = HAL_GetTick();
    uint32_t elapsed = now - s_ir_last_feedback_tick;
    if (elapsed < IRDA_FEEDBACK_MIN_INTERVAL_MS)
    {
        return;
    }

    s_ir_last_feedback_tick = now;
    (void)unit_test_buzzer_request_beep(IRDA_FEEDBACK_FREQUENCY_HZ,
                                        IRDA_FEEDBACK_DUTY_PERCENT,
                                        IRDA_FEEDBACK_DURATION_MS);
}

static void irda_publish_command(uint8_t command)
{
    taskENTER_CRITICAL();
    s_ir_last_command = command;
    s_ir_command_pending = 1U;
    taskEXIT_CRITICAL();
}

static void irda_log_event(const ir_remote_t *event)
{
    if (event == NULL)
    {
        return;
    }

    switch (event->status)
    {
        case IR_REMOTE_STATUS_OK:
        {
            if (!irda_should_accept_ok_frame())
            {
                break;
            }
            irda_publish_command(event->command);
            irda_request_feedback_beep();
            break;
        }
        case IR_REMOTE_STATUS_REPEAT:
        {
            irda_publish_command(event->command);
            irda_request_feedback_beep();
            break;
        }
        case IR_REMOTE_STATUS_ADDR_ERR:
        {
            elog_w("irda", "ir_remote: address error");
            break;
        }
        case IR_REMOTE_STATUS_CMD_ERR:
        {
            elog_w("irda", "ir_remote: command error");
            break;
        }
        default:
        {
            elog_w("irda", "ir_remote: frame invalid");
            break;
        }
    }
}

static void irda_receive_callback(ir_remote_t *data)
{
    if ((data == NULL) || (s_ir_queue == NULL))
    {
        return;
    }

    if (data->status == IR_REMOTE_STATUS_REPEAT)
    {
        return;
    }

    (void)osMessageQueuePut(s_ir_queue, data, 0U, 0U);
}

static void irda_init_handle(void)
{
    DRIVER_IR_REMOTE_LINK_INIT(&s_ir_handle, ir_remote_handle_t);
    DRIVER_IR_REMOTE_LINK_TIMESTAMP_READ(&s_ir_handle, ir_remote_interface_timestamp_read);
    DRIVER_IR_REMOTE_LINK_DELAY_MS(&s_ir_handle, ir_remote_interface_delay_ms);
    DRIVER_IR_REMOTE_LINK_DEBUG_PRINT(&s_ir_handle, ir_remote_interface_debug_print);
    DRIVER_IR_REMOTE_LINK_RECEIVE_CALLBACK(&s_ir_handle, irda_receive_callback);
}

static void irda_print_chip_info(void)
{
    ir_remote_info_t info;

    if (ir_remote_info(&info) == 0U)
    {
        elog_i("irda", "ir_remote: chip %s, driver %d.%d",
               info.chip_name,
               info.driver_version / 1000,
               (info.driver_version % 1000) / 100);
    }
}

void irda_Task(void *argument)
{
    (void)argument;

    const osMessageQueueAttr_t queue_attr = {
        .name = "irdaQueue"
    };

    s_ir_queue = osMessageQueueNew(IRDA_QUEUE_LENGTH, sizeof(ir_remote_t), &queue_attr);
    if (s_ir_queue == NULL)
    {
        elog_e("irda", "ir_remote: queue create failed");
        Error_Handler();
    }

    irda_init_handle();
    irda_print_chip_info();

    if (ir_remote_interface_timer_init() != 0U)
    {
        elog_e("irda", "ir_remote: timer init failed");
        Error_Handler();
    }
    if (ir_remote_init(&s_ir_handle) != 0U)
    {
        elog_e("irda", "ir_remote: init failed");
        Error_Handler();
    }

    s_ir_ready = 1U;

    for (;;)
    {
        ir_remote_t event;
        uint8_t processed = 0U;

        while ((processed < 2U) && (osMessageQueueGet(s_ir_queue, &event, NULL, 0U) == osOK))
        {
            irda_log_event(&event);
            processed++;
        }

        osDelay(5U);
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if ((GPIO_Pin == IRDA_IO_Pin) && (s_ir_ready != 0U))
    {
        if (ir_remote_irq_handler(&s_ir_handle) != 0U)
        {
            elog_e("irda", "ir_remote: irq handler failed");
        }
    }
}

bool unit_test_irda_take_command(uint8_t *command)
{
    if (command == NULL)
    {
        return false;
    }

    bool has_new = false;
    taskENTER_CRITICAL();
    if (s_ir_command_pending != 0U)
    {
        *command = s_ir_last_command;
        s_ir_command_pending = 0U;
        has_new = true;
    }
    taskEXIT_CRITICAL();

    return has_new;
}
