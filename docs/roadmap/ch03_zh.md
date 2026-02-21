# 3 堆内存管理

## 3.1 简介

### 3.1.1 先决条件

成为一名合格的 C 程序员是使用 FreeRTOS 的先决条件，所以
本章假设读者熟悉以下概念：

- 构建 C 项目的不同编译和链接阶段。
- 什么是栈和堆。
- 标准 C 库 `malloc()` 和 `free()` 函数。

### 3.1.2 范围

本章内容包括：

- 当 FreeRTOS 分配 RAM 时。
- FreeRTOS 提供的五个示例内存分配方案。
- 选择哪种内存分配方案。

### 3.1.3 静态和动态内存分配之间的切换

以下章节介绍了内核对象，例如任务、队列、
信号量和事件组。容纳这些物体所需的 RAM 可以
在编译时静态分配或在运行时动态分配。
动态分配减少了设计和规划工作，简化了
API，并最大限度地减少 RAM 占地面积。静态分配比较多
确定性，无需处理内存分配失败，
并消除堆碎片的风险（其中堆有足够的碎片）
释放内存但不在一个可用的连续块中）。

FreeRTOS API 函数静态创建内核对象
分配的内存仅在 `configSUPPORT_STATIC_ALLOCATION` 时可用
在 FreeRTOSConfig.h 中设置为 1。 FreeRTOS API 函数创建
使用动态分配内存的内核对象仅可用
当 `configSUPPORT_DYNAMIC_ALLOCATION` 设置为 1 或左侧时
FreeRTOSConfig.h 中未定义。将两个常量设置为有效
1 同时。

有关 `configSUPPORT_STATIC_ALLOCATION` 的更多信息位于
第 3.4 节使用静态内存分配。

### 3.1.4 使用动态内存分配

动态内存分配是一个C编程概念，而不是一个概念
特定于 FreeRTOS 或多任务处理。与FreeRTOS相关
因为可以选择使用动态创建内核对象
分配的内存，以及通用 C 库 `malloc()` 和 `free()`
由于以下一个或多个原因，函数可能不适合：

- 它们并不总是在小型嵌入式系统上可用。
- 它们的实施可能相对较大，占用宝贵的资源
  代码空间。
- 它们很少是线程安全的。
- 它们不是确定性的；执行该操作所花费的时间
  不同的通话功能会有所不同。
- 它们可能会遭受碎片（堆有足够的可用空间）
  内存但不在一个可用的连续块中）。
- 它们会使链接器配置复杂化。
- 如果堆
  允许空间增长到其他变量使用的内存中。

### 3.1.5 动态内存分配选项

FreeRTOS 的早期版本使用内存池分配方案，其中
不同大小的内存块池在编译时预先分配，
然后由内存分配函数返回。虽然块
分配在实时系统中很常见，它已从 FreeRTOS 中删除
因为它在非常小的嵌入式系统中对 RAM 的低效使用导致
许多支持请求。

FreeRTOS 现在将内存分配视为便携式 layer
(instead of part of the core codebase) 的一部分。这是因为不同
嵌入式系统具有不同的动态内存分配和时序
要求，因此单个动态内存分配算法只会
永远适合应用程序的子集。另外，删除动态
来自核心代码库的内存分配使应用程序编写者能够
在适当的时候提供自己的具体实现。

当 FreeRTOS 需要 RAM 时，它会调用 `pvPortMalloc()` 而不是 `malloc()`。
同样，当 FreeRTOS 释放先前分配的 RAM 时，它会调用
`vPortFree()` 而不是 `free()`。 `pvPortMalloc()` 具有相同的原型
标准C库`malloc()`函数，与`vPortFree()`相同
原型为标准C库`free()`函数。

`pvPortMalloc()` 和 `vPortFree()` 是公共函数，因此它们也可以
从应用程序代码调用。

FreeRTOS 附带 `pvPortMalloc()` 的五个示例实现和
`vPortFree()`，这些都记录在本章中。自由实时操作系统
应用程序可以使用示例实现之一或提供它们的
拥有。

这五个示例在 heap\_1.c、heap\_2.c、heap\_3.c 中定义，
分别是heap\_4.c和heap\_5.c源文件，都是
位于 FreeRTOS/Source/portable/MemMang 目录中。


## 3.2 内存分配方案示例

### 3.2.1 堆\_1

对于小型专用嵌入式系统来说，仅创建任务是很常见的
以及启动 FreeRTOS 调度程序之前的其他内核对象。当
是这样的，内核分配的内存只有gets (dynamically)
在应用程序开始执行任何实时功能之前，
并且内存在应用程序的生命周期内保持分配状态。这个
意味着选择的分配方案不必考虑更多
复杂的内存分配问题，例如确定性和碎片，
并且可以优先考虑代码大小和简单性等属性。

Heap\_1.c 实现了 `pvPortMalloc()` 的非常基本的版本，并且不
实施 `vPortFree()`。永远不会删除任务或其他任务的应用程序
内核对象有可能使用堆\_1。有的商业化
关键和安全关键系统，否则将禁止
使用动态内存分配也有可能使用heap\_1。
关键系统通常禁止动态内存分配，因为
与非决定论、内存碎片相关的不确定性
分配失败。 Heap\_1 始终是确定性的并且不能产生碎片
记忆。

