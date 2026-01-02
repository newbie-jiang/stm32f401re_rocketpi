#include "app.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"
#include "dma.h"
#include "driver_esp8266_at.h"
#include "gpio.h"
#include "i2c.h"
#include "main.h"
#include "spi.h"
#include "stm32f4xx_hal.h"
#include "tim.h"
#include "usart.h"

#ifndef APP_WIFI_SSID
#define APP_WIFI_SSID "ZTE-45c476"
#endif
#ifndef APP_WIFI_PASSWORD
#define APP_WIFI_PASSWORD "88888888"
#endif

#ifndef APP_MQTT_BROKER
#define APP_MQTT_BROKER "broker.emqx.io"
#endif
#ifndef APP_MQTT_PORT
#define APP_MQTT_PORT 1883
#endif
#ifndef APP_MQTT_CLIENT_ID
#define APP_MQTT_CLIENT_ID ""
#endif
#ifndef APP_MQTT_CLIENT_ID_PREFIX
#define APP_MQTT_CLIENT_ID_PREFIX "rocketpi-ota-"
#endif
#ifndef APP_MQTT_CLIENT_ID_MAX_LEN
#define APP_MQTT_CLIENT_ID_MAX_LEN 48U
#endif
#ifndef APP_MQTT_USERNAME
#define APP_MQTT_USERNAME ""
#endif
#ifndef APP_MQTT_PASSWORD
#define APP_MQTT_PASSWORD ""
#endif
#ifndef APP_MQTT_BASE_TOPIC
#define APP_MQTT_BASE_TOPIC "rocketpi"
#endif
#ifndef APP_DEVICE_ID_LEN
#define APP_DEVICE_ID_LEN 12U
#endif
#ifndef APP_DEVICE_ID
#define APP_DEVICE_ID "AUTO"
#endif
#ifndef APP_DEVICE_MODEL
#define APP_DEVICE_MODEL "demo-mcu"
#endif
#ifndef APP_DEVICE_VERSION
#define APP_DEVICE_VERSION "1.0.0"
#endif

#ifndef APP_MQTT_QOS
#define APP_MQTT_QOS 0U
#endif
#ifndef APP_MQTT_RECONNECT_MS
#define APP_MQTT_RECONNECT_MS 5000U
#endif

#ifndef OTA_CHUNK_SIZE
#define OTA_CHUNK_SIZE 1024U
#endif

/* STM32F4: sectors 6-7 (0x08040000-0x0807FFFF), total 256 KB. */
#ifndef OTA_FLASH_BASE
#define OTA_FLASH_BASE 0x08040000UL
#endif
#ifndef OTA_FLASH_SIZE
#define OTA_FLASH_SIZE 0x00040000UL
#endif
#ifndef OTA_FLASH_SECTOR_FIRST
#define OTA_FLASH_SECTOR_FIRST FLASH_SECTOR_6
#endif
#ifndef OTA_FLASH_SECTOR_LAST
#define OTA_FLASH_SECTOR_LAST FLASH_SECTOR_7
#endif

#define OTA_MAX_VERSION_LEN 16U
#define OTA_SHA256_LEN      64U
#define OTA_JSON_BUFFER_SIZE 2048U

typedef void (*app_entry_t)(void);

#ifndef APP_START_ADDRESS
#define APP_START_ADDRESS OTA_FLASH_BASE
#endif
#ifndef APP_END_ADDRESS
#define APP_END_ADDRESS (OTA_FLASH_BASE + OTA_FLASH_SIZE)
#endif
#ifndef SRAM_START_ADDRESS
#define SRAM_START_ADDRESS 0x20000000UL
#endif
#ifndef SRAM_SIZE_BYTES
#define SRAM_SIZE_BYTES (96UL * 1024UL)
#endif
#ifndef SRAM_END_ADDRESS
#define SRAM_END_ADDRESS (SRAM_START_ADDRESS + SRAM_SIZE_BYTES)
#endif

typedef struct
{
    bool     wifi_ready;
    bool     mqtt_ready;
    bool     mqtt_reconnect_pending;
    uint32_t last_mqtt_attempt_ms;
    bool     check_sent;
    bool     ota_active;
    bool     flash_unlocked;
    uint32_t total_size;
    uint32_t chunk_size;
    uint32_t total_chunks;
    uint32_t expected_index;
    uint32_t bytes_written;
    uint32_t last_progress;
    char     device_id[APP_DEVICE_ID_LEN + 1U];
    char     mqtt_client_id[APP_MQTT_CLIENT_ID_MAX_LEN];
    char     offer_version[OTA_MAX_VERSION_LEN];
    char     offer_sha256[OTA_SHA256_LEN + 1U];
} app_state_t;

static app_state_t s_app;
static char s_chunk_json_buffer[OTA_JSON_BUFFER_SIZE];
static size_t s_chunk_json_len = 0U;
static bool s_chunk_json_active = false;

static bool app_has_custom_device_id(void)
{
    return (APP_DEVICE_ID[0] != '\0'
            && strcmp(APP_DEVICE_ID, "AUTO") != 0
            && strcmp(APP_DEVICE_ID, "auto") != 0
            && strcmp(APP_DEVICE_ID, "YOUR_DEVICE_ID") != 0);
}

static bool app_has_custom_client_id(void)
{
    return (APP_MQTT_CLIENT_ID[0] != '\0'
            && strcmp(APP_MQTT_CLIENT_ID, "AUTO") != 0
            && strcmp(APP_MQTT_CLIENT_ID, "auto") != 0
            && strcmp(APP_MQTT_CLIENT_ID, "YOUR_MQTT_CLIENT_ID") != 0);
}

static void app_make_device_id(char *out, size_t out_size)
{
    if (out == NULL || out_size < (APP_DEVICE_ID_LEN + 1U))
    {
        return;
    }

    const uint8_t *uid = (const uint8_t *)UID_BASE;
    uint64_t hash = 1469598103934665603ULL;
    for (size_t i = 0U; i < 12U; ++i)
    {
        hash ^= (uint64_t)uid[i];
        hash *= 1099511628211ULL;
    }

    for (size_t i = 0U; i < APP_DEVICE_ID_LEN; ++i)
    {
        out[APP_DEVICE_ID_LEN - 1U - i] = (char)('a' + (hash % 26U));
        hash /= 26U;
    }
    out[APP_DEVICE_ID_LEN] = '\0';
}

static void app_build_mqtt_identifiers(void)
{
    if (app_has_custom_device_id())
    {
        strncpy(s_app.device_id, APP_DEVICE_ID, sizeof(s_app.device_id) - 1U);
        s_app.device_id[sizeof(s_app.device_id) - 1U] = '\0';
    }
    else
    {
        app_make_device_id(s_app.device_id, sizeof(s_app.device_id));
    }

    if (s_app.device_id[0] == '\0')
    {
        strncpy(s_app.device_id, "unknown", sizeof(s_app.device_id) - 1U);
        s_app.device_id[sizeof(s_app.device_id) - 1U] = '\0';
    }

    if (app_has_custom_client_id())
    {
        strncpy(s_app.mqtt_client_id, APP_MQTT_CLIENT_ID, sizeof(s_app.mqtt_client_id) - 1U);
        s_app.mqtt_client_id[sizeof(s_app.mqtt_client_id) - 1U] = '\0';
    }
    else
    {
        (void)snprintf(s_app.mqtt_client_id,
                       sizeof(s_app.mqtt_client_id),
                       "%s%s",
                       APP_MQTT_CLIENT_ID_PREFIX,
                       s_app.device_id);
    }
}

