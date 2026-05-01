# Weasel Win7 定制版 - 可开发功能清单

> 基于 Weasel 0.14.3 源代码，严格限制使用 Windows 8 及以上 API，UI 增强一律使用 GDI/GDI+ 实现。

## 项目结构

| 模块 | 说明 |
|------|------|
| **WeaselServer** | 主服务程序，托盘图标、IPC 服务端 |
| **WeaselTSF** | TSF 输入法框架实现（核心输入逻辑） |
| **WeaselIME** | 传统 IME 输入法实现 |
| **WeaselUI** | 候选窗口 UI 绘制（GDI/GDI+） |
| **WeaselDeployer** | 配置工具（UI 设置、方案切换等） |
| **WeaselSetup** | 安装/卸载程序 |
| **RimeWithWeasel** | 与 librime 引擎的桥接层 |
| **WeaselIPC** | 进程间通信（客户端/服务端） |

## 当前 0.14.3 的 UI 绘制方式

- 使用 **WTL + GDI** 绘制候选窗口
- 字体通过 `CreateFontW` + `TextOut` 绘制
- 无边框窗口（`WS_POPUP`），置顶（`WS_EX_TOPMOST`）
- 圆角通过 `RoundRect` 实现
- 配色方案通过 `UIStyle` 结构体管理

---

## 一、UI 增强类（纯 GDI/GDI+ 可实现）

| # | 功能 | 说明 | 难度 |
|---|------|------|------|
| 1 | **候选窗口圆角增强** | 0.14.3 已有 `RoundRect`，可增加 `corner_radius` 独立配置、高亮区域独立圆角 | 低 |
| 2 | **候选窗口阴影** | 使用 GDI+ `GraphicsPath` + 半透明绘制实现窗口阴影效果 | 中 |
| 3 | **鼠标悬停高亮（semi_hilite/hilite）** | `hover_type` 参数：悬停时半高亮或全高亮候选项 | 中 |
| 4 | **候选词截缩显示** | `candidate_abbreviate_length`：超长候选词缩略显示（如"中华人民共和国"→"中华人民..."） | 低 |
| 5 | **自定义 baseline/linespacing** | 解决候选窗口高度跳动问题，独立控制基线和行距 | 低 |
| 6 | **横竖排文字切换** | `text_orientation` 参数，控制文字绘制方向（GDI `SetTextCharacterExtra`） | 中 |
| 7 | **状态图标自定义** | 方案中自定义全/半角图标路径 | 低 |
| 8 | **ASCII 状态提示跟随光标** | `ascii_tip_follow_cursor`：切换英文时提示跟随鼠标位置 | 中 |

## 二、功能增强类

| # | 功能 | 说明 | 难度 |
|---|------|------|------|
| 9 | **全局 ASCII 模式** | `global_ascii: bool`：所有进程统一 ASCII 状态 | 低 |
| 10 | **应用专用配置** | `app_options` 中支持 `vim_mode`、`inline_preedit` 等每个应用的独立设置 | 中 |
| 11 | **点击候选词截图** | `click_to_capture`：鼠标点击候选词时截图（GDI `BitBlt`） | 中 |
| 12 | **翻页/候选切换可配置** | `paging_on_scroll`：滚轮行为可选翻页或切换候选 | 低 |
| 13 | **通知提示可定制** | `show_notifications` + `show_notifications_time`：控制切换提示的显示内容和时长 | 低 |
| 14 | **字体抗锯齿模式** | `antialias_mode`：Cleartype/Grayscale/Aliased 可选 | 低 |
| 15 | **64 位 Rime 服务** | 使用 librime 1.11.2 的 64 位 rime.dll，支持大内存部署大规模词库 | 中 |

## 三、稳定性/兼容性修复

| # | 功能 | 说明 | 难度 |
|---|------|------|------|
| 16 | **服务崩溃守护** | WeaselServer 崩溃后自动重启（创建看门狗进程） | 中 |
| 17 | **异步消息处理** | 避免服务崩溃时客户端程序卡死 | 高 |
| 18 | **IPC 数据长度扩容** | 增大 IPC 管道数据限制到 64KB，支持长候选 | 低 |
| 19 | **日志系统改进** | 日志输出到 `%TEMP%\rime.weasel`，方便排查问题 | 低 |
| 20 | **游戏模式** | `Ctrl+Shift+G` 开关，强制 inline_preedit，关闭候选框（针对老游戏兼容） | 中 |

## 四、搜狗风格 UI 增强

| # | 功能 | 说明 | 难度 |
|---|------|------|------|
| 21 | **候选窗口半透明背景** | GDI+ AlphaBlend 实现半透明候选条 | 中 |
| 22 | **候选词编号样式优化** | 类似搜狗的圆形/方形编号标签 | 中 |
| 23 | **拼音预览区渐变背景** | GDI+ 渐变填充实现搜狗风格的拼音区 | 低 |
| 24 | **候选窗口拖拽移动** | 鼠标拖拽候选窗口到任意位置，记忆位置 | 中 |
| 25 | **多皮肤快速切换** | 在托盘菜单中快速切换预置皮肤 | 低 |

---

## 开发约束

1. 严格限制使用 Windows 8 及以上才支持的 API（如 `DwmExtendFrameIntoClientArea` 新变体、`DirectComposition`、`CreateWindowEx` 的部分新扩展等）。若必须用到新版 API，需提供 Win7 下的等效 GDI/GDI+ 回退实现。
2. UI 增强一律使用 GDI+ 或 GDI 实现，禁止使用现代 DirectComposition。
3. 默认关闭且不实现彩色字体（emoji 彩色渲染），相关代码可放空或直接禁用。
