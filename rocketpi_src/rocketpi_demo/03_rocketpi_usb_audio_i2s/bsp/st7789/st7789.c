#include "st7789.h"
#include <string.h>
#include <stdio.h>

/*========================= 用户可配置区域 =========================*/

/* 分辨率与旋转 */
#ifndef ST7789_WIDTH
#define ST7789_WIDTH   240
#endif
#ifndef ST7789_HEIGHT
#define ST7789_HEIGHT  240
#endif

#ifndef ST7789_HIGHT
#define ST7789_HIGHT   ST7789_HEIGHT
#endif

/* 旋转角：0/90/180/270 */
#ifndef ST7789_ROTATION
#define ST7789_ROTATION  0
#endif

/* 若屏幕 240x240 模组实为 320 控制窗（常见 Y 偏移 80），可选其一： */
#ifndef X_SHIFT
#define X_SHIFT  0
#endif
#ifndef Y_SHIFT
#define Y_SHIFT  0   /* 如遇图像下移或截断，可尝试改为 80 */
#endif

/* RGB/BGR 颜色顺序：某些面板需要 BGR（=1） */
#ifndef ST7789_BGR
#define ST7789_BGR  0  /* 0=RGB, 1=BGR */
#endif

/* 发送像素的缓冲区大小（字节，需为偶数） */
#ifndef ST7789_BUF_SIZE
#define ST7789_BUF_SIZE  (240 * 2) 
#endif

/*========================= 外部硬件依赖宏 =========================*/
/* 需在 st7789.h 或工程配置里定义：
   - SPI 句柄：ST7789_SPI（SPI_HandleTypeDef*）
   - 控制脚：ST7789_CS_LOW/HIGH(), ST7789_DC_LOW/HIGH(), ST7789_RST_LOW/HIGH(), ST7789_BL_LOW/HIGH()
*/
#ifndef ST7789_SPI
#warning "Please define ST7789_SPI (SPI_HandleTypeDef*)"
#endif

/*========================= 字体结构 =========================*/
//#ifndef __FONT_DEF__
//#define __FONT_DEF__
//typedef struct {
//    uint16_t width;    /* 字符宽（像素） */
//    uint16_t height;   /* 字符高（像素） */
//    const uint16_t *data; /* 字形数据：假设每行 16bit 位图，宽<=16 */
//} FontDef;
//#endif
/*========================= 本地缓冲与工具函数 =========================*/
static uint8_t ST7789_Buf[ST7789_BUF_SIZE];

/* 阻塞发送（命令/小数据使用） */
static inline void st7789_tx_blocking(const uint8_t *buf, size_t len) {
    HAL_SPI_Transmit(ST7789_SPI, (uint8_t*)buf, len, HAL_MAX_DELAY);
}

/* 命令写入 */
static inline void ST7789_WriteCmd(uint8_t cmd) {
    ST7789_CS_LOW();
    ST7789_DC_LOW();
    st7789_tx_blocking(&cmd, 1);
    ST7789_CS_HIGH();
}

/* 数据写入（小块，阻塞） */
static inline void ST7789_WriteData(const uint8_t *data, size_t len) {
    if (len == 0) return;
    ST7789_CS_LOW();
    ST7789_DC_HIGH();
    st7789_tx_blocking(data, len);
    ST7789_CS_HIGH();
}

/* 数据写入（大块，DMA + 轮询结束；若要更高效可改为回调+双缓冲） */
static inline void ST7789_WriteDataDMA(const uint8_t *data, size_t len) {
    if (len == 0) return;
    ST7789_CS_LOW();
    ST7789_DC_HIGH();
    HAL_SPI_Transmit_DMA(ST7789_SPI, (uint8_t*)data, len);
    while (HAL_SPI_GetState(ST7789_SPI) == HAL_SPI_STATE_BUSY_TX) { /* 轮询等待 */ }
    ST7789_CS_HIGH();
}

/* 设置窗口：一次性下发 CASET/RASET/RAMWR，内部处理旋转与偏移 */
static void ST7789_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    uint16_t xs, xe, ys, ye;

