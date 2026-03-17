# TamaFi 虚拟宠物游戏 - 项目架构文档

## Context

本文档对 TamaFi V2 虚拟宠物游戏项目进行完整的架构分析。TamaFi 是一个基于 ESP32-S3 的 WiFi 感知虚拟宠物设备，宠物通过扫描周围 WiFi 网络来"进食"，拥有自主决策引擎、多阶段进化系统和完整的菜单 UI。本文档旨在帮助开发者理解项目整体运作原理和技术实现细节。

---

## 1. 项目概述和核心功能特性

### 1.1 项目定位

TamaFi 是一个现代化的电子宠物（类拓麻歌子）项目，核心创新点在于宠物以 **WiFi 信号为食物来源**。设备上的宠物会自主扫描周围 WiFi 网络，根据网络数量、信号强度、加密类型等信息获取营养。

### 1.2 核心功能

| 功能模块 | 说明 |
|---------|------|
| **WiFi 供养** | 宠物通过扫描 WiFi 网络获取饥饿/幸福/健康值 |
| **自主决策引擎** | 宠物根据自身状态和环境自主选择捕猎/探索/休息/闲置 |
| **多阶段进化** | BABY -> TEEN -> ADULT -> ELDER，基于年龄和平均属性 |
| **心情系统** | 7 种心情状态，影响动画速度、LED 颜色、行为倾向 |
| **个性特征** | 每只宠物随机生成好奇心/活动力/压力 3 项特征 |
| **持久化存储** | 所有状态通过 NVS 持久化，断电不丢失 |
| **完整菜单 UI** | 11 个屏幕页面，6 按钮导航（含快捷键） |
| **复古音效引擎** | 非阻塞式芯片音乐序列播放器 |
| **NeoPixel LED 反馈** | 4 颗 WS2812 RGB LED 提供心情/活动视觉反馈 |

### 1.3 技术栈

- **平台**: ESP32-S3 (Arduino 框架)
- **显示**: TFT ST7789 240x240, TFT_eSPI 库
- **LED**: Adafruit_NeoPixel 库
- **WiFi**: ESP32 WiFi 库（仅扫描，不连接）
- **存储**: Preferences 库 (NVS)
- **开发环境**: Arduino IDE

---

## 2. 硬件架构

### 2.1 系统框图

```
                    ┌─────────────────────────────┐
                    │        ESP32-S3 MCU          │
                    │                              │
  ┌────────────┐    │  GPIO 23 (MOSI) ──┐         │    ┌──────────────┐
  │  6x 按钮   │────│  GPIO 18 (SCLK) ──┼── SPI ──│───>│ ST7789 TFT   │
  │ (INPUT_PULLUP)  │  GPIO 22 (CS)   ──┘         │    │ 240x240      │
  └────────────┘    │  GPIO 21 (DC)               │    └──────────────┘
                    │  GPIO 12 (RST)              │
                    │                              │
                    │  GPIO 7 ── PWM CH0 ─────────│───> TFT 背光
                    │  GPIO 2 ── PWM CH5 ─────────│───> 蜂鸣器
                    │  GPIO 1 ── DATA ────────────│───> 4x WS2812 LED
                    │                              │
                    │  WiFi Radio (扫描模式) ──────│───> 环境感知
                    │  NVS Flash ─────────────────│───> 持久化存储
                    │                              │
                    │  USB-C (原生 USB/CP2102) ────│───> 供电 & 烧录
                    │  TP4056 + 锂电池 ───────────│───> 电池供电
                    └─────────────────────────────┘
```

### 2.2 GPIO 引脚分配