static const uint32_t s_crc_table[256] = {
    0x00000000UL, 0x77073096UL, 0xEE0E612CUL, 0x990951BAUL, 0x076DC419UL, 0x706AF48FUL, 0xE963A535UL, 0x9E6495A3UL,
    0x0EDB8832UL, 0x79DCB8A4UL, 0xE0D5E91EUL, 0x97D2D988UL, 0x09B64C2BUL, 0x7EB17CBDUL, 0xE7B82D07UL, 0x90BF1D91UL,
    0x1DB71064UL, 0x6AB020F2UL, 0xF3B97148UL, 0x84BE41DEUL, 0x1ADAD47DUL, 0x6DDDE4EBUL, 0xF4D4B551UL, 0x83D385C7UL,
    0x136C9856UL, 0x646BA8C0UL, 0xFD62F97AUL, 0x8A65C9ECUL, 0x14015C4FUL, 0x63066CD9UL, 0xFA0F3D63UL, 0x8D080DF5UL,
    0x3B6E20C8UL, 0x4C69105EUL, 0xD56041E4UL, 0xA2677172UL, 0x3C03E4D1UL, 0x4B04D447UL, 0xD20D85FDUL, 0xA50AB56BUL,
    0x35B5A8FAUL, 0x42B2986CUL, 0xDBBBC9D6UL, 0xACBCF940UL, 0x32D86CE3UL, 0x45DF5C75UL, 0xDCD60DCFUL, 0xABD13D59UL,
    0x26D930ACUL, 0x51DE003AUL, 0xC8D75180UL, 0xBFD06116UL, 0x21B4F4B5UL, 0x56B3C423UL, 0xCFBA9599UL, 0xB8BDA50FUL,
    0x2802B89EUL, 0x5F058808UL, 0xC60CD9B2UL, 0xB10BE924UL, 0x2F6F7C87UL, 0x58684C11UL, 0xC1611DABUL, 0xB6662D3DUL,
    0x76DC4190UL, 0x01DB7106UL, 0x98D220BCUL, 0xEFD5102AUL, 0x71B18589UL, 0x06B6B51FUL, 0x9FBFE4A5UL, 0xE8B8D433UL,
    0x7807C9A2UL, 0x0F00F934UL, 0x9609A88EUL, 0xE10E9818UL, 0x7F6A0DBBUL, 0x086D3D2DUL, 0x91646C97UL, 0xE6635C01UL,
    0x6B6B51F4UL, 0x1C6C6162UL, 0x856530D8UL, 0xF262004EUL, 0x6C0695EDUL, 0x1B01A57BUL, 0x8208F4C1UL, 0xF50FC457UL,
    0x65B0D9C6UL, 0x12B7E950UL, 0x8BBEB8EAUL, 0xFCB9887CUL, 0x62DD1DDFUL, 0x15DA2D49UL, 0x8CD37CF3UL, 0xFBD44C65UL,
    0x4DB26158UL, 0x3AB551CEUL, 0xA3BC0074UL, 0xD4BB30E2UL, 0x4ADFA541UL, 0x3DD895D7UL, 0xA4D1C46DUL, 0xD3D6F4FBUL,
    0x4369E96AUL, 0x346ED9FCUL, 0xAD678846UL, 0xDA60B8D0UL, 0x44042D73UL, 0x33031DE5UL, 0xAA0A4C5FUL, 0xDD0D7CC9UL,
    0x5005713CUL, 0x270241AAUL, 0xBE0B1010UL, 0xC90C2086UL, 0x5768B525UL, 0x206F85B3UL, 0xB966D409UL, 0xCE61E49FUL,
    0x5EDEF90EUL, 0x29D9C998UL, 0xB0D09822UL, 0xC7D7A8B4UL, 0x59B33D17UL, 0x2EB40D81UL, 0xB7BD5C3BUL, 0xC0BA6CADUL,
    0xEDB88320UL, 0x9ABFB3B6UL, 0x03B6E20CUL, 0x74B1D29AUL, 0xEAD54739UL, 0x9DD277AFUL, 0x04DB2615UL, 0x73DC1683UL,
    0xE3630B12UL, 0x94643B84UL, 0x0D6D6A3EUL, 0x7A6A5AA8UL, 0xE40ECF0BUL, 0x9309FF9DUL, 0x0A00AE27UL, 0x7D079EB1UL,
    0xF00F9344UL, 0x8708A3D2UL, 0x1E01F268UL, 0x6906C2FEUL, 0xF762575DUL, 0x806567CBUL, 0x196C3671UL, 0x6E6B06E7UL,
    0xFED41B76UL, 0x89D32BE0UL, 0x10DA7A5AUL, 0x67DD4ACCUL, 0xF9B9DF6FUL, 0x8EBEEFF9UL, 0x17B7BE43UL, 0x60B08ED5UL,
    0xD6D6A3E8UL, 0xA1D1937EUL, 0x38D8C2C4UL, 0x4FDFF252UL, 0xD1BB67F1UL, 0xA6BC5767UL, 0x3FB506DDUL, 0x48B2364BUL,
    0xD80D2BDAUL, 0xAF0A1B4CUL, 0x36034AF6UL, 0x41047A60UL, 0xDF60EFC3UL, 0xA867DF55UL, 0x316E8EEFUL, 0x4669BE79UL,
    0xCB61B38CUL, 0xBC66831AUL, 0x256FD2A0UL, 0x5268E236UL, 0xCC0C7795UL, 0xBB0B4703UL, 0x220216B9UL, 0x5505262FUL,
    0xC5BA3BBEUL, 0xB2BD0B28UL, 0x2BB45A92UL, 0x5CB36A04UL, 0xC2D7FFA7UL, 0xB5D0CF31UL, 0x2CD99E8BUL, 0x5BDEAE1DUL,
    0x9B64C2B0UL, 0xEC63F226UL, 0x756AA39CUL, 0x026D930AUL, 0x9C0906A9UL, 0xEB0E363FUL, 0x72076785UL, 0x05005713UL,
    0x95BF4A82UL, 0xE2B87A14UL, 0x7BB12BAEUL, 0x0CB61B38UL, 0x92D28E9BUL, 0xE5D5BE0DUL, 0x7CDCEFB7UL, 0x0BDBDF21UL,
    0x86D3D2D4UL, 0xF1D4E242UL, 0x68DDB3F8UL, 0x1FDA836EUL, 0x81BE16CDUL, 0xF6B9265BUL, 0x6FB077E1UL, 0x18B74777UL,
    0x88085AE6UL, 0xFF0F6A70UL, 0x66063BCAUL, 0x11010B5CUL, 0x8F659EFFUL, 0xF862AE69UL, 0x616BFFD3UL, 0x166CCF45UL,
    0xA00AE278UL, 0xD70DD2EEUL, 0x4E048354UL, 0x3903B3C2UL, 0xA7672661UL, 0xD06016F7UL, 0x4969474DUL, 0x3E6E77DBUL,
    0xAED16A4AUL, 0xD9D65ADCUL, 0x40DF0B66UL, 0x37D83BF0UL, 0xA9BCAE53UL, 0xDEBB9EC5UL, 0x47B2CF7FUL, 0x30B5FFE9UL,
    0xBDBDF21CUL, 0xCABAC28AUL, 0x53B39330UL, 0x24B4A3A6UL, 0xBAD03605UL, 0xCDD70693UL, 0x54DE5729UL, 0x23D967BFUL,
    0xB3667A2EUL, 0xC4614AB8UL, 0x5D681B02UL, 0x2A6F2B94UL, 0xB40BBE37UL, 0xC30C8EA1UL, 0x5A05DF1BUL, 0x2D02EF8DUL
};

