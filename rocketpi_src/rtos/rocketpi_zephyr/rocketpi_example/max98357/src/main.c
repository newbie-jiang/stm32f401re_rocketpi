#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2s.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include <string.h>
#include <stdint.h>

#include "audio.h"   /* audio_track[] + 宏定义 */

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

/* 通过别名拿到 I2S 设备： /aliases { i2s_tx = &i2s2; }; */
#define I2S_DEV_NODE DT_ALIAS(i2s_tx)
#if !DT_NODE_HAS_STATUS(I2S_DEV_NODE, okay)
#error "No i2s_tx alias found in devicetree"
#endif

static const struct device *const i2s_dev = DEVICE_DT_GET(I2S_DEV_NODE);

/* 可选：控制 MAX98357 的 SD 引脚（如果你在 dts 里做了 amp_sd 节点） */
#if DT_NODE_EXISTS(DT_NODELABEL(amp_sd))
#define AMP_SD_NODE DT_NODELABEL(amp_sd)
static const struct gpio_dt_spec amp_sd = GPIO_DT_SPEC_GET(AMP_SD_NODE, gpios);
#endif

/* 从 audio.h 读取音频格式参数 */
#define SAMPLE_RATE_HZ        AUDIO_SAMPLE_RATE_HZ
// #define CHANNELS              AUDIO_NUM_CHANNELS
#define CHANNELS              1
#define WORD_SIZE_BITS        AUDIO_BITS_PER_SAMPLE
#define BYTES_PER_SAMPLE      (WORD_SIZE_BITS / 8)

/* 选择每个 block 的“帧数”（frame=所有声道的一组采样） */
#define FRAMES_PER_BLOCK      256U
#define SAMPLES_PER_BLOCK     (FRAMES_PER_BLOCK * CHANNELS) /* int16 个数 */
#define BLOCK_SIZE_BYTES      (SAMPLES_PER_BLOCK * BYTES_PER_SAMPLE)

/* TX 队列 block 数量（>=2）。多一些更稳 */
#define BLOCK_COUNT           4

K_MEM_SLAB_DEFINE(tx_slab, BLOCK_SIZE_BYTES, BLOCK_COUNT, 4);

static int audio_i2s_start(void)
{
    if (!device_is_ready(i2s_dev)) {
        LOG_ERR("I2S device not ready");
        return -ENODEV;
    }

#if DT_NODE_EXISTS(DT_NODELABEL(amp_sd))
    if (!device_is_ready(amp_sd.port)) {
        LOG_ERR("AMP_SD GPIO not ready");
        return -ENODEV;
    }
    gpio_pin_configure_dt(&amp_sd, GPIO_OUTPUT_INACTIVE);
    gpio_pin_set_dt(&amp_sd, 1);
    k_msleep(10);
#endif

    struct i2s_config cfg = {0};

    cfg.word_size      = WORD_SIZE_BITS;
    cfg.channels       = CHANNELS;
    cfg.format         = I2S_FMT_DATA_FORMAT_I2S;
    cfg.options        = I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER;
    cfg.frame_clk_freq = SAMPLE_RATE_HZ;
    cfg.mem_slab       = &tx_slab;
    cfg.block_size     = BLOCK_SIZE_BYTES;
    cfg.timeout        = 2000; /* ms：队列满时 i2s_write 等待的超时 */

    int ret = i2s_configure(i2s_dev, I2S_DIR_TX, &cfg);
    if (ret < 0) {
        LOG_ERR("i2s_configure failed: %d", ret);
        return ret;
    }

    /* 启动前先把 TX 队列塞满一些 block，避免刚开始就 underrun */
    size_t sample_idx = 0; /* 以 int16_t 为单位的索引 */
    for (int i = 0; i < BLOCK_COUNT; i++) {
        void *blk = NULL;
        ret = k_mem_slab_alloc(&tx_slab, &blk, K_FOREVER);
        if (ret < 0) {
            LOG_ERR("k_mem_slab_alloc failed: %d", ret);
            return ret;
        }

        /* 这块要拷贝的 int16 数量 */
        size_t remain = AUDIO_TRACK_SAMPLE_COUNT - sample_idx;
        size_t to_copy = (remain > SAMPLES_PER_BLOCK) ? SAMPLES_PER_BLOCK : remain;

        memcpy(blk, &audio_track[sample_idx], to_copy * sizeof(int16_t));
        if (to_copy < SAMPLES_PER_BLOCK) {
            /* 最后一块不够时补 0（静音） */
            memset((uint8_t *)blk + to_copy * sizeof(int16_t), 0,
                   (SAMPLES_PER_BLOCK - to_copy) * sizeof(int16_t));
        }

        sample_idx += to_copy;

        ret = i2s_write(i2s_dev, blk, BLOCK_SIZE_BYTES);
        if (ret < 0) {
            LOG_ERR("i2s_write failed: %d", ret);
            return ret;
        }

        if (sample_idx >= AUDIO_TRACK_SAMPLE_COUNT) {
            break;
        }
    }

    ret = i2s_trigger(i2s_dev, I2S_DIR_TX, I2S_TRIGGER_START);
    if (ret < 0) {
        LOG_ERR("i2s_trigger(START) failed: %d", ret);
        return ret;
    }

    LOG_INF("Audio playback started: %u Hz, %u ch, %u-bit, samples=%u",
            SAMPLE_RATE_HZ, CHANNELS, WORD_SIZE_BITS, (uint32_t)AUDIO_TRACK_SAMPLE_COUNT);

    /* 边播边继续喂数据，直到播完整段 audio_track[] */
    while (sample_idx < AUDIO_TRACK_SAMPLE_COUNT) {
        void *blk = NULL;

        ret = k_mem_slab_alloc(&tx_slab, &blk, K_FOREVER);
        if (ret < 0) {
            LOG_ERR("k_mem_slab_alloc failed: %d", ret);
            return ret;
        }

        size_t remain = AUDIO_TRACK_SAMPLE_COUNT - sample_idx;
        size_t to_copy = (remain > SAMPLES_PER_BLOCK) ? SAMPLES_PER_BLOCK : remain;

        memcpy(blk, &audio_track[sample_idx], to_copy * sizeof(int16_t));
        if (to_copy < SAMPLES_PER_BLOCK) {
            memset((uint8_t *)blk + to_copy * sizeof(int16_t), 0,
                   (SAMPLES_PER_BLOCK - to_copy) * sizeof(int16_t));
        }

        sample_idx += to_copy;

        ret = i2s_write(i2s_dev, blk, BLOCK_SIZE_BYTES);
        if (ret < 0) {
            LOG_ERR("i2s_write failed: %d", ret);
            return ret;
        }
    }

    /* 播完：DRAIN 会把队列里剩余的播完再停 */
    ret = i2s_trigger(i2s_dev, I2S_DIR_TX, I2S_TRIGGER_DRAIN);
    if (ret < 0) {
        LOG_ERR("i2s_trigger(DRAIN) failed: %d", ret);
        return ret;
    }

    LOG_INF("Audio drain requested");
    return 0;
}

void main(void)
{
    int ret = audio_i2s_start();
    if (ret < 0) {
        LOG_ERR("playback failed: %d", ret);
    } else {
        LOG_INF("playback finished");
    }

    while (1) {
        k_msleep(1000);
    }
}