| GPIO | 功能 | 模式 | 说明 |
|------|------|------|------|
| 13 | BTN_UP | INPUT_PULLUP | 左侧上方按钮 |
| 12 | BTN_OK | INPUT_PULLUP | 左侧中间确认按钮 |
| 11 | BTN_DOWN | INPUT_PULLUP | 左侧下方按钮 |
| 8 | BTN_RIGHT1 | INPUT_PULLUP | 右侧按钮 1（快捷: 宠物状态） |
| 9 | BTN_RIGHT2 | INPUT_PULLUP | 右侧按钮 2（快捷: 环境信息） |
| 10 | BTN_RIGHT3 | INPUT_PULLUP | 右侧按钮 3（快捷: 诊断） |
| 23 | TFT_MOSI | SPI | TFT 数据线 |
| 18 | TFT_SCLK | SPI | TFT 时钟线 |
| 22 | TFT_CS | SPI | TFT 片选 |
| 21 | TFT_DC | SPI | TFT 数据/命令 |
| 12 | TFT_RST | OUTPUT | TFT 复位 |
| 7 | TFT_BRIGHTNESS | PWM (CH0, 12kHz, 8bit) | TFT 背光亮度 |
| 2 | BUZZER | PWM (CH5, 4kHz, 8bit) | 蜂鸣器 |
| 1 | LED_PIN | NeoPixel DATA | 4x WS2812-2020 RGB LED |

### 2.3 显示屏规格

- **驱动芯片**: ST7789
- **分辨率**: 240 x 240 像素（物理屏幕 240x340，使用 240x240 区域）
- **色深**: 16 位 RGB565
- **接口**: SPI (写入 27MHz / 读取 20MHz)
- **背光控制**: MOSFET + PWM，支持 3 档亮度 (60/150/255)

### 2.4 电源系统

- **充电芯片**: TP4056 单节锂电池充电管理
- **输入接口**: USB-C
- **背光控制**: MOSFET 控制 TFT LED 引脚，PWM 调光

---

## 3. 软件架构

### 3.1 文件结构

```
Arknights_Beans_PetGame/
├── TamaFi/
│   ├── TamaFi.ino        [~32KB] 主程序：初始化、游戏循环、逻辑、按钮、WiFi、音效、存储
│   ├── ui.h               [~3KB]  UI 头文件：枚举定义、结构体声明、extern 变量
│   ├── ui.cpp             [~24KB] UI 实现：所有屏幕渲染函数、菜单绘制、动画逻辑
│   ├── ui_anim.h          [~1KB]  动画常量：各类动画的帧延迟时间定义
│   ├── StoneGolem.h       [~1.8MB] 宠物精灵数据（idle/attack/dead 帧，115x110px）
│   ├── egg_hatch.h        [~480KB] 蛋孵化精灵帧（5 帧孵化序列 + 4 帧蛋空闲）
│   ├── effect.h           [~484KB] 特效精灵（饥饿效果 4 帧叠加层，100x95px）
│   └── background.h       [~1.4MB] 背景图像数据（340x240px）
├── PCB/                    PCB Gerber 文件和尺寸图
├── Schematic/              电路原理图和 BOM
├── Ui Graphics/            UI 设计源文件 (PSD) 和 PNG 精灵资源
├── User_Setup.h            TFT_eSPI 库硬件引脚配置
├── README.md               项目说明文档
└── LICENSE                 MIT 开源许可
```

### 3.2 模块职责划分

```
┌─────────────────────────────────────────────────────────────────┐
│                        TamaFi.ino (主模块)                       │
│                                                                  │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐           │
│  │ 硬件初始化 │ │ 游戏逻辑  │ │ WiFi 扫描 │ │ 音效引擎  │           │
│  │ setup()   │ │logicTick│ │startWifi │ │sndUpdate│           │
│  └──────────┘ │updateMood│ │checkDone │ │sndClick │           │
│               │updateEvo │ │resolveHunt│ │sndGood  │           │
│  ┌──────────┐ │decideNext│ │resolveDisc│ └──────────┘           │
│  │ 按钮处理  │ └──────────┘ └──────────┘                         │
│  │handleBtns│                             ┌──────────┐          │
│  └──────────┘ ┌──────────┐               │ LED 控制  │          │
│               │ 持久化    │               │ledsHappy │          │
│  ┌──────────┐ │saveState │               │ledsSad   │          │
│  │ 休息状态机│ │loadState │               │ledsWifi  │          │
│  │ stepRest │ └──────────┘               └──────────┘          │
│  └──────────┘                                                    │
└──────────────────────────┬──────────────────────────────────────┘
                           │ 调用
┌──────────────────────────▼──────────────────────────────────────┐
│                   ui.cpp / ui.h (UI 模块)                        │
│                                                                  │
│  ┌────────────┐  ┌────────────────┐  ┌────────────────────────┐ │
│  │ 屏幕渲染    │  │ 精灵帧表管理    │  │ 菜单动画 & 工具函数     │ │
│  │screenHome  │  │ IDLE_FRAMES    │  │ animateSelector       │ │
│  │screenMenu  │  │ EGG_FRAMES     │  │ drawBar               │ │
│  │screenHatch │  │ ATTACK_FRAMES  │  │ uiOnScreenChange      │ │
│  │screenStatus│  │ HUNGER_FRAMES  │  │ uiInit                │ │
│  │screenEnv   │  │ DEAD_FRAMES    │  └────────────────────────┘ │
│  │ ... 11个    │  └────────────────┘                             │
│  └────────────┘                                                  │
└─────────────────────────────────────────────────────────────────┘
```