Heap\_1对`pvPortMalloc()`的实现简单的细分了一个简单的
`uint8_t` 数组每次将 FreeRTOS 堆调用为更小的块
它被称为。 FreeRTOSConfig.h 常数 `configTOTAL_HEAP_SIZE` 套
数组的大小（以字节为单位）。将堆实现为静态的
分配的数组使 FreeRTOS 看起来消耗大量 RAM，因为
堆成为 FreeRTOS 数据的一部分。

每个动态分配的任务都会导致两次调用 `pvPortMalloc()`。
第一个分配任务控制 block (TCB)，第二个分配任务控制 block (TCB)
任务的堆栈。图3.1演示了heap\_1如何细分简单
创建任务时的数组。

参见图3.1：

- **A** 在创建任何任务之前显示数组 - 整个数组都是免费的。

- **B** 显示创建一个任务后的数组。

- **C** 显示创建三个任务后的数组。

<a name="fig3.1" title="Figure 3.1 RAM being allocated from the heap\_1 array each time a task is created"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image05.png)
***图 3.1*** *每次创建任务时从堆\_1数组中分配RAM*

* * *


### 3.2.2 堆\_2

Heap_2 被 heap_4 取代，其中包括增强的功能。
Heap\_2 保留在 FreeRTOS 发行版中以实现向后兼容性
不建议用于新设计。

Heap\_2.c 还可以通过细分尺寸为
`configTOTAL_HEAP_SIZE` 常数。它使用最佳拟合算法来分配
内存，并且与堆_1 不同，它确实实现了 `vPortFree()`。再次，
将堆实现为静态分配的数组使得 FreeRTOS
似乎消耗了大量 RAM，因为堆成为了
FreeRTOS 数据。

最佳拟合算法确保 `pvPortMalloc()` 使用空闲块
大小与请求的字节数最接近的内存。对于
例如，考虑以下场景：

- 堆包含三个空闲内存块，分别为 5 字节、25
  分别为 100 字节和 100 字节。
- `pvPortMalloc()` 请求 20 字节的 RAM。

RAM 的最小空闲块，其中包含请求的字节数
适合的是25字节块，因此`pvPortMalloc()`分割25字节块
返回前分成 1 个 20 字节块和 1 个 5 字节块
指向 20 字节块的指针[^2]。新的 5 字节块仍然可用
以便将来致电 `pvPortMalloc()`。

[^2]：这过于简单化了，因为堆\_2存储信息
取决于堆区域内的块大小，因此两个分割的总和
区块实际上会小于 25。

与 heap\_4 不同，heap\_2 不会将相邻的空闲块组合成一个
单个较大的块，因此比它更容易出现碎片
堆\_4。然而，如果分配的和
随后释放的块始终具有相同的大小。

<a name="fig3.2" title="Figure 3.2 RAM being allocated and freed from the heap\_2 array as tasks are created and deleted"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image06.png)
***图 3.2*** *随着任务的创建和删除，RAM 被分配并从堆\_2 数组中释放*

* * *

图 3.2 展示了当任务被执行时最佳拟合算法是如何工作的
创建、删除、再次创建。参见图3.2：

- **A** 显示分配三个任务后的数组。一大免费
  块保留在数组的顶部。

- **B** 显示删除其中一项任务后的数组。大的
  数组顶部的空闲块仍然存在。现在也有两个
  先前保存 TCB 和堆栈的较小空闲块
  已删除的任务。

- **C** 显示创建另一个任务后的情况。创造
  该任务导致从内部两次调用 `pvPortMalloc()`
  `xTaskCreate()` API 函数，一个分配新的 TCB，另一个
  分配任务堆栈。本书第3.4节描述了
  `xTaskCreate()`。

每个 TCB 的大小相同，因此最佳拟合算法会重用该块
  保存已删除任务的 TCB 的 RAM 保存已删除任务的 TCB
  创建的任务。

如果分配给新创建的任务的堆栈大小是
  与分配给先前删除的任务的大小相同，则
  最佳拟合算法重用了保存堆栈的 RAM 块
  删除的任务保存创建的任务的堆栈。

数组顶部较大的未分配块仍然存在
  未受影响。

Heap\_2 不是确定性的，但比大多数标准库更快
`malloc()` 和 `free()` 的实现。


### 3.2.3 堆\_3

Heap\_3.c 使用标准库 `malloc()` 和 `free()` 函数，因此
链接器配置定义了堆大小，以及
未使用 `configTOTAL_HEAP_SIZE` 常数。

Heap_3通过暂时挂起使`malloc()`和`free()`线程安全
FreeRTOS 调度程序在其执行期间。第8章，
资源管理，涵盖线程安全和调度程序挂起。


### 3.2.4 堆\_4

与 heap\_1 和 heap\_2 一样，heap\_4 的工作原理是将数组细分为
较小的块。和以前一样，数组是静态分配的并且
尺寸为 `configTOTAL_HEAP_SIZE`，这使得 FreeRTOS 看起来使用
大量 RAM，因为堆成为 FreeRTOS 数据的一部分。

Heap\_4使用first-fit算法来分配内存。与堆\_2不同，
heap\_4 combines (coalesces) 将相邻的空闲内存块放入
单个较大的块，最大限度地减少内存碎片的风险。

首次拟合算法确保 `pvPortMalloc()` 使用第一个空闲块
足够大以容纳所请求的字节数的内存。
例如，考虑以下场景：

- 堆包含三个空闲内存块，按顺序排列
  它们出现在数组中的大小分别是 5 个字节、200 个字节和 100 个字节
  分别为字节。
- `pvPortMalloc()` 请求 20 字节的 RAM。