#if (ST7789_ROTATION == 0)
    xs = x0; xe = x1;
    ys = y0; ye = y1;
#elif (ST7789_ROTATION == 90)
    xs = y0;
    xe = y1;
    ys = (ST7789_WIDTH - 1 - x1);
    ye = (ST7789_WIDTH - 1 - x0);
#elif (ST7789_ROTATION == 180)
    xs = (ST7789_WIDTH  - 1 - x1);
    xe = (ST7789_WIDTH  - 1 - x0);
    ys = (ST7789_HEIGHT - 1 - y1);
    ye = (ST7789_HEIGHT - 1 - y0);
#elif (ST7789_ROTATION == 270)
    xs = (ST7789_HEIGHT - 1 - y1);
    xe = (ST7789_HEIGHT - 1 - y0);
    ys = x0;
    ye = x1;
#else
# error "Invalid ST7789_ROTATION"
#endif

    xs += X_SHIFT; xe += X_SHIFT;
    ys += Y_SHIFT; ye += Y_SHIFT;

    uint8_t caset[4] = { xs >> 8, xs & 0xFF, xe >> 8, xe & 0xFF };
    uint8_t raset[4] = { ys >> 8, ys & 0xFF, ye >> 8, ye & 0xFF };

    ST7789_CS_LOW();

    /* CASET */
    { uint8_t c = 0x2A;
      ST7789_DC_LOW();  st7789_tx_blocking(&c,1);
      ST7789_DC_HIGH(); st7789_tx_blocking(caset,4);
    }

    /* RASET */
    { uint8_t c = 0x2B;
      ST7789_DC_LOW();  st7789_tx_blocking(&c,1);
      ST7789_DC_HIGH(); st7789_tx_blocking(raset,4);
    }

    /* RAMWR */
    { uint8_t c = 0x2C;
      ST7789_DC_LOW();  st7789_tx_blocking(&c,1);
    }

    ST7789_CS_HIGH();
}

/*========================= 公共 API 实现 =========================*/

void ST7789_Init(void)
{
    /* 背光优先点亮（可按需） */
    ST7789_BL_HIGH();
    ST7789_CS_HIGH();

    /* 复位：建议 >=10ms 低电平，>=120ms 出睡眠后延时 */
    ST7789_RST_HIGH();
    HAL_Delay(1);
    ST7789_RST_LOW();
    HAL_Delay(10);
    ST7789_RST_HIGH();
    HAL_Delay(120);

    /* Sleep Out */
    ST7789_WriteCmd(0x11);
    HAL_Delay(120);

    /* 像素格式：16bpp RGB565 */
    ST7789_WriteCmd(0x3A);
    { uint8_t fmt = 0x55; ST7789_WriteData(&fmt,1); }

    /* Porch control */
    ST7789_WriteCmd(0xB2); {
        uint8_t d[] = {0x0C,0x0C,0x00,0x33,0x33};
        ST7789_WriteData(d, sizeof(d));
    }

    /* Gate control */
    ST7789_WriteCmd(0xB7); { uint8_t d=0x35; ST7789_WriteData(&d,1); }

    /* VCOM */
    ST7789_WriteCmd(0xBB); { uint8_t d=0x32; ST7789_WriteData(&d,1); } /* 1.35V */

    /* LCM control */
    ST7789_WriteCmd(0xC0); { uint8_t d=0x2C; ST7789_WriteData(&d,1); } /* 可选 */

    /* VDV/VRH */
    ST7789_WriteCmd(0xC2); { uint8_t d=0x01; ST7789_WriteData(&d,1); }
    ST7789_WriteCmd(0xC3); { uint8_t d=0x19; ST7789_WriteData(&d,1); } /* GVDD=4.8V */
    ST7789_WriteCmd(0xC4); { uint8_t d=0x20; ST7789_WriteData(&d,1); } /* VDV, 0x20:0V */

    /* Frame rate */
    ST7789_WriteCmd(0xC6); { uint8_t d=0x0F; ST7789_WriteData(&d,1); } /* 60Hz */

    /* Power control */
    ST7789_WriteCmd(0xD0); { uint8_t d[2]={0xA4,0xA1}; ST7789_WriteData(d,2); }

    /* 正伽马 */
    ST7789_WriteCmd(0xE0); {
        uint8_t d[] = {0xD0,0x08,0x0E,0x09,0x09,0x05,0x31,0x33,0x48,0x17,0x14,0x15,0x31,0x34};
        ST7789_WriteData(d,sizeof(d));
    }
    /* 负伽马 */
    ST7789_WriteCmd(0xE1); {
        uint8_t d[] = {0xD0,0x08,0x0E,0x09,0x09,0x15,0x31,0x33,0x48,0x17,0x14,0x15,0x31,0x34};
        ST7789_WriteData(d,sizeof(d));
    }

		
    /* 反色 */
    ST7789_WriteCmd(0x21); /* Normal display */

    /* MADCTL：方向 + RGB/BGR */
    {
        uint8_t madctl = 0x00;
    #if (ST7789_ROTATION == 0)
        madctl = 0x00;
    #elif (ST7789_ROTATION == 90)
        madctl = 0x60;
    #elif (ST7789_ROTATION == 180)
        madctl = 0xC0;
    #elif (ST7789_ROTATION == 270)
        madctl = 0xA0;
    #endif
    #if (ST7789_BGR == 1)
        madctl |= 0x08; /* BGR 位 */
    #endif
        ST7789_WriteCmd(0x36);
        ST7789_WriteData(&madctl,1);
    }

    /* 显示开 */
    ST7789_WriteCmd(0x29);
}

