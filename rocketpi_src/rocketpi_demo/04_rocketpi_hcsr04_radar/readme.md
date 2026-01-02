## 效果展示

![image-20251117211339600](https://cloud.rocketpi.club/cloud/image-20251117211339600.png)

![image-20251117211359067](https://cloud.rocketpi.club/cloud/image-20251117211359067.png)**

## 功能说明

- HCSR04雷达+SG90电机+串口  结合上位机，实现简单的毫米波雷达

## 硬件连接

- SG90  PC9
- TRIGGER ---PC10
- ECHO --- PC11

## CubeMX配置

### SG90配置

![image-20251117212649305](https://cloud.rocketpi.club/cloud/image-20251117212649305.png)

### HCSR04  IO配置

![image-20251117212930408](https://cloud.rocketpi.club/cloud/image-20251117212930408.png)

定时器1配置为1us计数

![image-20251117213131232](https://cloud.rocketpi.club/cloud/image-20251117213131232.png)

