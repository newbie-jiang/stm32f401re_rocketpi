## 效果展示

![usb_msc](https://cloud.rocketpi.club/cloud/usb_msc.gif)

## 功能说明
- 使用 64 KB 片上 SRAM 作为 USB MSC 介质，块大小 512 字节，共 128 个扇区
- `usbd_storage_if.c` 直接在 RAM 数组上实现 MSC 读写回调；掉电即失效，适合调试或临时数据交换。
- `MSC_FlashStorage_Init()` 会在首次调用时将 RAM 盘填充为 `0xFF` 并标记为可用。

使用步骤

1. `main.c` 已在 `MX_USB_DEVICE_Init()` 前调用 `MSC_FlashStorage_Init()`，上电后即可被 PC 枚举为 64 KB U 盘。
2. 首次连接时若提示“未格式化”，可在 PC 端使用 FAT/FAT12 快速格式化；RAM 盘容量仍较小，请勿选择 NTFS/exFAT。
3. 每次复位或断电后数据都会丢失，如需持久化请手动备份到 PC。

代码改动

- `USB_DEVICE/App/usbd_storage_if.c`：实现 48 KB RAM 盘、范围检查和 MSC 回调逻辑。
- `USB_DEVICE/App/usbd_storage_if.h`：保留 `MSC_FlashStorage_Init()` 对外接口。
- `Core/Src/main.c`：在 USB 初始化之前调用 RAM 盘初始化函数。



## 硬件连接

![image-20251216003522059](https://cloud.rocketpi.club/cloud/image-20251216003522059.png)



![image-20251216004213540](https://cloud.rocketpi.club/cloud/image-20251216004213540.png)

![image-20251216004309579](https://cloud.rocketpi.club/cloud/image-20251216004309579.png)