/* 清屏（整屏填充 color） */
void ST7789_Clear(uint16_t color)
{
    uint32_t total_bytes = (uint32_t)ST7789_WIDTH * ST7789_HEIGHT * 2;
    /* 预填充缓冲 */
    for (uint32_t i = 0; i < ST7789_BUF_SIZE; i += 2) {
        ST7789_Buf[i]   = color >> 8;
        ST7789_Buf[i+1] = color & 0xFF;
    }

    ST7789_SetWindow(0, 0, ST7789_WIDTH - 1, ST7789_HEIGHT - 1);

    /* 启动 RAMWR 后，连续写数据 */
    uint8_t ramwr = 0x2C;
    ST7789_WriteCmd(ramwr);

    /* 按块写入 */
    uint32_t remain = total_bytes;
    while (remain) {
        uint32_t chunk = (remain > ST7789_BUF_SIZE) ? ST7789_BUF_SIZE : remain;
        ST7789_WriteDataDMA(ST7789_Buf, chunk);
        remain -= chunk;
    }
}

/* 画一个像素（不建议频繁调用，演示用） */
void ST7789_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if (x >= ST7789_WIDTH || y >= ST7789_HEIGHT) return;
    ST7789_SetWindow(x, y, x, y);
    uint8_t px[2] = { color >> 8, color & 0xFF };
    ST7789_WriteCmd(0x2C);
    ST7789_WriteData(px, 2);
}

/* 横线（一次性打包） */
void ST7789_DrawHLine(uint16_t xs, uint16_t xe, uint16_t y, uint16_t color)
{
    if (y >= ST7789_HEIGHT) return;
    if (xe < xs) { uint16_t t = xs; xs = xe; xe = t; }
    if (xs >= ST7789_WIDTH) return;
    if (xe >= ST7789_WIDTH) xe = ST7789_WIDTH - 1;

    uint16_t w = xe - xs + 1;
    uint32_t bytes = (uint32_t)w * 2;

    /* 准备一行颜色 */
    for (uint32_t i=0; i<bytes; i+=2) {
        ST7789_Buf[i]   = color >> 8;
        ST7789_Buf[i+1] = color & 0xFF;
    }

    ST7789_SetWindow(xs, y, xe, y);
    ST7789_WriteCmd(0x2C);
    ST7789_WriteDataDMA(ST7789_Buf, bytes);
}

