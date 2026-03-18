# TamaFi 项目主干代码功能与架构文档

## 1. 文档范围与分析边界

本文档基于仓库当前主干源码进行静态分析，目标是帮助后续开发者快速理解项目的整体结构、运行方式、关键模块职责、数据流向与主要业务流程。

### 1.1 分析范围

本次分析覆盖以下**主干代码与直接运行依赖**：

- `TamaFi/TamaFi.ino`：主程序入口与核心逻辑
- `TamaFi/ui.h`：共享状态、枚举、结构体、UI 接口声明
- `TamaFi/ui.cpp`：UI 渲染实现
- `TamaFi/ui_anim.h`：动画时序常量
- `TamaFi/StoneGolem.h`：宠物主动画资源
- `TamaFi/egg_hatch.h`：蛋待机动画资源
- `TamaFi/effect.h`：饥饿特效与 RIP 资源
- `TamaFi/background.h`：背景图资源
- `User_Setup.h`：TFT_eSPI 屏幕与 SPI 配置

### 1.2 明确排除范围

以下目录属于测试/诊断或硬件资料，不纳入“主干运行逻辑”分析：

- `BuzzerTest/`
- `DisplayTest_Mode3/`
- `TFT_Diagnostic/`
- `PCB/`
- `Schematic/`
- `Ui Graphics/`（设计源文件，不是运行时代码）

---

## 2. 项目概述

`TamaFi` 是一个运行在 `ESP32-S3` 上的虚拟宠物固件。项目的核心设定是：**宠物通过扫描周围 WiFi 网络来“觅食”**，并根据环境变化、自身属性和个性特征，自主决定是否执行捕猎、探索、休息或保持空闲。

项目采用典型的 Arduino 单线程循环架构，但实现方式是**非阻塞式**的：

- 主循环 `loop()` 高频运行
- 逻辑更新由 `logicTick()` 按固定节拍推进
- 音效、UI、WiFi 扫描、动画都以 `millis()` 时间片推进
- 不依赖大段 `delay()`（除 `setup()` 中很短的启动延时与一次 `vTaskDelay(1)`）

从代码结构看，系统可以抽象为四层：

1. **硬件接入层**：按键、WiFi、PWM、NeoPixel、TFT
2. **状态与逻辑层**：宠物属性、情绪、进化、活动决策、存档
3. **表现层**：动画、状态页、菜单页、特效、音效、灯效
4. **资源层**：精灵图、背景图、特效贴图

---

## 3. 主干代码目录结构

```text
Arknights_Beans_PetGame/
├── TamaFi/
│   ├── TamaFi.ino       主循环、状态机、输入、WiFi、音效、存档
│   ├── ui.h             共享枚举/结构体/extern 声明
│   ├── ui.cpp           屏幕渲染、动画推进、菜单绘制
│   ├── ui_anim.h        动画时间常量
│   ├── StoneGolem.h     宠物 idle / hatch / dead / attack 精灵数组
│   ├── egg_hatch.h      蛋待机精灵数组
│   ├── effect.h         饥饿特效与 RIP 资源
│   └── background.h     背景图数组
└── User_Setup.h         TFT_eSPI 的驱动与引脚配置
```

### 3.1 各文件职责总览

| 文件 | 角色 | 核心职责 |
| --- | --- | --- |
| `TamaFi.ino` | 运行时核心 | 初始化、全局状态、按键处理、WiFi 扫描、情绪/进化、活动决策、休息状态机、自动保存、主循环 |
| `ui.h` | 共享契约 | 定义 `Screen`、`Activity`、`Mood`、`Stage`、`RestPhase`、`Pet`、`WifiStats`，声明 UI API 与跨文件共享变量 |
| `ui.cpp` | UI 层 | 负责所有屏幕绘制、局部动画推进、状态条/高亮条/图标绘制 |
| `ui_anim.h` | 动画配置层 | 统一维护孵化、待机、休息、饥饿特效、死亡、菜单动画的时间参数 |
| `StoneGolem.h` | 运行时素材 | 存放主宠物帧图：idle、attack、death，以及 hatch 过渡帧 |
| `egg_hatch.h` | 运行时素材 | 存放蛋的待机动画帧 |
| `effect.h` | 运行时素材 | 存放饥饿覆盖特效与 `rip_ms` |
| `background.h` | 运行时素材 | 存放多个背景图数组 |
| `User_Setup.h` | 硬件配置 | 为 `TFT_eSPI` 指定 ST7789 驱动、分辨率、SPI 引脚和频率 |

