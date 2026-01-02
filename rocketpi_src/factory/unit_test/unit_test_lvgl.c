#include "unit_test.h"

#include "st7789.h"
#include "lv_conf.h"
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"

LV_IMG_DECLARE(rocket_cloud_80x80_01);
LV_IMG_DECLARE(rocket_cloud_80x80_02);
LV_IMG_DECLARE(rocket_cloud_80x80_03);
LV_IMG_DECLARE(rocket_cloud_80x80_04);
LV_IMG_DECLARE(rocket_cloud_80x80_05);
LV_IMG_DECLARE(rocket_cloud_80x80_06);
LV_IMG_DECLARE(rocket_cloud_80x80_07);
LV_IMG_DECLARE(rocket_cloud_80x80_08);
LV_IMG_DECLARE(rocket_cloud_80x80_09);
LV_IMG_DECLARE(rocket_cloud_80x80_10);
LV_IMG_DECLARE(temp_30x30);
LV_IMG_DECLARE(humidity_30x30);

#define FACTORY_UI_MAX_FOCUS_ITEMS 5U

typedef enum
{
    FACTORY_FOCUS_ROLE_INFO_CARD = 0,
    FACTORY_FOCUS_ROLE_CARD_PANEL
} factory_focus_role_t;

typedef struct
{
    lv_obj_t *obj;
    factory_focus_role_t role;
    bool toggled;
    bool focused;
} factory_focus_item_t;



static void factory_ui_create(void);
static void factory_ui_update_sensor_labels(const unit_test_aht30_data_t *data);
static void factory_ui_refresh_sensor_labels(void);
static void factory_ui_sensor_timer_cb(lv_timer_t *timer);
static void factory_ui_seed_placeholder_data(void);
static void factory_ui_update_button_label(unit_test_button_state_t state);
static void factory_ui_update_card_labels(const unit_test_card_info_t *info);
static void factory_ui_update_eeprom_label(unit_test_eeprom_status_t status);
static void factory_ui_update_usb_label(bool connected);
static void factory_ui_update_irda_label(uint8_t code);
static void factory_ui_update_theme_for_button_state(unit_test_button_state_t state);
static void factory_ui_apply_theme(uint8_t theme_index);
static bool factory_ui_card_info_equal(const unit_test_card_info_t *a, const unit_test_card_info_t *b);
static lv_obj_t *factory_ui_create_info_card(lv_obj_t *parent, const char *caption, const char *placeholder);
static void factory_ui_register_focusable(lv_obj_t *obj, factory_focus_role_t role);
static void factory_ui_focusable_event_cb(lv_event_t *event);
static lv_color_t factory_ui_focusable_base_color(factory_focus_role_t role);
static void factory_ui_apply_focusable_style(factory_focus_item_t *item);
static void factory_ui_refresh_focusable_styles(void);
static void factory_ui_reset_focusables(void);
static void factory_ui_attach_focusables_to_group(lv_group_t *group);
static void factory_ui_create_card_panel(lv_obj_t *parent);
void lvgl_debug_grid_screen(void);

static lv_obj_t *label_temp_value = NULL;
static lv_obj_t *label_hum_value = NULL;
static lv_obj_t *label_eeprom_value = NULL;
static lv_obj_t *label_key_value = NULL;
static lv_obj_t *label_card_total_value = NULL;
static lv_obj_t *label_card_free_value = NULL;
static lv_obj_t *label_usb_status = NULL;
static lv_obj_t *label_header = NULL;
static lv_obj_t *label_irda_value = NULL;
static lv_obj_t *screen_root = NULL;
static lv_obj_t *card_panel_obj = NULL;
static lv_obj_t *logo_image = NULL;
static lv_obj_t *icon_temp_image = NULL;
static lv_obj_t *icon_hum_image = NULL;
static lv_obj_t *info_card_objs[4];
static uint8_t info_card_obj_count = 0;
static unit_test_aht30_data_t latest_sensor_data;
static bool sensor_data_valid = false;
static unit_test_button_state_t displayed_button_state = UNIT_TEST_BUTTON_STATE_IDLE;
static unit_test_card_info_t displayed_card_info = {0};
static unit_test_eeprom_status_t displayed_eeprom_status = UNIT_TEST_EEPROM_STATUS_NOT_TESTED;
static bool displayed_usb_connected = false;
static uint8_t active_theme_index = 0;
static factory_focus_item_t factory_focus_items[FACTORY_UI_MAX_FOCUS_ITEMS];
static uint8_t factory_focus_item_count = 0U;


