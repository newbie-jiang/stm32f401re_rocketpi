## 效果展示

![motor](https://cloud.rocketpi.club/cloud/motor.gif)

## 功能说明

- TIM4 通道 1/2 输出 PWM 驱动 L9110：CH1→INB（PB6），CH2→INA（PB7），默认 20 kHz，可在 `tim.c` 调整。
- 刹车式 PWM：低电平阶段 IA=0、IB=0，低速力矩大、响应快。
- 驱动接口：`motor_l9110_init`、`motor_l9110_drive`（方向+占空比 0-100%）、`motor_l9110_drive_signed`（有符号占空比 ±100%）、`motor_l9110_brake`（双低刹车）。
- 演示状态机：`motor_l9110_test_init`/`motor_l9110_test_task` 循环慢速→加速→刹车→反转→加速→刹车，可调 `hold_ms` 改各阶段时长。

## 硬件连接

![image-20251216235933477](https://cloud.rocketpi.club/cloud/image-20251216235933477.png)

![image-20251217000100719](https://cloud.rocketpi.club/cloud/image-20251217000100719.png)

## 原理

### IA / IB 的逻辑关系（数据手册真值表）

- **IA=H, IB=L → OA=H, OB=L**（一个方向）
- **IA=L, IB=H → OA=L, OB=H**（反方向）
- **IA=L, IB=L → OA=L, OB=L**（刹车到地）
- **IA=H, IB=H → OA=L, OB=L**（同样刹车）

> 没有“输出高电平滑行（coast）”，停时两端被拉低，更像动态刹车。

### 推荐的 PWM 调速接法（最简单、最安全）

- **正转调速**：`IB = 0` 固定，`IA = PWM(duty)`
- **反转调速**：`IA = 0` 固定，`IB = PWM(duty)`
- **停/刹车**：`IA = 0, IB = 0`（或都 1 也会停，但推荐双低）

特点：PWM 的低电平阶段落在 **IA=0、IB=0 → OA/OB 都为 0**，即 PWM 刹车式调速（非滑行式）。

### PWM 频率

- 默认配置约 20 kHz，可根据电机啸叫与发热情况在 10–20 kHz 之间调整。