---

## 4. 运行时总体架构

### 4.1 架构分层

```text
输入/外设层
  ├─ 按键 GPIO
  ├─ WiFi 扫描
  ├─ TFT SPI
  ├─ NeoPixel
  └─ PWM（蜂鸣器 / 背光）
          │
          ▼
状态与逻辑层（TamaFi.ino）
  ├─ 宠物属性
  ├─ 情绪系统
  ├─ 进化系统
  ├─ 活动决策
  ├─ 休息状态机
  ├─ NVS 持久化
  └─ 页面导航状态
          │
          ▼
表现层（ui.cpp）
  ├─ Home 渲染
  ├─ 菜单渲染
  ├─ 状态页渲染
  ├─ 动画推进
  └─ 特效叠加
          │
          ▼
显示/反馈输出
  ├─ TFT 显示
  ├─ 音效序列器
  └─ LED 灯效
```

### 4.2 共享状态模型

项目没有引入类封装或模块对象，而是采用**全局共享状态 + 分层函数调用**的组织方式。

主共享状态包括：

- 页面状态：`currentScreen`
- 行为状态：`currentActivity`
- 休息子状态：`restPhase`
- 宠物属性：`pet`
- 环境数据：`wifiStats`
- 情绪：`currentMood`
- 成长阶段：`petStage`
- 效果状态：`hungerEffectActive`、`hungerEffectFrame`
- 设置项：声音、灯、亮度、自动保存间隔等

`ui.cpp` 不自行维护业务数据，而是通过 `extern` 直接读取上述共享状态进行渲染。

### 4.3 主循环执行框架

`loop()` 是项目的调度中心，当前执行顺序如下：

```text
loop()
  ├─ 记录屏幕切换时间 screenEnteredAt
  ├─ sndUpdate()                // 推进非阻塞音效序列
  ├─ 处理 NeoPixel 清空/禁用状态
  ├─ stopBuzzerIfNeeded()       // 蜂鸣器超时静音
  ├─ 每 100ms 调用 logicTick()  // 仅在非 BOOT / HATCH 页面执行
  ├─ BOOT 页面自动跳转（1.5s）
  ├─ HATCH 页面自动触发孵化（2s，未孵化时）
  ├─ handleButtons()            // 边沿检测 + 页面导航 + 设置修改
  └─ uiDrawScreen(...)          // 统一渲染当前页面
```

这里体现了两个关键设计点：

- **逻辑和渲染解耦**：业务状态先更新，再统一渲染
- **节拍式推进**：逻辑 tick 固定 100ms，而渲染每轮循环都进行

---

## 5. 核心数据结构与枚举

### 5.1 `Pet`

用于描述宠物本体状态：

```cpp
struct Pet {
  int hunger;
  int happiness;
  int health;
  unsigned long ageMinutes;
  unsigned long ageHours;
  unsigned long ageDays;
};
```

职责：

- 表示核心生存属性
- 作为情绪判断、进化判断、死亡判断的基础输入
- 作为 UI 状态页与主界面状态条的数据来源

### 5.2 `WifiStats`

用于承接一次 WiFi 扫描的统计结果：

- `netCount`：扫描到的网络数
- `strongCount`：强信号网络数（RSSI > -60）
- `hiddenCount`：隐藏 SSID 数量
- `avgRSSI`：平均信号强度
- `openCount`：开放网络数量
- `wpaCount`：非开放网络数量

职责：

- 决定捕猎/探索收益
- 参与情绪判断
- 参与自主决策评分
- 在环境页面中展示

### 5.3 页面与行为枚举

| 枚举 | 作用 |
| --- | --- |
| `Screen` | 页面状态机：Boot、Hatch、Home、Menu、Status、Environment、SysInfo、Controls、Settings、Diagnostics、GameOver |
| `Activity` | 行为状态：无行为、捕猎、探索、休息 |
| `Stage` | 成长阶段：`BABY`、`TEEN`、`ADULT`、`ELDER` |
| `Mood` | 情绪状态：`HUNGRY`、`HAPPY`、`CURIOUS`、`BORED`、`SICK`、`EXCITED`、`CALM` |
| `RestPhase` | 休息状态机：`NONE`、`ENTER`、`DEEP`、`WAKE` |

