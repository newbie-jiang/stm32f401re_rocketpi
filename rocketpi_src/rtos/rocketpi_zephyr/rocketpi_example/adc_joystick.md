## 功能描述

示例使用设备树别名 `joystick0`（analog-axis 绑定）和 `joystickbutton0`（gpio-keys）把摇杆的 X/Y 模拟电位器与按键接入 Zephyr `input` 子系统。`INPUT_CALLBACK_DEFINE()` 注册回调后，会收到 `INPUT_EV_ABS` 与 `INPUT_EV_KEY` 事件，`axis_name_from_code()` / `button_name_from_code()` 负责把 code 打印成人类可读的 X、Y、Select，并实时输出 0~255 的归一化数值。

ADC 通道由 overlay 配置为 ADC1_IN14/IN15，驱动内置死区、量程映射等参数，示例只需在 main() 中检查 `device_is_ready()`，然后空转等待事件，就能在日志里看到摇杆在两个轴方向上的位移与按钮的按压动作。

## 编译

```
west build -p always -b rocket_pi adc_joystick
```

## 日志

```
minicom -b 115200 -D /dev/ttyACM0
```

```
*** Booting Zephyr OS build v4.2.0-5860-ge8b08d32e572 ***
[00:00:00.000,000] <inf> adc_joystick: ADC joystick input demo is running
[00:00:00.012,000] <inf> adc_joystick: [joystick0] X axis: 128
[00:00:00.012,000] <inf> adc_joystick: [joystick0] Y axis: 128
[00:00:02.925,000] <inf> adc_joystick: [joystick0] X axis: 115
[00:00:02.940,000] <inf> adc_joystick: [joystick0] X axis: 83
[00:00:02.955,000] <inf> adc_joystick: [joystick0] X axis: 44
[00:00:02.970,000] <inf> adc_joystick: [joystick0] X axis: 2
[00:00:02.985,000] <inf> adc_joystick: [joystick0] X axis: 0
[00:00:03.150,000] <inf> adc_joystick: [joystick0] X axis: 25
[00:00:03.165,000] <inf> adc_joystick: [joystick0] X axis: 74
[00:00:03.180,000] <inf> adc_joystick: [joystick0] X axis: 128
[00:00:04.230,000] <inf> adc_joystick: [joystick0] X axis: 142
[00:00:04.245,000] <inf> adc_joystick: [joystick0] X axis: 153
[00:00:04.260,000] <inf> adc_joystick: [joystick0] X axis: 165
[00:00:04.275,000] <inf> adc_joystick: [joystick0] X axis: 178
[00:00:04.290,000] <inf> adc_joystick: [joystick0] X axis: 185
[00:00:04.305,000] <inf> adc_joystick: [joystick0] X axis: 186
[00:00:04.365,000] <inf> adc_joystick: [joystick0] X axis: 184
[00:00:04.380,000] <inf> adc_joystick: [joystick0] X axis: 176
[00:00:04.395,000] <inf> adc_joystick: [joystick0] X axis: 161
[00:00:04.410,000] <inf> adc_joystick: [joystick0] X axis: 140
[00:00:04.472,000] <inf> adc_joystick: [joystick0] X axis: 128
[00:00:05.519,000] <inf> adc_joystick: [joystick_buttons] Select pressed
[00:00:05.667,000] <inf> adc_joystick: [joystick_buttons] Select released

```

参考

 https://docs.zephyrproject.org/latest/build/dts/api/bindings/input/analog-axis.html