所请求的字节数适合的 RAM 的第一个空闲块是
200 字节块，因此 `pvPortMalloc()` 将 200 字节块拆分为一个
返回指针之前，包含 20 个字节的块和 180 个字节 [^3] 之一
到 20 字节块。新的 180 字节块将来仍然可用
致电 `pvPortMalloc()`。

[^3]：这过于简单化了，因为堆\_4存储信息
取决于堆区域内的块大小，因此两个分割的总和
块实际上将小于 200 字节。

Heap\_4 combines (coalesces) 将相邻的空闲块合并为单个较大的块
块，最大限度地减少碎片风险，并使其适合
重复分配和释放不同大小块的应用程序
RAM。


<a name="fig3.3" title="Figure 3.3 RAM being allocated and freed from the heap\_4 array"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image07.png)
***图 3.3*** *RAM 正在从堆_4 数组中分配和释放*

* * *

图3.3演示了heap\_4算法如何与内存进行首次拟合
合并工作。参见图3.3：

- **A** 显示创建三个任务后的数组。一大免费
  块保留在数组的顶部。

- **B** 显示删除其中一项任务后的数组。大的
  数组顶部的空闲块仍然存在。现在还有另一个
  已删除任务的 TCB 和堆栈所在的空闲块。
  与 heap_2 示例不同，heap_4 合并了两个内存块
  先前保存已删除任务的 TCB 和堆栈，
  分别进入更大的单个空闲块。

- **C** 显示创建 FreeRTOS 队列后的情况。
  本书第 5.3 节介绍了 `xQueueCreate()` API 函数
  用于动态分配队列。 `xQueueCreate()` 来电
  `pvPortMalloc()` 分配队列使用的 RAM。作为堆\_4使用
  首次适应算法，`pvPortMalloc()` 从第一个开始分配 RAM
  空闲的 RAM 块足够大以容纳队列，其中
  图3.3是通过删除任务释放的RAM。队列没有
  消耗掉空闲块中所有的RAM，因此该块被分割为
  两个，未使用的部分仍然可供将来调用
  `pvPortMalloc()`。

- **D**显示直接调用`pvPortMalloc()`后的情况
  来自应用程序代码，而不是通过调用 FreeRTOS 间接
  API 函数。用户分配的块足够小，可以容纳
  第一个空闲块，即内存之间的块
  分配给队列，以及分配给TCB的内存
  跟随它。

通过删除任务释放的内存现在已分为三部分
  单独的块；第一个块保存队列，第二个块保存队列
  保存用户分配的内存，第三块保持空闲。

- **E**显示删除队列后的情况，即
  自动释放分配给已删除队列的内存。那里
  现在用户分配块两侧都有空闲内存。

- **F** 显示释放分配的用户后的情况
  记忆。用户分配的块先前使用的内存已
  与两侧的空闲内存相结合以创建更大的
  单个空闲块。

Heap\_4 不是确定性的，但比大多数标准库更快
`malloc()` 和 `free()` 的实现。


### 3.2.5 堆\_5

堆_5 使用与堆_4 相同的分配算法。与堆\_4不同，
仅限于从单个数组分配内存，heap\_5 可以
将多个独立内存空间中的内存合并到一个堆中。
当 FreeRTOS 所在的系统提供 RAM 时，Heap\_5 很有用
正在运行不会显示为单个 contiguous (without space) 块
在系统的内存映射中。


### 3.2.6 初始化堆_5：vPortDefineHeapRegions() API 函数

`vPortDefineHeapRegions()`通过指定start来初始化heap\_5
组成堆的每个单独内存区域的地址和大小
由堆\_5 管理。 Heap\_5是唯一提供的堆分配方案
需要显式初始化并且在之后才能使用
致电 `vPortDefineHeapRegions()`。这意味着内核对象，例如
任务、队列和信号量，直到之后才能动态创建
呼叫 `vPortDefineHeapRegions()`。


<a name="list3.1" title="Listing 3.1 The vPortDefineHeapRegions() API function prototype"></a>


```c
void vPortDefineHeapRegions( const HeapRegion_t * const pxHeapRegions );
```
***清单 3.1*** *vPortDefineHeapRegions() API 功能原型*


`vPortDefineHeapRegions()` 采用 `HeapRegion_t` 结构数组作为
它的唯一参数。每个结构定义了起始地址和大小
将成为堆一部分的内存块——整个数组
结构体定义了整个堆空间。


<a name="list3.2" title="Listing 3.2 The HeapRegion\_t structure"></a>


```c
typedef struct HeapRegion
{
    /* The start address of a block of memory that will be part of the heap.*/
    uint8_t *pucStartAddress;

    /* The size of the block of memory in bytes. */
    size_t xSizeInBytes;

} HeapRegion_t;
```
***清单 3.2*** *HeapRegion\_t 结构*


**参数：**

- `pxHeapRegions`

指向 `HeapRegion_t` 结构数组开头的指针。
  每个结构体定义了内存块的起始地址和大小
  将成为堆的一部分。

数组中的 `HeapRegion_t` 结构必须按 start 排序
  地址；描述内存区域的 `HeapRegion_t` 结构
  最低起始地址必须是数组中的第一个结构，并且
  `HeapRegion_t` 结构描述了内存区域
  最高起始地址必须是数组中的最后一个结构。

使用 `HeapRegion_t` 结构标记数组的末尾，该结构具有其
  `pucStartAddress` 成员设置为 `NULL`。

