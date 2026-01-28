## 功能描述

示例基于官方 `samples/drivers/display`，首先通过 `bl0` alias 获取背光 GPIO，在 `main()` 入口就拉高背光电源，让 Rocket Pi TF-LCD 能亮屏。程序利用 `display_get_capabilities()` 查询屏幕的分辨率与像素格式，动态选择 `fill_buffer_*` 函数把四个角落填成红/绿/蓝/灰，并开启 `frame_incomplete` 逐块刷新，保证双缓冲面板一次 frame 内同步更新。

在初始化完四个角块后，主循环固定在底部区域不停写入不同灰度的条带，实现灰阶渐变滚动，用以验证控制器对各种像素格式（ARGB8888、RGB888/565、L8、AL88、MONO 等）的写入路径。通过该示例可以确认 Rocket Pi 的显示驱动、背光以及分块刷新的组合配置是否正确。

## 编译

```
west build -p always -b rocket_pi display
```

改动自示例samples/drivers/display   

直接使用samples/drivers/display示例屏幕不会亮，supply-gpios = <&gpiob5GPIO_ACTIVE_HIGH>;  不受控制需要单独驱动屏幕背光引脚

头文件引用

```
#include <zephyr/drivers/gpio.h>
```

声明

    static const struct gpio_dt_specbl = GPIO_DT_SPEC_GET(DT_ALIAS(bl0), gpios);

从设备树获取io点亮，添加至main.c最开始

    intret;
    if (!device_is_ready(bl.port)) {
    	LOG_ERR("Backlight GPIO not ready");
    } else {
    ret=gpio_pin_configure_dt(&bl,GPIO_OUTPUT_ACTIVE);
    if (ret) {
    	LOG_ERR("Failed to config backlight GPIO, err=%d",ret);
    	}
    }



![image-20251204055123200](https://cloud.rocketpi.club/cloud/image-20251204055123200.png)

![image-20251204055200254](https://cloud.rocketpi.club/cloud/image-20251204055200254.png)



![fb93197f2feeddc2e6c59b108b60ee34](https://cloud.rocketpi.club/cloud/fb93197f2feeddc2e6c59b108b60ee34.jpg)
