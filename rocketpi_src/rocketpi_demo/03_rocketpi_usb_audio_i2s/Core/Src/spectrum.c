#include "spectrum.h"

#include <math.h>
#include <string.h>

#include "main.h"
#include "st7789.h"

#define SPECTRUM_BIN_COUNT          40U
#define SPECTRUM_WINDOW_FRAMES      256U
#define SPECTRUM_DRAW_INTERVAL_MS   40U
#define SPECTRUM_MAX_HEIGHT         (ST7789_HIGHT - 10U)
#define SPECTRUM_BAR_GAP            2U
#define SPECTRUM_SMOOTH_ALPHA       0.30f
#define SPECTRUM_DECAY_ALPHA        0.90f
#define SPECTRUM_PEAK_INIT          2000.0f

#ifndef M_PI
#define M_PI 3.1415926f
#endif

typedef struct
{
  float pos;
  uint16_t color;
} spectrum_stop_t;

static const spectrum_stop_t spectrum_gradient[] = {
  {0.0f, BLUE},
  {0.25f, CYAN},
  {0.50f, GREEN},
  {0.75f, YELLOW},
  {1.0f, RED}
};

static uint32_t spectrum_sample_rate = 16000U;
static float spectrum_coeff[SPECTRUM_BIN_COUNT];
static float spectrum_smooth[SPECTRUM_BIN_COUNT];
static uint16_t spectrum_levels[SPECTRUM_BIN_COUNT];
static uint16_t spectrum_draw_levels[SPECTRUM_BIN_COUNT];
static float spectrum_peak_avg = SPECTRUM_PEAK_INIT;
static uint32_t spectrum_last_draw_ms = 0U;

static uint16_t Spectrum_LerpColor(uint16_t c0, uint16_t c1, float t);
static uint16_t Spectrum_ColorForBin(uint32_t index);

void Spectrum_Init(uint32_t sample_rate_hz)
{
  if (sample_rate_hz > 0U)
  {
    spectrum_sample_rate = sample_rate_hz;
  }

  const float bin_width_hz = ((float)spectrum_sample_rate * 0.5f) / (float)SPECTRUM_BIN_COUNT;

  for (uint32_t i = 0U; i < SPECTRUM_BIN_COUNT; ++i)
  {
    float freq = bin_width_hz * (float)(i + 1U);
    float omega = 2.0f * (float)M_PI * freq / (float)spectrum_sample_rate;
    spectrum_coeff[i] = 2.0f * cosf(omega);
    spectrum_smooth[i] = 0.0f;
    spectrum_levels[i] = 0U;
    spectrum_draw_levels[i] = 0U;
  }

  spectrum_peak_avg = SPECTRUM_PEAK_INIT;
  spectrum_last_draw_ms = HAL_GetTick();

  ST7789_Clear(BLACK);
}

void Spectrum_UpdateFromInterleaved(const int16_t *samples,
                                    size_t sample_count,
                                    uint32_t channels)
{
  if ((samples == NULL) || (sample_count == 0U))
  {
    return;
  }

  if (channels == 0U)
  {
    channels = 1U;
  }

  size_t frames = sample_count / channels;
  size_t window = (frames < (size_t)SPECTRUM_WINDOW_FRAMES) ? frames : (size_t)SPECTRUM_WINDOW_FRAMES;
  if (window < 8U)
  {
    return;
  }

  for (uint32_t bin = 0U; bin < SPECTRUM_BIN_COUNT; ++bin)
  {
    float coeff = spectrum_coeff[bin];
    float q0 = 0.0f;
    float q1 = 0.0f;
    float q2 = 0.0f;

    for (size_t n = 0U; n < window; ++n)
    {
      float x = (float)samples[n * channels];
      q0 = (coeff * q1) - q2 + x;
      q2 = q1;
      q1 = q0;
    }

    float power = (q1 * q1) + (q2 * q2) - (coeff * q1 * q2);
    if (power < 0.0f)
    {
      power = 0.0f;
    }
    float mag = sqrtf(power) / (float)window;

    spectrum_peak_avg = (SPECTRUM_DECAY_ALPHA * spectrum_peak_avg) +
                        ((1.0f - SPECTRUM_DECAY_ALPHA) * mag);
    float scaled = (mag * (float)SPECTRUM_MAX_HEIGHT) / (spectrum_peak_avg + 1.0f);

    if (scaled < 0.0f)
    {
      scaled = 0.0f;
    }
    if (scaled > (float)SPECTRUM_MAX_HEIGHT)
    {
      scaled = (float)SPECTRUM_MAX_HEIGHT;
    }

    float smooth = (1.0f - SPECTRUM_SMOOTH_ALPHA) * spectrum_smooth[bin] +
                   (SPECTRUM_SMOOTH_ALPHA * scaled);
    spectrum_smooth[bin] = smooth;
    spectrum_levels[bin] = (uint16_t)smooth;
  }
}

