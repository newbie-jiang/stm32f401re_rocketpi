# 当前共计100+示例程序

- Rocket-Pi 开发套件 (STM32F401RE | 512K FLASH | 96K SRAM)
- 更多信息访问  https://www.rocketpi.club/

## factory(厂测程序)

- [x] factory (开发板综合测试，不包含外部模块)

## base_samples   （42个示例）

- [x] rocketpi_led  （LED闪烁示例）
- [x] rocketpi_key_scan （阻塞式按键扫描）
- [x] rocketpi_key_irq （中断按键）
- [x] rocketpi_key_multi_button（MultiButton按键库，适配短按，双击，长按）
- [x] rocketpi_delay_us  （io翻转测量微秒延时）
- [x] rocketpi_uart_printf （三种不同方式打印输出）
- [x] rocketpi_uart_echo （USART2 + DMA 空闲中断回显示例）
- [x] rocketpi_uart_control_led（模拟cjson格式数据，串口控制三个led灯）
- [x] rocketpi_uart_control_led_cjson（使用cjson库，串口控制三个led灯）
- [x] rocketpi_uart_ymodem（uart ymodem 发送与接收实现）
- [x] rocketpi_uart_shell_microrl(microrl shell适配，支持自动补全，历史命令，控制led实例)
- [x] rocketpi_uart_easylogger （适配easylogger日志，支持不同颜色level日志打印）
- [x] rocketpi_uart_radar（5.8G微波雷达，USART1 读取并配置参数，同时检测感应状态）
- [x] rocketpi_i2c_aht30（aht30温湿度传感器驱动示例，一套代码支持软硬件i2c）
- [x] rocketpi_i2c_at24cxx（EEPROM AT24C02的libdriver读写示例,使用软件i2c）
- [x] rocketpi_pwm_passive_buzzer（无源蜂鸣器播放音乐示例）
- [x] rocketpi_pwm_sg90(sg90 舵机来回转动，同时串口打印角度值)
- [x] rocketpi_pwm_motor(电机驱动：循环慢速→加速→刹车→反转→加速→刹车)
- [x] rocketpi_hcsr04（hcsr04超声波测距，串口打印距离）
- [x] rocketpi_adc_mcu_temperature（获取芯片内部温度，中值滤波后打印出来）
- [x] rocketpi_adc_joystick（ADC双轴遥杆，打印原始12位adc采样值，支持按键）
- [x] rocketpi_irda（nec 红外解码，串口打印解码值）
- [x] rocketpi_ws2812b（ws2812b 可编程LED 30颗灯珠，不同色彩模式驱动示例） 视频待补充
- [x] rocketpi_spi_lcd_speedtest（1.44寸 spi lcd 240x240 帧率测试）
- [x] rocketpi_spi_lcd_bitmap（1.44寸 240x240 ST7789屏幕驱动，帧率测试，以及将图片显示在屏幕）
- [x] rocketpi_spi_lcd_240x240_lvgl（移植lvgl适配屏幕与遥杆输入）
- [x] rocketpi_i2s（i2s音频驱动，带上位机，任意字符转音频，支持简体中文，粤语，英语，以及任意音频）
- [x] rocketpi_sdio_card（sdio tf卡驱动示例，读写测试）
- [x] rocketpi_sdio_card_fatfs（sdio tf卡  fatfs文件系统驱动示例，读写测试）
- [x] rocketpi_sd_audio_to_i2s（从tf卡中读取音频，播放，屏幕显示频谱图）
- [x] rocketpi_sd_pic_to_lcd（从sd卡读取轮询图片，显示在屏幕上，动图效果，6fps）
- [x] rocketpi_usb_cdc（usb 虚拟串口）
- [x] rocketpi_usb_msc（usb虚拟U盘，使用内部的sram做介质）
- [x] rocketpi_w25qxx（w26q64jv的驱动示例）
- [x] rocketpi_flash_littlefs（内部flash+ littlefs文件系统演示示例）
- [x] rocketpi_w25qxx_littlefs（外部 nor flash w25q64jv + littlefs文件系统演示示例）
- [x] rocketpi_esp8266（esp01s at指令交互测试，包括MQTT,AP,STA）
- [x] rocketpi_esp8266_tcp(tcp连接测试，tcp client)
- [x] rocketpi_standby_wkup（低功耗的 standby模式，用户按键唤醒）
- [x] rocketpi_crc（常见crc多项式计算并验证）
- [x] rocketpi_mbedtls （测试AES，RSA以及 SHA256）
- [x] rocketpi_extern_io_check

## boot  （2个示例）

- [x] bootloader （纯bootloader 基础跳转示例）

- [x] bootloader_iap（可使用uart升级的示例 使用ymodem协议）

## rocketpi_demo  （6个示例）

- [x] 01_rocketpi_serial_plane_game（飞行大乱斗）
- [x] 02_don't_approach_me_alarm（别靠近我报警器）
- [x] 03_rocketpi_usb_audio_i2s （超炫桌面节奏大师）
- [x] 04_rocketpi_hcsr04_radar（雷达探测器）
- [x] 05_rocketpi_smart_home(物联网智能家居)
- [x] 06_rocketpi_mqtt_ota（FOTA固件升级）

