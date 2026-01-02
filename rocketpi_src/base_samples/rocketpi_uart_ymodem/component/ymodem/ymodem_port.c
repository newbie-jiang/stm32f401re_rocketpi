#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ymodem.h"
#include "main.h"
#include "usart.h"

#define FLASH_STORAGE_START_ADDR 0x08040000U
#define FLASH_STORAGE_END_ADDR   0x08080000U

typedef struct {
    const uint8_t* base;
    uint32_t len;
    uint32_t pos;
} DummyReadHandle;

static const uint8_t g_dummy_payload[] =
"RocketPi UART/Flash YMODEM demo\r\n"
"--------------------------------\r\n"
"This file is generated inside firmware and streamed via the YMODEM sender.\r\n"
"You can verify the protocol end-to-end by checking that this sentence and the\r\n"
"mini log below are present on the PC after reception.\r\n"
"\r\n"
"Timestamp,Item,Value\r\n"
"00:00.0,Board,STM32F401\r\n"
"00:00.1,Build,rocketpi_uart_ymodem\r\n"
"00:00.2,Size,~1 KB\r\n"
"\r\n"
"Have fun testing!\r\n";
static const uint32_t g_dummy_payload_len = (uint32_t)(sizeof(g_dummy_payload) - 1U);

static void* dummy_open_read(const char* path){
    (void)path;
    DummyReadHandle* h = (DummyReadHandle*)malloc(sizeof(DummyReadHandle));
    if (!h) return NULL;
    h->base = g_dummy_payload;
    h->len  = g_dummy_payload_len;
    h->pos  = 0;
    return h;
}
static int dummy_read_bytes(void* fh, void* buf, int len){
    DummyReadHandle* h=(DummyReadHandle*)fh;
    if (!h || !buf || len<=0) return -1;
    uint32_t remain = (h->pos < h->len)? (h->len - h->pos) : 0;
    if (remain==0) return 0;
    if ((uint32_t)len > remain) len = (int)remain;
    memcpy(buf, h->base + h->pos, (size_t)len);
    h->pos += (uint32_t)len;
    return len;
}
static int dummy_seek_any(void* fh, int64_t off, int whence){
    DummyReadHandle* h=(DummyReadHandle*)fh;
    if (!h) return -1;
    int64_t target = 0;
    switch (whence){
        case 0: target = off; break;
        case 1: target = (int64_t)h->pos + off; break;
        case 2: target = (int64_t)h->len + off; break;
        default: return -1;
    }
    if (target < 0 || target > (int64_t)h->len) return -1;
    h->pos = (uint32_t)target;
    return 0;
}
static int64_t dummy_tell_any(void* fh){
    DummyReadHandle* h=(DummyReadHandle*)fh;
    if (!h) return -1;
    return (int64_t)h->pos;
}
static int64_t dummy_size_any(void* fh){
    DummyReadHandle* h=(DummyReadHandle*)fh;
    if (!h) return -1;
    return (int64_t)h->len;
}
static void dummy_close_read(void* fh){
    DummyReadHandle* h=(DummyReadHandle*)fh;
    if (h) free(h);
}

typedef struct {
    uint32_t start;
    uint32_t end;
    uint32_t cursor;
    uint8_t  erased;
    uint8_t  busy;
} FlashStoreState;

typedef struct {
    FlashStoreState* state;
    uint32_t start_addr;
    uint32_t wrote;
    uint32_t capacity;
    uint32_t expected;
} FlashWriteHandle;

static FlashStoreState g_flash_store = {
    .start = FLASH_STORAGE_START_ADDR,
    .end   = FLASH_STORAGE_END_ADDR,
    .cursor = FLASH_STORAGE_START_ADDR,
    .erased = 0,
    .busy = 0,
};

static void flash_store_session_reset(void){
    g_flash_store.cursor = g_flash_store.start;
    g_flash_store.erased = 0;
    g_flash_store.busy = 0;
    ymodem_store_set_file_size_hint(-1);
}

static uint32_t flash_store_free_bytes(void){
    if (g_flash_store.cursor >= g_flash_store.end) return 0;
    return g_flash_store.end - g_flash_store.cursor;
}