void Spectrum_DrawIfDue(void)
{
  uint32_t now = HAL_GetTick();
  if ((now - spectrum_last_draw_ms) < SPECTRUM_DRAW_INTERVAL_MS)
  {
    return;
  }
  spectrum_last_draw_ms = now;

  const uint16_t bar_width = ST7789_WIDTH / SPECTRUM_BIN_COUNT;
  uint16_t usable_width = (bar_width > SPECTRUM_BAR_GAP) ? (bar_width - SPECTRUM_BAR_GAP) : bar_width;
  if (usable_width == 0U)
  {
    usable_width = 1U;
  }
  const uint16_t y_base = ST7789_HIGHT - 1U;

  for (uint32_t i = 0U; i < SPECTRUM_BIN_COUNT; ++i)
  {
    uint16_t h = spectrum_levels[i];
    uint16_t x0 = (uint16_t)(i * bar_width);
    x0 = (uint16_t)(x0 + ((bar_width - usable_width) / 2U));
    uint16_t x1 = x0 + usable_width - 1U;
    if (x1 >= ST7789_WIDTH)
    {
      x1 = ST7789_WIDTH - 1U;
    }

    ST7789_FillRect(x0, 0U, x1, y_base, BLACK);

    if (h > 0U)
    {
      uint16_t y0 = (h >= ST7789_HIGHT) ? 0U : (uint16_t)(ST7789_HIGHT - h);
      uint16_t color = Spectrum_ColorForBin(i);
      ST7789_FillRect(x0, y0, x1, y_base, color);
    }

    spectrum_draw_levels[i] = h;
  }
}

static uint16_t Spectrum_LerpColor(uint16_t c0, uint16_t c1, float t)
{
  if (t < 0.0f) t = 0.0f;
  if (t > 1.0f) t = 1.0f;

  float r0 = (float)((c0 >> 11) & 0x1FU);
  float g0 = (float)((c0 >> 5) & 0x3FU);
  float b0 = (float)(c0 & 0x1FU);

  float r1 = (float)((c1 >> 11) & 0x1FU);
  float g1 = (float)((c1 >> 5) & 0x3FU);
  float b1 = (float)(c1 & 0x1FU);

  float r = r0 + (r1 - r0) * t;
  float g = g0 + (g1 - g0) * t;
  float b = b0 + (b1 - b0) * t;

  uint16_t ri = (uint16_t)(r + 0.5f);
  uint16_t gi = (uint16_t)(g + 0.5f);
  uint16_t bi = (uint16_t)(b + 0.5f);

  if (ri > 0x1F) ri = 0x1F;
  if (gi > 0x3F) gi = 0x3F;
  if (bi > 0x1F) bi = 0x1F;

  return (uint16_t)((ri << 11) | (gi << 5) | bi);
}

static uint16_t Spectrum_ColorForBin(uint32_t index)
{
  const size_t stop_count = sizeof(spectrum_gradient) / sizeof(spectrum_gradient[0]);
  if (stop_count == 0U)
  {
    return WHITE;
  }

  float t = (SPECTRUM_BIN_COUNT > 1U) ?
            ((float)index / (float)(SPECTRUM_BIN_COUNT - 1U)) : 0.0f;
  if (t <= spectrum_gradient[0].pos)
  {
    return spectrum_gradient[0].color;
  }
  if (t >= spectrum_gradient[stop_count - 1U].pos)
  {
    return spectrum_gradient[stop_count - 1U].color;
  }

  for (size_t i = 0U; i < (stop_count - 1U); ++i)
  {
    float p0 = spectrum_gradient[i].pos;
    float p1 = spectrum_gradient[i + 1U].pos;
    if ((t >= p0) && (t <= p1))
    {
      float local_t = (p1 > p0) ? ((t - p0) / (p1 - p0)) : 0.0f;
      return Spectrum_LerpColor(spectrum_gradient[i].color,
                                spectrum_gradient[i + 1U].color,
                                local_t);
    }
  }

  return spectrum_gradient[stop_count - 1U].color;
}

uint32_t Spectrum_GetBinCount(void)
{
  return SPECTRUM_BIN_COUNT;
}

uint16_t Spectrum_GetLevel(uint32_t index)
{
  if (index >= SPECTRUM_BIN_COUNT)
  {
    return 0U;
  }

  return spectrum_levels[index];
}

uint16_t Spectrum_GetBinColor(uint32_t index)
{
  return Spectrum_ColorForBin(index);
}

uint16_t Spectrum_GetMaxHeight(void)
{
  return SPECTRUM_MAX_HEIGHT;
}
