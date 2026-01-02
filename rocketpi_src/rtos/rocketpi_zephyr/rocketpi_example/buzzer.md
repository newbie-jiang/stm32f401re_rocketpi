## 功能描述

示例通过 `buzzer0` alias 获取 PWM 通道（在设备树中已经把蜂鸣器引脚声明为 `pwms`），`buzzer_on()` 将其设置为固定周期（prj.conf 中定义为 2.7 kHz）并输出 50% 占空比，`buzzer_off()` 则把占空比置 0，使无源蜂鸣器发声/静音。主函数按 150 ms 哔—120 ms 停—150 ms 哔的节奏鸣叫两次，之后进入空闲。

该程序演示了如何判断 `pwm_is_ready_dt()`、如何通过 `pwm_set_dt()` 改变占空比来控制蜂鸣器，便于在 Rocket Pi 上实现提示音或报警声。

## 编译

```
west build -p always -b rocket_pi buzzer
```
