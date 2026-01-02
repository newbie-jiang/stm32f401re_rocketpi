

# 厂测程序

### 硬件框图

![image-20251222010006648](https://cloud.rocketpi.club/cloud/image-20251222010006648.png)

## 软件框架

![image-20251222013155948](https://cloud.rocketpi.club/cloud/image-20251222013155948.png)

## 厂测程序硬件测试（共11项）

### 屏幕

插入屏幕，烧录厂测程序，画面应如下显示

![image-20251222015336112](https://cloud.rocketpi.club/cloud/image-20251222015336112.png)

### LED

- 三个LED应该正常闪烁

![factory_led](https://cloud.rocketpi.club/cloud/factory_led.gif)

### 按键

- 按键可以切换当前屏幕不同颜色主题，支持短按，双击，长按

![image-20251222020314778](https://cloud.rocketpi.club/cloud/image-20251222020314778.png)

![image-20251222020401661](https://cloud.rocketpi.club/cloud/image-20251222020401661.png)

![image-20251222020440972](https://cloud.rocketpi.club/cloud/image-20251222020440972.png)

![image-20251222020512408](https://cloud.rocketpi.club/cloud/image-20251222020512408.png)

### 蜂鸣器

- 开机时候听到哔哔两声则工作正常

![image-20251222020750570](https://cloud.rocketpi.club/cloud/image-20251222020750570.png)

### debug串口输出

连接此处的usb,找到stlink的com口（未安装stlink驱动需要先装驱动）

![image-20251222021131603](https://cloud.rocketpi.club/cloud/image-20251222021131603.png)

![image-20251222020929778](https://cloud.rocketpi.club/cloud/image-20251222020929778.png)

- 波特率115200，显示出当前的日志以及实时显示freertos各任务的cpu使用情况，以及堆栈占用详情

![factory_log](https://cloud.rocketpi.club/cloud/factory_log.gif)

### usb输出

- 连接上此处的USB，打开串口，可使用shell控制LED

![image-20251222021807366](https://cloud.rocketpi.club/cloud/image-20251222021807366.png)

![image-20251222022135602](https://cloud.rocketpi.club/cloud/image-20251222022135602.png)

如下将看到led 100ms周期性闪烁

![factory_usbcdc](https://cloud.rocketpi.club/cloud/factory_usbcdc.gif)

### tf卡测试

- 插入TF卡，开机即可看到TF卡的容量信息，（如果TF卡没有文件系统，会格式化为文件系统，再挂载，第一次时间会比较长）
- 实际测试支持8G-128G的闪迪A1,A2卡

![image-20251222022853640](https://cloud.rocketpi.club/cloud/image-20251222022853640.png)

![image-20251222022911918](https://cloud.rocketpi.club/cloud/image-20251222022911918.png)

### AT24C02

- 开机时候在屏幕会显示EEPROM项，OK则代表AT24C02读写工作正常

![image-20251222023045882](https://cloud.rocketpi.club/cloud/image-20251222023045882.png)

### ADC遥杆

- 上下左右拨动遥杆，可以选中不同的框

![adc_joystick_lvgl](https://cloud.rocketpi.club/cloud/adc_joystick_lvgl.gif)

### 红外接收器器

![image-20251222024531032](https://cloud.rocketpi.club/cloud/image-20251222024531032.png)

### 外部18个io

- 连接LED测试板，验证外部的18个io是否工作正常
- 发货前每一片板都会验证，这里的LED测试板可自行绘制

![image-20251222025019646](https://cloud.rocketpi.club/cloud/image-20251222025019646.png)