这些枚举共同组成了系统的“离散状态空间”，也是整个固件最重要的骨架。

---

## 6. 核心模块详解

## 6.1 `TamaFi.ino`：主程序与核心逻辑中心

`TamaFi.ino` 集中了绝大多数业务逻辑，承担了“控制器 + 运行时状态仓库”的双重角色。

### 6.1.1 硬件对象初始化

文件中直接实例化：

- `TFT_eSPI tft`
- `TFT_eSprite fb`
- `TFT_eSprite petSprite`
- `TFT_eSprite effectSprite`
- `Adafruit_NeoPixel leds`
- `Preferences prefs`

这意味着：

- UI 模块并不拥有显示对象，只是借用这些全局对象
- 所有资源与渲染目标都常驻内存/全局作用域

### 6.1.2 设置与特征状态

主文件还维护下列运行时配置：

- `soundEnabled`
- `neoPixelsEnabled`
- `tftBrightnessIndex`
- `ledBrightnessIndex`
- `autoSleep`
- `autoSaveMs`
- `traitCuriosity`
- `traitActivity`
- `traitStress`

其中：

- `traitCuriosity` 和 `traitStress` 真正参与了决策逻辑
- `traitActivity` 当前仅被存档和展示，**没有参与行为频率或评分计算**
- `autoSleep` 当前只在设置页中切换和展示，**没有接入实际自动睡眠逻辑**

---

## 6.2 输入系统：按键与页面导航

### 6.2.1 按键模式

项目使用 6 个按键：

- 左侧：`UP`、`OK`、`DOWN`
- 右侧：`RIGHT1`、`RIGHT2`、`RIGHT3`

全部采用 `INPUT_PULLUP`，通过 `buttonPressed()` 做下降沿检测。

### 6.2.2 输入职责

`handleButtons()` 负责：

- 页面切换
- 菜单上下移动
- 设置值切换
- 快捷页跳转
- 游戏结束后重生
- 手动触发孵化

### 6.2.3 页面导航特点

- `HOME` 页面按 `OK` 进入主菜单
- `RIGHT1/2/3` 分别直达 `Pet Status`、`Environment`、`Diagnostics`
- 这些快捷页再次按右侧任一按钮会直接返回 `HOME`
- `BOOT` 页面理论上支持任意左键继续
- `HATCH` 页面支持 `OK` 触发孵化
- `GAMEOVER` 页面 `OK` 重新开始

### 6.2.4 当前主干中的特殊实现

为了适配当前的面包板原型，`loop()` 中额外加入了自动流程：

- `BOOT` 停留 1.5 秒后自动进入 `HOME` 或 `HATCH`
- 未孵化时，`HATCH` 停留 2 秒后自动置 `hatchTriggered = true`

因此，**当前代码已经不是“必须依赖按键才能进入游戏”**，而是兼容“无按键原型机”运行。

---

## 6.3 WiFi 交互系统

WiFi 是项目的环境输入来源，也是宠物“进食/探索”的业务核心。

### 6.3.1 扫描启动

`startWifiScan()` 做了三件事：

1. `WiFi.mode(WIFI_STA)`
2. `WiFi.disconnect(true)`
3. `WiFi.scanNetworks(true)` 发起异步扫描

扫描期间只标记 `wifiScanInProgress = true`，不阻塞主循环。

### 6.3.2 扫描完成检查

`checkWifiScanDone()` 在后续逻辑 tick 中轮询：

- 若仍在扫描：返回 `false`
- 若扫描失败：清空 `wifiStats`
- 若扫描成功：统计网络数量、信号强度、加密类型、平均 RSSI
- 最后 `WiFi.scanDelete()` 释放扫描结果

### 6.3.3 扫描结果落地

统计完成后，结果写入全局 `wifiStats`。从这一刻开始，以下模块都可消费同一份环境快照：

- 情绪系统
- 自主决策系统
- 捕猎结算
- 探索结算
- 环境页面 UI

---

## 6.4 宠物属性衰减与生存规则

`logicTick()` 负责按节拍推动宠物属性变化。

### 6.4.1 属性衰减规则

