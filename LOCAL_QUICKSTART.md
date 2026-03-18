# 本地快速跑通指南

这份文档是写给完全零基础用户的。  
目标只有一个：先把这个项目烧录进 `ESP32-S3`，并且让屏幕亮起来。

这不是一个 Node / Python 项目，而是一个基于 `ESP32-S3 + Arduino IDE` 的固件工程。  
最快跑通方式是：

`Arduino IDE + ESP32 开发板包 + 2 个库 + 正确接线 + 正确的 TFT_eSPI 配置 + 上传到 ESP32-S3`

项目主入口文件是：

- [`TamaFi/TamaFi.ino`](C:\Users\17787\Desktop\Arknights_Beans_PetGame\TamaFi\TamaFi.ino)

屏幕驱动配置在：

- [`User_Setup.h`](C:\Users\17787\Desktop\Arknights_Beans_PetGame\User_Setup.h)

## 0. 先说结论

如果你现在手上是：

- 一块 `ESP32-S3` 开发板
- 一块带排针的 `ST7789 240x240` SPI 屏幕模块
- 若干杜邦线
- 一根能传数据的 USB 线

那么你至少可以先做到：

1. 只接 `ESP32-S3 + 屏幕`
2. 暂时不接按钮、不接蜂鸣器、不接 NeoPixel
3. 在 Arduino IDE 里编译并上传
4. 看到屏幕亮起来

对于第一次上手的人，我建议你先完成“最小接线亮屏”，不要一开始就把所有外设都接满。

## 1. 你要准备什么

### 最少硬件

- 一块 `ESP32-S3` 开发板
- 一块 `ST7789 240x240` SPI 屏幕
- 一根 `USB 数据线`
- 7 到 8 根杜邦线

### 建议你额外准备

- 一块面包板
- 若干轻触按键
- 公对母 / 母对母杜邦线
- 已经焊好排针的开发板

### 最少软件

- `Arduino IDE`
- `ESP32 by Espressif Systems`
- `TFT_eSPI`
- `Adafruit NeoPixel`

## 2. 先分清你手上的硬件是哪一种

你大概率是下面两种情况之一：

### 情况 A：你拿到的是已经焊好的成品板 / 项目原 PCB

这种情况下通常不用自己一根一根接屏幕和按钮。  
你只需要：

- 用 USB 线连电脑
- 安装 Arduino IDE 和依赖
- 上传程序

如果你手上的板子本身已经把屏幕、按钮、蜂鸣器、灯珠都焊好了，这篇文档里“怎么接线”的部分你可以只当参考。

### 情况 B：你拿到的是普通 ESP32-S3 开发板 + 独立 TFT 屏幕模块

这种情况下你需要自己用杜邦线接线。  
如果你是这种情况，请继续往下看，重点看第 3 节和第 4 节。

## 3. 线怎么插，先理解 3 个最基础概念

### 1）什么叫“插到板子里”

通常不是把线硬塞进芯片里，而是：

- 开发板两侧有一排排针或排孔
- 屏幕模块边上也有一排针脚
- 你用杜邦线把“一个引脚”连到“另一个引脚”

比如：

- 屏幕的 `VCC` 接开发板的 `3V3`
- 屏幕的 `GND` 接开发板的 `GND`
- 屏幕的 `SCL` 接开发板的 `GPIO18`

“插到哪里”看的不是线的颜色，而是板子上印的字。

### 2）如果开发板没有焊排针，你是没法直接插杜邦线的

很多 ESP32-S3 开发板买回来两边只是金色焊盘，没有焊好的黑色排针。  
这种情况下：

- 你需要自己焊排针
- 或者换一块已经焊好排针的开发板

如果你完全不会焊，最省事的方式是直接买“已焊排针”的 ESP32-S3 开发板。

### 3）GPIO 是什么

`GPIO13`、`GPIO18`、`GPIO23` 这些名字，本质上就是开发板上的“编号引脚”。  
你接线的时候，只需要在开发板丝印上找到对应编号即可。

有些板子丝印写的是：

- `IO13`
- `GPIO13`
- `13`

它们通常说的是同一个引脚。

## 4. 最小接线：先只接屏幕

如果你的目标只是先亮屏，先只接下面这些线。

### 4.1 屏幕常见引脚长什么样

很多 ST7789 SPI 屏幕模块边上会印这些字：

- `VCC` 或 `3V3`
- `GND`
- `SCL` 或 `CLK`
- `SDA` 或 `MOSI`
- `RES` 或 `RST`
- `DC` 或 `A0`
- `CS`
- `BLK` / `LED`（背光）