static const lv_image_dsc_t *logo_anim_frames[] = {
    &rocket_cloud_80x80_01,
    &rocket_cloud_80x80_02,
    &rocket_cloud_80x80_03,
    &rocket_cloud_80x80_04,
    &rocket_cloud_80x80_05,
    &rocket_cloud_80x80_06,
    &rocket_cloud_80x80_07,
    &rocket_cloud_80x80_08,
    &rocket_cloud_80x80_09,
    &rocket_cloud_80x80_10};
#define LOGO_ANIM_FRAME_COUNT (sizeof(logo_anim_frames) / sizeof(logo_anim_frames[0]))

typedef struct
{
    uint32_t screen_color;
    uint32_t panel_bg_color;
    uint32_t accent_color;
    uint32_t card_color;
} factory_theme_t;

static const factory_theme_t factory_themes[] = {
    {0x0b1118, 0x121b29, 0x58f39f, 0x172131},                          /* default */
    {0x031b33, 0x0a1a31, 0x12b4ff, 0x1183ff},                          /* short - vivid blue */
    {0x2a052f, 0x1f0b2d, 0xff6bff, 0xe047ff},                          /* double - vivid magenta */
    {0x300b02, 0x240c05, 0xffb347, 0xff8a0a}                           /* long - vivid orange */
};

#define FACTORY_THEME_COUNT (sizeof(factory_themes) / sizeof(factory_themes[0]))

