# ✨ Begeerte for ARK: Survival Ascended

基于 [MinHook](https://github.com/TsudaKageyu/minhook) [Minimal-D3D12-Hook-ImGui](https://github.com/zetsr/Minimal-D3D12-Hook-ImGui) [ImGui](https://github.com/ocornut/imgui) [Dumper-7](https://github.com/Encryqed/Dumper-7) 开发的 C++ 内部作弊

<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/485e987f-d15f-465b-97d3-ca8cb7eb27b1" />

---

# 🛠️ 功能

## 📡 ESP

### 🔍 生物列表

### 🔍 建筑列表

### 📦 显示掉落物

### 🛡️ 显示建筑

### 💧 显示水源

### 🧬 显示方框

### 🧬 显示名称

### 🧬 显示血量

### 🧬 显示眩晕

### 🧬 显示尸体

### 🧬 显示距离

### 🏹 显示视野外的威胁

---

# 🚀 Lua API

基于 **Lua Jit 2.1** 与 **sol2** 构建。

## 📋 全局信息

* **Lua 版本**: 5.1
* **sol2 版本**: 3.3.0

---

# 💻 全局函数

| 函数签名 | 返回值 | 说明 | C++
| --- | --- | --- | --- |
| `OnPaint()` |  | 在这里使用 ImGui API 进行绘制 | `HRESULT STDMETHODCALLTYPE hkPresent(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags)` |
| `OnPaintMenu(float MenuAlpha)` |  | 在这里使用 ImGui API 进行绘制，只在外挂菜单被打开时触发 |  |
| `OnMenuOpen()` |  | 每次外挂菜单被打开时触发 |  |
---

# 💻 Client

| 函数签名 | 说明 |
| --- | --- |
| `Client.AddLog(r, g, b, a, message)` | 在屏幕左上角添加一条日志 |

---

# 🎨 ImGui

> ⚠️ **注意**
>
> **所有 ImGui 绘制 API 必须在 `OnPaint()` 全局函数中调用才会正常工作。**  
> 这些接口底层使用 `ImGui::GetBackgroundDrawList()`，仅在渲染阶段有效。
>
---

## 📦 ImGui 全局表

Lua 中通过全局表 `ImGui` 访问所有绘制与输入接口。

---

## 🎨 基础工具函数

| 函数签名 | 返回值 | 说明 |
| --- | --- | --- |
| `ImGui.Color(r, g, b, a)` | `ImU32` | 生成颜色值，参数范围 0~255 |
| `ImGui.GetDeltaTime()` | `float` | 上一帧时间间隔 |
| `ImGui.GetFPS()` | `float` | 当前帧率 |
| `ImGui.GetScreenSize()` | `table { x, y }` | 当前屏幕分辨率 |
| `ImGui.GetMousePos()` | `table { x, y }` | 当前鼠标屏幕坐标 |
| `ImGui.CalcTextSize(text)` | `table { x, y }` | 计算文本绘制尺寸 |

---

## 🖱️ 输入检测

| 函数签名 | 返回值 | 说明 |
| --- | --- | --- |
| `ImGui.IsMouseDown(button)` | `bool` | 鼠标是否按下（0 左 / 1 右 / 2 中） |
| `ImGui.IsKeyDown(key)` | `bool` | 键盘是否按下（`ImGuiKey` 枚举值） |

---

## 🖌️ 绘制 API

### 📐 线条与矩形

| 函数签名 | 说明 |
| --- | --- |
| `ImGui.AddLine(x1, y1, x2, y2, col, thickness)` | 绘制线段 |
| `ImGui.AddRect(x1, y1, x2, y2, col, rounding, thickness)` | 绘制矩形边框 |
| `ImGui.AddRectFilled(x1, y1, x2, y2, col, rounding)` | 绘制实心矩形 |
| `ImGui.AddRectFilledMultiColor(x1, y1, x2, y2, colUL, colUR, colBR, colBL)` | 多色渐变矩形 |

---

### 🔺 多边形

| 函数签名 | 说明 |
| --- | --- |
| `ImGui.AddTriangle(x1, y1, x2, y2, x3, y3, col, thickness)` | 绘制三角形 |
| `ImGui.AddTriangleFilled(x1, y1, x2, y2, x3, y3, col)` | 实心三角形 |
| `ImGui.AddQuad(x1, y1, x2, y2, x3, y3, x4, y4, col, thickness)` | 绘制四边形 |
| `ImGui.AddQuadFilled(x1, y1, x2, y2, x3, y3, x4, y4, col)` | 实心四边形 |
| `ImGui.AddNgon(x, y, radius, col, segments, thickness)` | 正多边形 |
| `ImGui.AddNgonFilled(x, y, radius, col, segments)` | 实心正多边形 |

---

### ⚪ 圆形 / 椭圆

| 函数签名 | 说明 |
| --- | --- |
| `ImGui.AddCircle(x, y, radius, col, segments, thickness)` | 圆形 |
| `ImGui.AddCircleFilled(x, y, radius, col, segments)` | 实心圆 |
| `ImGui.AddEllipse(x, y, rx, ry, col, rot, segments, thickness)` | 椭圆 |
| `ImGui.AddEllipseFilled(x, y, rx, ry, col, rot, segments)` | 实心椭圆 |

---

### 🌀 贝塞尔曲线

| 函数签名 | 说明 |
| --- | --- |
| `ImGui.AddBezierQuadratic(x1, y1, x2, y2, x3, y3, col, thickness, segments)` | 二阶贝塞尔 |
| `ImGui.AddBezierCubic(x1, y1, x2, y2, x3, y3, x4, y4, col, thickness, segments)` | 三阶贝塞尔 |

---

### 🔤 文本

| 函数签名 | 说明 |
| --- | --- |
| `ImGui.AddText(x, y, col, text)` | 绘制文本 |

---

# 🎮 System

---

## `获取硬件信息`

| 函数                          | 返回值                | 说明                  |
| --------------------------- | ------------------ | ------------------- |
| `System.GetCPUStats()`          | `float cpu_usage, float cpu_freq`        | CPU使用率、CPU频率 |

---

| 函数                          | 返回值                | 说明                  |
| --------------------------- | ------------------ | ------------------- |
| `System.GetGPUStats()`          | `float gpu_usage`        | GPU使用率 |

---

# 🎮 SDK 核心模块

---

## 📐 基础结构（Userdata）

### `FVector`

```lua
local v = FVector(x, y, z)
````

| 成员  | 类型      |
| --- | ------- |
| `X` | `float` |
| `Y` | `float` |
| `Z` | `float` |

---

## 🌐 Network（网络接口）

### `UNetDriver`

通过 `SDK.GetNetDriver()` 获取。

| 成员 | 类型 | 说明 |
| --- | --- | --- |
| `ServerConnection` | `UNetConnection` | 当前的服务器连接对象 |

---

### `UNetConnection`

代表与服务器的底层网络链接。

| 函数签名 | 返回值 | 说明 |
| --- | --- | --- |
| `ServerConnection:GetFirstIP()` | `string` | 获取服务器的主 IP 地址 |
| `ServerConnection:GetPort()` | `int` | 获取当前连接的远程端口 |

---

## 🌍 SDK 全局接口

| 函数                          | 返回值                | 说明                  |
| --------------------------- | ------------------ | ------------------- |
| `SDK.GetLocalPC()`          | `uintptr_t`        | 本地 PlayerController |
| `SDK.GetActors()`           | `table<uintptr_t>` | 当前 World 中所有 Actor  |
| `SDK.GetCharacterClass()`   | `uintptr_t`        | Character 类指针       |
| `SDK.GetDinoClass()`        | `uintptr_t`        | Dino 类指针            |
| `SDK.GetDroppedItemClass()` | `uintptr_t`        | 掉落物类                |
| `SDK.GetContainerClass()`   | `uintptr_t`        | 容器类                 |
| `SDK.GetTurretClass()`      | `uintptr_t`        | 炮塔类                 |
| `SDK.GetNetDriver()`      | `UNetDriver`        | 获取当前世界的网络驱动器，未连接时返回 `nil`                 |

---

## 🧱 Actor 通用接口

| 函数                         | 返回值        | 说明   |
| -------------------------- | ---------- | ---- |
| `Actor.IsA(addr, class)`   | `bool`     | 类型判断 |
| `Actor.GetLocation(addr)`  | `FVector?` | 世界坐标 |
| `Actor.GetDistance(a, b)`  | `float`    | 距离   |
| `Actor.IsHidden(addr)`     | `bool`     | 是否隐藏 |
| `Actor.GetClassName(addr)` | `string`   | 类名   |

---

## 🧬 Character（玩家 / 生物）

### `Character.GetInfo(addr)`

返回：

```lua
health, maxHealth, isDead, name
```

说明：

* 自动区分玩家 / 生物
* 优先使用 `PlayerState` 名字

---

### `Character.GetRelation(target, local)`

| 返回值 | 含义 |
| --- | -- |
| `0` | 敌对 |
| `1` | 友军 |

---

### `Character.GetExactPing(addr)`

| 返回值 | 说明 |
| --- | -- |
| `float` | 返回该玩家与服务器之间的延迟 |

---

## 🎒 Item（掉落物）

### `Item.GetDroppedInfo(addr)`

返回：

```lua
isValid, name, quantity, rating, isBlueprint, className
```

---

## 📦 Container（补给箱 / 容器）

### `Container.GetInfo(addr)`

返回：

```lua
name
```

---

## 🎮 PC（PlayerController）

| 函数                                 | 返回值          | 说明      |
| ---------------------------------- | ------------ | ------- |
| `PC.GetPawn(pc)`                   | `uintptr_t`  | 当前 Pawn |
| `PC.ProjectToScreen(pc, worldPos)` | `bool, x, y` | 世界 → 屏幕 |

---