名字不同很正常，只要功能对上就行。

### 4.2 这个项目当前参考接线

下面这组来自仓库里的 [`User_Setup.h`](C:\Users\17787\Desktop\Arknights_Beans_PetGame\User_Setup.h)：

| 屏幕模块引脚 | 接到 ESP32-S3 的哪里 |
| --- | --- |
| `VCC` | `3V3` |
| `GND` | `GND` |
| `SDA` / `MOSI` | `GPIO23` |
| `SCL` / `CLK` | `GPIO18` |
| `CS` | `GPIO22` |
| `DC` / `A0` | `GPIO21` |
| `RES` / `RST` | `GPIO12` |
| `BLK` / `LED` | `3V3` |

也就是说，第一次接线时，你可以按下面这个思路一根一根接：

1. 屏幕 `VCC` 接开发板 `3V3`
2. 屏幕 `GND` 接开发板 `GND`
3. 屏幕 `SDA` 接开发板 `GPIO23`
4. 屏幕 `SCL` 接开发板 `GPIO18`
5. 屏幕 `CS` 接开发板 `GPIO22`
6. 屏幕 `DC` 接开发板 `GPIO21`
7. 屏幕 `RST` 接开发板 `GPIO12`
8. 屏幕 `BLK` 接开发板 `3V3`

### 4.3 给完全新手的实际操作顺序

建议你按这个顺序插线：

1. 先不要给板子通电
2. 找到开发板上的 `3V3` 和 `GND`
3. 先接好屏幕的 `VCC` 和 `GND`
4. 再接信号线：`SDA`、`SCL`、`CS`、`DC`、`RST`
5. 最后接 `BLK`
6. 检查一遍有没有把 `3V3` 插错到 `5V`
7. 确认没问题后，再插 USB 到电脑

### 4.4 非常重要的安全提醒

- 如果你不确定屏幕是否支持 `5V`，就先接 `3V3`，这是更稳妥的选择
- 接线时尽量在断电状态下进行
- 插反 `VCC` / `GND` 有可能直接损坏屏幕或开发板
- 如果你看不清板子丝印，先别上电，先拍照放大确认

## 5. 按钮怎么接

这个项目代码里当前按钮引脚定义在 [`TamaFi/TamaFi.ino`](C:\Users\17787\Desktop\Arknights_Beans_PetGame\TamaFi\TamaFi.ino)：

- `UP = GPIO13`
- `OK = GPIO12`
- `DOWN = GPIO11`
- `RIGHT1 = GPIO8`
- `RIGHT2 = GPIO9`
- `RIGHT3 = GPIO10`

### 5.1 按钮的最简单接法

这些按钮在代码里使用的是 `INPUT_PULLUP`，这意味着最简单接法是：

- 按钮一侧接某个 `GPIO`
- 按钮另一侧接 `GND`

例如 `UP` 键：

- 按钮一侧接 `GPIO13`
- 另一侧接 `GND`

不需要你再额外接电阻。

### 5.2 先只接 3 个左侧按钮就够了

如果你只是想进入菜单，优先接这 3 个：

- `UP -> GPIO13`
- `OK -> GPIO12`
- `DOWN -> GPIO11`

### 5.3 这里有一个你必须知道的冲突

仓库当前参考配置里：

- 屏幕 `RST` 用的是 `GPIO12`
- `OK` 按钮也用的是 `GPIO12`

这对手工飞线原型来说是冲突的。  
也就是说，如果你是自己拿杜邦线一根一根接，而不是用项目原始 PCB，那么不要直接把“屏幕复位”和“OK 按钮”同时都接到 `GPIO12`。

### 5.4 新手最稳的做法

我建议你这样做：

1. 第一轮先只接屏幕，不接任何按钮
2. 先把程序烧进去，确认屏幕能亮
3. 亮屏以后，再决定怎么处理 `GPIO12` 冲突

### 5.5 冲突怎么处理

你有两个常见选择：

#### 方案 A：保留屏幕 `RST = GPIO12`

那么你需要把 `OK` 按钮改到别的空闲 GPIO，并修改 [`TamaFi/TamaFi.ino`](C:\Users\17787\Desktop\Arknights_Beans_PetGame\TamaFi\TamaFi.ino) 里的：

- `#define BTN_OK 12`

改成你实际接的 GPIO。

#### 方案 B：保留 `OK = GPIO12`

那么你需要修改 [`User_Setup.h`](C:\Users\17787\Desktop\Arknights_Beans_PetGame\User_Setup.h) 里的：