/* 竖线（一次性打包后多段写） */
void ST7789_DrawVLine(uint16_t ys, uint16_t ye, uint16_t x, uint16_t color)
{
    if (x >= ST7789_WIDTH) return;
    if (ye < ys) { uint16_t t = ys; ys = ye; ye = t; }
    if (ys >= ST7789_HEIGHT) return;
    if (ye >= ST7789_HEIGHT) ye = ST7789_HEIGHT - 1;

    uint16_t h = ye - ys + 1;
    uint32_t bytes = (uint32_t)h * 2;

    /* 若一整段能装入缓冲，则一次写完，否则分块 */
    if (bytes <= ST7789_BUF_SIZE) {
        for (uint32_t i=0; i<bytes; i+=2) {
            ST7789_Buf[i]   = color >> 8;
            ST7789_Buf[i+1] = color & 0xFF;
        }
        ST7789_SetWindow(x, ys, x, ye);
        ST7789_WriteCmd(0x2C);
        ST7789_WriteDataDMA(ST7789_Buf, bytes);
    } else {
        ST7789_SetWindow(x, ys, x, ye);
        ST7789_WriteCmd(0x2C);
        /* 先填满一次缓冲 */
        for (uint32_t i=0; i<ST7789_BUF_SIZE; i+=2) {
            ST7789_Buf[i]   = color >> 8;
            ST7789_Buf[i+1] = color & 0xFF;
        }
        uint32_t remain = bytes;
        while (remain) {
            uint32_t chunk = (remain > ST7789_BUF_SIZE) ? ST7789_BUF_SIZE : remain;
            ST7789_WriteDataDMA(ST7789_Buf, chunk);
            remain -= chunk;
        }
    }
}

/* 填充矩形（尽量用整块 DMA） */
void ST7789_FillRect(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t color)
{
    if (xe < xs) { uint16_t t=xs; xs=xe; xe=t; }
    if (ye < ys) { uint16_t t=ys; ys=ye; ye=t; }
    if (xs >= ST7789_WIDTH || ys >= ST7789_HEIGHT) return;
    if (xe >= ST7789_WIDTH)  xe = ST7789_WIDTH - 1;
    if (ye >= ST7789_HEIGHT) ye = ST7789_HEIGHT - 1;

    uint16_t w = xe - xs + 1;
    uint16_t h = ye - ys + 1;
    uint32_t total = (uint32_t)w * h * 2;

    /* 预填充缓冲 */
    for (uint32_t i=0; i<ST7789_BUF_SIZE; i+=2) {
        ST7789_Buf[i]   = color >> 8;
        ST7789_Buf[i+1] = color & 0xFF;
    }

    ST7789_SetWindow(xs, ys, xe, ye);
    ST7789_WriteCmd(0x2C);

    /* 按块写入 */
    uint32_t remain = total;
    while (remain) {
        uint32_t chunk = (remain > ST7789_BUF_SIZE) ? ST7789_BUF_SIZE : remain;
        ST7789_WriteDataDMA(ST7789_Buf, chunk);
        remain -= chunk;
    }
}

/**
 * 画一行 16BPP 位图（RGB565，小端存储：p[0]=low, p[1]=high）
 * xsize ∈ [1, ST7789_WIDTH]
 */
void ST7789_DrawBitLine16BPP(uint16_t xs, uint16_t y, const uint8_t *p, uint16_t xsize)
{
    if (y >= ST7789_HEIGHT || xs >= ST7789_WIDTH) return;
    if (xsize == 0) return;
    if (xs + xsize > ST7789_WIDTH) xsize = ST7789_WIDTH - xs;

    uint32_t bytes = (uint32_t)xsize * 2;

    /* 小端 -> 高字节在前（面板需要高字节先出） */
    for (uint32_t i = 0; i < bytes; i += 2) {
        uint8_t lo = p[i];
        uint8_t hi = p[i+1];
        ST7789_Buf[i]   = hi;
        ST7789_Buf[i+1] = lo;
    }

    ST7789_SetWindow(xs, y, xs + xsize - 1, y);
    ST7789_WriteCmd(0x2C);
    ST7789_WriteDataDMA(ST7789_Buf, bytes);
}

