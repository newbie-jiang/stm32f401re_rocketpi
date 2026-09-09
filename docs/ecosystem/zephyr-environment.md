# Zephyr环境搭建

哔哩哔哩：https://www.bilibili.com/video/BV1TF6ABHETc/?spm_id_from=333.1387.upload.video_card.click&vd_source=fa0653f564ca121b33d552a5a430612d

视频中的资料下载：百度网盘：https://pan.baidu.com/s/1yG5-ToPbYYhepjOfhhB04g?pwd=6fuj



支持的三种平台最推荐LInux下开发,因为在Linux下使用是功能最全的

Zephyr官方的环境搭建需要科学上网环境，可以参考官方文档：https://docs.zephyrproject.org/latest/develop/getting_started/index.html

按照官方的文档来安装，会劝退一部分人，所以我做了一个现成的镜像，只需要导入虚拟机就可以使用

大致的逻辑如图，将zephyr开发环境以及Rocket-PI 这款开发板的所需boards,以及开发工具都打包成现成的镜像，用vmware虚拟机打开即可

作者习惯在windows上编辑，所以就用ftp将zephyrproject的目录挂载到win本地（文件操作同步），SSH来编译

![zephyr_install (7)](https://cloud.rocketpi.club/cloud/zephyr_install (7).png)

按照视频教程安装好后，你应该可以编译一个简单的示例，编译没问题，环境就算ok了！

这是作者参考瑞芯微和全志的方法，直接提供镜像，省去麻烦的环境安装搭建，开箱即用