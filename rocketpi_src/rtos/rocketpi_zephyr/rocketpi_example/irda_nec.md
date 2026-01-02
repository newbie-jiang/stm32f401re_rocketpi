## 功能描述

示例通过别名 `ir-sen`（PA15）把红外接收头接入 GPIO 中断，`ir_recv.c` 在每次上升/下降沿记录持续时间并写入 `samples_buf`，超时由定时器补上收尾脉冲，然后调用 `nec_decode()` 解析 NEC 地址和命令。解析结果与原始脉宽都会打印在日志中，方便用示波器对比；当缓冲填满或超时时会重新 enable，确保可以连续接收多帧。

Shell 中提供了 `ir_recv enable/disable/set_timeout/get_timeout` 与 `ir_adj recv <pulse> <space>` 等命令，可以动态打开接收、调整超时时间以及对高电平/低电平脉宽做补偿，便于在不同遥控器、不同电源噪声下保持解码可靠性。示例同样支持 USB CDC 的 shell，可直接在 uart:~ 提示符下调试整个 IrDA 接收链路。
## 编译

```
west build -p always -b rocket_pi irda_nec
```

## 日志

```
uart:~$ 
*** Booting Zephyr OS build v4.2.0-5860-ge8b08d32e572 ***
[00:00:02.482,000] <inf> app: Receiving finished, 68 samples
[00:00:02.482,000] <inf> app: 000: 9226 - 4502
[00:00:02.492,000] <inf> app: 002: 568 - 608
[00:00:02.502,000] <inf> app: 004: 532 - 609
[00:00:02.512,000] <inf> app: 006: 531 - 608
[00:00:02.522,000] <inf> app: 008: 560 - 582
[00:00:02.533,000] <inf> app: 010: 559 - 582
[00:00:02.543,000] <inf> app: 012: 559 - 581
[00:00:02.553,000] <inf> app: 014: 559 - 582
[00:00:02.563,000] <inf> app: 016: 559 - 584
[00:00:02.573,000] <inf> app: 018: 557 - 1649
[00:00:02.583,000] <inf> app: 020: 594 - 1652
[00:00:02.593,000] <inf> app: 022: 590 - 1652
[00:00:02.603,000] <inf> app: 024: 594 - 1650
[00:00:02.613,000] <inf> app: 026: 589 - 1652
[00:00:02.623,000] <inf> app: 028: 587 - 1657
[00:00:02.634,000] <inf> app: 030: 586 - 1658
[00:00:02.644,000] <inf> app: 032: 584 - 1659
[00:00:02.654,000] <inf> app: 034: 583 - 1660
[00:00:02.664,000] <inf> app: 036: 584 - 589
[00:00:02.674,000] <inf> app: 038: 554 - 1657
[00:00:02.684,000] <inf> app: 040: 588 - 587
[00:00:02.694,000] <inf> app: 042: 555 - 589
[00:00:02.704,000] <inf> app: 044: 553 - 588
[00:00:02.714,000] <inf> app: 046: 556 - 1658
[00:00:02.724,000] <inf> app: 048: 585 - 588
[00:00:02.735,000] <inf> app: 050: 554 - 587
[00:00:02.745,000] <inf> app: 052: 556 - 1657
[00:00:02.755,000] <inf> app: 054: 586 - 587
[00:00:02.765,000] <inf> app: 056: 555 - 1657
[00:00:02.775,000] <inf> app: 058: 585 - 1659
[00:00:02.785,000] <inf> app: 060: 585 - 1657
[00:00:02.795,000] <inf> app: 062: 585 - 587
[00:00:02.805,000] <inf> app: 064: 555 - 1658
[00:00:02.815,000] <inf> app: 066: 583 - 9960
[00:00:02.825,000] <inf> app: NEC address: 0x0000
[00:00:02.825,000] <inf> app: NEC command: 0x0045

```

支持设置超时时间和 容差

```shell
uart:~$ ir_recv --help
ir_recv - Receive IR command
Subcommands:
  enable       : enable receiving

  disable      : disable receiving

  set_timeout  : set timeout
                 args: <miliseconds>

  get_timeout  : get timeout
```



```shell
uart:~$ ir_adj recv --help
recv - adjust receive IR timing
       args: <pulse> <space>
```

参考 https://github.com/privara/ir-tool