static void factory_ui_create(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_remove_style_all(scr);
    lv_obj_set_size(scr, 240, 240);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x0c111a), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_add_flag(scr, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    screen_root = scr;
    info_card_obj_count = 0;
    factory_ui_reset_focusables();

    label_usb_status = lv_label_create(scr);
    lv_label_set_text(label_usb_status, "");
    lv_obj_set_style_text_color(label_usb_status, lv_color_hex(0x6c768c), 0);
    lv_obj_set_style_text_font(label_usb_status, lv_theme_get_font_small(label_usb_status), 0);
    lv_obj_align(label_usb_status, LV_ALIGN_TOP_LEFT, 8, 2);
    lv_obj_add_flag(label_usb_status, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *header = lv_label_create(scr);
    lv_label_set_text(header, "Rocket-Pi");
    lv_obj_set_style_text_color(header, lv_color_hex(0x58f39f), 0);
    lv_obj_set_style_text_font(header, lv_theme_get_font_large(header), 0);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 2);
    label_header = header;

    label_irda_value = lv_label_create(scr);
    lv_label_set_text(label_irda_value, "0x--");
    lv_obj_set_style_text_color(label_irda_value, lv_color_hex(0x58f39f), 0);
    lv_obj_set_style_text_font(label_irda_value, lv_theme_get_font_small(label_irda_value), 0);
    lv_obj_align(label_irda_value, LV_ALIGN_TOP_RIGHT, -8, 2);

    logo_image = lv_animimg_create(scr);
    lv_animimg_set_src(logo_image, (const void **)logo_anim_frames, LOGO_ANIM_FRAME_COUNT);
    lv_animimg_set_duration(logo_image, 1000);
    lv_animimg_set_repeat_count(logo_image, LV_ANIM_REPEAT_INFINITE);
    lv_obj_set_size(logo_image, 80, 80);
    lv_obj_align(logo_image, LV_ALIGN_TOP_RIGHT, -12, 150);
    lv_animimg_start(logo_image);

    lv_obj_t *top_section = lv_obj_create(scr);
    lv_obj_remove_style_all(top_section);
    lv_obj_set_width(top_section, 216);
    lv_obj_set_height(top_section, LV_SIZE_CONTENT);

    lv_obj_align(top_section, LV_ALIGN_TOP_MID, 0, 23);
    lv_obj_set_style_pad_all(top_section, 6, 0);
    lv_obj_set_style_pad_gap(top_section, 10, 0);
    lv_obj_set_flex_flow(top_section, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(top_section, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_add_flag(top_section, LV_OBJ_FLAG_OVERFLOW_VISIBLE);

    label_temp_value = factory_ui_create_info_card(top_section, "TEMP", "--.- C");
    label_hum_value = factory_ui_create_info_card(top_section, "HUMI", "--.- %");
    label_eeprom_value = factory_ui_create_info_card(top_section, "EEPROM", "----");
    label_key_value = factory_ui_create_info_card(top_section, "KEY", "----");

    lv_obj_t *temp_card = lv_obj_get_parent(label_temp_value);
    factory_ui_register_focusable(temp_card, FACTORY_FOCUS_ROLE_INFO_CARD);
    if (temp_card != NULL)
    {
        icon_temp_image = lv_image_create(temp_card);
        lv_image_set_src(icon_temp_image, &temp_30x30);
        lv_obj_add_flag(icon_temp_image, LV_OBJ_FLAG_IGNORE_LAYOUT);
        lv_obj_align(icon_temp_image, LV_ALIGN_TOP_RIGHT, -1, 6);
        lv_obj_move_foreground(icon_temp_image);
    }

    lv_obj_t *hum_card = lv_obj_get_parent(label_hum_value);
    factory_ui_register_focusable(hum_card, FACTORY_FOCUS_ROLE_INFO_CARD);
    if (hum_card != NULL)
    {
        icon_hum_image = lv_image_create(hum_card);
        lv_image_set_src(icon_hum_image, &humidity_30x30);
        lv_obj_add_flag(icon_hum_image, LV_OBJ_FLAG_IGNORE_LAYOUT);
        lv_obj_align(icon_hum_image, LV_ALIGN_TOP_RIGHT, -1, 6);
        lv_obj_move_foreground(icon_hum_image);
    }

    factory_ui_register_focusable(lv_obj_get_parent(label_eeprom_value), FACTORY_FOCUS_ROLE_INFO_CARD);
    factory_ui_register_focusable(lv_obj_get_parent(label_key_value), FACTORY_FOCUS_ROLE_INFO_CARD);

    factory_ui_create_card_panel(scr);
    factory_ui_register_focusable(card_panel_obj, FACTORY_FOCUS_ROLE_CARD_PANEL);

    factory_ui_seed_placeholder_data();
    factory_ui_apply_theme(0);
    if (logo_image != NULL)
    {
        lv_obj_move_foreground(logo_image);
    }
    lv_screen_load(scr);
    lv_timer_create(factory_ui_sensor_timer_cb, 1000, NULL);

    lv_group_t *group = lv_group_get_default();
    if (group == NULL)
    {
        group = lv_group_create();
        lv_group_set_default(group);
    }
    lv_group_set_wrap(group, true);
    lv_group_remove_all_objs(group);
    factory_ui_attach_focusables_to_group(group);
}

void lvgl_debug_grid_screen(void);

void lvgl_Task(void *argument)
{	
	ST7789_Init();
	ST7789_Clear(WHITE);
	
	lv_init();
  	lv_port_disp_init();
  	lv_port_indev_init();
    unit_test_aht30_channel_init();
	factory_ui_create();
//	lvgl_debug_grid_screen();
	
	lv_indev_t * keypad = lv_port_indev_get_keypad();
  lv_group_t * group = lv_group_get_default();
  if ((keypad != NULL) && (group != NULL))
  {
    lv_indev_set_group(keypad, group);
  }
	
  for(;;)
  {
        unit_test_aht30_data_t measurement;
        if (unit_test_aht30_receive(&measurement, 0U))
        {
            factory_ui_update_sensor_labels(&measurement);
        }

        unit_test_button_state_t button_state = unit_test_button_get_state();
        if (button_state != UNIT_TEST_BUTTON_STATE_IDLE)
        {
            factory_ui_update_button_label(button_state);
        }

        unit_test_card_info_t card_info;
        unit_test_card_get_info(&card_info);
        if (!factory_ui_card_info_equal(&card_info, &displayed_card_info))
        {
            factory_ui_update_card_labels(&card_info);
        }

        unit_test_eeprom_status_t eeprom_status = unit_test_eeprom_get_status();
        if (eeprom_status != displayed_eeprom_status)
        {
            factory_ui_update_eeprom_label(eeprom_status);
        }

        bool usb_connected = unit_test_usb_cdc_is_connected();
        if (usb_connected != displayed_usb_connected)
        {
            factory_ui_update_usb_label(usb_connected);
        }

        uint8_t ir_code = 0U;
        if (unit_test_irda_take_command(&ir_code))
        {
            factory_ui_update_irda_label(ir_code);
        }

		lv_timer_handler();
    osDelay(2);
  }
}

static void factory_ui_update_sensor_labels(const unit_test_aht30_data_t *data)
{
    if (data == NULL)
    {
        return;
    }

    latest_sensor_data = *data;
    sensor_data_valid = true;
}

static void factory_ui_refresh_sensor_labels(void)
{
    if (!sensor_data_valid)
    {
        return;
    }

    if (label_temp_value != NULL)
    {
        int16_t temp10 = (int16_t)(latest_sensor_data.temperature * 10.0f);
        int16_t temp_abs = temp10 < 0 ? (int16_t)(-temp10) : temp10;
        lv_label_set_text_fmt(label_temp_value, "%d.%01d C",
                              (int)(temp10 / 10),
                              (int)(temp_abs % 10));
    }

    if (label_hum_value != NULL)
    {
        uint16_t hum10 = (uint16_t)(latest_sensor_data.humidity * 10.0f);
        lv_label_set_text_fmt(label_hum_value, "%u.%01u %%",
                              (unsigned)(hum10 / 10U),
                              (unsigned)(hum10 % 10U));
    }
}

static void factory_ui_sensor_timer_cb(lv_timer_t *timer)
{
    LV_UNUSED(timer);
    factory_ui_refresh_sensor_labels();
}

static void factory_ui_update_button_label(unit_test_button_state_t state)
{
    if (label_key_value == NULL)
    {
        displayed_button_state = state;
        return;
    }

    const char *text = "Idle";
    lv_color_t color = lv_color_hex(0x7f8ea3);

    switch (state)
    {
    case UNIT_TEST_BUTTON_STATE_SHORT:
        text = "Short";
        color = lv_color_hex(0xf4f8ff);
        break;
    case UNIT_TEST_BUTTON_STATE_DOUBLE:
        text = "Double";
        color = lv_color_hex(0x8de1ff);
        break;
    case UNIT_TEST_BUTTON_STATE_LONG:
        text = "Long";
        color = lv_color_hex(0xffb347);
        break;
    case UNIT_TEST_BUTTON_STATE_IDLE:
    default:
        break;
    }

    lv_label_set_text(label_key_value, text);
    lv_obj_set_style_text_color(label_key_value, color, 0);
    displayed_button_state = state;
    factory_ui_update_theme_for_button_state(state);
}

static void factory_ui_update_eeprom_label(unit_test_eeprom_status_t status)
{
    if (label_eeprom_value == NULL)
    {
        displayed_eeprom_status = status;
        return;
    }

    const char *text = "WAIT";
    lv_color_t color = lv_color_hex(0x7f8ea3);

    switch (status)
    {
    case UNIT_TEST_EEPROM_STATUS_TESTING:
        text = "TESTING";
        color = lv_color_hex(0xffd369);
        break;
    case UNIT_TEST_EEPROM_STATUS_PASS:
        text = "OK";
        color = lv_color_hex(0x58f39f);
        break;
    case UNIT_TEST_EEPROM_STATUS_FAIL:
        text = "ERROR";
        color = lv_color_hex(0xff6b6b);
        break;
    case UNIT_TEST_EEPROM_STATUS_NOT_TESTED:
    default:
        break;
    }

    lv_label_set_text(label_eeprom_value, text);
    lv_obj_set_style_text_color(label_eeprom_value, color, 0);
    displayed_eeprom_status = status;
}

static void factory_ui_update_theme_for_button_state(unit_test_button_state_t state)
{
    uint8_t desired_theme = active_theme_index;

    switch (state)
    {
    case UNIT_TEST_BUTTON_STATE_SHORT:
        desired_theme = 1U;
        break;
    case UNIT_TEST_BUTTON_STATE_DOUBLE:
        desired_theme = 2U;
        break;
    case UNIT_TEST_BUTTON_STATE_LONG:
        desired_theme = 3U;
        break;
    default:
        return;
    }

    if (desired_theme != active_theme_index)
    {
        factory_ui_apply_theme(desired_theme);
    }
}

static void factory_ui_apply_theme(uint8_t theme_index)
{
    if (theme_index >= FACTORY_THEME_COUNT)
    {
        return;
    }

    const factory_theme_t *theme = &factory_themes[theme_index];

    if (screen_root != NULL)
    {
        lv_obj_set_style_bg_color(screen_root, lv_color_hex(theme->screen_color), 0);
    }

    for (uint8_t i = 0; i < info_card_obj_count; i++)
    {
        if (info_card_objs[i] != NULL)
        {
            lv_obj_set_style_bg_color(info_card_objs[i], lv_color_hex(theme->card_color), 0);
        }
    }

    if (card_panel_obj != NULL)
    {
        lv_obj_set_style_bg_color(card_panel_obj, lv_color_hex(theme->panel_bg_color), 0);
    }

    if (label_header != NULL)
    {
        lv_obj_set_style_text_color(label_header, lv_color_hex(theme->accent_color), 0);
    }

    if (label_usb_status != NULL)
    {
        lv_obj_set_style_text_color(label_usb_status, lv_color_hex(theme->accent_color), 0);
    }

    if (label_irda_value != NULL)
    {
        lv_obj_set_style_text_color(label_irda_value, lv_color_hex(theme->accent_color), 0);
    }

    active_theme_index = theme_index;
    factory_ui_refresh_focusable_styles();
}

static void factory_ui_reset_focusables(void)
{
    for (uint8_t i = 0; i < FACTORY_UI_MAX_FOCUS_ITEMS; i++)
    {
        factory_focus_items[i].obj = NULL;
        factory_focus_items[i].role = FACTORY_FOCUS_ROLE_INFO_CARD;
        factory_focus_items[i].toggled = false;
        factory_focus_items[i].focused = false;
    }

    factory_focus_item_count = 0U;
}

static void factory_ui_register_focusable(lv_obj_t *obj, factory_focus_role_t role)
{
    if ((obj == NULL) || (factory_focus_item_count >= FACTORY_UI_MAX_FOCUS_ITEMS))
    {
        return;
    }

    factory_focus_item_t *item = &factory_focus_items[factory_focus_item_count++];
    item->obj = obj;
    item->role = role;
    item->toggled = false;
    item->focused = false;

    lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(obj, factory_ui_focusable_event_cb, LV_EVENT_ALL, item);
}

static void factory_ui_focusable_event_cb(lv_event_t *event)
{
    factory_focus_item_t *item = lv_event_get_user_data(event);
    if ((item == NULL) || (item->obj == NULL))
    {
        return;
    }

    switch (lv_event_get_code(event))
    {
    case LV_EVENT_FOCUSED:
        item->focused = true;
        factory_ui_apply_focusable_style(item);
        break;
    case LV_EVENT_DEFOCUSED:
        item->focused = false;
        factory_ui_apply_focusable_style(item);
        break;
    case LV_EVENT_KEY:
        if (lv_event_get_key(event) == LV_KEY_ENTER)
        {
            item->toggled = !item->toggled;
            factory_ui_apply_focusable_style(item);
        }
        break;
    case LV_EVENT_DELETE:
        item->obj = NULL;
        item->focused = false;
        item->toggled = false;
        break;
    default:
        break;
    }
}

static lv_color_t factory_ui_focusable_base_color(factory_focus_role_t role)
{
    const factory_theme_t *theme = &factory_themes[active_theme_index];
    uint32_t color = theme->card_color;

    if (role == FACTORY_FOCUS_ROLE_CARD_PANEL)
    {
        color = theme->panel_bg_color;
    }

    return lv_color_hex(color);
}

static void factory_ui_apply_focusable_style(factory_focus_item_t *item)
{
    if ((item == NULL) || (item->obj == NULL))
    {
        return;
    }

    const factory_theme_t *theme = &factory_themes[active_theme_index];
    lv_color_t accent = lv_color_hex(theme->accent_color);
    lv_color_t base = factory_ui_focusable_base_color(item->role);
    lv_color_t bg = item->toggled ? accent : base;

    lv_obj_set_style_bg_color(item->obj, bg, 0);
    lv_obj_set_style_outline_color(item->obj, accent, 0);
    lv_obj_set_style_outline_width(item->obj, item->focused ? 3 : 0, 0);
    lv_obj_set_style_outline_pad(item->obj, 2, 0);
    lv_obj_set_style_outline_opa(item->obj, item->focused ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
}

static void factory_ui_refresh_focusable_styles(void)
{
    for (uint8_t i = 0; i < factory_focus_item_count; i++)
    {
        factory_ui_apply_focusable_style(&factory_focus_items[i]);
    }
}

static void factory_ui_attach_focusables_to_group(lv_group_t *group)
{
    if (group == NULL)
    {
        return;
    }

    lv_obj_t *default_focus = NULL;

    for (uint8_t i = 0; i < factory_focus_item_count; i++)
    {
        if (factory_focus_items[i].obj != NULL)
        {
            lv_group_add_obj(group, factory_focus_items[i].obj);
            if ((factory_focus_items[i].role == FACTORY_FOCUS_ROLE_CARD_PANEL) && (default_focus == NULL))
            {
                default_focus = factory_focus_items[i].obj;
            }
        }
    }

    if ((default_focus == NULL) && (factory_focus_item_count > 0U))
    {
        default_focus = factory_focus_items[0].obj;
    }

    if (default_focus != NULL)
    {
        lv_group_focus_obj(default_focus);
    }
}

static void factory_ui_update_usb_label(bool connected)
{
    if (label_usb_status == NULL)
    {
        displayed_usb_connected = connected;
        return;
    }

    if (connected)
    {
        lv_label_set_text(label_usb_status, "USB");
        lv_obj_clear_flag(label_usb_status, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_add_flag(label_usb_status, LV_OBJ_FLAG_HIDDEN);
    }

    displayed_usb_connected = connected;
}

static void factory_ui_update_irda_label(uint8_t code)
{
    if (label_irda_value == NULL)
    {
        return;
    }

    lv_label_set_text_fmt(label_irda_value, "0x%02X", code);
}

static void factory_ui_update_card_labels(const unit_test_card_info_t *info)
{
    if (info == NULL)
    {
        return;
    }

    if ((label_card_total_value == NULL) || (label_card_free_value == NULL))
    {
        displayed_card_info = *info;
        return;
    }

    if (!info->initialized)
    {
        lv_label_set_text(label_card_total_value, "Total -- MB");
        lv_obj_set_style_text_color(label_card_total_value, lv_color_hex(0x7f8ea3), 0);
        lv_label_set_text(label_card_free_value, "Free  -- MB");
        lv_obj_set_style_text_color(label_card_free_value, lv_color_hex(0x7f8ea3), 0);
    }
    else if (!info->filesystem_available)
    {
        lv_label_set_text(label_card_total_value, "Card ERR");
        lv_obj_set_style_text_color(label_card_total_value, lv_color_hex(0xff6b6b), 0);
        lv_label_set_text(label_card_free_value, "No filesystem");
        lv_obj_set_style_text_color(label_card_free_value, lv_color_hex(0xff6b6b), 0);
    }
    else if (!info->test_passed)
    {
        lv_label_set_text(label_card_total_value, "Test FAIL");
        lv_obj_set_style_text_color(label_card_total_value, lv_color_hex(0xffb347), 0);
        lv_label_set_text(label_card_free_value, "Retry card");
        lv_obj_set_style_text_color(label_card_free_value, lv_color_hex(0xffb347), 0);
    }
    else
    {
			lv_label_set_text_fmt(label_card_total_value, "Total: %lu MB", (unsigned long)info->total_mb);
        lv_obj_set_style_text_color(label_card_total_value, lv_color_hex(0xf4f8ff), 0);
			lv_label_set_text_fmt(label_card_free_value,  "Free:  %lu MB", (unsigned long)info->free_mb);
        lv_obj_set_style_text_color(label_card_free_value, lv_color_hex(0x9ab9e8), 0);
    }

    displayed_card_info = *info;
}

static bool factory_ui_card_info_equal(const unit_test_card_info_t *a, const unit_test_card_info_t *b)
{
    if ((a == NULL) || (b == NULL))
    {
        return false;
    }

    return (a->initialized == b->initialized) &&
           (a->filesystem_available == b->filesystem_available) &&
           (a->test_passed == b->test_passed) &&
           (a->total_mb == b->total_mb) &&
           (a->free_mb == b->free_mb);
}

static lv_obj_t *factory_ui_create_info_card(lv_obj_t *parent, const char *caption, const char *placeholder)
{
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_remove_style_all(card);
    lv_obj_set_width(card, LV_PCT(48));
    lv_obj_set_height(card, 56);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x161f2d), 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(card, 14, 0);
    lv_obj_set_style_pad_all(card, 10, 0);
    lv_obj_set_style_pad_gap(card, 6, 0);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    if (info_card_obj_count < (sizeof(info_card_objs) / sizeof(info_card_objs[0])))
    {
        info_card_objs[info_card_obj_count++] = card;
    }

    lv_obj_t *name = lv_label_create(card);
    lv_label_set_text(name, caption);
    lv_obj_set_style_text_color(name, lv_color_hex(0x8fa4c2), 0);
    lv_obj_set_style_text_font(name, lv_theme_get_font_small(name), 0);

    lv_obj_t *value = lv_label_create(card);
    lv_label_set_text(value, placeholder);
    lv_obj_set_style_text_color(value, lv_color_hex(0xf4f8ff), 0);
    lv_obj_set_style_text_font(value, lv_theme_get_font_large(value), 0);

    return value;
}

static void factory_ui_create_card_panel(lv_obj_t *parent)
{
    lv_obj_t *panel = lv_obj_create(parent);
    lv_obj_remove_style_all(panel);
    lv_obj_set_size(panel, 210, 76);
    lv_obj_align(panel, LV_ALIGN_BOTTOM_MID, 0, -4);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x111926), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(panel, 12, 0);
    lv_obj_set_style_pad_all(panel, 10, 0);
    lv_obj_set_style_pad_gap(panel, 4, 0);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
    card_panel_obj = panel;

    lv_obj_t *title = lv_label_create(panel);
    lv_label_set_text(title, "CARD");
    lv_obj_set_style_text_color(title, lv_color_hex(0x8090a9), 0);
    lv_obj_set_style_text_font(title, lv_theme_get_font_small(title), 0);

    label_card_total_value = lv_label_create(panel);
    lv_label_set_text(label_card_total_value, "Total -- MB");
    lv_obj_set_style_text_color(label_card_total_value, lv_color_hex(0xf4f8ff), 0);
    lv_obj_set_style_text_font(label_card_total_value, lv_theme_get_font_normal(label_card_total_value), 0);

    label_card_free_value = lv_label_create(panel);
    lv_label_set_text(label_card_free_value, "Free  -- MB");
    lv_obj_set_style_text_color(label_card_free_value, lv_color_hex(0x9ab9e8), 0);
    lv_obj_set_style_text_font(label_card_free_value, lv_theme_get_font_small(label_card_free_value), 0);
}

static void factory_ui_seed_placeholder_data(void)
{
    if (label_card_total_value != NULL)
    {
        lv_label_set_text(label_card_total_value, "Total -- MB");
    }

    if (label_card_free_value != NULL)
    {
        lv_label_set_text(label_card_free_value, "Free  -- MB");
    }

    unit_test_card_info_t card_info;
    unit_test_card_get_info(&card_info);
    factory_ui_update_card_labels(&card_info);

    factory_ui_update_eeprom_label(unit_test_eeprom_get_status());
    factory_ui_update_button_label(unit_test_button_get_state());
    factory_ui_update_usb_label(unit_test_usb_cdc_is_connected());
}

void lvgl_debug_grid_screen(void)
{
    static const lv_coord_t grid_cols[] = {80, 80, 80, LV_GRID_TEMPLATE_LAST};
    static const lv_coord_t grid_rows[] = {80, 80, 80, LV_GRID_TEMPLATE_LAST};
    static const uint32_t color_codes[9] = {
        0xff6666, 0xffcc66, 0xffff66,
        0x66ff66, 0x66ffff, 0x6699ff,
        0xcc66ff, 0xff66cc, 0xffffff};

    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_remove_style_all(scr);
    lv_obj_set_size(scr, 240, 240);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    lv_obj_t *grid = lv_obj_create(scr);
    lv_obj_remove_style_all(grid);
    lv_obj_set_size(grid, 240, 240);
    lv_obj_center(grid);
    lv_obj_set_grid_dsc_array(grid, grid_cols, grid_rows);

    for (uint8_t i = 0; i < 3; i++)
    {
        for (uint8_t j = 0; j < 3; j++)
        {
            uint8_t idx = i * 3 + j;
            lv_obj_t *cell = lv_obj_create(grid);
            lv_obj_remove_style_all(cell);
            lv_obj_set_style_bg_color(cell, lv_color_hex(color_codes[idx]), 0);
            lv_obj_set_style_bg_opa(cell, LV_OPA_COVER, 0);
            lv_obj_set_style_radius(cell, 4, 0);
            lv_obj_set_grid_cell(cell, LV_GRID_ALIGN_STRETCH, j, 1, LV_GRID_ALIGN_STRETCH, i, 1);
        }
    }

    lv_screen_load(scr);
}