- `#define TFT_RST 12`

把它改成别的 GPIO，或者改成适合你屏幕模块的接法。

如果你现在只是想“先跑起来”，先别急着接按钮，先完成最小亮屏。

## 6. 蜂鸣器、NeoPixel、背光这些先别急

代码里还定义了这些引脚：

- `NeoPixel 数据脚 = GPIO1`
- `蜂鸣器 = GPIO2`
- `TFT 背光 PWM = GPIO7`

但对于第一次上手的人，这些都不是亮屏和烧录必须项。  
你可以先完全不接这些外设。

## 7. 安装 Arduino IDE

先安装 `Arduino IDE`。  
安装完之后先不要急着点上传，先把板卡包和依赖库装好。

## 8. 安装 ESP32 开发板包

在 Arduino IDE 里这样操作：

1. 打开 Arduino IDE
2. 点击左侧或顶部的 `开发板管理器`
3. 搜索 `esp32`
4. 找到 `Espressif Systems` 提供的 `ESP32`
5. 点击安装

安装完成后，开发板先选择：

- `ESP32S3 Dev Module`

如果你非常确定自己用的是某个具体型号，也可以改成对应型号。  
如果你不确定，先用 `ESP32S3 Dev Module`。

## 9. 安装依赖库

在 Arduino IDE 的 `库管理器` 里安装两个库：

1. `TFT_eSPI`
2. `Adafruit NeoPixel`

如果搜不到，先确认网络正常，再重开一次 Arduino IDE。

## 10. 替换 TFT_eSPI 的 `User_Setup.h`

这是很多人最容易漏掉的一步。  
这个项目能不能亮屏，很大程度取决于 `TFT_eSPI` 的配置是不是和你的接线一致。

### 10.1 你要做什么

找到你电脑里的 `TFT_eSPI` 库目录，然后把仓库里的：

- [`User_Setup.h`](C:\Users\17787\Desktop\Arknights_Beans_PetGame\User_Setup.h)

复制过去，覆盖 `TFT_eSPI` 自带的同名文件。

### 10.2 常见位置

常见路径一般类似：

- `Documents\Arduino\libraries\TFT_eSPI\User_Setup.h`

有些人也可能在：

- `C:\Users\你的用户名\Documents\Arduino\libraries\TFT_eSPI\User_Setup.h`

### 10.3 新手建议

覆盖之前你可以先备份一份原文件，比如改名成：

- `User_Setup_backup.h`

这样如果你后面要恢复原配置，会更方便。

## 11. 打开工程

在 Arduino IDE 里打开：

- [`TamaFi/TamaFi.ino`](C:\Users\17787\Desktop\Arknights_Beans_PetGame\TamaFi\TamaFi.ino)

Arduino IDE 会自动把同目录下的其他 `.cpp` 和 `.h` 一起编译，不需要你手动一个个添加。

## 12. 板子怎么接到电脑

### 12.1 用什么线

你需要的是“能传数据的 USB 线”，不是只能充电的线。  
很多“只能充电”的线会导致：

- 板子通电了
- 但电脑看不到串口
- Arduino IDE 也无法上传

### 12.2 插到哪里

把 USB 线一头插电脑，一头插 ESP32-S3 开发板的 USB 口。  
如果板子上有多个 USB 口，优先插带下载 / 烧录功能的那个口。

### 12.3 怎么判断电脑识别到了

插上板子前后，对比 Arduino IDE 的 `端口` 列表。  
哪个端口是“插上后新出现的”，通常就是它。

## 13. 选择板子和串口

在 Arduino IDE 顶部确认两件事：

- `开发板`: `ESP32S3 Dev Module`
- `端口`: 选择你刚插上板子后出现的新串口

如果端口列表里什么都没有，先不要怀疑代码，先检查：

- USB 线是不是数据线
- 驱动有没有装好
- 换一个 USB 口是否能识别

## 14. 上传程序

点击 Arduino IDE 里的 `上传`。

第一次上传可能会稍慢，这是正常的。  
如果一切正常，编译完成后程序会写进板子里。

项目启动时会在 [`TamaFi/TamaFi.ino`](C:\Users\17787\Desktop\Arknights_Beans_PetGame\TamaFi\TamaFi.ino) 的 `setup()` 里依次做这些事：

- 初始化屏幕
- 初始化按钮输入
- 初始化背光 PWM
- 初始化蜂鸣器 PWM
- 初始化 WiFi
- 从 `Preferences` 读取存档

## 15. 如果上传失败

