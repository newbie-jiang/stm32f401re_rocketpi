#include "unit_test.h"
#include "fatfs.h"
#include <inttypes.h>

static void FATFS_Test(void);
static bool card_update_space_info(const TCHAR *path);
static void card_info_reset(void);

static unit_test_card_info_t g_card_info = {
    .initialized = false,
    .filesystem_available = false,
    .test_passed = false,
    .total_mb = 0U,
    .free_mb = 0U};

void card_Task(void *argument)
{
  	osDelay(1000);
    FATFS_Test();

    for (;;)
    {
        osDelay(500);
				osThreadExit(); // 等价于 vTaskDelete(NULL)
    }
}


#ifndef _USE_MKFS
#define _USE_MKFS 1
#endif

#if (_USE_MKFS == 1)
#define FATFS_MKFS_BUFFER_SIZE 4096U
#endif
#define FATFS_SPEED_TEST_BUFFER_SIZE 4096U


static bool card_update_space_info(const TCHAR *path)
{
    FATFS *fs;
    DWORD fre_clust;

    FRESULT fr = f_getfree(path, &fre_clust, &fs);
    if (fr != FR_OK)
    {
        elog_e("card", "f_getfree failed: %d", (int)fr);
        g_card_info.total_mb = 0U;
        g_card_info.free_mb = 0U;
        return false;
    }

    DWORD tot_sect = (fs->n_fatent - 2U) * fs->csize;
    DWORD fre_sect = fre_clust * fs->csize;

    uint64_t tot_bytes = (uint64_t)tot_sect * 512ULL;
    uint64_t fre_bytes = (uint64_t)fre_sect * 512ULL;

    uint64_t tot_mb = tot_bytes / (1024ULL * 1024ULL);
    uint64_t fre_mb = fre_bytes / (1024ULL * 1024ULL);

    g_card_info.total_mb = (uint32_t)tot_mb;
    g_card_info.free_mb = (uint32_t)fre_mb;

    elog_i("card", "Total: %" PRIu64 " MB, Free: %" PRIu64 " MB", tot_mb, fre_mb);
    return true;
}

static void card_info_reset(void)
{
    g_card_info.initialized = false;
    g_card_info.filesystem_available = false;
    g_card_info.test_passed = false;
    g_card_info.total_mb = 0U;
    g_card_info.free_mb = 0U;
}


static void FATFS_Test(void)
{
  const char test_file[] = "rocketpi.txt";
  const char test_payload[] = "RocketPi FATFS SDIO write/read demo.\r\n";
  char read_buffer[sizeof(test_payload)] = {0};
  const UINT payload_len = (UINT)strlen(test_payload);
  UINT bytes_written = 0;
  UINT bytes_read = 0;
  FRESULT res;
  uint8_t is_mounted = 0;
  uint8_t file_opened = 0;
  bool filesystem_ready = false;
  card_info_reset();

  elog_i("card", "fatfs: mounting %s", SDPath);
  res = f_mount(&SDFatFS, (TCHAR const*)SDPath, 1);
  if (res == FR_OK)
  {
    is_mounted = 1;
    filesystem_ready = true;
  }
  else if (res == FR_NO_FILESYSTEM)
  {
#if (_USE_MKFS == 1)
#if (FATFS_MKFS_BUFFER_SIZE < 1024U)
#error "FATFS_MKFS_BUFFER_SIZE must be >= 1024 bytes"
#endif
    static uint8_t mkfs_work[FATFS_MKFS_BUFFER_SIZE];

    elog_i("card", "fatfs: no filesystem, formatting...");
    res = f_mkfs((TCHAR const*)SDPath, FM_ANY, 0, mkfs_work, sizeof(mkfs_work));
    if (res != FR_OK)
    {
      elog_e("card", "fatfs: format failed (%d)", (int)res);
      goto cleanup;
    }

    elog_i("card", "fatfs: format complete, remounting");
    res = f_mount(&SDFatFS, (TCHAR const*)SDPath, 1);
    if (res != FR_OK)
    {
      elog_e("card", "fatfs: mount after format failed (%d)", (int)res);
      goto cleanup;
    }
    is_mounted = 1;
    filesystem_ready = true;
#else
    elog_w("card", "fatfs: no filesystem and mkfs disabled");
    goto cleanup;
#endif
  }
  else
  {
    elog_e("card", "fatfs: mount failed (%d)", (int)res);
    goto cleanup;
  }

  elog_i("card", "fatfs: creating %s", test_file);
  res = f_open(&SDFile, test_file, FA_CREATE_ALWAYS | FA_WRITE);
  if (res != FR_OK)
  {
    elog_e("card", "fatfs: open for write failed (%d)", (int)res);
    goto cleanup;
  }
  file_opened = 1;

  res = f_write(&SDFile, test_payload, payload_len, &bytes_written);
  if ((res != FR_OK) || (bytes_written != payload_len))
  {
    elog_e("card", "fatfs: write failed (%d)", (int)res);
    goto cleanup;
  }
  elog_i("card", "fatfs: wrote %lu bytes", (unsigned long)bytes_written);

  f_close(&SDFile);
  file_opened = 0;

  res = f_open(&SDFile, test_file, FA_READ);
  if (res != FR_OK)
  {
    elog_e("card", "fatfs: open for read failed (%d)", (int)res);
    goto cleanup;
  }
  file_opened = 1;

  memset(read_buffer, 0, sizeof(read_buffer));
  res = f_read(&SDFile, read_buffer, sizeof(read_buffer) - 1, &bytes_read);
  if (res != FR_OK)
  {
    elog_e("card", "fatfs: read failed (%d)", (int)res);
    goto cleanup;
  }
  elog_i("card", "fatfs: read %lu bytes", (unsigned long)bytes_read);

  card_update_space_info(SDPath);
	
  f_close(&SDFile);
  file_opened = 0;

  if ((bytes_read == bytes_written) && (strncmp(read_buffer, test_payload, payload_len) == 0))
  {
    elog_i("card", "fatfs: verification OK");
    elog_i("card", "fatfs: content: %s", read_buffer);
    g_card_info.test_passed = true;
  }
  else
  {
    elog_e("card", "fatfs: verification failed");
    g_card_info.test_passed = false;
  }

cleanup:
  if (file_opened)
  {
    f_close(&SDFile);
  }

  if (is_mounted)
  {
    f_mount(NULL, (TCHAR const*)SDPath, 0);
    elog_i("card", "fatfs: unmounted");
  }

  g_card_info.filesystem_available = filesystem_ready;
  g_card_info.initialized = true;
}

void unit_test_card_get_info(unit_test_card_info_t *info)
{
    if (info == NULL)
    {
        return;
    }

    *info = g_card_info;
}