| 属性 | 周期 | 规则 |
| --- | --- | --- |
| `hunger` | 5 秒 | 固定 -2 |
| `happiness` | 7 秒 | 默认 -1；若长时间无 WiFi（30s 以上）则 -3 |
| `health` | 10 秒 | 默认 -1；若 `hunger < 20` 或 `happiness < 20` 则 -2 |

所有结果最终都通过 `max()` 或 `constrain()` 限制在合理范围内。

### 6.4.2 年龄推进规则

每 60 秒：

- `ageMinutes++`
- 分钟满 60 转为 `ageHours`
- 小时满 24 转为 `ageDays`

### 6.4.3 死亡条件

当以下条件同时成立时触发死亡：

- `pet.hunger <= 0`
- `pet.happiness <= 0`
- `pet.health <= 0`

处理结果：

- 切换到 `SCREEN_GAMEOVER`
- 清空当前行为
- 中止休息状态
- 点亮悲伤灯效

---

## 6.5 情绪系统

`updateMood()` 按固定优先级重算当前情绪。

### 6.5.1 判定顺序

1. `SICK`：`health < 25`，或长时间没有 WiFi 且上次扫描已超过 60 秒
2. `HUNGRY`：`hunger < 25`
3. `EXCITED`：`happiness > 80` 且网络数大于 8
4. `HAPPY`：`happiness > 60` 且扫描到网络
5. `BORED`：没有网络且距离上次扫描超过 30 秒
6. `CURIOUS`：附近存在隐藏网络或开放网络
7. `CALM`：默认状态

### 6.5.2 情绪的业务作用

情绪不仅是显示文本，还会直接影响：

- `HOME` 页面待机动画速度
- 自主活动评分偏置
- 间接改变视觉观感与行为倾向

例如：

- `EXCITED`：待机更快，探索/捕猎意愿提升
- `SICK`：休息意愿明显提升
- `HUNGRY`：捕猎意愿显著提升

---

## 6.6 进化系统

`updateEvolution()` 按年龄与平均属性判断成长阶段。

### 6.6.1 条件

| 目标阶段 | 年龄条件（以 `ageMinutes` 判定） | 平均属性条件 |
| --- | --- | --- |
| `TEEN` | `>= 20` | 平均值 `> 35` |
| `ADULT` | `>= 60` | 平均值 `> 45` |
| `ELDER` | `>= 180` | 平均值 `> 40` |

### 6.6.2 进化反馈

进化成功时：

- 更新 `petStage`
- 播放 `sndDiscover()`
- 触发 WiFi 蓝色灯效

### 6.6.3 实现说明

当前实现以 `pet.ageMinutes` 作为进化判断输入，因此它实际上代表“累计分钟数中的当前分钟值”。由于代码会在满 60 分钟时把分钟减回 0、小时 +1，这使得阶段判断与“总存活时长”并不完全等价。

这意味着：

- `TEEN` 比较容易满足
- `ADULT` / `ELDER` 的判定与直觉上的“总时长”并不完全一致

这是后续维护时需要特别关注的实现细节。

---

## 6.7 自主决策引擎

`decideNextActivity()` 是项目最核心的“行为 AI”。

### 6.7.1 触发时机

只有在以下条件同时满足时才会决策：

- 当前页面是 `SCREEN_HOME`
- 当前没有活动 `ACT_NONE`
- 不处于休息子状态中
- 距离上次决策超过当前决策间隔

### 6.7.2 决策频率

- 初始间隔：10 秒
- 每次决策后重新随机为 `8000 ~ 15000 ms`

### 6.7.3 评分项

系统计算 4 个欲望值：

- `desireHunt`
- `desireDisc`
- `desireRest`
- `desireIdle`

基础公式如下：

- 捕猎欲望 = 饥饿缺口 + 好奇心一半
- 探索欲望 = 好奇心 + 隐藏网络权重 + 开放网络权重 + 网络总数权重 + 随机扰动
- 休息欲望 = 健康缺口 + 压力一半
- 空闲欲望 = 常量 10

### 6.7.4 环境与情绪修正

- 没有网络时，捕猎/探索欲望会被减半
- 饥饿时，捕猎 +20，休息 -10
- 好奇时，探索 +15
- 生病时，休息 +20，探索 -10
- 兴奋时，探索 +10，捕猎 +5
- 无聊时，探索 +10，捕猎 +5

