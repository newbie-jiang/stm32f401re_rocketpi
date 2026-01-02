#ifndef ST7789_H
#define ST7789_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "gpio.h"
#include "spi.h"

/*================= 用户配置（与实际屏幕/连线匹配） =================*/

/* 分辨率 */
#ifndef ST7789_WIDTH
#define ST7789_WIDTH    240
#endif
#ifndef ST7789_HEIGHT
#define ST7789_HEIGHT   240
#endif

#ifndef ST7789_HIGHT
#define ST7789_HIGHT    ST7789_HEIGHT
#endif

/* 旋转角仅保留 0 度显示方向 */
#define ST7789_LCD_WIDTH   ST7789_WIDTH
#define ST7789_LCD_HEIGHT  ST7789_HEIGHT

/* 若 240x240 模组实际 IC 窗口为 240x320，需要 Y 方向偏移 80（常见）。
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

/* SPI 句柄 */
#ifndef ST7789_SPI
#define ST7789_SPI (&hspi1)
#endif

/* 发送像素缓冲大小（字节，偶数）。越大整屏填充越快（受 RAM 限制）。 */
#ifndef ST7789_BUF_SIZE
#define ST7789_BUF_SIZE (240 * 2)
#endif

/* 控制引脚 */
#define ST7789_RST_LOW()   HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET)
#define ST7789_RST_HIGH()  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET)

#define ST7789_CS_LOW()    HAL_GPIO_WritePin(LCD_CS_GPIO_Port,  LCD_CS_Pin,  GPIO_PIN_RESET)
#define ST7789_CS_HIGH()   HAL_GPIO_WritePin(LCD_CS_GPIO_Port,  LCD_CS_Pin,  GPIO_PIN_SET)

#define ST7789_DC_LOW()    HAL_GPIO_WritePin(LCD_DC_GPIO_Port,  LCD_DC_Pin,  GPIO_PIN_RESET)
#define ST7789_DC_HIGH()   HAL_GPIO_WritePin(LCD_DC_GPIO_Port,  LCD_DC_Pin,  GPIO_PIN_SET)

#define ST7789_BL_LOW()    HAL_GPIO_WritePin(LCD_BL_GPIO_Port,  LCD_BL_Pin,  GPIO_PIN_RESET)
#define ST7789_BL_HIGH()   HAL_GPIO_WritePin(LCD_BL_GPIO_Port,  LCD_BL_Pin,  GPIO_PIN_SET)

/* 颜色常量 */
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


#ifdef __cplusplus
}
#endif
#endif /* ST7789_H */