/* 画任意矩形位图：逐行送（p 为 RGB565 小端） */
void ST7789_DrawBitmap(uint16_t xs, uint16_t ys, uint16_t xsize, uint16_t ysize, const uint8_t *p)
{
    if (xs >= ST7789_WIDTH || ys >= ST7789_HEIGHT) return;
    if (xsize == 0 || ysize == 0) return;

    uint16_t xmax = (xs + xsize > ST7789_WIDTH)  ? (ST7789_WIDTH  - xs) : xsize;
    uint16_t ymax = (ys + ysize > ST7789_HEIGHT) ? (ST7789_HEIGHT - ys) : ysize;

    for (uint16_t i=0; i<ymax; i++) {
        ST7789_DrawBitLine16BPP(xs, ys + i, p + (uint32_t)i * xsize * 2, xmax);
    }
}

void ST7789_ShowChar(uint16_t x, uint16_t y, uint8_t ch, FontDef font, uint16_t color, uint16_t bgcolor)
{
    if (x >= ST7789_WIDTH || y >= ST7789_HEIGHT) return;
    if (font.width == 0 || font.height == 0) return;

    /* 使用局部变量承接，避免修改 font 本体 */
    uint16_t w = font.width;
    uint16_t h = font.height;

    /* 这版实现假设“每行最多 16 位位图”，超出则按 16 处理 */
    if (w > 16) w = 16;

    /* 越界裁剪 */
    if (x + w - 1 >= ST7789_WIDTH || y + h - 1 >= ST7789_HEIGHT) return;

    ST7789_SetWindow(x, y, x + w - 1, y + h - 1);
    ST7789_WriteCmd(0x2C);

    /* 把字符整块写入，按行拼进缓冲区，满了就发一块 */
    uint32_t wrote = 0;
    for (uint16_t row = 0; row < h; row++) {
        /* 注意：如果你的 fonts.h 定义不是每行 16bit，需要按你的格式改这里取位方式 */
        uint16_t bits = font.data[((uint32_t)(ch - 32) * h) + row];
        for (uint16_t col = 0; col < w; col++) {
            uint16_t c = (bits & (1U << (w - 1 - col))) ? color : bgcolor;

            /* 将像素写入发送缓冲（双字节：高位在前） */
            uint32_t idx = wrote % ST7789_BUF_SIZE;
            ST7789_Buf[idx + 0] = (uint8_t)(c >> 8);
            ST7789_Buf[idx + 1] = (uint8_t)(c & 0xFF);
            wrote += 2;

            if ((wrote % ST7789_BUF_SIZE) == 0) {
                ST7789_WriteDataDMA(ST7789_Buf, ST7789_BUF_SIZE);
            }
        }
    }

    /* 发送尾块 */
    uint32_t remain = wrote % ST7789_BUF_SIZE;
    if (remain) {
        ST7789_WriteDataDMA(ST7789_Buf, remain);
    }
}

void ST7789_ShowString(uint16_t x, uint16_t y, const char *str, FontDef font, uint16_t color, uint16_t bgcolor)
{
    if (!str) return;
    /* 同样用局部变量保存宽高 */
    uint16_t fw = font.width;
    uint16_t fh = font.height;
    if (fw == 0 || fh == 0) return;
    if (fw > 16) fw = 16;

    uint16_t cx = x, cy = y;
    while (*str) {
        /* 自动换行 */
        if (cx + fw > ST7789_WIDTH) {
            cx = 0;
            cy += fh;
            if (cy + fh > ST7789_HEIGHT) break;
            if (*str == ' ') { str++; continue; }
        }

        ST7789_ShowChar(cx, cy, (uint8_t)*str, font, color, bgcolor);
        cx += fw;
        str++;
    }
}