### 6.7.5 行为落地

- 若选中 `HUNT` 或 `DISCOVER`：
  - 写入 `currentActivity`
  - 点亮 WiFi 灯效
  - 启动异步 WiFi 扫描
- 若选中 `REST`：
  - 进入 `REST_ENTER`
  - 初始化休息动画与持续时间
  - 播放入睡音效
  - 点亮休息灯效

---

## 6.8 捕猎与探索结算

### 6.8.1 捕猎 `resolveHunt()`

这是“WiFi 当食物”的核心落地。

#### 无网络时

- `hunger -= 15`
- `happiness -= 10`
- `health -= 5`
- 播放失败音效

#### 有网络时

- `hungerDelta = min(35, netCount*2 + strongCount*3)`
- `happyDelta = min(30, varietyScore*3 + (avgRSSI+100)/3)`
- `healthDelta` 取决于平均信号与强信号数量

结算完成后还会：

- 激活 `hungerEffectActive`
- 重置饥饿特效帧索引

### 6.8.2 探索 `resolveDiscover()`

探索不是直接进食，而是对环境“好奇”。

#### 无网络时

- `happiness -= 5`
- `hunger -= 3`
- 播放失败音效

#### 有网络时

- 计算 `curiosity = hidden*4 + open*3 + netCount`
- `happyDelta = min(35, curiosity / 2)`
- `hungerDelta = -5`
- 播放发现音效

### 6.8.3 统一收尾

在 `logicTick()` 中，扫描完成后：

- 根据 `currentActivity` 调用对应结算函数
- 活动结束后设回 `ACT_NONE`
- 熄灭 WiFi 灯效

---

## 6.9 休息状态机

休息流程由 `stepRest()` 驱动，是代码中最完整的子状态机之一。

### 6.9.1 状态流转

```text
REST_NONE
  └─> REST_ENTER
        └─> REST_DEEP
              └─> REST_WAKE
                    └─> REST_NONE
```

### 6.9.2 各阶段职责

#### `REST_ENTER`

- 用蛋的孵化帧做反向播放
- 表示“进入睡眠”
- 每 `400ms` 前进一帧

#### `REST_DEEP`

- 保持在蛋帧 1
- 使用正弦函数驱动蓝色呼吸灯
- 休息中点为宠物恢复属性：
  - `hunger -3`
  - `happiness +10`
  - `health +15`

#### `REST_WAKE`

- 用蛋帧正向播放
- 播放醒来音效
- 结束后恢复到 `ACT_NONE`

### 6.9.3 休息持续时间

由随机区间决定：

- 最短 `5000 ms`
- 最长 `15000 ms`

---

## 6.10 音效系统

音效使用 `RetroSound` 结构 + `sndUpdate()` 非阻塞序列器实现。

### 6.10.1 机制

- 每种音效是一组频率数组和时长数组
- `sndIndex` 表示当前正在播哪一组音效
- `sndStep` 表示播到第几个音符
- `sndNext` 表示下一个音符切换时间
- `sndUpdate()` 在每轮 `loop()` 中推进

### 6.10.2 已实现音效

- 点击音 `SND_CLICK`
- 成功音 `SND_GOOD`
- 失败音 `SND_BAD`
- 发现音 `SND_DISC`
- 入睡音 `SND_REST_START`
- 醒来音 `SND_REST_END`
- 孵化音 `SND_HATCH`

### 6.10.3 与 LED 的联动

部分音效在触发时直接联动灯效：

- `sndGoodFeed()` → 紫色
- `sndBadFeed()` → 红色
- `sndDiscover()` → 蓝色

---

## 6.11 NeoPixel 与背光控制

### 6.11.1 NeoPixel

系统支持 4 颗 WS2812：

- `ledsHappy()`：柔和紫色
- `ledsSad()`：暗红色
- `ledsWifi()`：蓝色
- `ledsRest()`：深蓝色

亮度通过 `applyLedBrightness()` 统一设置为三档：

- 低：20
- 中：90
- 高：180

### 6.11.2 TFT 背光

背光由 PWM 控制，三档值分别为：

- 低：60
- 中：150
- 高：255

### 6.11.3 实现注意点