static uint32_t flash_store_expected_bytes(void){
    int64_t hint = ymodem_store_get_file_size_hint();
    if (hint <= 0) return 0;
    if (hint > (int64_t)UINT32_MAX) return UINT32_MAX;
    return (uint32_t)hint;
}

static void flash_clear_error_flags(void){
    uint32_t flags = 0;
#ifdef FLASH_FLAG_EOP
    flags |= FLASH_FLAG_EOP;
#endif
#ifdef FLASH_FLAG_OPERR
    flags |= FLASH_FLAG_OPERR;
#endif
#ifdef FLASH_FLAG_WRPERR
    flags |= FLASH_FLAG_WRPERR;
#endif
#ifdef FLASH_FLAG_PGAERR
    flags |= FLASH_FLAG_PGAERR;
#endif
#ifdef FLASH_FLAG_PGPERR
    flags |= FLASH_FLAG_PGPERR;
#endif
#ifdef FLASH_FLAG_PGSERR
    flags |= FLASH_FLAG_PGSERR;
#endif
    if (flags) {
        __HAL_FLASH_CLEAR_FLAG(flags);
    }
}

static int flash_store_erase_all(void){
    HAL_StatusTypeDef st;
    uint32_t err = 0;

    if (HAL_FLASH_Unlock() != HAL_OK) {
        return -1;
    }
    flash_clear_error_flags();

    FLASH_EraseInitTypeDef erase = {0};
    erase.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    erase.Sector = FLASH_SECTOR_6;
    erase.NbSectors = 2;
    st = HAL_FLASHEx_Erase(&erase, &err);

    HAL_FLASH_Lock();

    if (st != HAL_OK || err != 0xFFFFFFFFU) {
        printf("[FLASH] erase failed, status=%ld err=%lu\r\n", (long)st, (unsigned long)err);
        return -1;
    }
    return 0;
}

static void* flash_open_write(const char* out_dir, const char* name){
    (void)out_dir; (void)name;
    if (g_flash_store.busy) return NULL;
    if (!g_flash_store.erased){
        if (flash_store_erase_all() != 0) return NULL;
        g_flash_store.erased = 1;
    }
    uint32_t avail = flash_store_free_bytes();
    if (avail == 0) return NULL;

    FlashWriteHandle* h = (FlashWriteHandle*)malloc(sizeof(FlashWriteHandle));
    if (!h) return NULL;
    h->state = &g_flash_store;
    h->start_addr = g_flash_store.cursor;
    h->wrote = 0;
    h->capacity = avail;
    h->expected = flash_store_expected_bytes();
    if (h->expected > 0 && h->expected > h->capacity){
        free(h);
        return NULL;
    }
    g_flash_store.busy = 1;
    return h;
}

static int flash_write_bytes(void* fh, const void* buf, int len){
    FlashWriteHandle* h=(FlashWriteHandle*)fh;
    if (!h || !buf || len<=0) return 0;
    if ((uint32_t)len > (h->capacity - h->wrote)){
        len = (int)(h->capacity - h->wrote);
    }
    if (len<=0) return 0;

    const uint8_t* src=(const uint8_t*)buf;
    if (HAL_FLASH_Unlock() != HAL_OK) return -1;

    for (int i=0;i<len;++i){
        uint32_t addr = h->start_addr + h->wrote;
        HAL_StatusTypeDef st = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, addr, (uint64_t)src[i]);
        if (st != HAL_OK){
            HAL_FLASH_Lock();
            return -1;
        }
        h->wrote++;
    }

    HAL_FLASH_Lock();
    return len;
}

static void flash_close_write(void* fh, int ok){
    FlashWriteHandle* h=(FlashWriteHandle*)fh;
    if (!h) return;
    if (ok){
        h->state->cursor = h->start_addr + h->wrote;
    }
    h->state->busy = 0;
    free(h);
    ymodem_store_set_file_size_hint(-1);
}

static uint32_t stm32_now_ms(void){ return HAL_GetTick(); }
static void     stm32_sleep_ms(int ms){ HAL_Delay(ms); }

typedef struct {
    UART_HandleTypeDef* huart;
} STM32Serial;

