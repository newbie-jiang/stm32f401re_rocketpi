## 功能说明

基于 STM32F401 的引导加载器与应用示例，可通过 Keil µVision 的 `rocketpi_bootloader.uvmpw` 工作空间一次性打开 `boot` 与 `app` 两个工程，方便调试与下载。

1. **Boot 工程**  
   - 固定应用起始地址为 `0x08020000`，最大占用 `0x60000` 字节。  
   - 上电后读取应用向量表的初始栈指针与复位入口，确认栈指针位于 SRAM (`0x20000000` ~ `0x20017FFF`) 且复位入口位于应用地址范围内。  
   - 校验通过即清理时钟/SysTick 状态、重定位 VTOR，并跳转到应用入口函数；

2. **App 工程**  
   - 将 `SCB->VTOR` 重定位到 `0x08020000` 以匹配 Boot 的跳转策略。  
   - 初始化 GPIOA，引导示例每 500 ms 翻转一次 `GPIO_PIN_1` 灯，验证应用被正确执行。

目录结构

- `boot/`：引导加载器源码与 `MDK-ARM` 工程文件。  
- `app/`：用户应用源码与 `MDK-ARM` 工程文件，`app/readme.md` 记录了相同的起始地址信息。  
- `rocketpi_bootloader.uvmpw`：µVision 工作空间文件，直接双击即可加载全部目标。

使用说明

1. 通过 µVision 打开根目录下的 `rocketpi_bootloader.uvmpw`。  
2. 依次编译并下载 `boot` 与 `app` 工程（推荐先下载 Boot，再下载 App）。  
3. 复位开发板，如能看到 `PA1` LED 闪烁，即表示 Boot 完成应用校验与跳转，App 正常运行。

![image-20251215234616302](https://cloud.rocketpi.club/cloud/image-20251215234616302.png)
