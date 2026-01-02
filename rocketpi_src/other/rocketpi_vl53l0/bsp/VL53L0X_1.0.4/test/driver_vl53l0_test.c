/**
 * Copyright (c) 2025
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @file      driver_vl53l0_test.c
 * @brief     VL53L0X driver test helper
 * @version   1.0.0
 * @date      2025-10-29
 */

#include "driver_vl53l0_test.h"

#include "vl53l0x_platform.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "stm32f4xx_hal.h"

typedef struct
{
    VL53L0X_Dev_t device;
    uint8_t initialised;
} vl53l0x_test_context_t;

static vl53l0x_test_context_t g_vl53l0x_ctx;

static void vl53l0x_test_log(const char *format, ...);
static uint8_t vl53l0x_test_check_status(VL53L0X_Error status, const char *context);
static uint32_t vl53l0x_test_fix1616_to_milli(FixPoint1616_t value);
static uint32_t vl53l0x_test_effective_spad_to_milli(uint16_t spad_counts);
static void vl53l0x_test_print_measurement(uint32_t index, const VL53L0X_RangingMeasurementData_t *measurement);

static void vl53l0x_test_log(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

static uint8_t vl53l0x_test_check_status(VL53L0X_Error status, const char *context)
{
    if (status == VL53L0X_ERROR_NONE)
    {
        return 0U;
    }
    else
    {
        char message[VL53L0X_MAX_STRING_LENGTH];

        VL53L0X_GetPalErrorString(status, message);
        vl53l0x_test_log("vl53l0x: %s failed -> (%d) %s\r\n",
                         context,
                         (int)status,
                         message);

        return 1U;
    }
}

static uint32_t vl53l0x_test_fix1616_to_milli(FixPoint1616_t value)
{
    return (uint32_t)(((uint64_t)value * 1000U + 32768U) >> 16);
}

static uint32_t vl53l0x_test_effective_spad_to_milli(uint16_t spad_counts)
{
    return (((uint32_t)spad_counts * 1000U) + 128U) >> 8;
}

static void vl53l0x_test_print_measurement(uint32_t index, const VL53L0X_RangingMeasurementData_t *measurement)
{
    char status_text[VL53L0X_MAX_STRING_LENGTH];
    uint32_t distance_fraction;
    uint32_t signal_mcps_milli;
    uint32_t ambient_mcps_milli;
    uint32_t spad_milli;

    VL53L0X_GetRangeStatusString(measurement->RangeStatus, status_text);

    distance_fraction = ((uint32_t)measurement->RangeFractionalPart * 1000U + 128U) >> 8;
    signal_mcps_milli = vl53l0x_test_fix1616_to_milli(measurement->SignalRateRtnMegaCps);
    ambient_mcps_milli = vl53l0x_test_fix1616_to_milli(measurement->AmbientRateRtnMegaCps);
    spad_milli = vl53l0x_test_effective_spad_to_milli(measurement->EffectiveSpadRtnCount);

    vl53l0x_test_log("vl53l0x: sample %lu -> %u.%03u mm (status=%u:%s)\r\n",
                     (unsigned long)(index + 1U),
                     (unsigned int)measurement->RangeMilliMeter,
                     (unsigned int)distance_fraction,
                     (unsigned int)measurement->RangeStatus,
                     status_text);

    vl53l0x_test_log("          signal=%lu.%03lu MCPS ambient=%lu.%03lu MCPS spads=%lu.%03lu time=%lu us\r\n",
                     (unsigned long)(signal_mcps_milli / 1000U),
                     (unsigned long)(signal_mcps_milli % 1000U),
                     (unsigned long)(ambient_mcps_milli / 1000U),
                     (unsigned long)(ambient_mcps_milli % 1000U),
                     (unsigned long)(spad_milli / 1000U),
                     (unsigned long)(spad_milli % 1000U),
                     (unsigned long)measurement->MeasurementTimeUsec);
}

uint8_t vl53l0x_test_init_default(void)
{
    VL53L0X_Error status = VL53L0X_ERROR_NONE;
    VL53L0X_Version_t version;
    VL53L0X_DeviceInfo_t info;
    uint32_t ref_spad_count = 0U;
    uint8_t is_aperture_spads = 0U;
    uint8_t vhv_settings = 0U;
    uint8_t phase_cal = 0U;
    int32_t comms_status;

    if (g_vl53l0x_ctx.initialised != 0U)
    {
        return 0U;
    }

    memset(&g_vl53l0x_ctx, 0, sizeof(g_vl53l0x_ctx));
    g_vl53l0x_ctx.device.I2cDevAddr = VL53L0X_TEST_DEFAULT_I2C_ADDR;
    g_vl53l0x_ctx.device.comms_type = 1U;
    g_vl53l0x_ctx.device.comms_speed_khz = VL53L0X_TEST_DEFAULT_I2C_SPEED_KHZ;

    comms_status = VL53L0X_comms_initialise(g_vl53l0x_ctx.device.comms_type,
                                            g_vl53l0x_ctx.device.comms_speed_khz);
    if (comms_status != 0)
    {
        vl53l0x_test_log("vl53l0x: VL53L0X_comms_initialise failed (status=%ld)\r\n", (long)comms_status);
        return 1U;
    }

    status = VL53L0X_GetVersion(&version);
    if (vl53l0x_test_check_status(status, "VL53L0X_GetVersion") != 0U)
    {
        goto init_failed;
    }

    vl53l0x_test_log("vl53l0x: API version %u.%u.%u (rev %lu)\r\n",
                     version.major,
                     version.minor,
                     version.build,
                     (unsigned long)version.revision);

    status = VL53L0X_DataInit(&g_vl53l0x_ctx.device);
    if (vl53l0x_test_check_status(status, "VL53L0X_DataInit") != 0U)
    {
        goto init_failed;
    }

    status = VL53L0X_GetDeviceInfo(&g_vl53l0x_ctx.device, &info);
    if (vl53l0x_test_check_status(status, "VL53L0X_GetDeviceInfo") != 0U)
    {
        goto init_failed;
    }

    vl53l0x_test_log("vl53l0x: device %s (%s) rev %u.%u id=%s\r\n",
                     info.Name,
                     info.Type,
                     info.ProductRevisionMajor,
                     info.ProductRevisionMinor,
                     info.ProductId);

    status = VL53L0X_StaticInit(&g_vl53l0x_ctx.device);
    if (vl53l0x_test_check_status(status, "VL53L0X_StaticInit") != 0U)
    {
        goto init_failed;
    }

    status = VL53L0X_PerformRefCalibration(&g_vl53l0x_ctx.device, &vhv_settings, &phase_cal);
    if (vl53l0x_test_check_status(status, "VL53L0X_PerformRefCalibration") != 0U)
    {
        goto init_failed;
    }
    vl53l0x_test_log("vl53l0x: ref calibration done (VHV=%u, PhaseCal=%u)\r\n",
                     vhv_settings,
                     phase_cal);

    status = VL53L0X_PerformRefSpadManagement(&g_vl53l0x_ctx.device, &ref_spad_count, &is_aperture_spads);
    if (vl53l0x_test_check_status(status, "VL53L0X_PerformRefSpadManagement") != 0U)
    {
        goto init_failed;
    }
    vl53l0x_test_log("vl53l0x: ref spads=%lu aperture=%u\r\n",
                     (unsigned long)ref_spad_count,
                     (unsigned int)is_aperture_spads);

    status = VL53L0X_SetDeviceMode(&g_vl53l0x_ctx.device, VL53L0X_DEVICEMODE_SINGLE_RANGING);
    if (vl53l0x_test_check_status(status, "VL53L0X_SetDeviceMode") != 0U)
    {
        goto init_failed;
    }

    status = VL53L0X_SetLimitCheckEnable(&g_vl53l0x_ctx.device,
                                         VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE,
                                         1);
    if (vl53l0x_test_check_status(status, "VL53L0X_SetLimitCheckEnable(SIGMA)") != 0U)
    {
        goto init_failed;
    }

    status = VL53L0X_SetLimitCheckEnable(&g_vl53l0x_ctx.device,
                                         VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE,
                                         1);
    if (vl53l0x_test_check_status(status, "VL53L0X_SetLimitCheckEnable(SIGNAL)") != 0U)
    {
        goto init_failed;
    }

    status = VL53L0X_SetMeasurementTimingBudgetMicroSeconds(&g_vl53l0x_ctx.device, 20000U);
    if (status == VL53L0X_ERROR_NOT_SUPPORTED)
    {
        vl53l0x_test_log("vl53l0x: timing budget configuration not supported, keep default\r\n");
        status = VL53L0X_ERROR_NONE;
    }
    if (vl53l0x_test_check_status(status, "VL53L0X_SetMeasurementTimingBudgetMicroSeconds") != 0U)
    {
        goto init_failed;
    }

    g_vl53l0x_ctx.initialised = 1U;
    return 0U;

init_failed:
    (void)VL53L0X_comms_close();
    memset(&g_vl53l0x_ctx, 0, sizeof(g_vl53l0x_ctx));

    return 1U;
}

uint8_t vl53l0x_test_perform_single(VL53L0X_RangingMeasurementData_t *measurement)
{
    VL53L0X_Error status = VL53L0X_ERROR_NONE;

    if (measurement == NULL)
    {
        return 1U;
    }

    if (vl53l0x_test_init_default() != 0U)
    {
        return 1U;
    }

    status = VL53L0X_PerformSingleRangingMeasurement(&g_vl53l0x_ctx.device, measurement);
    if (vl53l0x_test_check_status(status, "VL53L0X_PerformSingleRangingMeasurement") != 0U)
    {
        return 1U;
    }

    return 0U;
}

uint8_t vl53l0x_test_run(uint32_t sample_count, uint32_t interval_ms)
{
    uint32_t i;

    if (sample_count == 0U)
    {
        sample_count = 1U;
    }

    if (vl53l0x_test_init_default() != 0U)
    {
        return 1U;
    }

    vl53l0x_test_log("vl53l0x: starting ranging test (%lu samples, interval=%lu ms)\r\n",
                     (unsigned long)sample_count,
                     (unsigned long)interval_ms);

    for (i = 0U; i < sample_count; ++i)
    {
        VL53L0X_RangingMeasurementData_t measurement;

        if (vl53l0x_test_perform_single(&measurement) != 0U)
        {
            vl53l0x_test_log("vl53l0x: measurement %lu failed\r\n", (unsigned long)(i + 1U));
            return 1U;
        }

        vl53l0x_test_print_measurement(i, &measurement);

        if ((interval_ms > 0U) && ((i + 1U) < sample_count))
        {
            HAL_Delay(interval_ms);
        }
    }

    vl53l0x_test_log("vl53l0x: test complete\r\n");

    return 0U;
}

void vl53l0x_test_deinit(void)
{
    if (g_vl53l0x_ctx.initialised != 0U)
    {
        (void)VL53L0X_comms_close();
        memset(&g_vl53l0x_ctx, 0, sizeof(g_vl53l0x_ctx));
    }
}