`neoPixelsEnabled` 为 `false` 时，`ledsOff()` 会直接返回，因此“关闭灯光”更像是**禁止后续写灯**，而不是强制清空当前灯态。这属于当前实现中的一个行为细节。

---

## 6.12 持久化存储

项目使用 `Preferences`，命名空间为 `tamafi2`。

### 6.12.1 保存内容

`saveState()` 当前会保存：

- 宠物属性：`hunger`、`happy`、`health`
- 年龄：`ageMin`、`ageHr`、`ageDay`
- 阶段：`stage`
- 是否孵化：`hatched`
- 控制项：`sound`、`tftBri`、`ledBri`、`neo`
- 特征值：`tCur`、`tAct`、`tStr`

### 6.12.2 首次启动逻辑

`loadState()` 通过 `prefs.getInt("hunger", -1)` 判断是否首次运行：

- 若不存在，则写入默认属性
- 随机生成 3 个特征值
- 立即保存

### 6.12.3 自动保存机制

- 默认间隔：30 秒
- 在 `logicTick()` 中按 `autoSaveMs` 触发

### 6.12.4 当前实现未持久化的设置

以下设置虽然在 UI 中可修改，但**不会写入 NVS**：

- `autoSleep`
- `autoSaveMs`

也就是说，这两项在重启后会回到代码默认值。

---

## 7. UI 架构与渲染系统

## 7.1 `ui.h`：主逻辑与 UI 的共享边界

`ui.h` 的价值不在于复杂逻辑，而在于建立清晰的跨文件契约：

- 所有 UI 所需的状态都由这里 `extern`
- 所有与页面相关的枚举也都由这里统一定义
- `ui.cpp` 与 `TamaFi.ino` 的耦合点几乎都集中在这里

这使得 UI 层虽然依赖全局变量，但接口边界相对明确。

## 7.2 `ui.cpp`：以页面函数为中心的渲染模块

`ui.cpp` 采用“每个页面一个静态函数”的写法，结构直观。

### 7.2.1 页面函数

- `screenBoot()`
- `screenHatch()`
- `screenHome()`
- `screenMenu()`
- `screenPetStatus()`
- `screenEnvironment()`
- `screenSysInfo()`
- `screenControls()`
- `screenSettings()`
- `screenDiagnostics()`
- `screenGameOver()`

### 7.2.2 顶层调度

`uiDrawScreen(...)` 根据 `currentScreen` 分发到具体页面渲染函数。

### 7.2.3 页面切换回调

`uiOnScreenChange()` 负责：

- 重置菜单高亮条位置
- 重置设置页高亮条位置
- 在进入孵化页时清理孵化相关帧索引

---

## 7.3 渲染方式

项目使用多个 `TFT_eSprite`：

- `fb`：主帧缓冲
- `petSprite`：宠物绘制缓冲
- `effectSprite`：特效绘制缓冲

总体流程：

```text
先绘制到 sprite
  ├─ fb 画背景、文本、状态条
  ├─ petSprite 画宠物或蛋
  └─ effectSprite 画饥饿特效
最后统一 pushSprite 到屏幕
```

这种方式避免了直接在 TFT 上多次分散绘制造成的明显闪烁。

### 7.3.1 `HOME` 页面图层关系

1. 顶部标题栏
2. 背景图
3. 宠物/蛋/攻击动画
4. 状态条
5. 文本信息（Mood、Stage、Activity）
6. 饥饿特效叠加层

### 7.3.2 动画资源映射

当前资源组织有一个很重要的实现事实：

- `StoneGolem.h` 不只包含宠物 idle，还包含：
  - `idle_1 ~ idle_4`
  - `egg_hatch_1 ~ egg_hatch_5`
  - `dead_1 ~ dead_3`
  - `attack_0 ~ attack_2`
- `egg_hatch.h` 主要承载的是蛋待机动画：
  - `egg_hatch_11`
  - `egg_hatch_21`
  - `egg_hatch_31`
  - `egg_hatch_41`
  - `egg_hatch_51`
- `effect.h` 主要承载：
  - `hunger1 ~ hunger4`
  - `rip_ms`
- `background.h` 当前可见：
  - `backgroundImage`
  - `backgroundImage1`
  - `backgroundImage2`

换言之，素材文件名和实际用途并非严格一一对应，后续维护资源时需要以代码引用关系为准。

