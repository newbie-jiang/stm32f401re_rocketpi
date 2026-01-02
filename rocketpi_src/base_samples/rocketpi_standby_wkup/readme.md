## 效果展示

![standby](https://cloud.rocketpi.club/cloud/standby.gif)

## 功能说明
- 上电后启用全部可用外设时钟，方便在普通运行模式下测量（并对比），再进入低功耗模式前做参考。
- 检查 `PWR_FLAG_SB` 判断是否为 Standby 唤醒：若是，清除唤醒/Standby 标志并点亮三色 LED；若不是，保持三色 LED 熄灭。
- 延时 3 秒便于观察 LED 状态后，通过 `PWR_WAKEUP_PIN1`（PA0）配置唤醒源并进入 Standby 模式。
- PA0 触发唤醒后程序重新启动并执行同样逻辑，可用来验证 Standby 进出及唤醒标志处理。
