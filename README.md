# 小狼毫 Weasel Win7 定制版
**为 Windows 7 用户打造的 Rime 输入法**

原版小狼毫自 0.15.0 起放弃了 Windows 7 支持。本项目基于 **Weasel 0.14.3** 稳定版源码，通过升级核心引擎和精心移植部分新版本 UI 特性，为 Windows 7 用户提供一个开箱即用、稳定美观、持续可维护的 Rime 输入法。

---

## ✨ 核心特性

| 方面 | 说明 |
|------|------|
| 🖥️ **Win7 原生兼容** | 专门为 Windows 7 SP1 (32/64 位) 编译，无需修改系统或安装兼容层即可高效运行。 |
| ⚙️ **升级核心引擎** | 替换了原版携带的旧版 librime 核心，采用单独验证可在 Win7 正常工作的 **librime 1.11.2**，提供更好的性能和方案支持。 |
| 🧩 **移植 UI 增强（开发中）** | 通过 GDI+/GDI 在新版前端中加入了部分后期才出现的 UI 特性（圆角、阴影、更丰富的色彩格式），同时暂不引入 Win7 难以支持的特性（如彩色 emoji 渲染）。 |
| 🔒 **开源 & 自由** | 沿用 [GNU General Public License v3](LICENSE) 协议发布，没有数据上传，没有后台广告，所有代码开放审计。 |

## 📦 技术栈

- **前端**：Weasel 0.14.3（C++，GDI+/GDI UI 渲染）
- **后端引擎**：librime 1.11.2
- **打包工具**：NSIS

## 🤝 贡献与支持
- 欢迎 Win7 用户和 Rime 爱好者体验、提交问题或参与开发。
- 发现 bug 或提出建议：[Issues](https://github.com/Liora-Wells/weasel-win7/issues)

## 📜 许可证

本项目基于 **GNU General Public License v3** 协议发布，继承自原始 [rime/weasel](https://github.com/rime/weasel) 项目。你可以在 [LICENSE](LICENSE) 文件中查看完整条文。

## 🙏 致谢

- [rime/weasel](https://github.com/rime/weasel) — 原始小狼毫输入法前端
- [rime/librime](https://github.com/rime/librime) — Rime 输入法引擎
- 以及所有为 Rime 生态做出贡献的开发者们
```