### 3.3 主循环数据流

```
loop() [每帧执行]
  │
  ├─> sndUpdate()                    // 非阻塞音符序列推进
  ├─> stopBuzzerIfNeeded()           // 蜂鸣器超时静音
  │
  ├─> logicTick() [每 100ms]         // 核心游戏逻辑
  │   ├─> 属性衰减 (hunger/happiness/health)
  │   ├─> 年龄增长 (分->时->天)
  │   ├─> 饥饿特效管理
  │   ├─> WiFi 扫描完成检查 -> resolveHunt()/resolveDiscover()
  │   ├─> stepRest() (休息状态机)
  │   ├─> updateMood() (心情重新评估)
  │   ├─> updateEvolution() (进化条件检查)
  │   ├─> 死亡检测 (三项属性均为 0)
  │   ├─> 自动保存 (每 30s)
  │   └─> decideNextActivity() (仅 HOME 屏幕 + 空闲时)
  │
  ├─> handleButtons()                // 按钮边沿检测 & 屏幕导航
  │
  └─> uiDrawScreen()                 // 当前屏幕渲染 -> 推送到 TFT
```

---

## 4. 核心系统分析

### 4.1 宠物状态管理

#### 数据结构 (`ui.h`)

```cpp
struct Pet {
  int hunger;              // 0-100，饥饿度
  int happiness;           // 0-100，幸福感
  int health;              // 0-100，健康值
  unsigned long ageMinutes;
  unsigned long ageHours;
  unsigned long ageDays;
};
```

#### 属性衰减规则

| 属性 | 衰减周期 | 正常衰减 | 加速衰减条件 | 加速衰减量 |
|------|---------|---------|------------|----------|
| hunger | 5 秒 | -2 | -- | -- |
| happiness | 7 秒 | -1 | 无 WiFi > 30 秒 | -3 |
| health | 10 秒 | -1 | hunger < 20 或 happiness < 20 | -2 |

所有属性被 `constrain()` 限制在 0-100 范围内。

#### 死亡条件

当 `hunger <= 0 && happiness <= 0 && health <= 0` 时触发游戏结束，跳转 `SCREEN_GAMEOVER`，显示死亡动画并闪烁红色 LED。玩家可按 OK 键重置宠物重新开始。

### 4.2 情绪系统

7 种心情按优先级从高到低判定：

| 优先级 | 心情 | 触发条件 |
|-------|------|---------|
| 1 | MOOD_SICK | health < 25 或 WiFi 缺失 > 60s |
| 2 | MOOD_HUNGRY | hunger < 25 |
| 3 | MOOD_EXCITED | happiness > 80 且 WiFi 网络数 > 8 |
| 4 | MOOD_HAPPY | happiness > 60 且 WiFi 网络数 > 0 |
| 5 | MOOD_BORED | WiFi 缺失 > 30s |
| 6 | MOOD_CURIOUS | 附近有隐藏网络或开放网络 |
| 7 | MOOD_CALM | 默认状态 |

**心情影响**:
- **动画速度**: EXCITED = 120ms/帧, CALM = 200ms/帧, BORED/SICK = 280ms/帧
- **LED 颜色**: 对应心情显示不同颜色
- **自主决策**: 影响各活动的欲望评分（详见 4.4）

### 4.3 进化机制

4 个生命阶段，不可逆转：