static uint32_t app_crc32(const uint8_t *data, size_t length)
{
    uint32_t crc = 0xFFFFFFFFUL;
    for (size_t i = 0U; i < length; ++i)
    {
        const uint8_t index = (uint8_t)((crc ^ data[i]) & 0xFFU);
        crc = (crc >> 8U) ^ s_crc_table[index];
    }
    return crc ^ 0xFFFFFFFFUL;
}

typedef struct
{
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t  data[64];
    size_t   datalen;
} app_sha256_ctx_t;

static const uint32_t s_sha256_k[64] = {
    0x428A2F98UL, 0x71374491UL, 0xB5C0FBCFUL, 0xE9B5DBA5UL, 0x3956C25BUL, 0x59F111F1UL, 0x923F82A4UL, 0xAB1C5ED5UL,
    0xD807AA98UL, 0x12835B01UL, 0x243185BEUL, 0x550C7DC3UL, 0x72BE5D74UL, 0x80DEB1FEUL, 0x9BDC06A7UL, 0xC19BF174UL,
    0xE49B69C1UL, 0xEFBE4786UL, 0x0FC19DC6UL, 0x240CA1CCUL, 0x2DE92C6FUL, 0x4A7484AAUL, 0x5CB0A9DCUL, 0x76F988DAUL,
    0x983E5152UL, 0xA831C66DUL, 0xB00327C8UL, 0xBF597FC7UL, 0xC6E00BF3UL, 0xD5A79147UL, 0x06CA6351UL, 0x14292967UL,
    0x27B70A85UL, 0x2E1B2138UL, 0x4D2C6DFCUL, 0x53380D13UL, 0x650A7354UL, 0x766A0ABBUL, 0x81C2C92EUL, 0x92722C85UL,
    0xA2BFE8A1UL, 0xA81A664BUL, 0xC24B8B70UL, 0xC76C51A3UL, 0xD192E819UL, 0xD6990624UL, 0xF40E3585UL, 0x106AA070UL,
    0x19A4C116UL, 0x1E376C08UL, 0x2748774CUL, 0x34B0BCB5UL, 0x391C0CB3UL, 0x4ED8AA4AUL, 0x5B9CCA4FUL, 0x682E6FF3UL,
    0x748F82EEUL, 0x78A5636FUL, 0x84C87814UL, 0x8CC70208UL, 0x90BEFFFAUL, 0xA4506CEBUL, 0xBEF9A3F7UL, 0xC67178F2UL
};

static uint32_t app_rotr32(uint32_t value, uint32_t shift)
{
    return (value >> shift) | (value << (32U - shift));
}

static void app_sha256_transform(app_sha256_ctx_t *ctx, const uint8_t data[64])
{
    uint32_t m[64];
    for (size_t i = 0U; i < 16U; ++i)
    {
        const size_t j = i * 4U;
        m[i] = ((uint32_t)data[j] << 24U)
             | ((uint32_t)data[j + 1U] << 16U)
             | ((uint32_t)data[j + 2U] << 8U)
             | ((uint32_t)data[j + 3U]);
    }
    for (size_t i = 16U; i < 64U; ++i)
    {
        const uint32_t s0 = app_rotr32(m[i - 15U], 7U) ^ app_rotr32(m[i - 15U], 18U) ^ (m[i - 15U] >> 3U);
        const uint32_t s1 = app_rotr32(m[i - 2U], 17U) ^ app_rotr32(m[i - 2U], 19U) ^ (m[i - 2U] >> 10U);
        m[i] = m[i - 16U] + s0 + m[i - 7U] + s1;
    }

    uint32_t a = ctx->state[0];
    uint32_t b = ctx->state[1];
    uint32_t c = ctx->state[2];
    uint32_t d = ctx->state[3];
    uint32_t e = ctx->state[4];
    uint32_t f = ctx->state[5];
    uint32_t g = ctx->state[6];
    uint32_t h = ctx->state[7];

    for (size_t i = 0U; i < 64U; ++i)
    {
        const uint32_t ch = (e & f) ^ ((~e) & g);
        const uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        const uint32_t s1 = app_rotr32(e, 6U) ^ app_rotr32(e, 11U) ^ app_rotr32(e, 25U);
        const uint32_t s0 = app_rotr32(a, 2U) ^ app_rotr32(a, 13U) ^ app_rotr32(a, 22U);
        const uint32_t t1 = h + s1 + ch + s_sha256_k[i] + m[i];
        const uint32_t t2 = s0 + maj;

        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

static void app_sha256_init(app_sha256_ctx_t *ctx)
{
    ctx->datalen = 0U;
    ctx->bitlen = 0U;
    ctx->state[0] = 0x6A09E667UL;
    ctx->state[1] = 0xBB67AE85UL;
    ctx->state[2] = 0x3C6EF372UL;
    ctx->state[3] = 0xA54FF53AUL;
    ctx->state[4] = 0x510E527FUL;
    ctx->state[5] = 0x9B05688CUL;
    ctx->state[6] = 0x1F83D9ABUL;
    ctx->state[7] = 0x5BE0CD19UL;
}

static void app_sha256_update(app_sha256_ctx_t *ctx, const uint8_t *data, size_t len)
{
    for (size_t i = 0U; i < len; ++i)
    {
        ctx->data[ctx->datalen++] = data[i];
        if (ctx->datalen == 64U)
        {
            app_sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512U;
            ctx->datalen = 0U;
        }
    }
}

static void app_sha256_final(app_sha256_ctx_t *ctx, uint8_t hash[32])
{
    size_t i = ctx->datalen;

    ctx->data[i++] = 0x80U;
    if (i > 56U)
    {
        while (i < 64U)
        {
            ctx->data[i++] = 0U;
        }
        app_sha256_transform(ctx, ctx->data);
        i = 0U;
    }

    while (i < 56U)
    {
        ctx->data[i++] = 0U;
    }

    ctx->bitlen += (uint64_t)ctx->datalen * 8U;
    ctx->data[63] = (uint8_t)(ctx->bitlen);
    ctx->data[62] = (uint8_t)(ctx->bitlen >> 8U);
    ctx->data[61] = (uint8_t)(ctx->bitlen >> 16U);
    ctx->data[60] = (uint8_t)(ctx->bitlen >> 24U);
    ctx->data[59] = (uint8_t)(ctx->bitlen >> 32U);
    ctx->data[58] = (uint8_t)(ctx->bitlen >> 40U);
    ctx->data[57] = (uint8_t)(ctx->bitlen >> 48U);
    ctx->data[56] = (uint8_t)(ctx->bitlen >> 56U);
    app_sha256_transform(ctx, ctx->data);

    for (i = 0U; i < 4U; ++i)
    {
        hash[i]      = (uint8_t)((ctx->state[0] >> (24U - i * 8U)) & 0xFFU);
        hash[i + 4U] = (uint8_t)((ctx->state[1] >> (24U - i * 8U)) & 0xFFU);
        hash[i + 8U] = (uint8_t)((ctx->state[2] >> (24U - i * 8U)) & 0xFFU);
        hash[i + 12U] = (uint8_t)((ctx->state[3] >> (24U - i * 8U)) & 0xFFU);
        hash[i + 16U] = (uint8_t)((ctx->state[4] >> (24U - i * 8U)) & 0xFFU);
        hash[i + 20U] = (uint8_t)((ctx->state[5] >> (24U - i * 8U)) & 0xFFU);
        hash[i + 24U] = (uint8_t)((ctx->state[6] >> (24U - i * 8U)) & 0xFFU);
        hash[i + 28U] = (uint8_t)((ctx->state[7] >> (24U - i * 8U)) & 0xFFU);
    }
}

static char app_hex_lower(char ch)
{
    if (ch >= 'A' && ch <= 'Z')
    {
        return (char)(ch + 32);
    }
    return ch;
}

static void app_bytes_to_hex(const uint8_t *bytes, size_t length, char *out, size_t out_size)
{
    const char *hex = "0123456789abcdef";
    if (out_size < (length * 2U + 1U))
    {
        return;
    }
    for (size_t i = 0U; i < length; ++i)
    {
        out[i * 2U] = hex[(bytes[i] >> 4U) & 0x0FU];
        out[i * 2U + 1U] = hex[bytes[i] & 0x0FU];
    }
    out[length * 2U] = '\0';
}

static bool app_sha256_match(const char *calc, const char *expect)
{
    if (calc == NULL || expect == NULL)
    {
        return false;
    }
    for (size_t i = 0U; i < OTA_SHA256_LEN; ++i)
    {
        if (calc[i] == '\0' || expect[i] == '\0')
        {
            return false;
        }
        if (app_hex_lower(calc[i]) != app_hex_lower(expect[i]))
        {
            return false;
        }
    }
    return true;
}

static bool app_verify_sha256(const char **reason)
{
    if (strlen(s_app.offer_sha256) != OTA_SHA256_LEN)
    {
        printf("[OTA] sha256 missing\r\n");
        if (reason != NULL)
        {
            *reason = "sha256_missing";
        }
        return false;
    }

    app_sha256_ctx_t ctx;
    app_sha256_init(&ctx);

    const uint8_t *ptr = (const uint8_t *)OTA_FLASH_BASE;
    size_t remaining = s_app.total_size;
    while (remaining > 0U)
    {
        size_t chunk = remaining > 512U ? 512U : remaining;
        app_sha256_update(&ctx, ptr, chunk);
        ptr += chunk;
        remaining -= chunk;
    }

    uint8_t hash[32];
    app_sha256_final(&ctx, hash);
    char calc_hex[OTA_SHA256_LEN + 1U];
    app_bytes_to_hex(hash, sizeof(hash), calc_hex, sizeof(calc_hex));

    printf("[OTA] sha256 calc=%s expect=%s\r\n", calc_hex, s_app.offer_sha256);
    if (!app_sha256_match(calc_hex, s_app.offer_sha256))
    {
        if (reason != NULL)
        {
            *reason = "sha256_mismatch";
        }
        return false;
    }
    return true;
}

static void app_log_status(const char *label, esp8266_at_status_t status)
{
    printf("[OTA][%s] %s\r\n", label, esp8266_at_status_string(status));
}

static void app_log_payload_preview(const char *label, const char *data, size_t length)
{
    if (label == NULL || data == NULL)
    {
        return;
    }

    const size_t head_len = (length > 60U) ? 60U : length;
    char head[61];
    memcpy(head, data, head_len);
    head[head_len] = '\0';

    if (length <= head_len)
    {
        printf("[OTA][%s] len=%lu head=%s\r\n",
               label,
               (unsigned long)length,
               head);
        return;
    }

    const size_t tail_len = 60U;
    char tail[61];
    memcpy(tail, data + (length - tail_len), tail_len);
    tail[tail_len] = '\0';

    printf("[OTA][%s] len=%lu head=%s tail=%s\r\n",
           label,
           (unsigned long)length,
           head,
           tail);
}

static size_t app_trim_tail_length(const char *buffer, size_t length)
{
    while (length > 0U)
    {
        const unsigned char ch = (unsigned char)buffer[length - 1U];
        if (ch > ' ')
        {
            break;
        }
        length--;
    }
    return length;
}

static void app_chunk_buffer_reset(void)
{
    s_chunk_json_len = 0U;
    s_chunk_json_active = false;
}

static bool app_is_image_valid(void)
{
    const uint32_t app_sp = *(__IO uint32_t *)(APP_START_ADDRESS);
    const uint32_t app_reset = *(__IO uint32_t *)(APP_START_ADDRESS + 4U);
    const bool sp_ok = (app_sp >= SRAM_START_ADDRESS) && (app_sp <= SRAM_END_ADDRESS);
    const bool reset_ok = (app_reset >= APP_START_ADDRESS) && (app_reset <= APP_END_ADDRESS);
    return sp_ok && reset_ok;
}

static void app_deinit_peripherals(void)
{
    esp8266_at_deinit();
    HAL_UART_DeInit(&huart6);
    HAL_UART_DeInit(&huart2);
    (void)HAL_TIM_PWM_DeInit(&htim3);
    (void)HAL_TIM_PWM_DeInit(&htim4);
    (void)HAL_I2C_DeInit(&hi2c1);
    (void)HAL_SPI_DeInit(&hspi1);
    HAL_NVIC_DisableIRQ(DMA1_Stream5_IRQn);
    HAL_NVIC_DisableIRQ(DMA1_Stream6_IRQn);
    HAL_NVIC_DisableIRQ(DMA2_Stream1_IRQn);
    HAL_NVIC_DisableIRQ(DMA2_Stream3_IRQn);
    HAL_NVIC_DisableIRQ(DMA2_Stream6_IRQn);
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_All);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_All);
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_All);
    HAL_GPIO_DeInit(GPIOH, GPIO_PIN_All);
}

