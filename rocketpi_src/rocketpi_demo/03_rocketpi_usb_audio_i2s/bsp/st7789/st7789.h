#ifndef ST7789_H
#define ST7789_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "gpio.h"
#include "spi.h"
#include "fonts.h"     /* 需要提供 FontDef 定义；若与你现有不一致，可告诉我改配 */

/*================= 用户配置（与实际屏幕/连线匹配） =================*/

/* 分辨率 */
#ifndef ST7789_WIDTH
#define ST7789_WIDTH    240
#endif
#ifndef ST7789_HEIGHT
#define ST7789_HEIGHT   240
#endif

/* 兼容你之前代码使用的 HIGHT 宏名 */
#ifndef ST7789_HIGHT
#define ST7789_HIGHT    ST7789_HEIGHT
#endif

/* 旋转角：0/90/180/270 */
#ifndef ST7789_ROTATION
#define ST7789_ROTATION 0
#endif

/* 若你的 240x240 模组实际 IC 窗口为 240x320，需要 Y 方向偏移 80（常见）。
   如发现图像上下移/被裁剪，尝试把 Y_SHIFT 改为 80。 */
#ifndef X_SHIFT
#define X_SHIFT 0
#endif
#ifndef Y_SHIFT
#define Y_SHIFT 0 /* 常见：0 或 80 */
#endif

/* 颜色顺序：有的屏需要 BGR。0=RGB，1=BGR */
#ifndef ST7789_BGR
#define ST7789_BGR 0
#endif

/* SPI 句柄（沿用你的写法） */
#ifndef ST7789_SPI
#define ST7789_SPI (&hspi1)
#endif

/* 发送像素缓冲大小（字节，偶数）。越大整屏填充越快（受 RAM 限制）。 */
#ifndef ST7789_BUF_SIZE
#define ST7789_BUF_SIZE (240 * 2)
#endif

/* 控制引脚（沿用你的宏） */
#define ST7789_RST_LOW()   HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET)
#define ST7789_RST_HIGH()  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET)

#define ST7789_CS_LOW()    HAL_GPIO_WritePin(LCD_CS_GPIO_Port,  LCD_CS_Pin,  GPIO_PIN_RESET)
#define ST7789_CS_HIGH()   HAL_GPIO_WritePin(LCD_CS_GPIO_Port,  LCD_CS_Pin,  GPIO_PIN_SET)

#define ST7789_DC_LOW()    HAL_GPIO_WritePin(LCD_DC_GPIO_Port,  LCD_DC_Pin,  GPIO_PIN_RESET)
#define ST7789_DC_HIGH()   HAL_GPIO_WritePin(LCD_DC_GPIO_Port,  LCD_DC_Pin,  GPIO_PIN_SET)

#define ST7789_BL_LOW()    HAL_GPIO_WritePin(LCD_BL_GPIO_Port,  LCD_BL_Pin,  GPIO_PIN_RESET)
#define ST7789_BL_HIGH()   HAL_GPIO_WritePin(LCD_BL_GPIO_Port,  LCD_BL_Pin,  GPIO_PIN_SET)

/* 颜色常量（沿用你的） */
#define WHITE     0xFFFF
#define BLACK     0x0000
#define BLUE      0x001F
#define BRED      0xF81F
#define GRED      0xFFE0
#define GBLUE     0x07FF
#define RED       0xF800
#define MAGENTA   0xF81F
#define GREEN     0x07E0
#define CYAN      0x7FFF
#define YELLOW    0xFFE0
#define BROWN     0xBC40
#define BRRED     0xFC07
#define GRAY      0x8430


typedef struct {
    const char *text;         // 要滚动的字符串
    const FontDef *font;      // 指向字体（允许是 const FontDef）
    uint16_t y;               // 顶部 Y 坐标
    uint16_t color;           // 字颜色
    uint16_t bgcolor;         // 背景色（用于擦除带）
    int16_t  x;               // 当前起点 X（像素，可为负）
    int16_t  speed_px;        // 每步像素（>0 左->右；<0 右->左）
    uint16_t gap_px;          // 循环间距
    uint32_t interval_ms;     // 帧间隔
    uint32_t last_ms;         // 内部：上次刷新时间戳
} ST7789_Marquee;


/**
 * @brief 跑马灯初始化（非阻塞版本使用）。
 *
 * 将一段字符串设置为在指定 Y 行进行水平滚动显示。配合 ST7789_MarqueeStep()
 * 周期调用实现动画。支持像素级平滑滚动：当文本尚未完全进入屏幕时，
 * 也会绘制可见的“半字符”像素。
 *
 * @param m            跑马灯控制结构体指针（由调用方提供保存状态）。
 * @param text         要滚动的 C 字符串（生命周期需覆盖整个显示期间）。
 * @param font         字体指针（与现有 FontDef 格式一致：每字符 24 行×16 位等）。
 * @param y            文本左上角 Y 坐标（0..HEIGHT-font->height），单行绘制，不换行。
 * @param color        文本前景色（RGB565）。
 * @param bgcolor      背景色（RGB565），用于擦除该文字带（只重绘这条带，避免闪烁）。
 * @param speed_px     每帧移动像素：>0 表示“左→右”，<0 表示“右→左”，=0 将被归一为 1。
 * @param gap_px       每轮滚动结束后重新从屏外进入前的额外留白（像素），用于视觉节奏。
 * @param interval_ms  帧间隔（毫秒），例如 16≈60FPS，33≈30FPS；=0 将默认 16ms。
 *
 * @note  本实现是单行合成+DMA 推送，刷新开销低；建议在主循环/定时器中调用 Step。
 * @note  若需要更改方向，直接传入 speed_px 的符号即可（不需要重新 init）。
 */
