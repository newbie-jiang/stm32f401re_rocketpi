## 效果展示

![crc](https://cloud.rocketpi.club/cloud/crc.gif)

## 功能说明

- 集成 `component/crc` 目录下的多种 CRC 软件实现（CRC4/7/8/16/24/32、CRC32C、CRC32K/4.2 等），并提供统一的 `crc.h` 接口方便裸机项目引用。
- 新增 `component/crc/crc_test.c` 与 `crc_test.h` 自检框架，内置 `123456789` 标准向量，对每一个 CRC 算法执行期望值对比，可在主函数中通过 `crc_run_all_tests()` 获取结果。
- `Core/Src/main.c` 中在系统初始化后调用 CRC 自检，并通过 `USART2` 打印每项的期望/实测值以及整体 PASS/FAIL，若存在错误会进入 `Error_Handler` 便于调试。

在线校验

https://www.lddgo.net/encrypt/crc 