static void app_jump_to_image(void)
{
    const uint32_t app_sp = *(__IO uint32_t *)(APP_START_ADDRESS);
    const uint32_t app_reset = *(__IO uint32_t *)(APP_START_ADDRESS + 4U);
    app_entry_t application = (app_entry_t)app_reset;

    __disable_irq();
    app_deinit_peripherals();
    HAL_RCC_DeInit();
    HAL_DeInit();
    SysTick->CTRL = 0U;
    SysTick->LOAD = 0U;
    SysTick->VAL = 0U;
    __set_MSP(app_sp);
    SCB->VTOR = APP_START_ADDRESS;
    __enable_irq();

    application();
}

static bool app_chunk_buffer_append(const char *data, size_t length)
{
    if (data == NULL || length == 0U)
    {
        return false;
    }
    if ((s_chunk_json_len + length) >= sizeof(s_chunk_json_buffer))
    {
        printf("[OTA] chunk buffer overflow len=%lu\r\n", (unsigned long)(s_chunk_json_len + length));
        app_chunk_buffer_reset();
        return false;
    }
    memcpy(&s_chunk_json_buffer[s_chunk_json_len], data, length);
    s_chunk_json_len += length;
    s_chunk_json_buffer[s_chunk_json_len] = '\0';
    return true;
}

static bool app_is_placeholder(const char *value, const char *placeholder)
{
    if (value == NULL || placeholder == NULL)
    {
        return true;
    }
    return (strcmp(value, placeholder) == 0);
}

static void app_drain_events(uint32_t delay_ms)
{
    if (delay_ms > 0U)
    {
        HAL_Delay(delay_ms);
    }

    esp8266_at_poll();

    esp8266_at_event_t event;
    while (esp8266_at_fetch_event(&event))
    {
    }
}

static void app_build_ota_topic(char *buffer, size_t size, const char *suffix)
{
    (void)snprintf(buffer, size, "%s/ota/%s/%s",
                   APP_MQTT_BASE_TOPIC,
                   s_app.device_id,
                   suffix);
}

static esp8266_at_status_t app_mqtt_subscribe_topic(const char *topic, const char *label)
{
    if (topic == NULL || topic[0] == '\0')
    {
        return ESP8266_AT_STATUS_INVALID_ARGUMENT;
    }

    char sub_args[ESP8266_AT_MAX_LINE_LENGTH];
    (void)snprintf(sub_args,
                   sizeof(sub_args),
                   "%u,\"%s\",%u",
                   0U,
                   topic,
                   (unsigned int)APP_MQTT_QOS);

    const esp8266_at_status_t status =
        esp8266_at_send_command(ESP8266_AT_CMD_MQTTSUB,
                                ESP8266_AT_COMMAND_MODE_SET,
                                sub_args,
                                ESP8266_AT_DEFAULT_TIMEOUT_MS,
                                false);
    if (label != NULL)
    {
        app_log_status(label, status);
    }
    app_drain_events(150U);
    return status;
}