| 当前阶段 | 目标阶段 | 年龄要求 | 平均属性要求 |
|---------|---------|---------|------------|
| BABY | TEEN | >= 20 分钟 | avg(hunger, happiness, health) > 35 |
| TEEN | ADULT | >= 60 分钟 | avg > 45 |
| ADULT | ELDER | >= 180 分钟 | avg > 40 |

进化触发时播放发现音效 (`sndDiscover()`) 并闪烁 WiFi 蓝色 LED。

### 4.4 WiFi 交互系统

#### 扫描流程

```
startWifiScan()              // 异步启动
  │  WiFi.mode(WIFI_STA)
  │  WiFi.disconnect(true)
  │  WiFi.scanNetworks(true) // async=true
  ▼
logicTick() 轮询
  │  checkWifiScanDone()
  │  WiFi.scanComplete() > 0 ?
  ▼
统计网络特征 -> WifiStats
  │  netCount, strongCount (>-60dBm)
  │  hiddenCount, openCount, wpaCount
  │  avgRSSI
  ▼
resolveHunt() 或 resolveDiscover()
```

#### 捕猎 (Hunt) 收益计算

- **无网络**: hunger -15, happiness -10, health -5（播放失败音效）
- **有网络**:
  - `hungerDelta = min(35, netCount*2 + strongCount*3)`
  - `happyDelta = min(30, varietyScore*3 + (avgRSSI+100)/3)`
  - `healthDelta`: avgRSSI > -75 时 +5, > -65 时再 +5, strongCount > 5 时再 +3

#### 探索 (Discover) 收益计算

- `curiosity = hiddenCount*4 + openCount*3 + netCount`
- `happyDelta = min(35, curiosity/2)`
- `hungerDelta = -5`（轻微消耗）

#### 自主决策引擎

每 8-15 秒（随机间隔）宠物在 HOME 屏幕空闲时自主决策：

```
欲望计算:
  desireHunt = (100 - hunger) + traitCuriosity/2
  desireDisc = traitCuriosity + hiddenCount*10 + openCount*6
  desireRest = (100 - health) + traitStress/2
  desireIdle = 10（基线）

心情调整:
  HUNGRY  -> desireHunt +20, desireRest -10
  CURIOUS -> desireDisc +15
  SICK    -> desireRest +20, desireDisc -10
  EXCITED -> desireDisc +10, desireHunt +5
  BORED   -> desireDisc +10, desireHunt +5

选择最高欲望的活动执行
```

### 4.5 休息状态机

```
REST_NONE ──[决策选择休息]──> REST_ENTER
                                │
                          蛋帧 5->4->3->2->1
                          每帧 400ms
                                │
                                ▼
                            REST_DEEP
                          蓝色呼吸 LED
                          持续 5-15s
                          中点恢复属性:
                            hunger -3
                            happiness +10
                            health +15
                                │
                                ▼
                            REST_WAKE
                          蛋帧 1->2->3->4->5
                          播放苏醒音效
                                │
                                ▼
                            REST_NONE (回到空闲)
```

### 4.6 个性特征系统

首次启动时随机生成，永久保存：

| 特征 | 取值范围 | 影响 |
|------|---------|------|
| traitCuriosity | 40-90 | 提高探索欲望，增强对隐藏/开放网络的反应 |
| traitActivity | 30-90 | 影响自主活动频率 |
| traitStress | 20-80 | 提高休息欲望，增强对环境压力的敏感度 |

---

## 5. 用户界面架构

### 5.1 渲染管线

采用三层精灵缓冲的双缓冲架构，消除画面闪烁：

```
┌─────────────────┐     ┌─────────────────┐
│ petSprite       │────>│                 │
│ 115x110, 16bit  │     │   fb (帧缓冲)    │     ┌──────────┐
└─────────────────┘     │   240x240       │────>│ TFT 屏幕  │
┌─────────────────┐     │   16bit RGB565  │     │ 240x240  │
│ effectSprite    │────>│                 │     └──────────┘
│ 100x95, 16bit   │     └─────────────────┘
└─────────────────┘       pushSprite(0,0)

精灵使用 TFT_WHITE 作为透明色键
```

### 5.2 屏幕状态机

