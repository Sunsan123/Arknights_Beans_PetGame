# 项目文件总览

这份文档按当前仓库里的实际文件生成，帮助快速理解每个文件的作用。  
说明基于文件名、目录结构和源码内容整理；其中少量纯素材文件按命名语义做了概括。

## 根目录

- `README.md`：项目说明文档，介绍 TamaFi 的功能、硬件组成、行为逻辑和开发背景。
- `LICENSE`：MIT 许可证，说明该项目可以自由使用、修改和分发。
- `User_Setup.h`：给 `TFT_eSPI` 使用的屏幕驱动配置文件，定义 ST7789、分辨率和 SPI 引脚。
- `LOCAL_QUICKSTART.md`：本地快速跑通文档，说明如何用 Arduino IDE 把项目最快烧录起来。
- `install_esp32.ps1`：本地安装 ESP32 Arduino 开发板包的辅助脚本，是当前工作区里的运维脚本，不属于上游固件源码。

## `PCB/`

- `PCB/PCB Dimension.jpg`：PCB 外形和尺寸示意图，用于确认板子大小和布局。
- `PCB/Tamafi-v2-Gerber.zip`：PCB 生产用 Gerber 打包文件，可直接交给板厂打样。

## `Schematic/`

- `Schematic/Schematic.jpg`：原理图导出图片，用于查看整体电路连接。
- `Schematic/BOM.jpg`：物料清单的图片版，适合快速预览元件信息。
- `Schematic/Tamafi-BOM.xlsx`：完整 BOM 表格，通常用于采购、替代料比对和数量统计。

## `TamaFi/` 固件源码

- `TamaFi/TamaFi.ino`：主程序入口，负责硬件初始化、按键处理、宠物逻辑、WiFi 扫描、存档和主循环。
- `TamaFi/ui.cpp`：UI 绘制实现文件，负责主界面、菜单、状态页和动画显示。
- `TamaFi/ui.h`：UI 相关头文件，定义屏幕枚举、宠物状态结构体、共享变量和函数声明。
- `TamaFi/ui_anim.h`：动画时间参数配置，例如 idle、孵化、饥饿特效、休眠和死亡动画的帧间隔。
- `TamaFi/StoneGolem.h`：宠物主立绘/动作相关的大型图像数据头文件，供固件直接渲染。
- `TamaFi/egg_hatch.h`：宠物孵化动画图像数据头文件。
- `TamaFi/effect.h`：特效贴图数据头文件，主要用于饥饿或行为相关覆盖效果。
- `TamaFi/background.h`：背景图像数据头文件，用于主界面或页面背景绘制。

## `Ui Graphics/` 设计源文件

- `Ui Graphics/Pet.psd`：宠物角色图形的 Photoshop 源文件。
- `Ui Graphics/UI.psd`：界面设计稿的 Photoshop 源文件。
- `Ui Graphics/UI_240x240.psd`：针对 `240x240` 屏幕尺寸整理的 UI 源文件。

## `Ui Graphics/pic/` 位图素材

### 宠物基础动作

- `Ui Graphics/pic/1.png`：宠物动作素材之一，通常是基础动作或帧图的一部分。
- `Ui Graphics/pic/2.png`：宠物动作素材之一。
- `Ui Graphics/pic/3.png`：宠物动作素材之一。
- `Ui Graphics/pic/4.png`：宠物动作素材之一。
- `Ui Graphics/pic/idle1.png`：宠物待机动画第 1 帧。
- `Ui Graphics/pic/idle2.png`：宠物待机动画第 2 帧。
- `Ui Graphics/pic/idle3.png`：宠物待机动画第 3 帧。
- `Ui Graphics/pic/idle4.png`：宠物待机动画第 4 帧。
- `Ui Graphics/pic/attack_1.png`：宠物攻击/狩猎动画第 1 帧。
- `Ui Graphics/pic/attack_2.png`：宠物攻击/狩猎动画第 2 帧。
- `Ui Graphics/pic/attack_3.png`：宠物攻击/狩猎动画第 3 帧。

### 孵化与休眠相关

- `Ui Graphics/pic/egg_hatch_1.png`：孵化动画素材第 1 帧。
- `Ui Graphics/pic/egg_hatch_2.png`：孵化动画素材第 2 帧。
- `Ui Graphics/pic/egg_hatch_3.png`：孵化动画素材第 3 帧。
- `Ui Graphics/pic/egg_hatch_4.png`：孵化动画素材第 4 帧。
- `Ui Graphics/pic/egg_hatch_5.png`：孵化动画素材第 5 帧。
- `Ui Graphics/pic/egg_hatch_frame1.png`：蛋状态/孵化过程帧图第 1 张。
- `Ui Graphics/pic/egg_hatch_frame2.png`：蛋状态/孵化过程帧图第 2 张。
- `Ui Graphics/pic/egg_hatch_frame3.png`：蛋状态/孵化过程帧图第 3 张。
- `Ui Graphics/pic/egg_hatch_frame4.png`：蛋状态/孵化过程帧图第 4 张。

### 死亡、眼部与表情细节

- `Ui Graphics/pic/dead_1.png`：死亡动画第 1 帧。
- `Ui Graphics/pic/dead_2.png`：死亡动画第 2 帧。
- `Ui Graphics/pic/dead_3.png`：死亡动画第 3 帧。
- `Ui Graphics/pic/eye1.png`：眼部表情素材第 1 张。
- `Ui Graphics/pic/eye2.png`：眼部表情素材第 2 张。
- `Ui Graphics/pic/eye3.png`：眼部表情素材第 3 张。
- `Ui Graphics/pic/eye4.png`：眼部表情素材第 4 张。
- `Ui Graphics/pic/rip_ms.png`：死亡或墓碑相关画面素材。

### 特效与状态覆盖

- `Ui Graphics/pic/hunger1.png`：饥饿特效第 1 帧。
- `Ui Graphics/pic/hunger2.png`：饥饿特效第 2 帧。
- `Ui Graphics/pic/hunger3.png`：饥饿特效第 3 帧。
- `Ui Graphics/pic/hunger4.png`：饥饿特效第 4 帧。

### 背景与界面图

- `Ui Graphics/pic/backgroundImage.png`：背景图主版本。
- `Ui Graphics/pic/backgroundImage1.png`：背景图变体 1。
- `Ui Graphics/pic/backgroundImage2.png`：背景图变体 2。
- `Ui Graphics/pic/backgroundImage3.png`：背景图变体 3。
- `Ui Graphics/pic/backgroundImage4.png`：背景图变体 4。
- `Ui Graphics/pic/UI.jpg`：UI 设计稿导出预览图。

## 建议先看的文件

如果你是第一次接手这个项目，建议阅读顺序如下：

1. `README.md`
2. `LOCAL_QUICKSTART.md`
3. `TamaFi/TamaFi.ino`
4. `TamaFi/ui.h`
5. `TamaFi/ui.cpp`
6. `User_Setup.h`

## 备注

- `TamaFi/*.h` 里几个体积很大的图像头文件，本质上是把位图资源转成 C 数组后直接编进固件。
- `Ui Graphics/` 是设计源文件目录，更适合美术修改；`TamaFi/` 里的图片头文件更偏向最终固件使用。
- `install_esp32.ps1` 和 `LOCAL_QUICKSTART.md` 是当前工作区内新增的辅助文件，不是项目原始上游的一部分。