static void app_mqtt_subscribe_all(void)
{
    char cmd_topic[96];
    char data_topic[96];
    app_build_ota_topic(cmd_topic, sizeof(cmd_topic), "cmd");
    app_build_ota_topic(data_topic, sizeof(data_topic), "data");
    (void)app_mqtt_subscribe_topic(cmd_topic, "sub_cmd");
    (void)app_mqtt_subscribe_topic(data_topic, "sub_data");
}

static void app_mqtt_schedule_reconnect(void)
{
    s_app.mqtt_reconnect_pending = true;
}

static void app_mqtt_try_reconnect(void)
{
    if (!s_app.wifi_ready || s_app.mqtt_ready)
    {
        return;
    }

    const uint32_t now = HAL_GetTick();
    if ((uint32_t)(now - s_app.last_mqtt_attempt_ms) < APP_MQTT_RECONNECT_MS)
    {
        return;
    }
    if (!esp8266_at_is_ready())
    {
        return;
    }

    s_app.last_mqtt_attempt_ms = now;
    printf("[OTA] mqtt reconnecting...\r\n");

    const esp8266_at_status_t status =
        esp8266_at_mqtt_connect(0U,
                                APP_MQTT_BROKER,
                                APP_MQTT_PORT,
                                120U,
                                true);
    app_log_status("mqtt_reconn", status);
    app_drain_events(200U);

    if (status == ESP8266_AT_STATUS_OK)
    {
        s_app.mqtt_ready = true;
        s_app.mqtt_reconnect_pending = false;
        app_mqtt_subscribe_all();
        s_app.check_sent = false;
    }
}

static bool app_parse_mqtt_subrecv(const char *payload,
                                   char *topic,
                                   size_t topic_size,
                                   const char **data_ptr,
                                   size_t *data_len)
{
    if (payload == NULL || topic == NULL || topic_size == 0U
        || data_ptr == NULL || data_len == NULL)
    {
        return false;
    }

    const char *cursor = payload;
    while (*cursor == ' ')
    {
        ++cursor;
    }

    char *endptr = NULL;
    (void)strtol(cursor, &endptr, 10);
    if (endptr == cursor || *endptr != ',')
    {
        return false;
    }
    cursor = endptr + 1;

    if (*cursor != '"')
    {
        return false;
    }
    ++cursor;

    const char *topic_start = cursor;
    while (*cursor != '\0' && *cursor != '"')
    {
        ++cursor;
    }
    if (*cursor != '"')
    {
        return false;
    }

    size_t topic_len = (size_t)(cursor - topic_start);
    if (topic_len >= topic_size)
    {
        topic_len = topic_size - 1U;
    }
    memcpy(topic, topic_start, topic_len);
    topic[topic_len] = '\0';

    ++cursor;
    if (*cursor != ',')
    {
        return false;
    }
    ++cursor;

    long parsed_len = strtol(cursor, &endptr, 10);
    if (endptr == cursor || *endptr != ',')
    {
        return false;
    }
    if (parsed_len < 0)
    {
        return false;
    }
    cursor = endptr + 1;

    *data_ptr = cursor;
    *data_len = (size_t)parsed_len;
    return true;
}

static bool app_base64_decode(const char *input,
                              size_t input_len,
                              uint8_t *output,
                              size_t output_size,
                              size_t *out_len)
{
    if (input == NULL || output == NULL || out_len == NULL)
    {
        return false;
    }

    uint32_t buffer = 0U;
    uint8_t bits = 0U;
    size_t out_index = 0U;

    for (size_t i = 0U; i < input_len; ++i)
    {
        const char ch = input[i];
        if (ch == '=')
        {
            break;
        }
        int8_t val = -1;
        if (ch >= 'A' && ch <= 'Z')
        {
            val = (int8_t)(ch - 'A');
        }
        else if (ch >= 'a' && ch <= 'z')
        {
            val = (int8_t)(ch - 'a' + 26);
        }
        else if (ch >= '0' && ch <= '9')
        {
            val = (int8_t)(ch - '0' + 52);
        }
        else if (ch == '+')
        {
            val = 62;
        }
        else if (ch == '/')
        {
            val = 63;
        }
        else
        {
            continue;
        }

        buffer = (buffer << 6U) | (uint32_t)val;
        bits += 6U;
        if (bits >= 8U)
        {
            bits -= 8U;
            if (out_index >= output_size)
            {
                return false;
            }
            output[out_index++] = (uint8_t)((buffer >> bits) & 0xFFU);
        }
    }

    *out_len = out_index;
    return true;
}

static bool app_parse_hex_u32(const char *text, uint32_t *value)
{
    if (text == NULL || value == NULL)
    {
        return false;
    }
    char *endptr = NULL;
    unsigned long result = strtoul(text, &endptr, 16);
    if (endptr == text)
    {
        return false;
    }
    *value = (uint32_t)result;
    return true;
}

static bool app_flash_check_blank(uint32_t address, size_t length)
{
    const uint8_t *ptr = (const uint8_t *)address;
    for (size_t i = 0U; i < length; ++i)
    {
        if (ptr[i] != 0xFFU)
        {
            printf("[OTA] flash not blank at 0x%08lx value=0x%02x\r\n",
                   (unsigned long)(address + i),
                   (unsigned int)ptr[i]);
            return false;
        }
    }
    return true;
}

static bool app_flash_prepare(void)
{
    printf("[OTA] flash region base=0x%08lx size=%lu\r\n",
           (unsigned long)OTA_FLASH_BASE,
           (unsigned long)OTA_FLASH_SIZE);
    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef erase;
    uint32_t error_sector = 0U;
    memset(&erase, 0, sizeof(erase));
    erase.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    erase.Sector = OTA_FLASH_SECTOR_FIRST;
    erase.NbSectors = (OTA_FLASH_SECTOR_LAST - OTA_FLASH_SECTOR_FIRST) + 1U;

    const HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&erase, &error_sector);
    if (status != HAL_OK)
    {
        printf("[OTA] flash erase failed, err=0x%08lx code=0x%08lx\r\n",
               (unsigned long)error_sector,
               (unsigned long)HAL_FLASH_GetError());
        HAL_FLASH_Lock();
        return false;
    }

    s_app.flash_unlocked = true;
    printf("[OTA] flash erased (sectors %lu-%lu)\r\n",
           (unsigned long)OTA_FLASH_SECTOR_FIRST,
           (unsigned long)OTA_FLASH_SECTOR_LAST);
    (void)app_flash_check_blank(OTA_FLASH_BASE, 16U);
    (void)app_flash_check_blank(OTA_FLASH_BASE + OTA_FLASH_SIZE - 16U, 16U);
    return true;
}

static void app_flash_lock(void)
{
    if (s_app.flash_unlocked)
    {
        HAL_FLASH_Lock();
        s_app.flash_unlocked = false;
    }
}

