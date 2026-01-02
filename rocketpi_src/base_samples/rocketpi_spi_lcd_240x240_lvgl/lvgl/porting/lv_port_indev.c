#include "lv_port_indev.h"

#include "driver_adc_joystick_test.h"
#include "lvgl.h"

#include <stddef.h>

#define JOYSTICK_X_LEFT_PRESS        1600U
#define JOYSTICK_X_LEFT_RELEASE      1900U
#define JOYSTICK_X_RIGHT_PRESS       2900U
#define JOYSTICK_X_RIGHT_RELEASE     2600U
#define JOYSTICK_Y_UP_PRESS          1600U
#define JOYSTICK_Y_UP_RELEASE        1900U
#define JOYSTICK_Y_DOWN_PRESS        2900U
#define JOYSTICK_Y_DOWN_RELEASE      2600U

static lv_indev_t * s_keypad_indev = NULL;
static uint32_t s_active_key = 0U;
static uint32_t s_last_key = 0U;

static void joystick_keypad_init(void);
static void joystick_keypad_read(lv_indev_t * indev_drv, lv_indev_data_t * data);
static uint32_t joystick_keypad_get_key(const adc_joystick_sample_t *sample);

void lv_port_indev_init(void)
{
    joystick_keypad_init();

    s_keypad_indev = lv_indev_create();
    lv_indev_set_type(s_keypad_indev, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(s_keypad_indev, joystick_keypad_read);
}

lv_indev_t * lv_port_indev_get_keypad(void)
{
    return s_keypad_indev;
}

static void joystick_keypad_init(void)
{
    s_active_key = 0U;
    s_last_key = 0U;
}

static void joystick_keypad_read(lv_indev_t * indev_drv, lv_indev_data_t * data)
{
    LV_UNUSED(indev_drv);

    adc_joystick_sample_t sample;
    if (adc_joystick_test_sample(&sample) != 0U)
    {
        data->state = LV_INDEV_STATE_RELEASED;
        data->key = s_last_key;
        s_active_key = 0U;

        return;
    }

    uint32_t key = joystick_keypad_get_key(&sample);
    if (key != 0U)
    {
        s_active_key = key;
        s_last_key = key;
        data->state = LV_INDEV_STATE_PRESSED;
    }
    else
    {
        s_active_key = 0U;
        data->state = LV_INDEV_STATE_RELEASED;
    }

    data->key = s_last_key;
}

static uint32_t joystick_keypad_get_key(const adc_joystick_sample_t *sample)
{
    if (sample == NULL)
    {
        return 0U;
    }

    if (sample->key_pressed != 0U)
    {
        return LV_KEY_ENTER;
    }

    switch (s_active_key)
    {
        case LV_KEY_LEFT:
            if (sample->x.raw <= JOYSTICK_X_LEFT_RELEASE)
            {
                return LV_KEY_LEFT;
            }
            break;
        case LV_KEY_RIGHT:
            if (sample->x.raw >= JOYSTICK_X_RIGHT_RELEASE)
            {
                return LV_KEY_RIGHT;
            }
            break;
        case LV_KEY_UP:
            if (sample->y.raw <= JOYSTICK_Y_UP_RELEASE)
            {
                return LV_KEY_UP;
            }
            break;
        case LV_KEY_DOWN:
            if (sample->y.raw >= JOYSTICK_Y_DOWN_RELEASE)
            {
                return LV_KEY_DOWN;
            }
            break;
        case LV_KEY_ENTER:
        default:
            break;
    }

//    if (sample->x.raw <= JOYSTICK_X_LEFT_PRESS)
//    {
//        return LV_KEY_LEFT;
//    }
//    if (sample->x.raw >= JOYSTICK_X_RIGHT_PRESS)
//    {
//        return LV_KEY_RIGHT;
//    }
//    if (sample->y.raw <= JOYSTICK_Y_UP_PRESS)
//    {
//        return LV_KEY_UP;
//    }
//    if (sample->y.raw >= JOYSTICK_Y_DOWN_PRESS)
//    {
//        return LV_KEY_DOWN;
//    }
		
		    if (sample->x.raw <= JOYSTICK_X_LEFT_PRESS)
    {
        return LV_KEY_RIGHT;
    }
    if (sample->x.raw >= JOYSTICK_X_RIGHT_PRESS)
    {
        return LV_KEY_LEFT;
    }
    if (sample->y.raw <= JOYSTICK_Y_UP_PRESS)
    {
        return LV_KEY_DOWN;
    }
    if (sample->y.raw >= JOYSTICK_Y_DOWN_PRESS)
    {
        return LV_KEY_UP;
    }

    return 0U;
}