static int stm32_open(YPort* p, const char* name, uint32_t baud){
    (void)name; (void)baud;
    STM32Serial* s = (STM32Serial*)p->self;
    return (s && s->huart) ? 0 : -1;
}
static void stm32_close(YPort* p){ (void)p; }

static int stm32_putc(YPort* p, uint8_t ch){
    STM32Serial* s=(STM32Serial*)p->self;
    if (!s || !s->huart) return -1;
    if (HAL_UART_Transmit(s->huart, &ch, 1, 1000) == HAL_OK) return 1;
    return -1;
}
static int stm32_getc(YPort* p, uint8_t* ch, int ms){
    STM32Serial* s=(STM32Serial*)p->self;
    if (!s || !s->huart) return -1;
    if (HAL_UART_Receive(s->huart, ch, 1, (ms>=0)? (uint32_t)ms : HAL_MAX_DELAY) == HAL_OK) return 1;
    return 0;
}
static int stm32_write(YPort* p, const void* buf, int len){
    STM32Serial* s=(STM32Serial*)p->self;
    if (!s || !s->huart) return -1;
    if (HAL_UART_Transmit(s->huart, (uint8_t*)buf, (uint16_t)len, 5000) == HAL_OK) return len;
    return -1;
}
static int stm32_read_exact(YPort* p, void* buf, int len, int ms){
    STM32Serial* s=(STM32Serial*)p->self;
    if (!s || !s->huart) return -1;
    uint32_t start = HAL_GetTick();
    uint8_t* q=(uint8_t*)buf; int got=0;
    while (got < len){
        uint32_t elapsed = HAL_GetTick() - start;
        uint32_t budget = (ms>0 && ms > (int)elapsed) ? (ms - elapsed) : 0;
        if (budget==0) break;
        if (HAL_UART_Receive(s->huart, q+got, 1, budget) == HAL_OK){
            got++;
        }
    }
    return (got==len)? len : -1;
}

static void cli_log(const char* s){
    printf("%s\r\n", s);
}
static void cli_prog(const char* tag, const char* name, uint64_t done, int64_t total){
    (void)tag; (void)name; (void)done; (void)total;
}

extern UART_HandleTypeDef huart2;

static void ymodem_make_ctx(YContext* out_ctx, UART_HandleTypeDef* huart){
    static STM32Serial ss;
    static YPort  port;
    static YTimer timer;
    static YStore store;

    ss.huart = huart;

    port.self = &ss;
    port.open = stm32_open;
    port.close= stm32_close;
    port.putc = stm32_putc;
    port.getc = stm32_getc;
    port.write= stm32_write;
    port.read_exact = stm32_read_exact;

    timer.self = NULL;
    timer.now_ms = stm32_now_ms;
    timer.sleep_ms = stm32_sleep_ms;

    store.self = NULL;
    store.open_read  = dummy_open_read;
    store.read       = dummy_read_bytes;
    store.seek       = dummy_seek_any;
    store.tell       = dummy_tell_any;
    store.size       = dummy_size_any;
    store.close_read = dummy_close_read;

    store.open_write = flash_open_write;
    store.write      = flash_write_bytes;
    store.close_write= flash_close_write;

    out_ctx->port  = &port;
    out_ctx->timer = &timer;
    out_ctx->store = &store;
    out_ctx->hooks.on_log = cli_log;
    out_ctx->hooks.on_progress = cli_prog;
    out_ctx->cfg.rx_timeout_ms    = 3000;
    out_ctx->cfg.hs_total_ms      = 20000;
    out_ctx->cfg.retry_max        = 10;
    out_ctx->cfg.packet_prefer_1k = 1;
}

void ymodem_recv(const char* out_dir){
    (void)out_dir;
    flash_store_session_reset();
    YContext ctx;
    ymodem_make_ctx(&ctx, &huart2);
    int rc = ymd_recv_multi(&ctx, "");
    printf("[YMODEM] recv rc=%d\r\n", rc);
}

int ymodem_send(const char* const* files, int n){
    YContext ctx;
    ymodem_make_ctx(&ctx, &huart2);
    if (!files || n<=0) return -1;
    return ymd_send_multi(&ctx, files, n);
}