## RTOS

- [x] rocketpi_freertos （提供freertos示例，基于cubemx生成的，以及手工移植的keil和gcc示例）

- [x] rocketpi_rt-thread（rt-thread 基础示例 创建三个线程，三个led翻转验证调度正常）

- [x] rocketpi_threadx（threadx 基础示例 创建三个线程，三个led翻转验证调度正常）

- [x] rocketpi_zephyr（适配Rocket-Pi全部驱动，50+示例程序）

## freertos（3个示例）

| 目录                                  | 说明                                            |
| ------------------------------------- | ----------------------------------------------- |
| rocketpi_freertos_v202406.04-LTS-GCC  | 手工移植freertosv202406.04-LTS  ，cmake gcc工程 |
| rocketpi_freertos_cubemx_keil         | STM32CubeMX 生成的freertos工程，使用cmsis v2    |
| rocketpi_freertos_v202406.04-LTS-keil | 手工移植freertosv202406.04-LTS  ，keil工程      |

## rt-thread（1个示例）

- rt-thread 基础示例


## threadx（2个示例）

| 目录                         | 说明                            |
| ---------------------------- | ------------------------------- |
| rocketpi_threadx_v6.4.3_keil | 基于threadx_v6.4.3 keil基础示例 |
| rocketpi_threadx_v6.4.3_gcc  | 基于threadx_v6.4.3 gcc基础示例  |

## zephyr(50+示例)

- Rocket-Pi适配zephyr官方示例 （36个）

```
west build -p always -b rocket_pi samples/basic/blinky

west build -p always -b rocket_pi samples/hello_world

west build -p always -b rocket_pi samples/basic/button

west build -p always -b rocket_pi samples/sensor/die_temp_polling
/* 测试w25qxx */
west build -p always -b rocket_pi samples/drivers/spi_flash

west build -p always -b rocket_pi samples/subsys/input/input_dump
/* 测试tf卡 */
west build -p always -b rocket_pi samples/subsys/fs/fs_sample

west build -p always -b rocket_pi samples/subsys/usb/cdc_acm
/* 测试hcsr04 */
west build -p always -b rocket_pi samples/sensor/sensor_shell

west build -p always -b rocket_pi samples/basic/hash_map

west build -p always -b rocket_pi samples/basic/minimal

west build -p always -b rocket_pi samples/basic/sys_heap

west build -p always -b rocket_pi samples/basic/threads

west build -p always -b rocket_pi samples/cpp/hello_world

west build -p always -b rocket_pi samples/cpp/cpp_synchronization

west build -p always -b rocket_pi samples/data_structures/min-heap

west build -p always -b rocket_pi samples/drivers/crc

west build -p always -b rocket_pi samples/drivers/uart/echo_bot
 
west build -p always -b rocket_pi samples/drivers/uart/async_api

west build -p always -b rocket_pi samples/modules/cmsis_dsp/moving_average

west build -p always -b rocket_pi samples/modules/tflite-micro/hello_world
 
west build -p always -b rocket_pi samples/subsys/bindesc/hello_bindesc
 
west build -p always -b rocket_pi samples/subsys/bindesc/read_bindesc
 
west build -p always -b rocket_pi samples/subsys/console/echo
 
west build -p always -b rocket_pi samples/subsys/console/getchar
  
west build -p always -b rocket_pi samples/subsys/console/getline
  
west build -p always -b rocket_pi samples/subsys/crc

west build -p always -b rocket_pi samples/subsys/debug/debugmon

west build -p always -b rocket_pi samples/subsys/portability/cmsis_rtos_v2/philosophers

west build -p always -b rocket_pi samples/subsys/portability/cmsis_rtos_v2/timer_synchronization

west build -p always -b rocket_pi samples/subsys/rtio/producer_consumer

west build -p always -b rocket_pi samples/subsys/rtio/sensor_batch_processing

west build -p always -b rocket_pi samples/subsys/shell/shell_module

west build -p always -b rocket_pi samples/subsys/shell/devmem_load

west build -p always -b rocket_pi samples/subsys/smf/hsm_psicc2

west build -p always -b rocket_pi samples/subsys/smf/smf_calculator

```

- Rocket-Pi 驱动示例  (14个)
- 需在rocketpi_example目录下编译

```
west build -p always -b rocket_pi display

west build -p always -b rocket_pi aht30

west build -p always -b rocket_pi hcsr04

west build -p always -b rocket_pi buzzer

west build -p always -b rocket_pi sg90

west build -p always -b rocket_pi ws2812b

west build -p always -b rocket_pi max98357

west build -p always -b rocket_pi --sysbuild with_mcuboot

west build -p always -b rocket_pi irda_nec

west build -p always -b rocket_pi l9110h

west build -p always -b rocket_pi adc_joystick

west build -p always -b rocket_pi esp8266

west build -p always -b rocket_pi radar

west build -p always -b rocket_pi at24c02
```