---

## 8. 主要业务流程

## 8.1 启动与恢复流程

```text
setup()
  ├─ 初始化 LED / 按键 / 屏幕 / Sprite / PWM / WiFi
  ├─ prefs.begin("tamafi2")
  ├─ loadState()
  ├─ 应用背光与 LED 亮度
  ├─ 初始化各种逻辑计时器
  ├─ currentScreen = SCREEN_BOOT
  ├─ uiInit()
  └─ uiOnScreenChange(SCREEN_BOOT)
```

启动完成后，系统进入 `BOOT` 页面，并根据是否已孵化决定后续流向：

- 已孵化：进入 `HOME`
- 未孵化：进入 `HATCH`

在当前主干里，这一步既可以靠按键，也可以靠自动跳转完成。

## 8.2 首次孵化流程

```text
BOOT
  └─> HATCH
       ├─ 未触发时：播放蛋待机动画
       ├─ 触发后：播放孵化帧序列 + 孵化音效
       └─ 完成后：hasHatchedOnce = true，进入 HOME
```

这个流程把“蛋”作为新手引导状态，也承担了正式游戏开始前的仪式感。

## 8.3 正常运行主流程

```text
HOME
  ├─ 属性按时间衰减
  ├─ 年龄持续增长
  ├─ 心情持续重算
  ├─ 每隔一段时间自主决策
  ├─ 若触发 HUNT / DISCOVER 则开始 WiFi 扫描
  ├─ 若触发 REST 则进入休息状态机
  └─ UI 持续反映当前行为与状态
```

## 8.4 捕猎流程

```text
decideNextActivity()
  └─ chosen = ACT_HUNT
       ├─ currentActivity = ACT_HUNT
       ├─ ledsWifi()
       ├─ startWifiScan()
       ├─ logicTick() 中等待扫描完成
       ├─ resolveHunt()
       ├─ 更新 pet / 特效 / 音效
       └─ currentActivity = ACT_NONE
```

## 8.5 探索流程

```text
decideNextActivity()
  └─ chosen = ACT_DISCOVER
       ├─ currentActivity = ACT_DISCOVER
       ├─ startWifiScan()
       ├─ 等待扫描完成
       ├─ resolveDiscover()
       └─ currentActivity = ACT_NONE
```

## 8.6 休息流程

```text
decideNextActivity()
  └─ chosen = ACT_REST
       ├─ REST_ENTER
       ├─ REST_DEEP（中点恢复属性）
       ├─ REST_WAKE
       └─ ACT_NONE
```

## 8.7 死亡与重开流程

```text
logicTick()
  └─ 三项属性都为 0
       ├─ SCREEN_GAMEOVER
       ├─ 播放死亡表现
       └─ 等待 OK 重启 / 或重新进入孵化流程
```

## 8.8 控制与设置流程

### Controls 页

可修改：

- 屏幕亮度
- LED 亮度
- 声音开关
- NeoPixel 开关

其中亮度与声音/灯开关会写入持久化。

### Settings 页

可修改：

- Theme（当前只有占位）
- Auto Sleep（当前无实际逻辑）
- Auto Save（仅影响当前运行期）
- Reset Pet
- Reset All

---

## 9. 数据流向说明

## 9.1 环境数据流

```text
WiFi 扫描结果
  └─> checkWifiScanDone()
       └─> wifiStats
            ├─> updateMood()
            ├─> decideNextActivity()
            ├─> resolveHunt()
            ├─> resolveDiscover()
            └─> screenEnvironment()
```

## 9.2 宠物状态流

```text
pet（hunger / happiness / health / age）
  ├─> updateMood()
  ├─> updateEvolution()
  ├─> decideNextActivity()
  ├─> screenHome()
  ├─> screenPetStatus()
  └─> 死亡判定
```

## 9.3 设置数据流

```text
handleButtons()
  └─ 修改设置变量
       ├─ applyTftBrightness()
       ├─ applyLedBrightness()
       ├─ saveState()（部分设置）
       └─ UI 页面实时显示
```

## 9.4 渲染数据流

```text
全局共享状态
  └─> uiDrawScreen()
       └─> 各 screenXxx()
            └─> fb / petSprite / effectSprite
                 └─> pushSprite 到 TFT
```

---

## 10. 资源与硬件配置说明

