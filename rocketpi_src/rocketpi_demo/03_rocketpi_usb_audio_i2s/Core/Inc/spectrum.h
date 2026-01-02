#ifndef SPECTRUM_H
#define SPECTRUM_H

#include <stddef.h>
#include <stdint.h>

void Spectrum_Init(uint32_t sample_rate_hz);
void Spectrum_UpdateFromInterleaved(const int16_t *samples,
                                    size_t sample_count,
                                    uint32_t channels);
void Spectrum_DrawIfDue(void);
uint32_t Spectrum_GetBinCount(void);
uint16_t Spectrum_GetLevel(uint32_t index);
uint16_t Spectrum_GetBinColor(uint32_t index);
uint16_t Spectrum_GetMaxHeight(void);

#endif /* SPECTRUM_H */
