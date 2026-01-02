## 功能描述

示例演示如何用 sysbuild 同时编译 MCUboot + Rocket Pi 应用：`west build --sysbuild` 会先在 `build/mcuboot` 下生成引导程序，再在 `build/with_mcuboot` 下创建带签名的镜像。应用本体只做了最小化输出，打印 `__rom_region_start` 地址与 `CONFIG_BOARD`，方便确认代码确实被放在 slot0（0x08020000）并由 MCUboot 跳转执行。

配合日志与 `west flash` 输出，可以看到先烧录 MCUboot，再烧录 `zephyr.signed.hex` 的完整流程，以及 STM32CubeProgrammer 如何分别写入 bank0/slot0。这个示例可用作 Rocket Pi 上移植 MCUboot 的模板，后续只需在此基础上加入自己应用逻辑即可。

## 编译

```
west build -p always -b rocket_pi --sysbuild with_mcuboot

west flash
```

![image-20251215210709420](https://cloud.rocketpi.club/cloud/image-20251215210709420.png)

![image-20251215210746120](https://cloud.rocketpi.club/cloud/image-20251215210746120.png)



building

```
[9/16] Performing build step for 'mcuboot'
[1/293] Preparing syscall dependency handling

[2/293] Generating include/generated/zephyr/version.h
-- Zephyr version: 4.2.99 (/home/hdj/zephyrproject/zephyr), build: v4.2.0-5860-ge8b08d32e572
[293/293] Linking C executable zephyr/zephyr.elf
Memory region         Used Size  Region Size  %age Used
           FLASH:       41664 B        64 KB     63.57%
             RAM:       24000 B        96 KB     24.41%
           SRAM0:          0 GB        96 KB      0.00%
        IDT_LIST:          0 GB        32 KB      0.00%
Generating files from /home/hdj/zephyrproject/rocketpi_example/build/mcuboot/zephyr/zephyr.elf for board: rocket_pi
[11/16] Performing build step for 'with_mcuboot'
[1/157] Preparing syscall dependency handling

[2/157] Generating include/generated/zephyr/version.h
-- Zephyr version: 4.2.99 (/home/hdj/zephyrproject/zephyr), build: v4.2.0-5860-ge8b08d32e572
[157/157] Linking C executable zephyr/zephyr.elf
Memory region         Used Size  Region Size  %age Used
           FLASH:       25384 B     130688 B     19.42%
             RAM:        4928 B        96 KB      5.01%
           SRAM0:          0 GB        96 KB      0.00%
        IDT_LIST:          0 GB        32 KB      0.00%
Generating files from /home/hdj/zephyrproject/rocketpi_example/build/with_mcuboot/zephyr/zephyr.elf for board: rocket_pi
image.py: sign the payload
image.py: sign the payload
[16/16] Completed 'with_mcuboot'
hdj@hdj-virtual-machine:~/zephyrproject/rocketpi_example$ west flash
-- west flash: rebuilding
[0/6] Performing build step for 'with_mcuboot'
ninja: no work to do.
[1/6] Performing build step for 'mcuboot'
ninja: no work to do.
[6/6] Completed 'mcuboot'
-- west flash: using runner stm32cubeprogrammer
      -------------------------------------------------------------------
                        STM32CubeProgrammer v2.20.0                  
      -------------------------------------------------------------------

ST-LINK SN  : 0670FF534953867067222244
ST-LINK FW  : V2J45M31
Board       : --
Voltage     : 3.24V
SWD freq    : 4000 KHz
Connect mode: Under Reset
Reset mode  : Hardware reset
Device ID   : 0x433
Revision ID : Rev Z
Device name : STM32F401xD/E
Flash size  : 512 KBytes
Device type : MCU
Device CPU  : Cortex-M4
BL Version  : 0xD1



Opening and parsing file: zephyr.hex


Memory Programming ...
  File          : zephyr.hex
  Size          : 40.69 KB 
  Address       : 0x08000000


Erasing memory corresponding to segment 0:
Erasing internal memory sectors [0 2]
Download in Progress:
[==================================================] 100% 

File download complete
Time elapsed during download operation: 00:00:01.565

RUNNING Program ... 
  Address:      : 0x8000000
Application is running, Please Hold on...
Start operation achieved successfully
-- west flash: using runner stm32cubeprogrammer
      -------------------------------------------------------------------
                        STM32CubeProgrammer v2.20.0                  
      -------------------------------------------------------------------

ST-LINK SN  : 0670FF534953867067222244
ST-LINK FW  : V2J45M31
Board       : --
Voltage     : 3.24V
SWD freq    : 4000 KHz
Connect mode: Under Reset
Reset mode  : Hardware reset
Device ID   : 0x433
Revision ID : Rev Z
Device name : STM32F401xD/E
Flash size  : 512 KBytes
Device type : MCU
Device CPU  : Cortex-M4
BL Version  : 0xD1



Opening and parsing file: zephyr.signed.hex


Memory Programming ...
  File          : zephyr.signed.hex
  Size          : 25.12 KB 
  Address       : 0x08020000


Erasing memory corresponding to segment 0:
Erasing internal memory sector 5
Download in Progress:
[==================================================] 100% 

File download complete
Time elapsed during download operation: 00:00:01.758

RUNNING Program ... 
  Address:      : 0x8000000
Application is running, Please Hold on...
Start operation achieved successfully

```



```
*** Booting Zephyr OS build v4.2.0-5860-ge8b08d32e572 ***
Address of sample 0x8020000
Hello sysbuild with mcuboot! rocket_pi
```
