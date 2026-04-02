# 2 FreeRTOS 内核发行版

## 2.1 简介

帮助用户熟悉 FreeRTOS 内核
文件和目录，本章：

- 提供 FreeRTOS 目录结构的顶级视图。
- 描述任何特定 FreeRTOS 项目所需的源文件。
- 介绍演示应用程序。
- 提供有关如何创建新 FreeRTOS 项目的信息。

这里的描述仅涉及官方 FreeRTOS 发行版。
本书附带的示例使用略有不同的
组织。


## 2.2 了解 FreeRTOS 发行版

### 2.2.1 定义：FreeRTOS 端口

FreeRTOS 可以使用大约二十种不同的编译器构建，并且可以
运行在四十多种不同的处理器架构上。每个支持
编译器和处理器的组合称为 FreeRTOS 端口。


### 2.2.2 构建 FreeRTOS

FreeRTOS 是一个提供多任务功能的库
否则将是单线程、裸机应用程序。

FreeRTOS 作为一组 C 源文件提供。一些源文件是
所有端口通用，而其他则特定于某个端口。建设
源文件作为项目的一部分，使 FreeRTOS API 可供
您的申请。可以作为参考的演示应用程序是
为每个官方 FreeRTOS 端口提供。演示应用程序是
预先配置以构建正确的源文件并包含正确的
头文件。