void ST7789_MarqueeInit(ST7789_Marquee *m,
                        const char *text,
                        const FontDef *font,
                        uint16_t y,
                        uint16_t color,
                        uint16_t bgcolor,
                        int16_t speed_px,
                        uint16_t gap_px,
                        uint32_t interval_ms);

/**
 * @brief 跑马灯步进（非阻塞）：到时才刷新一帧。
 *
 * 依据 ST7789_MarqueeInit() 设置的参数与上次刷新的时间戳，判断是否需要
 * 刷新；若到时则仅重绘目标文字带的一行行像素（像素级平滑），并推进 x 位置。
 *
 * @param m       跑马灯控制结构体指针。
 * @param now_ms  当前毫秒计数（通常传 HAL_GetTick()）。
 *
 * @note  内部会维护 last_ms，以 interval_ms 为周期刷新；未到时间直接返回。
 * @note  若 speed_px > 0：文本从屏幕左外侧进入，穿过屏幕，越过右边界后从左外侧
 *        重新进入；若 speed_px < 0：方向相反（右入左出再右入）。
 * @note  仅重绘文字所占的 y..y+font->height-1 区域，其余画面不受影响。
 */
void ST7789_MarqueeStep(ST7789_Marquee *m, uint32_t now_ms);

/**
 * @brief 跑马灯阻塞演示：在指定时长内循环播放（便于快速验证效果）。
 *
 * 该函数内部会创建一个临时 ST7789_Marquee，调用 Step 并延时，直到达到
 * 指定 duration_ms。适合作为 Demo/自检；在正式工程中建议使用非阻塞
 * 的 Init + Step 方案。
 *
 * @param text         要滚动的 C 字符串。
 * @param font         字体指针（与 FontDef 格式一致）。
 * @param y            文本左上角 Y 坐标（单行，不换行）。
 * @param color        文本前景色（RGB565）。
 * @param bgcolor      背景色（RGB565），用于擦除该文字带。
 * @param speed_px     每帧移动像素：>0 左→右；<0 右→左；=0 将归一为 1。
 * @param gap_px       每轮滚动之间的额外留白（像素）。
 * @param interval_ms  帧间隔（毫秒），如 16≈60FPS。
 * @param duration_ms  播放总时长（毫秒）。
 *
 * @note  阻塞式实现：内部 while 循环持续到 duration_ms 结束。适合上电演示、
 *        单步调试；不适合作为正式 UI 的主循环方式。
 */
void ST7789_MarqueeRunBlocking(const char *text,
                               const FontDef *font,
                               uint16_t y,
                               uint16_t color,
                               uint16_t bgcolor,
                               int16_t speed_px,
                               uint16_t gap_px,
                               uint32_t interval_ms,
                               uint32_t duration_ms);


/*================= 对外 API =================*/

void ST7789_Init(void);
void ST7789_Clear(uint16_t color);

void ST7789_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void ST7789_DrawHLine(uint16_t xs, uint16_t xe, uint16_t y, uint16_t color);
void ST7789_DrawVLine(uint16_t ys, uint16_t ye, uint16_t x, uint16_t color);
void ST7789_FillRect(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t color);

/* 位图（RGB565 小端：低字节在前） */
void ST7789_DrawBitLine16BPP(uint16_t xs, uint16_t y, const uint8_t *p, uint16_t xsize);
void ST7789_DrawBitmap(uint16_t xs, uint16_t ys, uint16_t xsize, uint16_t ysize, const uint8_t *p);

/* 文本（FontDef 要与你的 fonts.h 匹配；若每行不是 16bit 位图，告诉我改位取法） */
void ST7789_ShowChar(uint16_t x, uint16_t y, uint8_t ch, FontDef font, uint16_t color, uint16_t bgcolor);
void ST7789_ShowString(uint16_t x, uint16_t y, const char *str, FontDef font, uint16_t color, uint16_t bgcolor);

/* 简单帧率测试 */
void ST7789_TestFrameRate(void);

/*================= 可选：保留旧类型（如有外部引用就留，否则可删） =================*/
typedef enum {
    ST7789_CMD,
    ST7789_DATA,
} ST7789_DCType;

/* 若你的外部代码还在用 GUI_BITMAP，可保留；否则可删除 */
typedef struct {
    uint16_t XSize;
    uint16_t YSize;
    uint16_t BytesPerLine;
    uint16_t BitsPerPixel;
    const uint8_t *pData;
} GUI_BITMAP;

#ifdef __cplusplus
}
#endif
#endif /* ST7789_H */










