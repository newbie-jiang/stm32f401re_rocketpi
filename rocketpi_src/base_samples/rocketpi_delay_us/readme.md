## 效果展示

**使用HAL库做io翻转，1us  延时存在抖动**

![image-20251116045439545](https://cloud.rocketpi.club/cloud/image-20251116045439545.png)

**使用寄存器操作io翻转1us 延时不存在抖动**

![image-20251116050031412](https://cloud.rocketpi.club/cloud/image-20251116050031412.png)

## 功能说明

- 使用基本定时器计数 实现微秒级延时 ,io翻转测量实际效果（PC10） 需要使用示波器或逻辑分析仪观察  

## 硬件连接

PC10接逻辑分析仪，共地

## io翻转使用HAL库测量

实际测量，抖动很大，结果在870ns - 1.1us 之间

![image-20251116045149447](https://cloud.rocketpi.club/cloud/image-20251116045149447.png)

![image-20251116045439545](https://cloud.rocketpi.club/cloud/image-20251116045439545.png)

## io翻转使用寄存器

![image-20251116045845236](https://cloud.rocketpi.club/cloud/image-20251116045845236.png)

很标准的1us，不存在抖动，（HAL 为了通用、易用，做了很多检查和封装，造成这个原因，当然还有一个原因取决于单片机的主频） 

- 测量一段时间，所有脉宽都是1us

![image-20251116050031412](https://cloud.rocketpi.club/cloud/image-20251116050031412.png)