static bool app_flash_write(uint32_t address, const uint8_t *data, size_t length)
{
    if (!s_app.flash_unlocked || data == NULL || length == 0U)
    {
        return false;
    }

    const uint32_t flash_end = OTA_FLASH_BASE + OTA_FLASH_SIZE;
    if (address < OTA_FLASH_BASE || (address + length) > flash_end)
    {
        printf("[OTA] flash range overflow addr=0x%08lx len=%lu\r\n",
               (unsigned long)address,
               (unsigned long)length);
        return false;
    }

    uint32_t write_addr = address;
    for (size_t offset = 0U; offset < length; ++offset)
    {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, write_addr, data[offset]) != HAL_OK)
        {
            printf("[OTA] flash program failed at 0x%08lx code=0x%08lx\r\n",
                   (unsigned long)write_addr,
                   (unsigned long)HAL_FLASH_GetError());
            return false;
        }
        write_addr += 1U;
    }

    if (length > 0U)
    {
        const uint8_t first = *(const uint8_t *)address;
        const uint8_t last = *(const uint8_t *)(address + length - 1U);
        if (first != data[0] || last != data[length - 1U])
        {
            printf("[OTA] flash verify mismatch addr=0x%08lx first=0x%02x last=0x%02x\r\n",
                   (unsigned long)address,
                   (unsigned int)first,
                   (unsigned int)last);
            return false;
        }
    }

    return true;
}

static esp8266_at_status_t app_mqtt_publish_json(const char *topic, cJSON *root)
{
    esp8266_at_status_t status = ESP8266_AT_STATUS_INVALID_ARGUMENT;
    if (root == NULL || topic == NULL)
    {
        return status;
    }

    char *payload = cJSON_PrintUnformatted(root);
    if (payload != NULL)
    {
        const size_t payload_len = strlen(payload);
        status = esp8266_at_mqtt_publish_raw(0U,
                                             topic,
                                             (const uint8_t *)payload,
                                             payload_len,
                                             APP_MQTT_QOS,
                                             false);
        printf("[OTA] mqtt pub topic=%s len=%lu status=%s\r\n",
               topic,
               (unsigned long)payload_len,
               esp8266_at_status_string(status));
        free(payload);
    }
    return status;
}

static void app_send_check(void)
{
    char topic[96];
    app_build_ota_topic(topic, sizeof(topic), "req");

    cJSON *root = cJSON_CreateObject();
    cJSON *cap = cJSON_CreateObject();
    if (root == NULL || cap == NULL)
    {
        cJSON_Delete(root);
        cJSON_Delete(cap);
        return;
    }
    cJSON_AddStringToObject(root, "type", "check");
    cJSON_AddStringToObject(root, "model", APP_DEVICE_MODEL);
    cJSON_AddStringToObject(root, "version", APP_DEVICE_VERSION);
    cJSON_AddNumberToObject(cap, "chunkSize", (double)OTA_CHUNK_SIZE);
    cJSON_AddItemToObject(root, "cap", cap);

    (void)app_mqtt_publish_json(topic, root);
    cJSON_Delete(root);
    s_app.check_sent = true;
    printf("[OTA] check sent\r\n");
}

static void app_send_start(void)
{
    char topic[96];
    app_build_ota_topic(topic, sizeof(topic), "req");

    cJSON *root = cJSON_CreateObject();
    if (root == NULL)
    {
        return;
    }
    cJSON_AddStringToObject(root, "type", "start");
    cJSON_AddStringToObject(root, "model", APP_DEVICE_MODEL);
    (void)app_mqtt_publish_json(topic, root);
    cJSON_Delete(root);
    printf("[OTA] start sent\r\n");
}

static void app_send_ack(uint32_t next_index, const char *status)
{
    char topic[96];
    app_build_ota_topic(topic, sizeof(topic), "ack");

    cJSON *root = cJSON_CreateObject();
    if (root == NULL)
    {
        return;
    }
    cJSON_AddStringToObject(root, "type", "ack");
    cJSON_AddNumberToObject(root, "next", (double)next_index);
    if (status != NULL)
    {
        cJSON_AddStringToObject(root, "status", status);
    }
    (void)app_mqtt_publish_json(topic, root);
    cJSON_Delete(root);
}

static void app_send_state(uint32_t progress, const char *phase, const char *status)
{
    char topic[96];
    app_build_ota_topic(topic, sizeof(topic), "state");

    cJSON *root = cJSON_CreateObject();
    if (root == NULL)
    {
        return;
    }
    cJSON_AddStringToObject(root, "type", "state");
    if (phase != NULL)
    {
        cJSON_AddStringToObject(root, "phase", phase);
    }
    cJSON_AddNumberToObject(root, "progress", (double)progress);
    if (status != NULL)
    {
        cJSON_AddStringToObject(root, "status", status);
    }
    (void)app_mqtt_publish_json(topic, root);
    cJSON_Delete(root);
}

static void app_send_result(bool ok, const char *reason)
{
    char topic[96];
    app_build_ota_topic(topic, sizeof(topic), "state");

    cJSON *root = cJSON_CreateObject();
    if (root == NULL)
    {
        return;
    }
    cJSON_AddStringToObject(root, "type", "result");
    cJSON_AddStringToObject(root, "status", ok ? "ok" : "fail");
    if (!ok && reason != NULL)
    {
        cJSON_AddStringToObject(root, "reason", reason);
    }
    (void)app_mqtt_publish_json(topic, root);
    cJSON_Delete(root);
}

static void app_ota_reset_state(void)
{
    s_app.ota_active = false;
    s_app.total_size = 0U;
    s_app.chunk_size = OTA_CHUNK_SIZE;
    s_app.total_chunks = 0U;
    s_app.expected_index = 0U;
    s_app.bytes_written = 0U;
    s_app.last_progress = 0U;
    memset(s_app.offer_version, 0, sizeof(s_app.offer_version));
    memset(s_app.offer_sha256, 0, sizeof(s_app.offer_sha256));
    app_chunk_buffer_reset();
}

static void app_handle_offer(const cJSON *root)
{
    if (s_app.ota_active)
    {
        app_flash_lock();
        app_ota_reset_state();
    }
    const cJSON *status = cJSON_GetObjectItem(root, "status");
    if (cJSON_IsString(status))
    {
        if (strcmp(status->valuestring, "no_update") == 0)
        {
            printf("[OTA] no update\r\n");
            return;
        }
        if (strcmp(status->valuestring, "update") != 0)
        {
            printf("[OTA] offer status=%s\r\n", status->valuestring);
            return;
        }
    }

    const cJSON *size = cJSON_GetObjectItem(root, "size");
    const cJSON *chunk = cJSON_GetObjectItem(root, "chunkSize");
    const cJSON *version = cJSON_GetObjectItem(root, "version");
    const cJSON *sha256 = cJSON_GetObjectItem(root, "sha256");

    if (!cJSON_IsNumber(size) || !cJSON_IsNumber(chunk))
    {
        printf("[OTA] invalid offer\r\n");
        return;
    }

    s_app.total_size = (uint32_t)size->valuedouble;
    s_app.chunk_size = (uint32_t)chunk->valuedouble;
    s_app.total_chunks = (s_app.total_size + s_app.chunk_size - 1U) / s_app.chunk_size;
    s_app.expected_index = 0U;
    s_app.bytes_written = 0U;
    s_app.last_progress = 0U;

    if (s_app.total_size == 0U || s_app.total_size > OTA_FLASH_SIZE)
    {
        printf("[OTA] image size overflow: %lu (limit %lu)\r\n",
               (unsigned long)s_app.total_size,
               (unsigned long)OTA_FLASH_SIZE);
        app_send_result(false, "size_overflow");
        app_ota_reset_state();
        return;
    }

    if (s_app.chunk_size == 0U)
    {
        printf("[OTA] chunk size invalid\r\n");
        app_send_result(false, "chunk_size");
        app_ota_reset_state();
        return;
    }

    if (s_app.chunk_size > OTA_CHUNK_SIZE)
    {
        printf("[OTA] chunk too large: %lu\r\n", (unsigned long)s_app.chunk_size);
        app_send_result(false, "chunk_size");
        app_ota_reset_state();
        return;
    }

    if (cJSON_IsString(version))
    {
        strncpy(s_app.offer_version, version->valuestring, sizeof(s_app.offer_version) - 1U);
    }
    if (cJSON_IsString(sha256))
    {
        strncpy(s_app.offer_sha256, sha256->valuestring, sizeof(s_app.offer_sha256) - 1U);
    }

    printf("[OTA] offer size=%lu chunk=%lu total=%lu version=%s\r\n",
           (unsigned long)s_app.total_size,
           (unsigned long)s_app.chunk_size,
           (unsigned long)s_app.total_chunks,
           s_app.offer_version[0] ? s_app.offer_version : "--");

    if (!app_flash_prepare())
    {
        app_send_result(false, "flash_erase");
        return;
    }

    s_app.ota_active = true;
    app_send_start();
}