先看下面这些最常见原因。

### 15.1 找不到库

表现：

- `TFT_eSPI.h: No such file or directory`
- `Adafruit_NeoPixel.h: No such file or directory`

处理：

- 回到 `库管理器`，确认这两个库已经安装

### 15.2 板子能亮，但没有串口

优先检查：

- USB 线是不是数据线
- 换一个电脑 USB 口试试
- 换一根 USB 线试试

### 15.3 上传时报下载模式错误

有些 ESP32-S3 板子需要手动进下载模式。  
常见做法是：

1. 按住 `BOOT`
2. 短按一下 `RESET`
3. 松开 `RESET`
4. 再松开 `BOOT`
5. 然后重新点击上传

不同板子按键名字可能略有不同，但思路基本类似。

## 16. 首次成功的标准

如果你现在只接了屏幕，那么最基本的成功标准是：

- 板子能被 Arduino IDE 识别
- 代码能编译通过
- 程序能上传成功
- 屏幕亮起
- 能看到启动画面或宠物界面

如果你后面又接了按钮，那么继续检查：

- `UP / DOWN / OK` 是否能操作菜单

## 17. 最常见的卡点

### 1）能编译、能上传，但屏幕不亮

优先检查：

- 屏幕是不是 `ST7789`
- 分辨率是不是 `240x240`
- `User_Setup.h` 是否真的已经覆盖到 `TFT_eSPI` 库目录
- 你的实际接线是否和文档里的引脚一致
- `VCC` 是否误接到错误电压
- `GND` 是否真的共地

### 2）屏幕亮了，但花屏 / 颜色乱 / 不稳定

优先检查：

- `SDA/MOSI` 有没有接错
- `SCL/CLK` 有没有接错
- `CS/DC/RST` 有没有接错
- 杜邦线是否松动

### 3）接了按钮以后行为异常

先检查你有没有踩到 `GPIO12` 冲突。  
如果你把屏幕 `RST` 和 `OK` 按钮都直接接到了 `GPIO12`，先把按钮断开，再测试一次。

### 4）上传成功了，但每次表现不一样

这个项目会把状态存到 `Preferences` 里。  
所以有时候“奇怪状态”不一定是代码坏了，也可能是之前存下来的数据在影响当前启动结果。

## 18. 给完全新手的最短执行顺序

如果你只想尽快看到结果，就按这个顺序做：

1. 准备好 `ESP32-S3 + ST7789 屏幕 + 杜邦线 + USB 数据线`
2. 先只接屏幕，不接按钮
3. 屏幕按表连接到 `3V3 / GND / GPIO23 / GPIO18 / GPIO22 / GPIO21 / GPIO12`
4. 安装 `Arduino IDE`
5. 安装 `ESP32 by Espressif Systems`
6. 安装 `TFT_eSPI` 和 `Adafruit NeoPixel`
7. 用仓库里的 [`User_Setup.h`](C:\Users\17787\Desktop\Arknights_Beans_PetGame\User_Setup.h) 覆盖 `TFT_eSPI` 的同名文件
8. 打开 [`TamaFi/TamaFi.ino`](C:\Users\17787\Desktop\Arknights_Beans_PetGame\TamaFi\TamaFi.ino)
9. 选择 `ESP32S3 Dev Module`
10. 选择串口
11. 点击上传
12. 先确认屏幕亮起来
13. 亮屏之后，再考虑接按钮和其他外设

## 19. 这个仓库里和启动最相关的文件

- [`TamaFi/TamaFi.ino`](C:\Users\17787\Desktop\Arknights_Beans_PetGame\TamaFi\TamaFi.ino)：主程序入口、引脚定义、初始化逻辑
- [`User_Setup.h`](C:\Users\17787\Desktop\Arknights_Beans_PetGame\User_Setup.h)：`TFT_eSPI` 的屏幕和 SPI 引脚配置
- [`TamaFi/ui.cpp`](C:\Users\17787\Desktop\Arknights_Beans_PetGame\TamaFi\ui.cpp)：UI 绘制
- [`TamaFi/ui.h`](C:\Users\17787\Desktop\Arknights_Beans_PetGame\TamaFi\ui.h)：共享状态和 UI 结构
- [`README.md`](C:\Users\17787\Desktop\Arknights_Beans_PetGame\README.md)：项目背景和功能概览

## 20. 备注

这份文档是根据仓库里的源码和配置整理的，没有在当前机器上实际执行 Arduino 编译，因为本机没有安装 `arduino-cli`，仓库里也没有现成的自动化构建脚本。