```
SCREEN_BOOT ──[任意按钮]──> SCREEN_HATCH (首次) 或 SCREEN_HOME (已孵化)
                                │
SCREEN_HATCH ──[OK 按钮]──> SCREEN_HOME
                                │
                                ├──[OK]──> SCREEN_MENU
                                │            ├─[0] SCREEN_PET_STATUS
                                │            ├─[1] SCREEN_ENVIRONMENT
                                │            ├─[2] SCREEN_SYSINFO
                                │            ├─[3] SCREEN_CONTROLS
                                │            ├─[4] SCREEN_SETTINGS
                                │            ├─[5] SCREEN_DIAGNOSTICS
                                │            └─[6] 返回 HOME
                                │
                                ├──[R1]──> SCREEN_PET_STATUS (快捷)
                                ├──[R2]──> SCREEN_ENVIRONMENT (快捷)
                                └──[R3]──> SCREEN_DIAGNOSTICS (快捷)

SCREEN_GAMEOVER ──[OK]──> SCREEN_HATCH (重置重生)
```

### 5.3 HOME 屏幕图层合成

```
图层 1: 黑色背景填充
图层 2: 背景图像 (backgroundImage)
图层 3: 宠物精灵 (根据活动状态选择 idle/attack/egg/dead 帧)
图层 4: 状态条 (红色=饥饿, 黄色=幸福, 绿色=健康)
图层 5: 文本叠加 (心情、阶段、活动标签)
图层 6: 饥饿效果叠加层 (当 hungerEffectActive 时, 4帧循环)
```

### 5.4 菜单系统

主菜单 7 项，使用 UP/DOWN 滚动，OK 确认：

| 索引 | 菜单项 | 目标屏幕 | 内容 |
|------|--------|---------|------|
| 0 | Pet Status | SCREEN_PET_STATUS | 属性值、心情、年龄、阶段、特征描述 |
| 1 | Environment | SCREEN_ENVIRONMENT | WiFi 统计（网络数/强信号/隐藏/开放/RSSI） |
| 2 | System Info | SCREEN_SYSINFO | 固件版本、MCU 型号、堆内存、运行时间 |
| 3 | Controls | SCREEN_CONTROLS | TFT 亮度、LED 亮度、声音开关、NeoPixel 开关 |
| 4 | Settings | SCREEN_SETTINGS | 主题、自动睡眠、自动保存、重置选项 |
| 5 | Diagnostics | SCREEN_DIAGNOSTICS | 当前活动、心情、休息阶段、WiFi 状态 |
| 6 | Back | SCREEN_HOME | 返回主页 |

菜单选中项使用动画高亮条（灰色底 + 青色边框），通过 `animateSelector()` 实现平滑滚动。

### 5.5 动画帧配置 (`ui_anim.h`)

| 动画类型 | 帧延迟 | 帧数 | 说明 |
|---------|-------|------|------|
| 蛋空闲 | 350ms | 4 | 孵化前蛋的摇晃动画 |
| 孵化 | 300ms | 5 | 蛋裂开到宠物出现 |
| 宠物空闲（基础） | 200ms | 4 | 正常呼吸/待机动画 |
| 宠物空闲（兴奋） | 120ms | 4 | 加速动画 |
| 宠物空闲（无聊/病） | 280ms | 4 | 减速动画 |
| 攻击/捕猎 | 300ms | 3 | WiFi 扫描时的攻击动画 |
| 饥饿效果 | 100ms | 4 | 低饥饿时的视觉警告 |
| 休息进入/唤醒 | 400ms | 5 | 入睡和苏醒过渡动画 |
| 呼吸周期 | 400ms | - | 睡眠时 LED 正弦波明暗 |
| 死亡 | 300ms | 3 | 游戏结束动画 |

---

## 6. 代码组织和关键实现细节

### 6.1 TamaFi.ino 代码结构（按功能区域）

