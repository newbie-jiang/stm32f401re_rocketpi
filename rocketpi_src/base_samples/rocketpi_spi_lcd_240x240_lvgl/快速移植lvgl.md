# keil快速移植LVGL

## 1.了解如何实现

keil中传统移植lvgl的方式为  

- 在工程中创建文件夹
- 依次添加文件
- 包含头文件

![image-20240922161243654](https://newbie-typora.oss-cn-shenzhen.aliyuncs.com/TyporaJPG/image-20240922161243654.png)

以上的这些步骤执行的结果实际上都保存在工程文件中，遵循XML语法

![image-20240922161404890](https://newbie-typora.oss-cn-shenzhen.aliyuncs.com/TyporaJPG/image-20240922161404890.png)

- 在工程中创建文件夹与添加文件 步骤实现如下

![image-20240922161640345](https://newbie-typora.oss-cn-shenzhen.aliyuncs.com/TyporaJPG/image-20240922161640345.png)

- 在工程中包含头文件路径实现如下

![image-20240922161906365](https://newbie-typora.oss-cn-shenzhen.aliyuncs.com/TyporaJPG/image-20240922161906365.png)

上述的这些语法很有规律，看着并不难，

## 2.直接使用配置好的xml语法

- 快速移植是直接使用已经移植好工程中的lvgl部分xml代码

前提条件：

- demo工程（keil工程）中首先测试自己的屏幕可以正常显示，并且颜色正常，
- 下载裁剪好的源码以及xml文件







## 移植步骤1：将lvgl放在 *.uvprojx的上一层目录

lvgl文件里面是裁剪好的源码

比如：

![image-20240922164019903](https://newbie-typora.oss-cn-shenzhen.aliyuncs.com/TyporaJPG/image-20240922164019903.png)

![image-20240922164051832](https://newbie-typora.oss-cn-shenzhen.aliyuncs.com/TyporaJPG/image-20240922164051832.png)



## 移植步骤2：添加到lvgl文件夹与文件部分xml到工程中

![image-20240922162914732](https://newbie-typora.oss-cn-shenzhen.aliyuncs.com/TyporaJPG/image-20240922162914732.png)

![image-20240922163251780](https://newbie-typora.oss-cn-shenzhen.aliyuncs.com/TyporaJPG/image-20240922163251780.png)

将文件夹与文件的代码放在结尾处   </Groups>上方（搜索一下就出来了）

![image-20240922164649622](https://newbie-typora.oss-cn-shenzhen.aliyuncs.com/TyporaJPG/image-20240922164649622.png)

执行完这个步骤keil工程中的文件与文件夹就出来了

## 移植步骤2：将lvgl的头文件添加到xml工程中

![image-20240922163321970](https://newbie-typora.oss-cn-shenzhen.aliyuncs.com/TyporaJPG/image-20240922163321970.png)

搜索一下<IncludePath> 添加到后面就可以了

![image-20240922165025084](https://newbie-typora.oss-cn-shenzhen.aliyuncs.com/TyporaJPG/image-20240922165025084.png)



编译无报错实现对应的接口就可以了