/* 帧率测试：60 帧整屏填充不同色 */
void ST7789_TestFrameRate(void)
{
    static const uint16_t test_colors[] = { 0xF800, /*RED*/ 0x07E0, /*GREEN*/ 0x001F, /*BLUE*/
                                            0xFFFF, /*WHITE*/ 0x0000, /*BLACK*/ 0x07FF, /*CYAN*/
                                            0xF81F, /*MAGENTA*/ 0xFFE0 /*YELLOW*/ };
    const uint32_t frame_count = 60U;
    uint32_t tick_start = 0, elapsed = 0;

    /* 清屏 */
    ST7789_Clear(0x0000);

    tick_start = HAL_GetTick();
    for (uint32_t f=0; f<frame_count; f++) {
        uint16_t color = test_colors[f % (sizeof(test_colors)/sizeof(test_colors[0]))];
        ST7789_FillRect(0, 0, ST7789_WIDTH - 1, ST7789_HEIGHT - 1, color);
    }
    elapsed = HAL_GetTick() - tick_start;

    double fps = (elapsed > 0) ? ((double)frame_count * 1000.0) / (double)elapsed : 0.0;
    printf("ST7789 frame test: %lu frames in %lums (%.2f FPS)\r\n",
           (unsigned long)frame_count, (unsigned long)elapsed, fps);
}



/* 计算字符串像素宽度（不考虑换行） */
static uint16_t ST7789_TextPixelWidth(const char *s, const FontDef *font)
{
    if (!s || !font) return 0;
    uint32_t n = 0;
    while (*s++) n++;
    return (uint16_t)(n * font->width);
}




/* 像素级：在一行缓冲里合成当前帧，再一次性送出（极顺滑） */
static void ST7789_DrawVisibleText_SmoothRow(int16_t x, uint16_t y,
                                             const char *s,
                                             const FontDef *font,
                                             uint16_t color, uint16_t bgcolor)
{
    if (!s || !font) return;
    const int16_t fw = (int16_t)font->width;
    const int16_t fh = (int16_t)font->height;
    if (y >= ST7789_HEIGHT || (int32_t)y + fh - 1 > (int32_t)ST7789_HEIGHT - 1) return;

    /* 对每一行做合成，然后整行发出去 */
    for (int16_t row = 0; row < fh; row++) {

        /* 1) 先把整行清成背景色（高字节在前） */
        for (uint16_t xi = 0; xi < ST7789_WIDTH; xi++) {
            uint32_t idx = (uint32_t)xi * 2;
            ST7789_Buf[idx + 0] = (uint8_t)(bgcolor >> 8);
            ST7789_Buf[idx + 1] = (uint8_t)(bgcolor & 0xFF);
        }

        /* 2) 把这一行的文字逐“像素”覆写到行缓冲上 */
        int16_t cx = x;                 /* 本字符左上角 X（像素，可为负） */
        const char *p = s;
        while (*p) {
            /* 字形在屏幕上的 X 范围 */
            int16_t gx0 = cx;           /* 包含当前字符的首像素 */
            int16_t gx1 = cx + fw - 1;  /* 包含当前字符的末像素 */

            if (gx1 < 0) {              /* 这个字完全在屏幕左侧之外 */
                cx += fw;                /* 跳到下一个字 */
                p++;
                continue;
            }
            if (gx0 >= (int16_t)ST7789_WIDTH) {
                break;                   /* 右侧之外，后面都不用画了 */
            }

            /* 取该字符此行的位图（每行16bit，MSB是最左像素） */
            uint16_t bits = font->data[((uint32_t)((uint8_t)(*p) - 32) * fh) + row];

            /* 可见区域裁剪到屏幕 */
            int16_t vis0 = (gx0 < 0) ? 0 : gx0;
            int16_t vis1 = (gx1 >= (int16_t)ST7789_WIDTH) ? ((int16_t)ST7789_WIDTH - 1) : gx1;

            /* 把可见列逐像素画进行缓冲 */
            for (int16_t sx = vis0; sx <= vis1; sx++) {
                /* 对应到字形内部的列编号（0..fw-1） */
                int16_t col = sx - cx;              /* col 可能是 0..fw-1 的任意值 */
                /* 测这一位是否为 1（前景像素） */
                if (bits & (1U << (fw - 1 - col))) {
                    uint32_t idx = (uint32_t)sx * 2;
                    ST7789_Buf[idx + 0] = (uint8_t)(color >> 8);
                    ST7789_Buf[idx + 1] = (uint8_t)(color & 0xFF);
                }
            }

            cx += fw;
            p++;
        }

        /* 3) 发送这一行到 (0, y+row) .. (WIDTH-1, y+row) */
        ST7789_SetWindow(0, (uint16_t)(y + row), ST7789_WIDTH - 1, (uint16_t)(y + row));
        ST7789_WriteCmd(0x2C);
        ST7789_WriteDataDMA(ST7789_Buf, (uint32_t)ST7789_WIDTH * 2);
    }
}