static void app_handle_cmd(const char *json, size_t length)
{
    char buffer[ESP8266_AT_MAX_LINE_LENGTH];
    if (length >= sizeof(buffer))
    {
        printf("[OTA] cmd json too long len=%lu\r\n", (unsigned long)length);
        return;
    }
    memcpy(buffer, json, length);
    buffer[length] = '\0';

    cJSON *root = cJSON_Parse(buffer);
    if (root == NULL)
    {
        printf("[OTA] cmd invalid json\r\n");
        app_log_payload_preview("cmd_payload", buffer, length);
        return;
    }

    const cJSON *type = cJSON_GetObjectItem(root, "type");
    if (cJSON_IsString(type))
    {
        if (strcmp(type->valuestring, "offer") == 0)
        {
            app_handle_offer(root);
        }
        else if (strcmp(type->valuestring, "abort") == 0)
        {
            printf("[OTA] abort\r\n");
            app_ota_reset_state();
            app_flash_lock();
        }
        else if (strcmp(type->valuestring, "finish") == 0)
        {
            printf("[OTA] finish command received\r\n");
        }
    }

    cJSON_Delete(root);
}

static void app_handle_chunk_json(const char *json, size_t length)
{
    if (!s_app.ota_active)
    {
        printf("[OTA] chunk ignored (inactive)\r\n");
        return;
    }

    char buffer[ESP8266_AT_MAX_LINE_LENGTH];
    if (length >= sizeof(buffer))
    {
        printf("[OTA] chunk json too long len=%lu\r\n", (unsigned long)length);
        return;
    }
    memcpy(buffer, json, length);
    buffer[length] = '\0';

    cJSON *root = cJSON_Parse(buffer);
    if (root == NULL)
    {
        printf("[OTA] chunk invalid json\r\n");
        app_log_payload_preview("chunk_payload", buffer, length);
        return;
    }

    const cJSON *type = cJSON_GetObjectItem(root, "type");
    const cJSON *index = cJSON_GetObjectItem(root, "index");
    const cJSON *offset = cJSON_GetObjectItem(root, "offset");
    const cJSON *data = cJSON_GetObjectItem(root, "data");
    const cJSON *crc32 = cJSON_GetObjectItem(root, "crc32");
    const cJSON *size = cJSON_GetObjectItem(root, "size");

    if (!cJSON_IsString(type) || strcmp(type->valuestring, "chunk") != 0
        || !cJSON_IsNumber(index) || !cJSON_IsString(data))
    {
        printf("[OTA] chunk missing fields\r\n");
        cJSON_Delete(root);
        return;
    }

    const uint32_t chunk_index = (uint32_t)index->valuedouble;
    if (chunk_index != s_app.expected_index)
    {
        printf("[OTA] chunk out of order: got=%lu expect=%lu\r\n",
               (unsigned long)chunk_index,
               (unsigned long)s_app.expected_index);
        app_send_ack(s_app.expected_index, "wait");
        cJSON_Delete(root);
        return;
    }

    const uint32_t chunk_offset = cJSON_IsNumber(offset)
                                ? (uint32_t)offset->valuedouble
                                : (chunk_index * s_app.chunk_size);

    const char *data_str = data->valuestring;
    const size_t data_len = strlen(data_str);
    static uint8_t chunk_buffer[OTA_CHUNK_SIZE];
    size_t out_len = 0U;

    if (!app_base64_decode(data_str, data_len, chunk_buffer, sizeof(chunk_buffer), &out_len))
    {
        printf("[OTA] base64 decode failed\r\n");
        app_send_ack(s_app.expected_index, "decode_error");
        cJSON_Delete(root);
        return;
    }
    if (out_len == 0U)
    {
        printf("[OTA] empty chunk\r\n");
        app_send_ack(s_app.expected_index, "empty");
        cJSON_Delete(root);
        return;
    }
    if (s_app.total_size > 0U && (chunk_offset + out_len) > s_app.total_size)
    {
        printf("[OTA] chunk exceeds total size\r\n");
        app_send_ack(s_app.expected_index, "size_error");
        cJSON_Delete(root);
        return;
    }

    if (cJSON_IsNumber(size) && out_len != (size_t)size->valuedouble)
    {
        printf("[OTA] size mismatch: payload=%lu expected=%lu\r\n",
               (unsigned long)out_len,
               (unsigned long)size->valuedouble);
    }

    printf("[OTA] chunk index=%lu offset=%lu size=%lu\r\n",
           (unsigned long)chunk_index,
           (unsigned long)chunk_offset,
           (unsigned long)out_len);

    if (cJSON_IsString(crc32))
    {
        uint32_t expected_crc = 0U;
        if (app_parse_hex_u32(crc32->valuestring, &expected_crc))
        {
            const uint32_t actual_crc = app_crc32(chunk_buffer, out_len);
            if (actual_crc != expected_crc)
            {
                printf("[OTA] crc mismatch exp=0x%08lx got=0x%08lx\r\n",
                       (unsigned long)expected_crc,
                       (unsigned long)actual_crc);
                app_send_ack(s_app.expected_index, "crc_error");
                cJSON_Delete(root);
                return;
            }
        }
    }

    const uint32_t write_addr = OTA_FLASH_BASE + chunk_offset;
    if (!app_flash_write(write_addr, chunk_buffer, out_len))
    {
        app_send_ack(s_app.expected_index, "flash_error");
        app_send_result(false, "flash_error");
        s_app.ota_active = false;
        app_flash_lock();
        cJSON_Delete(root);
        return;
    }

    s_app.expected_index++;
    const uint32_t written = chunk_offset + (uint32_t)out_len;
    if (written > s_app.bytes_written)
    {
        s_app.bytes_written = written;
    }

    app_send_ack(s_app.expected_index, "ok");

    uint32_t progress = 0U;
    if (s_app.total_size > 0U)
    {
        progress = (uint32_t)((s_app.bytes_written * 100U) / s_app.total_size);
    }
    if (progress >= 100U || progress >= s_app.last_progress + 5U)
    {
        s_app.last_progress = progress;
        app_send_state(progress, "downloading", "ok");
        printf("[OTA] progress %lu%%\r\n", (unsigned long)progress);
    }

    if (s_app.bytes_written >= s_app.total_size || s_app.expected_index >= s_app.total_chunks)
    {
        printf("[OTA] download complete, size=%lu\r\n", (unsigned long)s_app.bytes_written);
        const char *reason = NULL;
        if (app_verify_sha256(&reason))
        {
            if (app_is_image_valid())
            {
                app_send_result(true, NULL);
                app_drain_events(200U);
                printf("[OTA] image verified, jumping to app\r\n");
                s_app.ota_active = false;
                app_flash_lock();
                app_chunk_buffer_reset();
                app_jump_to_image();
                return;
            }
            const uint32_t app_sp = *(__IO uint32_t *)(APP_START_ADDRESS);
            const uint32_t app_reset = *(__IO uint32_t *)(APP_START_ADDRESS + 4U);
            printf("[OTA] invalid image sp=0x%08lx reset=0x%08lx\r\n",
                   (unsigned long)app_sp,
                   (unsigned long)app_reset);
            app_send_result(false, "invalid_image");
        }
        else
        {
            app_send_result(false, reason);
        }
        s_app.ota_active = false;
        app_flash_lock();
        printf("[OTA] flash write finished (no jump)\r\n");
        app_chunk_buffer_reset();
    }

    cJSON_Delete(root);
}

