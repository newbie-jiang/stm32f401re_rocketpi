# reviwe

## base_samples   27

- [x] rocketpi_led  （LED闪烁示例）
- [x] rocketpi_printf （三种不同方式打印输出）
- [ ] rocketpi_key_scan （阻塞式按键扫描）
- [ ] rocketpi_key_irq （中断按键）
- [ ] rocketpi_delay_us（io翻转测量微秒延时）
- [ ] rocketpi_aht30_hardware_i2c_base（aht30温湿度传感器驱动示例）
- [ ]  rocketpi_at24xx_soft_i2c（EEPROM AT24C02的驱动示例）
- [ ] rocketpi_sg90(sg90 舵机来回转动示例，同时串口打印角度值)
- [ ] rocketpi_hcsr04（超声波测距）
- [ ] rocketpi_radar（5.8G微波雷达，UART 通信，已写好库，可读取和设置参数）
- [ ] rocketpi_pwm_passive_buzzer（无源蜂鸣器播放音乐示例）
- [ ] rocketpi_irda（nec 红外解码）
- [ ] rocketpi_motor（电机驱动示例，正反转，启动，停止）
- [ ] rocketpi_ws2812b（可编程LED 30颗灯珠，不同色彩模式驱动示例）
- [ ] rocketpi_spi_lcd_bitmap（1.44寸 240x240 ST7789屏幕驱动，帧率测试，以及将图片显示在屏幕）
- [ ] rocketpi_spi_lcd_240x240_lvgl（邪修移植lvgl并适配）
- [ ] rocketpi_i2s（i2s音频驱动，带上位机，任意字符转音频，支持简体中文，粤语，英语，以及任意音频）
- [ ] rocketpi_sdio_card（sdio tf卡驱动示例，读写测试）
- [ ] rocketpi_sdio_card_fatfs（sdio tf卡  fatfs文件系统驱动示例，读写测试）
- [ ] rocketpi_sd_audio_to_i2s（从tf卡中读取音频，播放，屏幕显示频谱图）
- [ ] rocketpi_sd_pic_to_lcd（从sd卡读取轮询图片，显示在屏幕上，动图效果，6fps）
- [ ] rocketpi_standby_wkup（低功耗的 standby模式，按键唤醒）
- [ ] rocketpi_usb_cdc（usb 虚拟串口）
- [ ] rocketpi_usb_msc（usb虚拟U盘，使用内部的sram做介质）
- [ ] rocketpi_w25qxx（w26q64jv的驱动示例）
- [ ] rocketpi_flash_littlefs（内部flash+ littlefs文件系统演示示例）
- [ ] rocketpi_w25qxx_littlefs（外部 nor flash w25q64jv + littlefs文件系统演示示例）

## boot  2

- [ ] bootloader （纯bootloader 跳转基础示例）

- [ ] bootloader_iap（可使用uart升级的示例 使用ymodem协议）

## rocketpi_demo  2

- [ ] 01_rocketpi_serial_plane_game（遥杆游戏示例）
- [ ] 02_don't_approach_me_alarm（预警雷达示例，可使用红外遥控器设置感应距离）

## rtos  4

- [ ] rocketpi_freertos （提供freertos示例，基于cubemx生成的，以及手工移植的keil和gcc示例）

- [ ] rocketpi_rt-thread（rt-thread 基础示例 创建三个任务，三个led翻转）

- [ ] rocketpi_threadx（threadx 基础示例 创建三个任务，三个led翻转）

- [ ] rocketpi_zephyr（适配zephyr绝大部分驱动，10+测试程序）