| 行号范围 | 功能区域 |
|---------|---------|
| 1-55 | 库引入、宏定义、全局变量声明 |
| 56-100 | 定时器变量、状态标志 |
| 100-220 | 按钮处理函数 (`buttonPressed`, `handleButtons`) |
| 223-306 | 音效系统 (`RetroSound` 结构体、7 种音效定义、`sndUpdate`) |
| 315-359 | WiFi 扫描 (`startWifiScan`, `checkWifiScanDone`) |
| 362-428 | 持久化存储 (`saveState`, `loadState`) |
| 431-480 | 心情和进化更新 (`updateMood`, `updateEvolution`) |
| 483-559 | 休息状态机 (`stepRest`) |
| 564-615 | 活动解算 (`resolveHunt`, `resolveDiscover`) |
| 618-691 | 自主决策引擎 (`decideNextActivity`) |
| 693-715 | LED 控制函数 (`ledsHappy`, `ledsSad`, `ledsWifi`, etc.) |
| 717-809 | 逻辑更新主函数 (`logicTick`) |
| 810-1070 | 按钮处理各屏幕分支逻辑 |
| 1074-1144 | `setup()` 初始化 |
| 1146-1168 | `loop()` 主循环 |

### 6.2 ui.cpp 代码结构

| 功能区域 | 说明 |
|---------|------|
| 精灵帧表定义 | 各动画的 `const uint16_t*` 数组 |
| `uiInit()` | UI 模块初始化 |
| `uiOnScreenChange()` | 屏幕切换回调，重置动画和高亮位置 |
| `uiDrawScreen()` | 顶层分发函数，根据 currentScreen 调用对应渲染器 |
| `screenBoot()` | 启动画面 |
| `screenHatch()` | 孵化界面（蛋空闲 + 孵化动画） |
| `screenHome()` | 主界面渲染（背景+精灵+状态条+文本+特效） |
| `screenMenu()` | 主菜单（7项+图标+高亮动画） |
| `screenPetStatus()` | 宠物详细状态 |
| `screenEnvironment()` | WiFi 环境统计 |
| `screenSysInfo()` | 系统信息 |
| `screenControls()` | 控制设置（亮度/声音/LED） |
| `screenSettings()` | 游戏设置 |
| `screenDiagnostics()` | 调试诊断 |
| `screenGameOver()` | 游戏结束（死亡动画） |
| `drawBar()` | 通用状态条绘制 |
| `animateSelector()` | 菜单选中项平滑滚动动画 |

### 6.3 关键设计模式

- **非阻塞架构**: 全局无 `delay()` 调用，所有定时基于 `millis()` 时间戳比较
- **状态机模式**: 屏幕导航 (Screen 枚举)、休息阶段 (RestPhase)、活动类型 (Activity) 均为独立状态机
- **双缓冲渲染**: 所有绘制先写入帧缓冲精灵，最后一次性 `pushSprite()` 到屏幕
- **边沿检测按钮**: 记录上次状态，仅在 HIGH->LOW 跳变时触发，避免重复响应
- **关注点分离**: 游戏逻辑 (TamaFi.ino) 和 UI 渲染 (ui.cpp) 通过 extern 变量和枚举交互

---

## 7. 持久化存储和配置管理

### 7.1 NVS 存储方案

使用 ESP32 Preferences 库，命名空间 `"tamafi2"`，读写模式。

#### 存储键值表

| 键名 | 类型 | 内容 | 默认值（首次启动） |
|------|------|------|----------------|
| `"hunger"` | int | 饥饿度 | 70 |
| `"happy"` | int | 幸福感 | 70 |
| `"health"` | int | 健康值 | 70 |
| `"ageMin"` | ulong | 存活分钟数 | 0 |
| `"ageHr"` | ulong | 存活小时数 | 0 |
| `"ageDay"` | ulong | 存活天数 | 0 |
| `"stage"` | uchar | 进化阶段 | 0 (BABY) |
| `"hatched"` | bool | 是否已孵化 | false |
| `"sound"` | bool | 声音开关 | true |
| `"tftBri"` | uchar | TFT 亮度档位 (0/1/2) | 1 |
| `"ledBri"` | uchar | LED 亮度档位 (0/1/2) | 1 |
| `"neo"` | bool | NeoPixel 开关 | true |
| `"tCur"` | uchar | 好奇心特征 | random(40,90) |
| `"tAct"` | uchar | 活动力特征 | random(30,90) |
| `"tStr"` | uchar | 压力特征 | random(20,80) |

### 7.2 首次启动检测

