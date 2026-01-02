#include "st7789.h"
#include <string.h>
#include <stdio.h>

#define ST7789_X_RES   (ST7789_LCD_WIDTH)
#define ST7789_Y_RES   (ST7789_LCD_HEIGHT)

#ifndef ST7789_SPI
#warning "Please define ST7789_SPI (SPI_HandleTypeDef*)"
#endif

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

/* 数据写入（小块，阻塞式） */
static inline void ST7789_WriteData(const uint8_t *data, size_t len) {
    if (len == 0) return;
    ST7789_CS_LOW();
    ST7789_DC_HIGH();
    st7789_tx_blocking(data, len);
    ST7789_CS_HIGH();
}

/* 数据写入（大块，DMA + 轮询；需更高效可改用回调或双缓冲） */
static inline void ST7789_WriteDataDMA(const uint8_t *data, size_t len) {
    if (len == 0) return;
    ST7789_CS_LOW();
    ST7789_DC_HIGH();
    HAL_SPI_Transmit_DMA(ST7789_SPI, (uint8_t*)data, len);
    while (HAL_SPI_GetState(ST7789_SPI) == HAL_SPI_STATE_BUSY_TX) { /* 轮询等待 */ }
    ST7789_CS_HIGH();
}

/* 设置窗口：一次性下发 CASET/RASET/RAMWR，仅应用坐标偏移 */
static void ST7789_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    uint16_t xs = x0;
    uint16_t xe = x1;
    uint16_t ys = y0;
    uint16_t ye = y1;

    xs += X_SHIFT;
    xe += X_SHIFT;
    ys += Y_SHIFT;
    ye += Y_SHIFT;

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
    /* 背光优先点亮（可按需调整） */
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
    ST7789_WriteCmd(0xC0); { uint8_t d=0x2C; ST7789_WriteData(&d,1); } /* 可调 */

    /* VDV/VRH */
    ST7789_WriteCmd(0xC2); { uint8_t d=0x01; ST7789_WriteData(&d,1); }
    ST7789_WriteCmd(0xC3); { uint8_t d=0x19; ST7789_WriteData(&d,1); } /* GVDD=4.8V */
    ST7789_WriteCmd(0xC4); { uint8_t d=0x20; ST7789_WriteData(&d,1); } /* VDV, 0x20:0V */

    /* Frame rate */
    ST7789_WriteCmd(0xC6); { uint8_t d=0x0F; ST7789_WriteData(&d,1); } /* 60Hz */

    /* Power control */
    ST7789_WriteCmd(0xD0); { uint8_t d[2]={0xA4,0xA1}; ST7789_WriteData(d,2); }

    /* 正伽玛 */
    ST7789_WriteCmd(0xE0); {
        uint8_t d[] = {0xD0,0x08,0x0E,0x09,0x09,0x05,0x31,0x33,0x48,0x17,0x14,0x15,0x31,0x34};
        ST7789_WriteData(d,sizeof(d));
    }
    /* 负伽玛 */
    ST7789_WriteCmd(0xE1); {
        uint8_t d[] = {0xD0,0x08,0x0E,0x09,0x09,0x15,0x31,0x33,0x48,0x17,0x14,0x15,0x31,0x34};
        ST7789_WriteData(d,sizeof(d));
    }

		
    /* 反色 */
    ST7789_WriteCmd(0x21); /* Normal display */

    /* MADCTL：方向 + RGB/BGR */
    {
        uint8_t madctl = 0x00;
    #if (ST7789_BGR == 1)
        madctl |= 0x08; /* BGR 模式 */
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
    uint32_t total_bytes = (uint32_t)ST7789_X_RES * ST7789_Y_RES * 2;
    /* 预填充缓冲 */
    for (uint32_t i = 0; i < ST7789_BUF_SIZE; i += 2) {
        ST7789_Buf[i]   = color >> 8;
        ST7789_Buf[i+1] = color & 0xFF;
    }

    ST7789_SetWindow(0, 0, ST7789_X_RES - 1, ST7789_Y_RES - 1);

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
    if (x >= ST7789_X_RES || y >= ST7789_Y_RES) return;
    ST7789_SetWindow(x, y, x, y);
    uint8_t px[2] = { color >> 8, color & 0xFF };
    ST7789_WriteCmd(0x2C);
    ST7789_WriteData(px, 2);
}

/* 横线（一次性打包） */
void ST7789_DrawHLine(uint16_t xs, uint16_t xe, uint16_t y, uint16_t color)
{
    if (y >= ST7789_Y_RES) return;
    if (xe < xs) { uint16_t t = xs; xs = xe; xe = t; }
    if (xs >= ST7789_X_RES) return;
    if (xe >= ST7789_X_RES) xe = ST7789_X_RES - 1;

    uint16_t w = xe - xs + 1;
    uint32_t bytes = (uint32_t)w * 2;

    /* 准备一行颜色数据 */
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
    if (x >= ST7789_X_RES) return;
    if (ye < ys) { uint16_t t = ys; ys = ye; ye = t; }
    if (ys >= ST7789_Y_RES) return;
    if (ye >= ST7789_Y_RES) ye = ST7789_Y_RES - 1;

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
    if (xs >= ST7789_X_RES || ys >= ST7789_Y_RES) return;
    if (xe >= ST7789_X_RES)  xe = ST7789_X_RES - 1;
    if (ye >= ST7789_Y_RES) ye = ST7789_Y_RES - 1;

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
 * xsize 范围 [1, ST7789_X_RES]
 */
void ST7789_DrawBitLine16BPP(uint16_t xs, uint16_t y, const uint8_t *p, uint16_t xsize)
{
    if (y >= ST7789_Y_RES || xs >= ST7789_X_RES) return;
    if (xsize == 0) return;
    if (xs + xsize > ST7789_X_RES) xsize = ST7789_X_RES - xs;

    uint32_t bytes = (uint32_t)xsize * 2;

    /* 小端 -> 转换成高字节在前（面板需要高字节先出） */
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

/* 画任意矩形位图 */
void ST7789_DrawBitmap(uint16_t xs, uint16_t ys, uint16_t xsize, uint16_t ysize, const uint8_t *p)
{
    if (xs >= ST7789_X_RES || ys >= ST7789_Y_RES) return;
    if (xsize == 0 || ysize == 0) return;

    uint16_t xmax = (xs + xsize > ST7789_X_RES)  ? (ST7789_X_RES  - xs) : xsize;
    uint16_t ymax = (ys + ysize > ST7789_Y_RES) ? (ST7789_Y_RES - ys) : ysize;

    for (uint16_t i=0; i<ymax; i++) {
        ST7789_DrawBitLine16BPP(xs, ys + i, p + (uint32_t)i * xsize * 2, xmax);
    }
}