举例来说，考虑如图所示的假设内存映射
3.4 **A** 包含三个独立的 RAM 块：RAM1、RAM2
和 RAM3。假设可执行代码放置在只读存储器中，
这没有显示。


<a name="fig3.4" title="Figure 3.4 Memory Map"></a>

* * *
![](https://cloud.rocketpi.club/cloud/image08.png)
***图 3.4*** *内存映射*
* * *

清单 3.3 显示了一组 `HeapRegion_t` 结构，它们一起
完整描述 RAM 的三个块。


<a name="list3.3" title="Listing 3.3 An array of HeapRegion\_t structures that together describe the 3 regions of RAM in their entirety"></a>


```c
/* Define the start address and size of the three RAM regions. */
#define RAM1_START_ADDRESS ( ( uint8_t * ) 0x00010000 )
#define RAM1_SIZE ( 64 * 1024 )

#define RAM2_START_ADDRESS ( ( uint8_t * ) 0x00020000 )
#define RAM2_SIZE ( 32 * 1024 )

#define RAM3_START_ADDRESS ( ( uint8_t * ) 0x00030000 )
#define RAM3_SIZE ( 32 * 1024 )

/* Create an array of HeapRegion_t definitions, with an index for each
   of the three RAM regions, and terminate the array with a HeapRegion_t
   structure containing a NULL address. The HeapRegion_t structures must
   appear in start address order, with the structure that contains the
   lowest start address appearing first. */
const HeapRegion_t xHeapRegions[] =
{
    { RAM1_START_ADDRESS, RAM1_SIZE },
    { RAM2_START_ADDRESS, RAM2_SIZE },
    { RAM3_START_ADDRESS, RAM3_SIZE },
    { NULL,               0         } /* Marks the end of the array. */
};

int main( void )
{
    /* Initialize heap_5. */
    vPortDefineHeapRegions( xHeapRegions );

    /* Add application code here. */
}
```
***清单 3.3*** *HeapRegion\_t 结构数组共同描述了 RAM 的 3 个区域的整体*


尽管清单 3.3 正确地描述了 RAM，但它并没有演示
可用的示例，因为它将所有 RAM 分配到堆，不留下任何内容
RAM 可供其他变量免费使用。

构建过程的链接阶段为每个链接分配一个 RAM 地址
变量。通常描述可供链接器使用的 RAM
通过链接器配置文件，例如链接器脚本。在图3.4中
**B** 假设链接器脚本包含以下信息
RAM1，但不包括有关 RAM2 或 RAM3 的信息。结果，
链接器将变量放置在 RAM1 中，仅保留上面 RAM1 的部分
地址 0x0001nnnn 可供堆_5 使用。实际价值
0x0001nnnn 取决于包含在中的所有变量的组合大小
该应用程序。链接器未使用所有 RAM2 和所有 RAM3，
使整个 RAM2 和整个 RAM3 可供使用
堆\_5。

清单 3.3 中所示的代码将导致 RAM 分配到堆\_5
下面的地址 0x0001nnnn 与用于保存变量的 RAM 重叠。
如果您设置第一个 `HeapRegion_t` 结构的起始地址
`xHeapRegions[]` 数组为 0x0001nnnn，而不是起始地址
0x00010000，堆不会与链接器使用的RAM重叠。
但是，这不是推荐的解决方案，因为：

- 起始地址可能不容易确定。
- 链接器使用的 RAM 数量可能会在未来的版本中发生变化，
  这将更新中使用的起始地址
  `HeapRegion_t` 结构必需。
- 构建工具不会知道，因此无法警告
  应用程序编写器，如果链接器使用的 RAM 和使用的 RAM
  通过堆\_5重叠。

清单 3.4 演示了一个更方便且可维护的示例。它
声明一个名为 `ucHeap` 的数组。 `ucHeap` 是一个普通变量，所以它
成为链接器分配给 RAM1 的数据的一部分。第一个
`xHeapRegions`数组中的`HeapRegion_t`结构描述了启动
`ucHeap` 的地址和大小，因此 `ucHeap` 成为内存管理的一部分
通过堆\_5。 `ucHeap`的大小可以增加，直到RAM被使用
链接器消耗全部 RAM1，如图 3.4 **C** 所示。


<a name="list3.4" title="Listing 3.4 An array of HeapRegion\_t structures that describe all of RAM2, all of RAM3, but only part of RAM1"></a>

```c
/* Define the start address and size of the two RAM regions not used by
   the linker. */
#define RAM2_START_ADDRESS ( ( uint8_t * ) 0x00020000 )
#define RAM2_SIZE ( 32 * 1024 )

#define RAM3_START_ADDRESS ( ( uint8_t * ) 0x00030000 )
#define RAM3_SIZE ( 32 * 1024 )

/* Declare an array that will be part of the heap used by heap_5. The
   array will be placed in RAM1 by the linker. */
#define RAM1_HEAP_SIZE ( 30 * 1024 )
static uint8_t ucHeap[ RAM1_HEAP_SIZE ];

/* Create an array of HeapRegion_t definitions. Whereas in Listing 3.3 the
   first entry described all of RAM1, so heap_5 will have used all of
   RAM1, this time the first entry only describes the ucHeap array, so
   heap_5 will only use the part of RAM1 that contains the ucHeap array.
   The HeapRegion_t structures must still appear in start address order,
   with the structure that contains the lowest start address appearing first. */

const HeapRegion_t xHeapRegions[] =
{
    { ucHeap,             RAM1_HEAP_SIZE },
    { RAM2_START_ADDRESS, RAM2_SIZE },
    { RAM3_START_ADDRESS, RAM3_SIZE },
    { NULL,               0 }           /* Marks the end of the array. */
};
```
***清单 3.4*** *HeapRegion\_t 结构数组，描述所有 RAM2、所有 RAM3，但仅描述部分 RAM1*


清单 3.4 中展示的技术的优点包括：

- 没有必要使用硬编码的起始地址。
- `HeapRegion_t`结构中使用的地址将被设置
  由链接器自动执行，因此它始终是正确的，即使
  链接器使用的 RAM 数量在未来版本中会发生变化。
- 分配给heap\_5的RAM不可能与放置的数据重叠
  通过链接器进入 RAM1。
- 如果 `ucHeap` 太大，应用程序将无法链接。


## 3.3 堆相关实用函数和宏

### 3.3.1 定义堆起始地址

Heap\_1、heap\_2 和 heap\_4 从静态分配的内存中分配内存
数组尺寸为 `configTOTAL_HEAP_SIZE`。本节参考了这些
分配方案统称为堆\_n。

有时需要将堆放置在特定的内存地址处。对于
例如，分配给动态创建的任务的堆栈来自
堆，因此可能需要将堆定位在快速内部
内存而不是缓慢的外部内存。 （参见“放置”小节
下面的快速内存中的任务堆栈是分配任务的另一种方法
堆栈在快速内存中）。 `configAPPLICATION_ALLOCATED_HEAP`
编译时配置常量使应用程序能够声明
数组代替声明，否则该声明将位于
heap\_n.c 源文件。在应用程序代码中声明数组
使应用程序编写者能够指定其起始地址。

如果 FreeRTOSConfig.h 中的 `configAPPLICATION_ALLOCATED_HEAP` 设置为 1，
使用 FreeRTOS 的应用程序必须分配 `uint8_t` 数组
称为 `ucHeap`，并由 `configTOTAL_HEAP_SIZE` 常数确定尺寸。

将变量放置在特定内存地址所需的语法是
取决于所使用的编译器，因此请参阅您的编译器的
文档。两个编译器的示例如下：

- 清单 3.5 显示了 GCC 编译器声明所需的语法
  该数组并将该数组放置在名为 `.my_heap` 的内存部分中。
- 清单 3.6 显示了 IAR 编译器声明所需的语法
  数组并将数组放置在绝对内存地址处
  0x20000000。


<a name="list3.5" title="Listing 3.5 Using GCC syntax to declare the array that will be used by heap\_4, and place the array in a memory section named .my\_heap"></a>


```c
uint8_t ucHeap[ configTOTAL_HEAP_SIZE ] __attribute__ ( ( section( ".my_heap" ) ) );
```
***清单3.5*** *使用GCC语法声明heap\_4将使用的数组，并将该数组放置在名为.my\_heap的内存段中*



<a name="list3.6" title="Listing 3.6 Using IAR syntax to declare the array that will be used by heap\_4, and place the array at the absolute address 0x20000000"></a>


```c
uint8_t ucHeap[ configTOTAL_HEAP_SIZE ] @ 0x20000000;
```
***清单3.6*** *使用IAR语法声明heap\_4将使用的数组，并将该数组放置在绝对地址0x20000000*



### 3.3.2 xPortGetFreeHeapSize() API 功能

`xPortGetFreeHeapSize()` API 函数返回空闲字节数
调用函数时在堆中。它不提供
有关堆碎片的信息。

`xPortGetFreeHeapSize()` 未针对堆\_3 实现。


<a name="list3.7" title="Listing 3.7 The xPortGetFreeHeapSize() API function prototype"></a>


```c
size_t xPortGetFreeHeapSize( void );
```
***清单 3.7*** *xPortGetFreeHeapSize() API 函数原型*


**返回值：**

- `xPortGetFreeHeapSize()` 返回堆中未分配的字节数
   被调用的时间。


### 3.3.3 xPortGetMinimumEverFreeHeapSize() API 功能

`xPortGetMinimumEverFreeHeapSize()` API 函数返回最小值
自创建以来堆中曾经存在的未分配字节数
FreeRTOS 应用程序开始执行。

`xPortGetMinimumEverFreeHeapSize()` 返回的值指示如何
关闭应用程序曾经耗尽堆空间。对于
例如，如果 `xPortGetMinimumEverFreeHeapSize()` 返回 200，那么，在某些情况下
自应用程序开始执行以来的时间，它在 200 字节以内
堆空间耗尽。

`xPortGetMinimumEverFreeHeapSize()`也可用于优化堆
尺寸。例如，如果 `xPortGetMinimumEverFreeHeapSize()` 返回 2000
执行您知道堆使用率最高的代码后，
`configTOTAL_HEAP_SIZE` 最多可减少 2000 个字节。

`xPortGetMinimumEverFreeHeapSize()` 仅在堆\_4 和堆\_5 中实现。


<a name="list3.8" title="Listing 3.8 The xPortGetMinimumEverFreeHeapSize() API function prototype"></a>


```c
size_t xPortGetMinimumEverFreeHeapSize( void );
```
***清单 3.8*** *xPortGetMinimumEverFreeHeapSize() API 功能原型*


**返回值：**

- `xPortGetMinimumEverFreeHeapSize()` 返回未分配的最小数量
  自 FreeRTOS 应用程序开始执行以来堆中存在的字节数。


### 3.3.4 vPortGetHeapStats() API 功能

堆\_4和堆\_5实现`vPortGetHeapStats()`，从而完成
`HeapStats_t` 结构作为函数的唯一参数按引用传递。

清单 3.9 显示了 `vPortGetHeapStats()` 函数原型。清单 3.10
显示 `HeapStats_t` 结构成员。


<a name="list3.9" title="Listing 3.9 The vPortGetHeapStatus() API function prototype"></a>


```c
void vPortGetHeapStats( HeapStats_t *xHeapStats );
```
***清单 3.9*** *vPortGetHeapStatus() API 功能原型*



<a name="list3.10" title="Listing 3.10 The HeapStatus\_t() structure"></a>


```c
/* Prototype of the vPortGetHeapStats() function. */
void vPortGetHeapStats( HeapStats_t *xHeapStats );

/* Definition of the HeapStats_t structure. All sizes specified in bytes. */
typedef struct xHeapStats
{
    /* The total heap size currently available - this is the sum of all the
       free blocks, not the largest available block. */
    size_t xAvailableHeapSpaceInBytes;

    /* The size of the largest free block within the heap at the time
       vPortGetHeapStats() is called. */
    size_t xSizeOfLargestFreeBlockInBytes;

    /* The size of the smallest free block within the heap at the time
       vPortGetHeapStats() is called. */
    size_t xSizeOfSmallestFreeBlockInBytes;

    /* The number of free memory blocks within the heap at the time
       vPortGetHeapStats() is called. */
    size_t xNumberOfFreeBlocks;

    /* The minimum amount of total free memory (sum of all free blocks)
       there has been in the heap since the system booted. */
    size_t xMinimumEverFreeBytesRemaining;

    /* The number of calls to pvPortMalloc() that have returned a valid
       memory block. */
    size_t xNumberOfSuccessfulAllocations;

    /* The number of calls to vPortFree() that has successfully freed a
       block of memory. */
    size_t xNumberOfSuccessfulFrees;
} HeapStats_t;
```
***清单 3.10*** *HeapStatus\_t() 结构*



### 3.3.5 收集每个任务堆使用统计信息

应用程序编写者可以使用以下跟踪宏来收集每个任务
堆使用统计：
- `traceMALLOC`
- `traceFREE`

清单 3.11 显示了这些要收集的跟踪宏的实现示例
每个任务的堆使用统计信息。

<a name="list3.11" title="Listing 3.11 Collecting Per-task Heap Usage Statistics"></a>


```c
#define mainNUM_ALLOCATION_ENTRIES          512
#define mainNUM_PER_TASK_ALLOCATION_ENTRIES 32

/*-----------------------------------------------------------*/

/*
 * +-----------------+--------------+----------------+-------------------+
 * | Allocating Task | Entry in use | Allocated Size | Allocated Pointer |
 * +-----------------+--------------+----------------+-------------------+
 * |                 |              |                |                   |
 * +-----------------+--------------+----------------+-------------------+
 * |                 |              |                |                   |
 * +-----------------+--------------+----------------+-------------------+
 */
typedef struct AllocationEntry
{
    BaseType_t xInUse;
    TaskHandle_t xAllocatingTaskHandle;
    size_t uxAllocatedSize;
    void * pvAllocatedPointer;
} AllocationEntry_t;

AllocationEntry_t xAllocationEntries[ mainNUM_ALLOCATION_ENTRIES ];

/*
 * +------+-----------------------+----------------------+
 * | Task | Memory Currently Held | Max Memory Ever Held |
 * +------+-----------------------+----------------------+
 * |      |                       |                      |
 * +------+-----------------------+----------------------+
 * |      |                       |                      |
 * +------+-----------------------+----------------------+
 */
typedef struct PerTaskAllocationEntry
{
    TaskHandle_t xTask;
    size_t uxMemoryCurrentlyHeld;
    size_t uxMaxMemoryEverHeld;
} PerTaskAllocationEntry_t;

PerTaskAllocationEntry_t xPerTaskAllocationEntries[ mainNUM_PER_TASK_ALLOCATION_ENTRIES ];

/*-----------------------------------------------------------*/

void TracepvPortMalloc( size_t uxAllocatedSize, void * pv )
{
    size_t i;
    TaskHandle_t xAllocatingTaskHandle;
    AllocationEntry_t * pxAllocationEntry = NULL;
    PerTaskAllocationEntry_t * pxPerTaskAllocationEntry = NULL;

    if( xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED )
    {
        xAllocatingTaskHandle = xTaskGetCurrentTaskHandle();

        for( i = 0; i < mainNUM_ALLOCATION_ENTRIES; i++ )
        {
            if( xAllocationEntries[ i ].xInUse == pdFALSE )
            {
                pxAllocationEntry = &( xAllocationEntries[ i ] );
                break;
            }
        }

        /* Do we already have an entry in the per task table? */
        for( i = 0; i < mainNUM_PER_TASK_ALLOCATION_ENTRIES; i++ )
        {
            if( xPerTaskAllocationEntries[ i ].xTask == xAllocatingTaskHandle )
            {
                pxPerTaskAllocationEntry = &( xPerTaskAllocationEntries[ i ] );
                break;
            }
        }

        /* We do not have an entry in the per task table. Find an empty slot. */
        if( pxPerTaskAllocationEntry == NULL )
        {
            for( i = 0; i < mainNUM_PER_TASK_ALLOCATION_ENTRIES; i++ )
            {
                if( xPerTaskAllocationEntries[ i ].xTask == NULL )
                {
                    pxPerTaskAllocationEntry = &( xPerTaskAllocationEntries[ i ] );
                    break;
                }
            }
        }

        /* Ensure that we have space in both the tables. */
        configASSERT( pxAllocationEntry != NULL );
        configASSERT( pxPerTaskAllocationEntry != NULL );

        pxAllocationEntry->xAllocatingTaskHandle = xAllocatingTaskHandle;
        pxAllocationEntry->xInUse = pdTRUE;
        pxAllocationEntry->uxAllocatedSize = uxAllocatedSize;
        pxAllocationEntry->pvAllocatedPointer = pv;

        pxPerTaskAllocationEntry->xTask = xAllocatingTaskHandle;
        pxPerTaskAllocationEntry->uxMemoryCurrentlyHeld += uxAllocatedSize;
        if( pxPerTaskAllocationEntry->uxMaxMemoryEverHeld < pxPerTaskAllocationEntry->uxMemoryCurrentlyHeld )
        {
            pxPerTaskAllocationEntry->uxMaxMemoryEverHeld = pxPerTaskAllocationEntry->uxMemoryCurrentlyHeld;
        }
    }
}
/*-----------------------------------------------------------*/

void TracevPortFree( void * pv )
{
    size_t i;
    AllocationEntry_t * pxAllocationEntry = NULL;
    PerTaskAllocationEntry_t * pxPerTaskAllocationEntry = NULL;

    for( i = 0; i < mainNUM_ALLOCATION_ENTRIES; i++ )
    {
        if( ( xAllocationEntries[ i ].xInUse == pdTRUE ) &&
            ( xAllocationEntries[ i ].pvAllocatedPointer == pv ) )
        {
            pxAllocationEntry = &( xAllocationEntries [ i ] );
            break;
        }
    }

    /* Attempt to free a block that was never allocated. */
    configASSERT( pxAllocationEntry != NULL );

    for( i = 0; i < mainNUM_PER_TASK_ALLOCATION_ENTRIES; i++ )
    {
        if( xPerTaskAllocationEntries[ i ].xTask == pxAllocationEntry->xAllocatingTaskHandle )
        {
            pxPerTaskAllocationEntry = &( xPerTaskAllocationEntries[ i ] );
            break;
        }
    }

    /* An entry must exist in the per task table. */
    configASSERT( pxPerTaskAllocationEntry != NULL );

    pxPerTaskAllocationEntry->uxMemoryCurrentlyHeld -= pxAllocationEntry->uxAllocatedSize;

    pxAllocationEntry->xInUse = pdFALSE;
    pxAllocationEntry->xAllocatingTaskHandle = NULL;
    pxAllocationEntry->uxAllocatedSize = 0;
    pxAllocationEntry->pvAllocatedPointer = NULL;
}
/*-----------------------------------------------------------*/

/* The following goes in FreeRTOSConfig.h: */
extern void TracepvPortMalloc( size_t uxAllocatedSize, void * pv );
extern void TracevPortFree( void * pv );

#define traceMALLOC( pvReturn, xAllocatedBlockSize ) \
TracepvPortMalloc( xAllocatedBlockSize, pvReturn )

#define traceFREE( pv, xAllocatedBlockSize ) \
TracevPortFree( pv )
```
***清单 3.11*** *收集每个任务堆使用统计信息*

### 3.3.6 Malloc 失败的钩子函数

与标准库 `malloc()` 函数一样，`pvPortMalloc()` 返回 NULL
如果无法分配请求的 RAM 金额。 malloc 失败 hook
(or callback) 是应用程序提供的函数，如果
`pvPortMalloc()` 返回 NULL。您必须将 `configUSE_MALLOC_FAILED_HOOK` 设置为
FreeRTOSConfig.h 中的 1 以便发生回调。如果 malloc 失败
在使用动态内存的 FreeRTOS API 函数内调用钩子
分配创建一个内核对象，该对象没有创建。

如果 FreeRTOSConfig.h 中的 `configUSE_MALLOC_FAILED_HOOK` 设置为 1，则
应用程序必须提供一个 malloc 失败的钩子函数，其名称为
原型如清单 3.12 所示。该应用程序可以实现
以适合应用程序的任何方式运行。许多
提供的 FreeRTOS 演示应用程序将分配失败视为
致命错误，但这不是生产系统的最佳实践，
它应该从分配失败中正常恢复。


<a name="list3.12" title="Listing 3.12 The malloc failed hook function name and prototype"></a>


```c
void vApplicationMallocFailedHook( void );
```
***清单 3.12*** *malloc 失败的钩子函数名称和原型*



### 3.3.7 将任务堆栈放入快速内存中

由于堆栈的写入和读取速率很高，因此它们应该
放置在快速内存中，但这可能不是您想要堆的位置
居住。 FreeRTOS 使用 `pvPortMallocStack()` 和 `vPortFreeStack()`
用于选择启用在 FreeRTOS API 内分配的堆栈的宏
代码有自己的内存分配器。如果你想让堆栈来
从 `pvPortMalloc()` 管理的堆中然后离开 `pvPortMallocStack()`
和 `vPortFreeStack()` 未定义，因为它们默认调用
分别为 `pvPortMalloc()` 和 `vPortFree()`。否则，定义
宏来调用应用程序提供的函数，如清单 3.13 所示。


<a name="list3.13" title="Listing 3.13 Mapping the pvPortMallocStack() and vPortFreeStack() macros to an application defined memory allocator"></a>


```c
/* Functions provided by the application writer than allocate and free
   memory from a fast area of RAM. */

void *pvMallocFastMemory( size_t xWantedSize );

void vPortFreeFastMemory( void *pvBlockToFree );

/* Add the following to FreeRTOSConfig.h to map the pvPortMallocStack()
   and vPortFreeStack() macros to the functions that use fast memory. */

#define pvPortMallocStack( x ) pvMallocFastMemory( x )

#define vPortFreeStack( x ) vPortFreeFastMemory( x )
```
***清单 3.13*** *将 pvPortMallocStack() 和 vPortFreeStack() 宏映射到应用程序定义的内存分配器*



## 3.4 使用静态内存分配

3.1.4 节列出了动态内存分配带来的一些缺点。为了避免这些问题，静态
内存分配允许开发人员显式创建应用程序所需的每个内存块。这有
以下优点：

- 所有需要的内存在编译时都是已知的。
- 所有记忆都是确定性的。

还有其他优点，但这些优点也带来了一些并发症。主要的并发症是增加了一个
很少有额外的用户函数来管理一些内核内存，第二个复杂之处是需要确保所有静态
内存被声明在合适的范围内。


### 3.4.1 启用静态内存分配

通过将 FreeRTOSConfig.h 中的 `configSUPPORT_STATIC_ALLOCATION` 设置为 1 来启用静态内存分配。  当这个
配置启用后，内核启用所有`static`版本的内核功能。这些都是：

- `xTaskCreateStatic`
- `xEventGroupCreateStatic`
- `xEventGroupGetStaticBuffer`
- `xQueueGenericCreateStatic`
- `xQueueGenericGetStaticBuffers`
- `xQueueCreateMutexStatic`
  - *如果 `configUSE_MUTEXES` 为 1*
- `xQueueCreateCountingSemaphoreStatic`
  - *如果 `configUSE_COUNTING_SEMAPHORES` 为 1*
- `xStreamBufferGenericCreateStatic`
- `xStreamBufferGetStaticBuffers`
- `xTimerCreateStatic`
  - *如果 `configUSE_TIMERS` 为 1*
- `xTimerGetStaticBuffer`
  - *如果 `configUSE_TIMERS` 为 1*

这些功能将在本书的相应章节中进行解释。

### 3.4.2 静态内部内核内存

当启用静态内存分配器时，空闲任务和定时器 task (if enabled) 将使用提供的静态内存
通过用户功能。这些用户功能是：

- `vApplicationGetTimerTaskMemory`
  - *如果 `configUSE_TIMERS` 为 1*
- `vApplicationGetIdleTaskMemory`


#### 3.4.2.1 vApplicationGetTimerTaskMemory

如果 `configSUPPORT_STATIC_ALLOCATION` 和 `configUSE_TIMERS` 都启用，内核将调用 `vApplicationGetTimerTaskMemory()`
允许应用程序为定时器任务 TCB 和定时器任务堆栈创建并返回内存缓冲区。该函数将
还返回定时器任务堆栈的大小。定时器任务内存函数的建议实现如清单 3.14 所示。


<a name="list3.14" title="Listing 3.14 Typical implementation of vApplicationGetTimerTaskMemory"></a>


```c
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
                                     StackType_t **ppxTimerTaskStackBuffer,
                                     uint32_t *pulTimerTaskStackSize )
{
  /* If the buffers to be provided to the Timer task are declared inside this
  function then they must be declared static - otherwise they will be allocated on
  the stack and hence would not exists after this function exits. */
  static StaticTask_t xTimerTaskTCB;
  static StackType_t uxTimerTaskStack[ configMINIMAL_STACK_SIZE ];

  /* Pass out a pointer to the StaticTask_t structure in which the Timer task's
  state will be stored. */
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

  /* Pass out the array that will be used as the Timer task's stack. */
  *ppxTimerTaskStackBuffer = uxTimerTaskStack;

  /* Pass out the stack size of the array pointed to by *ppxTimerTaskStackBuffer.
  Note the stack size is a count of StackType_t */
  *pulTimerTaskStackSize = sizeof(uxTimerTaskStack) / sizeof(*uxTimerTaskStack);
}
```
***清单 3.14*** *vApplicationGetTimerTaskMemory 的典型实现*


由于包括 SMP 在内的任何系统中都只有一个定时器任务，因此是定时器任务内存问题的有效解决方案
就是在`vApplicationGetTimeTaskMemory()`函数中分配静态缓冲区并将缓冲区指针返回给内核。


#### 3.4.2.2 vApplicationGetIdleTaskMemory

当核心用完计划的工作时，将运行空闲任务。空闲任务执行一些内务处理，也可以触发
用户的 `vTaskIdleHook()`（如果已启用）。  在对称多处理 system (SMP) 中，还有非内务处理
其余每个核心的空闲任务，但这些任务在内部静态分配给 `configMINIMAL_STACK_SIZE` 字节。

调用 `vApplicationGetIdleTaskMemory` 函数以允许应用程序为“主”创建所需的缓冲区
闲置任务。清单 3.15 显示了使用静态本地的 `vApplicationIdleTaskMemory()` 函数的典型实现
变量来创建所需的缓冲区。


<a name="list3.15" title="Listing 3.15 Typical implementation of vApplicationGetIdleTaskMemory"></a>

```c
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
{
  static StaticTask_t xIdleTaskTCB;
  static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

  *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
  *ppxIdleTaskStackBuffer = uxIdleTaskStack;
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
```
***清单 3.15*** *vApplicationGetIdleTaskMemory 的典型实现*