## 10.1 动画时间常量 `ui_anim.h`

该文件集中定义：

- 蛋待机动画节拍
- 孵化节拍
- 宠物 idle 快慢速
- 饥饿特效节拍
- 睡眠进入/唤醒节拍
- 死亡节拍
- 菜单高亮动画节拍

这样做的好处是：

- UI 表现参数集中可调
- 行为逻辑与表现速度解耦

## 10.2 `User_Setup.h` 的现实意义

当前主干中的 `User_Setup.h` 使用的是**当前工作副本对应的面包板/原型接线**，核心参数包括：

- 驱动：`ST7789_DRIVER`
- 分辨率：`240x240`
- `TFT_MISO = 6`
- `TFT_MOSI = 17`
- `TFT_SCLK = 18`
- `TFT_CS = 16`
- `TFT_DC = 15`
- `TFT_RST = 4`

这与一些旧文档中提到的引脚并不一致。因此后续开发应以**当前 `User_Setup.h` 和实际接线**为准，而不是以历史说明为准。

---

## 11. 当前实现中的重要观察与维护提示

以下内容不是“设计意图”，而是**根据当前代码观察到的真实实现状态**：

### 11.1 原型适配逻辑已进入主干

`loop()` 里明确写了：

- `BOOT` 自动跳转
- `HATCH` 自动触发孵化

这说明当前主干已经包含“无按键也能跑通”的原型机逻辑。

### 11.2 `traitActivity` 尚未真正参与行为系统

它会：

- 被随机生成
- 被保存/读取
- 被状态页显示

但当前不会影响：

- 决策间隔
- 欲望评分
- 动画速度

### 11.3 `autoSleep` 是设置占位

当前只存在于：

- 全局变量
- 设置页切换
- 设置页显示

但没有任何地方消费它来驱动真实睡眠或息屏逻辑。

### 11.4 `autoSaveMs` 可运行期修改，但不会持久化

用户在设置页里改完自动保存间隔后：

- 当前运行期生效
- 重启后恢复默认 `30000 ms`

### 11.5 “全量重置”实现并不彻底

`resetPet(true)` 只会重置：

- `pet.ageMinutes`

不会同步清空：

- `pet.ageHours`
- `pet.ageDays`

如果未来要把重置做严谨，这里需要补齐。

### 11.6 文案与行为并非完全一致

例如：

- `screenBoot()` 文案是 “Press any button...”，但实际可能自动跳过
- `screenHatch()` 也可自动进入孵化

因此测试与验收时，建议同时检查**UI 文案、逻辑实现、硬件接法**三者是否一致。

### 11.7 素材命名和代码引用关系存在历史包袱

例如孵化相关帧既分散在 `StoneGolem.h`，也分散在 `egg_hatch.h`。维护资源时不要只凭文件名猜用途，应以 `ui.cpp` 的引用关系为准。

---

## 12. 给后续开发者的接手建议

### 12.1 建议优先阅读顺序

1. `TamaFi/TamaFi.ino`
2. `TamaFi/ui.h`
3. `TamaFi/ui.cpp`
4. `TamaFi/ui_anim.h`
5. `User_Setup.h`
6. 资源头文件（按 `ui.cpp` 引用关系阅读）

### 12.2 如果要继续演进功能，建议优先处理的点

- 把 `autoSleep` 做成真正可执行的逻辑
- 让 `traitActivity` 参与决策频率或评分
- 修正进化判定使用的时间基准
- 修正 `resetPet(true)` 的年龄重置不完整问题
- 清理历史文档与当前代码的引脚配置差异
- 进一步收敛资源文件的命名与归属

---

## 13. 结论

`TamaFi` 的主干代码虽然体量不大，但已经具备了一个完整嵌入式宠物系统的关键要素：

- 明确的页面状态机
- 非阻塞式运行循环
- 基于环境感知的行为系统
- 宠物属性、情绪、进化、休息等核心玩法闭环
- 完整的显示、音效、灯效与存档支持

它的核心设计特点不是“复杂算法”，而是**把 WiFi 环境扫描映射成宠物生存行为，并通过轻量状态机把输入、决策、反馈和 UI 串成闭环**。这使得项目非常适合作为后续扩展的基础：既可以继续增强 AI/成长系统，也可以继续完善交互、硬件和美术表现。