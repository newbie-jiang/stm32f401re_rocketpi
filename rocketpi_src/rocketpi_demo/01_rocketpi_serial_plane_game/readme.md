## 效果展示

<img src="https://cloud.rocketpi.club/cloud/image-20251116200846693.png" width="400" height="800" />

## 功能说明

使用ADC双轴遥控杆 发送控制指令，通过USB串口连接到电脑上,即可开启打飞机游戏

遥杆上滑，串口发送8，飞机前进，

遥杆下滑，串口发送4，飞机左移，

遥杆右滑，串口发送6，飞机右移，

遥杆左滑，串口发送5，飞机后退

## 使用方法

下载程序到开发板

访问 https://game.rocketpi.club/ （复制到浏览器打开，最好是chrome浏览器或者edg浏览器）  （若画面不全，则刷新一下，等待加载完成即可）

- 点击连接串口

![image-20251205011835874](https://cloud.rocketpi.club/cloud/image-20251205011835874.png)

- 选择ST-Link的串口并连接

![image-20251205012211477](https://cloud.rocketpi.club/cloud/image-20251205012211477.png)

- 确保串口成功连接 ，点击开始游戏即可进入游戏界面，使用遥杆就可以开始飞行大作战了

![image-20251205012357605](https://cloud.rocketpi.club/cloud/image-20251205012357605.png)

## 所需外设

- uart
- adc_joystick(ADC遥杆)

## 硬件连接

PC4--y轴

PC5--x轴

PC13 --key

## CubeMX配置

配置双通道 IN14 IN15对应PC4 PC5

分频配置PCLK2 =84M     84/4 = 21M   =  47.6ns

对 STM32F4 来说，**总转换时间 ≈ 采样时间 + 12.5 个周期**     （112+12.5）x 47.6   每个通道一次转换大约 6 µs 左右

![image-20251116192037813](https://cloud.rocketpi.club/cloud/image-20251116192037813.png)

该双轴遥杆还带一个按键，低电平触发

![image-20251205011146912](https://cloud.rocketpi.club/cloud/image-20251205011146912.png)

