#ifndef __YMODEM_H
#define __YMODEM_H

#include <stdint.h>

/* -------------------------------
 *           Protocol defs
 * ------------------------------- */
#define YMD_SOH   0x01
#define YMD_STX   0x02
#define YMD_EOT   0x04
#define YMD_ACK   0x06
#define YMD_NAK   0x15
#define YMD_CAN   0x18
#define YMD_CHC   0x43  /* 'C' */

#define YMD_BLK128 128
#define YMD_BLK1K  1024

/* -------------------------------
 *       OOP-like Port Abstraction
 * ------------------------------- */
typedef struct YPort {
    void*  self;
    int  (*open)(struct YPort*, const char* name, uint32_t baud);
    void (*close)(struct YPort*);
    int  (*putc)(struct YPort*, uint8_t ch);
    int  (*getc)(struct YPort*, uint8_t* ch, int ms);
    int  (*write)(struct YPort*, const void* buf, int len);
    int  (*read_exact)(struct YPort*, void* buf, int len, int ms);
} YPort;

typedef struct YTimer {
    void* self;
    uint32_t (*now_ms)(void);
    void     (*sleep_ms)(int ms);
} YTimer;

typedef struct YStore {
    void* self;
    /* sender side */
    void*   (*open_read)(const char* path);
    int     (*read)(void* fh, void* buf, int len);
    int     (*seek)(void* fh, int64_t off, int whence);   /* optional */
    int64_t (*tell)(void* fh);                            /* optional */
    int64_t (*size)(void* fh);                            /* optional */
    void    (*close_read)(void* fh);
    /* receiver side */
    void*   (*open_write)(const char* out_dir, const char* name);
    int     (*write)(void* fh, const void* buf, int len);
    void    (*close_write)(void* fh, int ok);
} YStore;

/* 回调 */
typedef struct YHooks {
    void (*on_log)(const char* s);
    void (*on_progress)(const char* tag, const char* name, uint64_t done, int64_t total);
} YHooks;

/* 配置 */
typedef struct YConfig {
    int rx_timeout_ms;      /* 单步读超时，默认 3000 */
    int hs_total_ms;        /* 握手总时长，默认 20000 */
    int retry_max;          /* 数据块重试次数，默认 10 */
    int packet_prefer_1k;   /* 1=1K 包，0=128B 包 */
} YConfig;

/* 上下文 */
typedef struct YContext {
    YPort*  port;
    YTimer* timer;
    YStore* store;
    YHooks  hooks;
    YConfig cfg;
} YContext;

void ymodem_recv(const char* out_dir);
int ymodem_send(const char* const* files, int n);
int ymd_recv_multi(YContext* ctx, const char* out_dir);
int ymd_send_multi(YContext* ctx, const char* const* files, int nfiles);

void ymodem_store_set_file_size_hint(int64_t sz);
int64_t ymodem_store_get_file_size_hint(void);

#endif /* __YMODEM_H */