在创建时，每个演示应用程序都是“基于
框'，没有编译器错误或警告。请使用FreeRTOS
支持 forums (<https://forums.FreeRTOS.org>) 让我们知道是否
构建工具的后续更改意味着情况不再如此。
2.3 节描述了演示应用程序。


### 2.2.3 FreeRTOSConfig.h

名为 FreeRTOSConfig.h 的头文件中定义的常量配置
内核。不要将 FreeRTOSConfig.h 直接包含在源文件中！
相反，包括 FreeRTOS.h，这将包括 FreeRTOSConfig.h
适当的时间。

FreeRTOSConfig.h 用于定制 FreeRTOS 内核以用于
具体应用。例如，FreeRTOSConfig.h 包含常量
例如 `configUSE_PREEMPTION` 定义是否
FreeRTOS 使用协作或抢占式调度[^1]。

[^1]：4.13节描述了调度算法。

FreeRTOSConfig.h 针对特定应用程序定制了 FreeRTOS，因此它
应位于应用程序一部分的目录中，而不是
包含 FreeRTOS 源代码的目录。

主要的 FreeRTOS 发行版包含一个演示应用程序，适用于每个
FreeRTOS 端口，每个演示应用程序都有自己的 FreeRTOSConfig.h
文件。建议首先使用 FreeRTOSConfig.h，然后进行调整
由为您使用的 FreeRTOS 端口提供的演示应用程序使用
而不是从头开始创建文件。

FreeRTOS 参考手册和 <https://www.freertos.org/a00110.html>
描述 FreeRTOSConfig.h 中出现的常数。它是
不必包含 FreeRTOSConfig.h 中的所有常量 — 许多常量都会得到
如果省略则使用默认值。


### 2.2.4 官方发行版

各个 FreeRTOS 库（包括内核）可从
他们自己的 Github 存储库和 zip 文件存档。有能力
在以下环境中使用 FreeRTOS 时，获取各个库很方便
生产代码。但是，最好下载主要的 FreeRTOS
开始发行，因为它包含库和示例
项目。

主要发行版包含所有 FreeRTOS 的源代码
库、所有 FreeRTOS 内核端口以及所有项目文件
FreeRTOS 演示应用程序。不要因文件数量而推迟！
应用程序只需要一小部分。

使用 <https://github.com/FreeRTOS/FreeRTOS/releases/latest> 下载
包含最新发行版的 zip 文件。或者，使用其中之一
以下 Git 命令用于从 GitHub 克隆主发行版，
包括从各自的 Git 子模块化的各个库
存储库：

* * *
```
git clone https://github.com/FreeRTOS/FreeRTOS.git --recurse-submodules

git clone git@github.com:FreeRTOS/FreeRTOS.git --recurse-submodules
```
* * *


图2.1为FreeRTOS的一级、二级目录
分布：

<a name="fig2.1" title="Figure 2.1 Top level directories within the FreeRTOS distribution"></a>

* * *
```
FreeRTOS
│ │
│ ├─Source  Contains the FreeRTOS kernel source files
│ │
│ └─Demo    Contains pre-configured and port specific FreeRTOS kernel demo projects
│
FreeRTOS-Plus
│
├─Source    Contains source code for other FreeRTOS and ecosystem libraries
│
└─Demo      Contains demo projects for other FreeRTOS and ecosystem libraries
```
***图 2.1*** *FreeRTOS 发行版中的顶级目录*
* * *

该发行版仅包含 FreeRTOS 内核源代码的一份副本
文件；所有演示项目都希望在以下位置找到内核源文件
FreeRTOS/Source 目录，如果该目录可能无法构建
结构发生改变。


### 2.2.5 所有端口通用的 FreeRTOS 源文件

tasks.c 和 list.c 实现核心 FreeRTOS 内核功能
总是需要的。它们直接位于 FreeRTOS/Source 中
目录，如图2.2所示。同一目录还包含
以下可选源文件：

- **queue.c**

queue.c同时提供队列和信号量服务，稍后介绍
   在这本书中。几乎总是需要 queue.c。

- **timers.c**

timers.c 提供软件定时器功能，如后面所述
   这本书。仅当应用程序使用软件时才需要构建
   计时器。

- **event\_groups.c**

event\_groups.c 提供事件组功能，如下所述
   在这本书中。仅当应用程序使用事件时才需要构建
   组。

- **stream\_buffer.c**

stream\_buffer.c同时提供流缓冲区和消息缓冲区
   功能，如本书后面所述。它只需要
   如果应用程序使用
   流或消息缓冲区。

- **croutine.c**

croutine.c 实现 FreeRTOS 协同例程功能。它仅
   如果应用程序使用协同例程，则需要构建。协同例程是
   旨在用于非常小的微控制器，现在很少使用。
   因此，它们不再被维护，并且它们的使用也不再被使用。
   推荐用于新设计。协同例程不在本文中描述
   书。


<a name="fig2.2" title="Figure 2.2 Core FreeRTOS source files within the FreeRTOS directory tree"></a>

* * *
```
FreeRTOS
│
└─Source
    │
    ├─tasks.c         FreeRTOS source file - always required
    ├─list.c          FreeRTOS source file - always required
    ├─queue.c         FreeRTOS source file - nearly always required
    ├─timers.c        FreeRTOS source file - optional
    ├─event_groups.c  FreeRTOS source file – optional
    ├─stream_buffer.c FreeRTOS source file - optional
    └─croutine.c      FreeRTOS source file – optional and no longer maintained
```
***图 2.2*** *FreeRTOS 目录树中的核心 FreeRTOS 源文件*
* * *

可以识别 zip 文件分发中使用的文件名
可能会导致命名空间冲突，因为许多项目已经使用文件
具有相同的名称。如果需要，用户可以更改文件名，但是
名称不能在发行版中更改，因为这样做会破坏
与现有用户项目的兼容性以及 FreeRTOS 感知
开发工具。


### 2.2.6 特定于端口的 FreeRTOS 源文件

FreeRTOS/Source/portable 目录包含特定于 FreeRTOS 的源文件
端口。可移植目录按层次结构排列，首先由
编译器，然后是处理器架构。图 2.3 显示了层次结构。

要在具有“*architecture*”架构的处理器上运行 FreeRTOS，请使用
编译器“*compiler*”，除了核心 FreeRTOS 源代码
文件，您还必须构建位于
FreeRTOS/Source/portable/\[*compiler*\]/\[*architecture*\] 目录。

如第 3 章“堆内存管理”中所述，FreeRTOS 还
认为堆内存分配是可移植层的一部分。如果
`configSUPPORT_DYNAMIC_ALLOCATION`设置为0，则不包含堆
项目中的内存分配方案。

FreeRTOS 在以下位置提供了示例堆分配方案：
FreeRTOS/Source/portable/MemMang 目录。如果 FreeRTOS 配置为
使用动态内存分配，必须包括以下之一
项目中该目录中的堆实现源文件，或者
提供您自己的实现。

> **！请勿在项目中包含多个示例堆分配实现。**


<a name="fig2.3" title="Figure 2.3 Port specific source files within the FreeRTOS directory tree"></a>

* * *
```
FreeRTOS
│
└─Source
    │
    └─portable Directory containing all port specific source files
        │
        ├─MemMang Directory containing the alternative heap allocation source files
        │
        ├─[compiler 1] Directory containing port files specific to compiler 1
        │   │
        │   ├─[architecture 1] Contains files for the compiler 1 architecture 1 port
        │   ├─[architecture 2] Contains files for the compiler 1 architecture 2 port
        │   └─[architecture 3] Contains files for the compiler 1 architecture 3 port
        │
        └─[compiler 2] Directory containing port files specific to compiler 2
            │
            ├─[architecture 1] Contains files for the compiler 2 architecture 1 port
            ├─[architecture 2] Contains files for the compiler 2 architecture 2 port
            └─[etc.]
```
***图 2.3*** *在 FreeRTOS 目录树中移植特定源文件*
* * *

### 2.2.7 包含路径

FreeRTOS 要求编译器中包含三个目录
包括路径。这些都是：

1. 核心FreeRTOS内核头文件的路径，FreeRTOS/Source/include。

2. 特定于所使用的 FreeRTOS 端口的源文件的路径，
   FreeRTOS/源/便携式/\[*编译器*\]/\[*架构*\]。

3. 正确的FreeRTOSConfig.h头文件的路径。


### 2.2.8 头文件

使用 FreeRTOS API 的源文件必须包含 FreeRTOS.h，
接下来是包含 API 原型的头文件
功能 — task.h、queue.h、semphr.h、timers.h、
event_groups.h、stream_buffer.h、message_buffer.h 或 croutine.h。不
显式包含任何其他 FreeRTOS 头文件 - FreeRTOS.h 自动包含
FreeRTOSConfig.h。


## 2.3 演示应用程序

每个 FreeRTOS 端口都至少附带一个演示应用程序，该应用程序在
创建时，“开箱即用”，没有编译器错误或
警告。请使用 FreeRTOS 支持 forums
(<https://forums.FreeRTOS.org>) 让我们知道后续是否有更改
构建工具意味着情况不再如此。

> **跨平台支持**：FreeRTOS 在 Windows 上开发和测试，
> Linux 和 MacOS 系统以及各种工具链，包括嵌入式和
> 传统。  但是，由于差异，偶尔会出现构建错误
> 版本或错过测试。  请使用 FreeRTOS 支持论坛
> (<https://forums.FreeRTOS.org>) 提醒我们任何此类错误。

演示应用程序有几个目的：

- 提供一个工作和预配置项目的示例，其中
  包含正确的文件，以及正确的编译器选项集。
- 允许以最少的设置进行“开箱即用”的实验或
  先验知识。
- 演示如何使用 FreeRTOS API。
- 作为创建实际应用程序的基础。
- 对内核的实现进行压力测试。

每个演示项目都位于
FreeRTOS/Demo 目录。子目录的名称表示要访问的端口
与演示项目相关。

FreeRTOS.org 网站包含每个演示应用程序的页面。的
网页包含以下信息：

- 如何在 FreeRTOS 目录中找到演示的项目文件
  结构。
- 项目配置使用的硬件或模拟器。
- 如何设置硬件来运行演示。
- 如何构建演示。
- 演示的预期行为。

所有演示项目都会创建“常见演示任务”的子集，即
其实现位于 FreeRTOS/Demo/Common/Minimal
目录。存在常见的演示任务来演示如何使用
FreeRTOS API 和测试 FreeRTOS 内核移植 - 它们没有实现任何
特别有用的功能。

许多演示项目还可以配置为创建简单的“blinky”
风格的启动项目，通常创建两个 RTOS 任务和一个
队列。

每个演示项目都包含一个名为 main.c 的文件，其中包含
`main()` 函数，在启动之前创建演示应用程序任务
FreeRTOS 内核。请参阅
有关特定信息的各个 main.c 文件中的注释
那个演示。

<a name="fig2.4" title="Figure 2.4 The demo directory hierarchy"></a>

* * *
```
FreeRTOS
    │
    └─Demo          Directory containing all the demo projects
        │
        ├─[Demo x]  Contains the project file that builds demo 'x'
        ├─[Demo y]  Contains the project file that builds demo 'y'
        ├─[Demo z]  Contains the project file that builds demo 'z'
        └─Common    Contains files that are built by all the demo applications
```
***图2.4*** *演示目录层次结构*
* * *


## 2.4 创建 FreeRTOS 项目

### 2.4.1 调整提供的演示项目之一

每个 FreeRTOS 端口都附带至少一个预配置的演示
应用程序。建议通过调整其中之一来创建新项目
这些现有项目以确保新项目具有正确的文件
包括，安装了正确的中断处理程序，以及正确的
编译器选项设置。

要从现有演示项目创建新应用程序：

1. 打开提供的演示项目并确保其构建并执行为
   预计。

2. 删除实现demo任务的源文件，分别是
   文件位于 Demo/Common 目录中。

3.删除`main()`内的所有函数调用，除了
   `prvSetupHardware()` 和 `vTaskStartScheduler()`，如清单 2.1 所示。

4. 验证项目是否仍在构建。

当您按照这些步骤操作时，您将创建一个包含正确的项目
FreeRTOS 源文件，但没有定义任何功能。


<a name="list2.1" title="Listing 2.1 The template for a new main() function"></a>


```c
int main( void )
{
    /* Perform any hardware setup necessary. */
    prvSetupHardware();

    /* --- APPLICATION TASKS CAN BE CREATED HERE --- */

    /* Start the created tasks running. */
    vTaskStartScheduler();

    /* Execution will only reach here if there was insufficient heap to
       start the scheduler. */
    for( ;; );
    return 0;
}
```
***清单 2.1*** *新 main() 函数的模板*



### 2.4.2 从头开始​​创建新项目

正如已经提到的，建议从
现有的演示项目。如果这不合需要，请使用以下方法
创建新项目的过程：

1. 使用您选择的工具链，创建一个尚未包含的新项目
   包括任何 FreeRTOS 源文件。

1. 确保新项目构建、下载到您的目标硬件，
   并执行。

1. 仅当您确定已经有一个工作项目时，才添加
   FreeRTOS 项目的源文件详细信息如表 1 所示。

1、复制demo工程使用的`FreeRTOSConfig.h`头文件并
   为您的新项目目录中使用的端口提供。

1. 将以下目录添加到项目将搜索到的路径中
   找到头文件：

- FreeRTOS/源/包含
   - FreeRTOS/Source/portable/\[*编译器*\]/\[*架构*\]（其中
     \[*compiler*\] 和 \[*architecture*\] 对于您选择的端口是正确的）
   - 包含`FreeRTOSConfig.h`头文件的目录

1. 从相关演示项目复制编译器设置。

1. 安装任何可能需要的 FreeRTOS 中断处理程序。使用
   描述正在使用的端口和演示项目的网页
   为正在使用的端口提供参考。

<a name="tbl1" title="Table 1 FreeRTOS source files to include in the project"></a>

* * *
|文件|地点 |
|---------------------------------|------------------------------|
| tasks.c | FreeRTOS/源 |
| queue.c | FreeRTOS/源 |
| list.c | FreeRTOS/源 |
| timers.c | FreeRTOS/源 |
| event\_groups.c | FreeRTOS/源 |
| stream\_buffer.c | FreeRTOS/源 |
|所有 C 和汇编文件 | FreeRTOS/源/便携式/\[编译器\]/\[架构\] |
| heap\_n.c | FreeRTOS/Source/portable/MemMang，其中 n 为 1、2、3、4 或 5 |

***表 1*** *要包含在项目中的 FreeRTOS 源文件*
* * *

**关于堆内存的注意事项：**
 如果 `configSUPPORT_DYNAMIC_ALLOCATION` 为 0，则不包含堆内存
 您项目中的分配方案。否则包括堆内存分配方案
 在您的项目中，要么是 heap\_*n*.c 文件之一，要么是由
 你自己。有关详细信息，请参阅第 3 章“堆内存管理”。


## 2.5 数据类型和编码风格指南

### 2.5.1 数据类型

FreeRTOS 的每个端口都有一个唯一的 portmacro.h 头文件，contains
(amongst other things) 为两种特定于端口的数据类型定义：
`TickType_t` 和 `BaseType_t`。以下列表描述了宏或
使用的 typedef 和实际类型：

- `TickType_t`

FreeRTOS 配置一个称为节拍中断的周期性中断。

自 FreeRTOS 以来发生的滴答中断数
  启动的应用程序称为“滴答计数”。刻度数是
  用作时间的度量。

两次滴答中断之间的时间称为“滴答周期”。
  时间被指定为刻度周期的倍数。

`TickType_t` 是用于保存刻度计数值的数据类型，并用于
  指定时间。

`TickType_t` 可以是无符号 16 位类型、无符号 32 位类型
  类型，或无符号 64 位类型，具体取决于 `configTICK_TYPE_WIDTH_IN_BITS` 的设置
  FreeRTOSConfig.h。 `configTICK_TYPE_WIDTH_IN_BITS`的设置是架构
  依赖。 FreeRTOS 端口还将检查此设置是否有效。

使用16位类型可以大大提高8位和
  16位架构，但严重限制了最大出块时间
  可以在 FreeRTOS API 调用中指定。没有理由使用
  32 位或 64 位架构上的 16 位 `TickType_t` 类型。

之前使用的 `configUSE_16_BIT_TICKS` 已替换为 `configTICK_TYPE_WIDTH_IN_BITS` 以支持
  刻度计数大于 32 位。新设计应使用 `configTICK_TYPE_WIDTH_IN_BITS`
  而不是 `configUSE_16_BIT_TICKS`。

   <a name="tbl2" title="Table 2 TickType_t data type and the configTICK_TYPE_WIDTH_IN_BITS configuration"></a>

* * *
| configTICK\_TYPE\_WIDTH\_IN\_BITS | 8 位架构 | 16 位架构 | 32 位架构 | 64 位架构 |
   | --- | --- | --- | --- | --- |
| TICK\_类型\_宽度\_16_BITS | uint16\_t | uint16\_t | uint16\_t |不适用 |
   | TICK\_类型\_宽度\_32_BITS | uint32\_t | uint32\_t | uint32\_t |不适用 |
   | TICK\_类型\_宽度\_64_BITS |不适用 |不适用 | uint64\_t | uint64\_t |

***表 2*** *TickType_t 数据类型和 configTICK_TYPE_WIDTH_IN_BITS 配置*
* * *

- `BaseType_t`

这始终被定义为最有效的数据类型
  架构。通常，这是 64 位架构上的 64 位类型，
  32 位架构上的 32 位类型，16 位架构上的 16 位类型
  架构，以及 8 位架构上的 8 位类型。

`BaseType_t` 通常用于只需要很长时间的返回类型
  有限的值范围，以及 `pdTRUE`/`pdFALSE` 类型布尔值。


*FreeRTOS 使用的端口特定数据类型列表*


### 2.5.2 变量名称

变量以其类型为前缀：“c”代表 `char`，“s”代表 `int16_t`
(short)，“l”代表 `int32_t` (long)，“x”代表 `BaseType_t` 和任何其他变量
非标types (structures, task handles, queue handles, etc.)。

如果变量是无符号的，它也会以“u”为前缀。如果一个变量
是一个指针，它也以“p”为前缀。例如，变量
`uint8_t` 类型将带有“uc”前缀，并且是指针类型的变量
char (`char *`) 将带有“pc”前缀。


### 2.5.3 函数名称

函数的前缀是它们返回的类型和它们返回的文件
内定义。例如：

- v**Task**PrioritySet() 返回 *v*oid 并在 **tasks**.c 中定义。
- x**Queue**Receive() 返回 *BaseType_t* 类型的变量，并在 **queue**.c 中定义。
- pv**Timer**GetTimerID() 返回一个 *v*oid 的 *p* 指针，并在 **timers**.c 中定义。

文件 scope (private) 函数的前缀为“prv”。


### 2.5.4 格式化

选项卡用于一些演示应用程序，其中一个选项卡始终设置为
等于四个空格。内核不再使用制表符。


### 2.5.5 宏名称

大多数宏都是用大写字母编写的，并以小写字母开头
指示宏定义位置的字母。表 3 提供了
前缀列表。

<a name="tbl3" title="Table 3 Macro prefixes"></a>

* * *
|前缀 |宏定义位置|
|----------------------------------------------|--------------------------------|
| port (for example, `portMAX_DELAY`) | `portable.h` 或 `portmacro.h` |
| task (for example, `taskENTER_CRITICAL()`) | `task.h` |
| pd (for example, `pdTRUE`) | `projdefs.h` |
| config (for example, `configUSE_PREEMPTION`) | `FreeRTOSConfig.h` |
| err (for example, `errQUEUE_FULL`) | `projdefs.h` |

***表 3*** *宏前缀*
* * *

请注意，信号量 API 几乎完全被写为一组
宏，但遵循函数命名约定，而不是
宏命名约定。

表 4 中定义的宏在整个 FreeRTOS 源代码中使用。

<a name="tbl4" title="Table 4 Common macro definitions"></a>

* * *
|宏|价值|
|--------------|-------|
| `pdTRUE` | 1 |
| `pdFALSE` | 0 |
| `pdPASS` | 1 |
| `pdFAIL` | 0 |

***表4*** *常用宏定义*
* * *


### 2.5.6 过度类型转换的基本原理

FreeRTOS 源代码可以使用许多不同的编译器进行编译，其中许多
它们生成警告的方式和时间有所不同。特别是，
不同的编译器希望以不同的方式使用转换。结果，
FreeRTOS 源代码包含比通常更多的类型转换
是有保证的。