static void app_handle_chunk_fragment(const char *data, size_t length)
{
    if (data == NULL || length == 0U)
    {
        return;
    }

    const bool starts_json = (data[0] == '{');
    if (starts_json)
    {
        app_chunk_buffer_reset();
        s_chunk_json_active = true;
    }
    else if (!s_chunk_json_active)
    {
        printf("[OTA] chunk fragment without header len=%lu\r\n", (unsigned long)length);
        app_log_payload_preview("chunk_fragment", data, length);
        return;
    }

    if (!app_chunk_buffer_append(data, length))
    {
        return;
    }

    const size_t trimmed_len = app_trim_tail_length(s_chunk_json_buffer, s_chunk_json_len);
    if (trimmed_len == 0U)
    {
        return;
    }
    if (s_chunk_json_buffer[0] != '{' || s_chunk_json_buffer[trimmed_len - 1U] != '}')
    {
        printf("[OTA] chunk fragment stored len=%lu\r\n", (unsigned long)s_chunk_json_len);
        return;
    }

    printf("[OTA] chunk assembled len=%lu\r\n", (unsigned long)trimmed_len);
    app_handle_chunk_json(s_chunk_json_buffer, trimmed_len);
    app_chunk_buffer_reset();
}

static void app_process_mqtt_event(const esp8266_at_event_t *event)
{
    if (event == NULL)
    {
        return;
    }

    if (strcmp(event->prefix, "+MQTTSUBRECV") == 0)
    {
        char topic[96];
        const char *data = NULL;
        size_t data_len = 0U;
        if (!app_parse_mqtt_subrecv(event->payload,
                                    topic,
                                    sizeof(topic),
                                    &data,
                                    &data_len))
        {
            printf("[OTA] mqtt parse failed: %s\r\n", event->payload);
            return;
        }

        size_t available = strlen(data);
        if (data_len > available)
        {
            printf("[OTA] mqtt payload truncated len=%lu available=%lu\r\n",
                   (unsigned long)data_len,
                   (unsigned long)available);
            data_len = available;
        }

        char cmd_topic[96];
        char data_topic[96];
        app_build_ota_topic(cmd_topic, sizeof(cmd_topic), "cmd");
        app_build_ota_topic(data_topic, sizeof(data_topic), "data");

        if (strcmp(topic, cmd_topic) == 0)
        {
            printf("[OTA] rx cmd len=%lu\r\n", (unsigned long)data_len);
            app_handle_cmd(data, data_len);
        }
        else if (strcmp(topic, data_topic) == 0)
        {
            if (s_app.expected_index == 0U)
            {
                app_log_payload_preview("first_chunk", data, data_len);
            }
            app_handle_chunk_fragment(data, data_len);
        }
    }
    else if (strcmp(event->prefix, "+MQTTDISCONNECTED") == 0)
    {
        s_app.mqtt_ready = false;
        s_app.check_sent = false;
        printf("[OTA] mqtt disconnected\r\n");
        app_mqtt_schedule_reconnect();
    }
    else if (strcmp(event->prefix, "+MQTTCONNECTED") == 0)
    {
        s_app.mqtt_ready = true;
        s_app.check_sent = false;
        printf("[OTA] mqtt connected\r\n");
        app_mqtt_subscribe_all();
    }
    else if (event->type == ESP8266_AT_EVENT_TYPE_ERROR
             || event->type == ESP8266_AT_EVENT_TYPE_FAIL
             || event->type == ESP8266_AT_EVENT_TYPE_BUSY)
    {
        printf("[OTA] event %s (cmd=%s)\r\n",
               event->raw_line,
               esp8266_at_command_name(event->command));
    }
}

void App_Init(void)
{
    memset(&s_app, 0, sizeof(s_app));
    app_ota_reset_state();
    app_build_mqtt_identifiers();
    printf("[OTA] device id: %s\r\n", s_app.device_id);
    printf("[OTA] mqtt client id: %s\r\n", s_app.mqtt_client_id);

    HAL_Delay(2000U);

    esp8266_at_status_t status = esp8266_at_init();
    app_log_status("esp_init", status);
    if (status != ESP8266_AT_STATUS_OK)
    {
        return;
    }

    status = esp8266_at_reset(2000U);
    app_log_status("esp_reset", status);
    if (status != ESP8266_AT_STATUS_OK)
    {
        return;
    }
    esp8266_at_clear_events();
    app_drain_events(200U);

    status = esp8266_at_disable_echo(true);
    app_log_status("echo_off", status);
    if (status != ESP8266_AT_STATUS_OK)
    {
        return;
    }
    app_drain_events(50U);

    const bool wifi_configured =
        !app_is_placeholder(APP_WIFI_SSID, "YOUR_WIFI_SSID")
        && !app_is_placeholder(APP_WIFI_PASSWORD, "YOUR_WIFI_PASSWORD");
    if (wifi_configured)
    {
        status = esp8266_at_set_wifi_mode(1U, false);
        app_log_status("wifi_mode", status);
        app_drain_events(50U);

        status = esp8266_at_connect_ap(APP_WIFI_SSID,
                                       APP_WIFI_PASSWORD,
                                       20000U,
                                       false);
        app_log_status("wifi_join", status);
        app_drain_events(500U);
        s_app.wifi_ready = (status == ESP8266_AT_STATUS_OK);
    }
    else
    {
        printf("[OTA] wifi skipped (set APP_WIFI_SSID/PASSWORD)\r\n");
    }

    const bool mqtt_configured =
        !app_is_placeholder(APP_MQTT_BROKER, "YOUR_MQTT_BROKER");
    if (mqtt_configured)
    {
        status = esp8266_at_mqtt_user_config(0U,
                                             s_app.mqtt_client_id,
                                             APP_MQTT_USERNAME,
                                             APP_MQTT_PASSWORD);
        app_log_status("mqtt_usercfg", status);
        app_drain_events(50U);

        status = esp8266_at_mqtt_connect(0U,
                                         APP_MQTT_BROKER,
                                         APP_MQTT_PORT,
                                         120U,
                                         true);
        app_log_status("mqtt_conn", status);
        app_drain_events(200U);
        s_app.mqtt_ready = (status == ESP8266_AT_STATUS_OK);

        if (s_app.mqtt_ready)
        {
            app_mqtt_subscribe_all();
        }
    }
    else
    {
        printf("[OTA] mqtt skipped (set APP_MQTT_* macros)\r\n");
    }

    printf("[OTA] init done (wifi=%u mqtt=%u)\r\n",
           s_app.wifi_ready ? 1U : 0U,
           s_app.mqtt_ready ? 1U : 0U);
}

void App_Loop(void)
{
    esp8266_at_poll();

    esp8266_at_event_t event;
    while (esp8266_at_fetch_event(&event))
    {
        app_process_mqtt_event(&event);
    }

    if (!s_app.mqtt_ready)
    {
        if (s_app.mqtt_reconnect_pending)
        {
            app_mqtt_try_reconnect();
        }
        return;
    }

    if (!s_app.check_sent)
    {
        app_send_check();
    }
}
