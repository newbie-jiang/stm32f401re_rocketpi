
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "app.h"

typedef void (*task_entry_t)(void *argument);

typedef struct {
    task_entry_t entry;
    void *arg;
    const osThreadAttr_t *attr;
    osThreadId_t *out_handle;
} task_desc_t;

/* 任务句柄 */
static osThreadId_t lvglTaskHandle;
static osThreadId_t ledTaskHandle;
static osThreadId_t buttonTaskHandle;
static osThreadId_t shellTaskHandle;
static osThreadId_t aht30TaskHandle;
static osThreadId_t buzzerTaskHandle;
static osThreadId_t at24cxxTaskHandle;
static osThreadId_t irdaTaskHandle;
static osThreadId_t cardTaskHandle;
static osThreadId_t usb_cdc_TaskHandle;
static osThreadId_t admin_TaskHandle;
static osThreadId_t extern_io_TaskHandle;

/* 任务属性（必须是 static/global，不能用局部变量） */
static const osThreadAttr_t lvglTask_attributes = {
    .name = "lvglTask",                   
		.stack_size = 4096,  										// bytes
    .priority = (osPriority_t)osPriorityHigh,
};

static const osThreadAttr_t ledTask_attributes = {
    .name = "ledTask",
    .stack_size = 512,                       // bytes（按需调整）
    .priority = (osPriority_t)osPriorityHigh,
};

static const osThreadAttr_t buttonTask_attributes = {
    .name = "buttonTask",
    .stack_size = 1024,                       // bytes（按需调整）
    .priority = (osPriority_t)osPriorityNormal,
};

static const osThreadAttr_t shellTask_attributes = {
    .name = "shellTask",
    .stack_size = 2048,                       // bytes（按需调整）
    .priority = (osPriority_t)osPriorityNormal,
};

static const osThreadAttr_t aht30Task_attributes = {
    .name = "aht30Task",
    .stack_size = 1024+512,                       // bytes（按需调整）
    .priority = (osPriority_t)osPriorityNormal,
};

static const osThreadAttr_t buzzerTask_attributes = {
    .name = "buzzerTask",
    .stack_size = 1024,                       // bytes（按需调整）
    .priority = (osPriority_t)osPriorityNormal,
};

static const osThreadAttr_t at24cxxTask_attributes = {
    .name = "at24cxxTask",
    .stack_size = 1024+512,                       // bytes（按需调整）
    .priority = (osPriority_t)osPriorityNormal,
};

static const osThreadAttr_t irdaTask_attributes = {
    .name = "irdaTask",
    .stack_size = 1024+512,                       // bytes（按需调整）
    .priority = (osPriority_t)osPriorityLow,
};

static const osThreadAttr_t cardTask_attributes = {
    .name = "cardTask",
    .stack_size = 2048,                       // bytes（按需调整）
    .priority = (osPriority_t)osPriorityNormal,
};

static const osThreadAttr_t usb_cdc_Task_attributes = {
    .name = "usb_cdc_Task",
    .stack_size = 2048,                       // bytes（按需调整）
    .priority = (osPriority_t)osPriorityHigh,
};

static const osThreadAttr_t admin_Task_attributes = {
    .name = "admin_Task",
    .stack_size = 2048,                       // bytes（按需调整）
    .priority = (osPriority_t)osPriorityLow,
};

static const osThreadAttr_t extern_io_Task_attributes = {
    .name = "extern_io_Task",
    .stack_size = 512,                       // bytes（按需调整）
    .priority = (osPriority_t)osPriorityHigh,
};

/* 任务入口声明 */
extern void lvgl_Task(void *argument);
extern void led_Task(void *argument);
extern void button_Task(void *argument);
extern void shell_Task(void *argument);
extern void aht30_Task(void *argument);
extern void buzzer_Task(void *argument);
extern void at24cxx_Task(void *argument);
extern void irda_Task(void *argument);
extern void card_Task(void *argument);
extern void usb_cdc_Task(void *argument);
extern void admin_Task(void *argument);
extern void extern_io_Task(void *argument);
/* 任务表 */
static const task_desc_t g_tasks[] = {
    { lvgl_Task, 	 		NULL, &lvglTask_attributes, 	 		&lvglTaskHandle },
    { led_Task, 	 		NULL, &ledTask_attributes, 	   		&ledTaskHandle },
		{ button_Task, 		NULL, &buttonTask_attributes,  		&buttonTaskHandle },
		{ shell_Task, 		NULL, &shellTask_attributes,   		&shellTaskHandle },
		{ aht30_Task, 		NULL, &aht30Task_attributes,  		&aht30TaskHandle },
		{ buzzer_Task, 		NULL, &buzzerTask_attributes, 		&buzzerTaskHandle },
		{ at24cxx_Task, 	NULL, &at24cxxTask_attributes, 		&at24cxxTaskHandle },
		{ irda_Task, 			NULL, &irdaTask_attributes, 	 		&irdaTaskHandle },
		{ card_Task, 			NULL, &cardTask_attributes, 	 		&cardTaskHandle },
		{ usb_cdc_Task, 	NULL, &usb_cdc_Task_attributes,		&usb_cdc_TaskHandle },
		{ admin_Task, 		NULL, &admin_Task_attributes,	 		&admin_TaskHandle },
		{ extern_io_Task, NULL, &extern_io_Task_attributes,	&extern_io_TaskHandle },
};

void bsp_init(void)
{
	
	
	
}

void app(void)
{
    bsp_init();
	 
    for (size_t i = 0; i < (sizeof(g_tasks)/sizeof(g_tasks[0])); i++) {
        const task_desc_t *t = &g_tasks[i];
        osThreadId_t h = osThreadNew(t->entry, t->arg, t->attr);
        if (t->out_handle) {
            *t->out_handle = h;
        }
        // 建议加错误处理：h == NULL 说明创建失败（通常是内存/栈不足）
    }
}