通过读取 `"hunger"` 键值是否为 -1（默认返回值）来判断是否首次启动。首次启动时初始化所有默认值并随机生成个性特征，然后立即保存。

### 7.3 自动保存机制

- 自动保存间隔: 30 秒 (`autoSaveMs`)
- 在 `logicTick()` 中检查时间戳触发
- 菜单中更改控制/设置后也会触发保存

---

## 8. 音频和灯光反馈系统

### 8.1 复古音效引擎

#### 音效序列器架构

```cpp
struct RetroSound {
  const int *freqs;   // 频率数组 (Hz)
  const int *times;   // 每音符持续时间 (ms)
  int length;         // 音符数量
};
```

使用 ESP32 LEDc PWM 驱动蜂鸣器，通过 `ledcWriteTone()` 设置频率。

#### 音效列表

| 音效 | 音符数 | 总时长 | 触发场景 | 同步 LED |
|------|-------|-------|---------|---------|
| SND_CLICK | 3 | ~60ms | 菜单点击 | -- |
| SND_GOOD | 4 | ~180ms | 捕猎成功 | 紫色 |
| SND_BAD | 4 | ~240ms | 捕猎失败 | 红色 |
| SND_DISC | 5 | ~250ms | 网络发现 | 蓝色 |
| SND_REST_START | 3 | ~220ms | 入睡 | -- |
| SND_REST_END | 3 | ~200ms | 苏醒 | -- |
| SND_HATCH | 5 | ~360ms | 孵化完成 | -- |

#### 非阻塞播放实现

`sndUpdate()` 在主循环每帧调用，维护当前播放索引 (`sndIndex`)、当前音符步进 (`sndStep`) 和下一音符时间戳 (`sndNext`)。每帧检查是否到达下一音符时间，是则切换频率并推进步进。

### 8.2 NeoPixel LED 系统

4 颗 WS2812-2020 RGB LED，通过 Adafruit_NeoPixel 库控制。

#### LED 颜色方案

| 状态 | 函数 | RGB | 视觉效果 |
|------|------|-----|---------|
| 快乐/好饲料 | `ledsHappy()` | (120, 40, 200) | 柔和紫色 |
| 悲伤/坏饲料 | `ledsSad()` | (200, 0, 0) | 暗红色 |
| WiFi/发现 | `ledsWifi()` | (0, 90, 255) | 柔和蓝色 |
| 休息/睡眠 | `ledsRest()` | (0, 25, 90) | 深蓝色 |
| 关闭 | `ledsOff()` | (0, 0, 0) | 全灭 |

#### LED 亮度档位

| 档位 | 亮度值 | 说明 |
|------|-------|------|
| 0 (低) | 20 | 省电/暗环境 |
| 1 (中) | 90 | 默认 |
| 2 (高) | 180 | 明亮环境 |

#### 呼吸灯效果（深度睡眠）

```
亮度 = sin(时间相位) * 40 + 60
颜色: 纯蓝色 (0, 0, 亮度)
周期: REST_BREATHE_MS = 400ms
```

用正弦函数调制蓝色通道亮度，产生平缓的呼吸节奏效果。

### 8.3 音效-LED 联动

音效触发函数同时设置 LED 颜色，确保视听同步：
- `sndGoodFeed()` -> 播放成功音 + `ledsHappy()` 紫色
- `sndBadFeed()` -> 播放失败音 + `ledsSad()` 红色
- `sndDiscover()` -> 播放发现音 + `ledsWifi()` 蓝色

---

## 关键文件路径

| 文件 | 路径 |
|------|------|
| 主程序 | `TamaFi/TamaFi.ino` |
| UI 实现 | `TamaFi/ui.cpp` |
| UI 头文件 | `TamaFi/ui.h` |
| 动画常量 | `TamaFi/ui_anim.h` |
| TFT 配置 | `User_Setup.h` |
| 宠物精灵 | `TamaFi/StoneGolem.h` |
| 背景图像 | `TamaFi/background.h` |
| 孵化动画 | `TamaFi/egg_hatch.h` |
| 特效精灵 | `TamaFi/effect.h` |
| PCB 文件 | `PCB/` |
| 原理图 | `Schematic/` |
| UI 设计源文件 | `Ui Graphics/` |
