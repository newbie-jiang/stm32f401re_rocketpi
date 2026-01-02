## 编译

```
cmake -S . -B build -G Ninja

cmake --build build
```

## 下载（win）

```
STM32_Programmer_CLI.exe -c port=SWD freq=4000 -w build/rocketpi_led_sample.bin 0x08000000 -v -rst
```

