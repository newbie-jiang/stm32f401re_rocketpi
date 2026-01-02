## 效果展示

![image-20251216191527746](https://cloud.rocketpi.club/cloud/image-20251216191527746.png)

## 功能说明

面向 RocketPI STM32F401RE 开发板的  单线 **SDIO MicroSD卡 FATFS  读写 演示工程**。主要特性：

- 实现MicroSD卡的FATFS读写测试，

## 硬件连接

![image-20251216185932203](https://cloud.rocketpi.club/cloud/image-20251216185932203.png)

- 插入tf卡 
- 目前测试支持品牌（闪迪，朗科）

![image-20251216185829743](https://cloud.rocketpi.club/cloud/image-20251216185829743.png)

## CubeMX配置

![image-20251119004637362](https://cloud.rocketpi.club/cloud/image-20251119004637362.png)

### DMA配置

发送和接收配置，注意内存到外设  /   外设到内存

![image-20251119004731858](https://cloud.rocketpi.club/cloud/image-20251119004731858.png)

### 开启全局中断，并确认使能

![image-20251119004930253](https://cloud.rocketpi.club/cloud/image-20251119004930253.png)

![image-20251119005101176](https://cloud.rocketpi.club/cloud/image-20251119005101176.png)

### 配置FATFS

![image-20251119013356011](https://cloud.rocketpi.club/cloud/image-20251119013356011.png)

![image-20251119013449037](https://cloud.rocketpi.club/cloud/image-20251119013449037.png)

![image-20251119013532421](https://cloud.rocketpi.club/cloud/image-20251119013532421.png)

![image-20251119013621979](https://cloud.rocketpi.club/cloud/image-20251119013621979.png)

sd_diskio.c文件中可以看出是使用dma传输

![image-20251119013252381](https://cloud.rocketpi.club/cloud/image-20251119013252381.png)

