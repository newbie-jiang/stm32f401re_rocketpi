#include "unit_test.h"
#include "driver_buzzer_test.h"

#define BUZZER_QUEUE_LENGTH     8U
#define BUZZER_STARTUP_FREQUENCY_HZ     2700U
#define BUZZER_STARTUP_DUTY_PERCENT     50U
#define BUZZER_STARTUP_ON_MS            100U
#define BUZZER_STARTUP_GAP_MS           150U

typedef struct
{
    uint32_t frequency_hz;
    uint8_t duty_percent;
    uint32_t duration_ms;
} unit_test_buzzer_cmd_t;

static osMessageQueueId_t s_buzzer_queue = NULL;

static void buzzer_play_startup_chime(void)
{
    for (uint8_t i = 0; i < 2U; i++)
    {
        (void)buzzer_test_start(BUZZER_STARTUP_FREQUENCY_HZ, BUZZER_STARTUP_DUTY_PERCENT);
        osDelay(BUZZER_STARTUP_ON_MS);
        (void)buzzer_test_stop();
        osDelay(BUZZER_STARTUP_GAP_MS);
    }
}

bool unit_test_buzzer_request_beep(uint32_t frequency_hz, uint8_t duty_percent, uint32_t duration_ms)
{
    unit_test_buzzer_cmd_t cmd = {
        .frequency_hz = frequency_hz,
        .duty_percent = duty_percent,
        .duration_ms = duration_ms};

    if (s_buzzer_queue == NULL)
    {
        return false;
    }

    return osMessageQueuePut(s_buzzer_queue, &cmd, 0U, 0U) == osOK;
}

void buzzer_Task(void *argument)
{
    (void)argument;	


    const osMessageQueueAttr_t queue_attr = {
        .name = "buzzerQueue"
    };

    s_buzzer_queue = osMessageQueueNew(BUZZER_QUEUE_LENGTH, sizeof(unit_test_buzzer_cmd_t), &queue_attr);
    if (s_buzzer_queue == NULL)
    {
        Error_Handler();
    }

    buzzer_play_startup_chime();

    for (;;)
    {
        unit_test_buzzer_cmd_t cmd;
        if (osMessageQueueGet(s_buzzer_queue, &cmd, NULL, osWaitForever) == osOK)
        {
            uint32_t freq = (cmd.frequency_hz == 0U) ? BUZZER_TEST_DEFAULT_FREQUENCY_HZ : cmd.frequency_hz;
            uint8_t duty = (cmd.duty_percent == 0U) ? BUZZER_TEST_DEFAULT_DUTY_PERCENT : cmd.duty_percent;
            uint32_t duration = (cmd.duration_ms == 0U) ? BUZZER_TEST_DEFAULT_DURATION_MS : cmd.duration_ms;
            if (duration == 0U)
            {
                duration = 1U;
            }

            if (buzzer_test_start(freq, duty) == 0U)
            {
                osDelay(duration);
                (void)buzzer_test_stop();
            }
            else
            {
                (void)buzzer_test_stop();
            }
        }
    }
}
