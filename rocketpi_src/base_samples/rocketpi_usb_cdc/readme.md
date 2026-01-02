

## 效果展示

![usb_cdc](https://cloud.rocketpi.club/cloud/usb_cdc.gif)

## 功能描述

USB CDC 回环示例

将 STM32F401 配置成 USB CDC（虚拟串口）回环设备。执行 ``MX_USB_DEVICE_Init()`` 后，开发板会以 VCP 形式枚举至 PC，用于验证主机与 MCU 之间的收发链路。

- 上电后固件只发送一次提示:CDC echo demo ready. Type to see loopback.
- 串口终端输入的每帧数据都会由 CDC 驱动缓存，并通过 ``CDC_Transmit_FS()`` 原样回传。
- 适合快速确认 USB 协议栈、时钟以及端点配置是否正常。

## 硬件连接

![image-20251216002759090](https://cloud.rocketpi.club/cloud/image-20251216002759090.png)

![image-20251216003522059](https://cloud.rocketpi.club/cloud/image-20251216003522059.png)

![image-20251216002138462](https://cloud.rocketpi.club/cloud/image-20251216002138462.png)