/* 初始化（注意 font 传指针） */
void ST7789_MarqueeInit(ST7789_Marquee *m,
                        const char *text,
                        const FontDef *font,
                        uint16_t y,
                        uint16_t color,
                        uint16_t bgcolor,
                        int16_t speed_px,
                        uint16_t gap_px,
                        uint32_t interval_ms)
{
    if (!m) return;
    m->text   = text;
    m->font   = font;
    m->y      = y;
    m->color  = color;
    m->bgcolor= bgcolor;
    m->speed_px = (speed_px == 0) ? 1 : speed_px;           // 避免 0
    m->gap_px   = gap_px;
    m->interval_ms = (interval_ms == 0) ? 16 : interval_ms;
    m->last_ms  = 0;

    int16_t tw = (int16_t)ST7789_TextPixelWidth(text, font);
    // 左->右：从左侧外开始；右->左：从右侧外开始
    m->x = (m->speed_px > 0) ? (int16_t)(-tw) : (int16_t)(ST7789_WIDTH + gap_px);
}

/* 非阻塞步进：按时间刷新、只重绘文字带 */
void ST7789_MarqueeStep(ST7789_Marquee *m, uint32_t now_ms)
{
    if (!m || !m->text || !m->font) return;

    if (m->last_ms && (uint32_t)(now_ms - m->last_ms) < m->interval_ms) return;
    m->last_ms = now_ms;

    uint16_t band_y0 = m->y;
    uint16_t band_y1 = (uint16_t)(m->y + m->font->height - 1);
    if (band_y1 >= ST7789_HEIGHT) band_y1 = ST7789_HEIGHT - 1;

    /* 1) 擦除这一行带 */
    ST7789_FillRect(0, band_y0, ST7789_WIDTH - 1, band_y1, m->bgcolor);

    /* 2) 绘制当前帧 */
	  ST7789_DrawVisibleText_SmoothRow(m->x, m->y, m->text, m->font, m->color, m->bgcolor);

    /* 3) 更新位置 */
    m->x += m->speed_px;

    /* 4) 回绕逻辑 */
    int16_t tw = (int16_t)ST7789_TextPixelWidth(m->text, m->font);
    if (m->speed_px > 0) {
        if (m->x > (int16_t)ST7789_WIDTH) {
            m->x = (int16_t)(-tw - (int16_t)m->gap_px);
        }
    } else { // 右->左
        if ((m->x + tw) < 0) {
            m->x = (int16_t)(ST7789_WIDTH + m->gap_px);
        }
    }
}

/* 阻塞演示（方便快速验证） */
void ST7789_MarqueeRunBlocking(const char *text,
                               const FontDef *font,
                               uint16_t y,
                               uint16_t color,
                               uint16_t bgcolor,
                               int16_t speed_px,
                               uint16_t gap_px,
                               uint32_t interval_ms,
                               uint32_t duration_ms)
{
    ST7789_Marquee m;
    ST7789_MarqueeInit(&m, text, font, y, color, bgcolor, speed_px, gap_px, interval_ms);

    uint32_t t0 = HAL_GetTick();
    while ((uint32_t)(HAL_GetTick() - t0) < duration_ms) {
        ST7789_MarqueeStep(&m, HAL_GetTick());
        HAL_Delay(1);
    }
}













