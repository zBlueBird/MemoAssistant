# 第三方软件声明

MemoAssistant 的 Windows 二进制发行包使用或随附下列第三方组件。本文档仅记录第三方组件，不授予 MemoAssistant 自有源代码许可证；项目自有代码的许可状态仍需由权利人确定。

## Qt 6.11.1

Copyright (C) The Qt Company Ltd. and other contributors.

当前构建动态链接 Qt，并由 `windeployqt` 部署 Qt DLL 和插件。Qt 安装包随附的 SPDX 清单将已部署的 Qt Core、Gui、Widgets、Sql、Network、Svg 模块以及相关插件标记为以下多选许可证：

```text
LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
```

当前发布流程按 **LGPL-3.0-only** 选项处理。LGPL 3.0 和其所纳入的 GPL 3.0 完整文本分别见：

- `licenses/LGPL-3.0.txt`
- `licenses/GPL-3.0.txt`

本机 Qt 6.11.1 安装包生成的组件 SPDX 清单见：

- `licenses/qt/qtbase-6.11.1.spdx`
- `licenses/qt/qtsvg-6.11.1.spdx`

Qt 项目主页及源码：

- <https://www.qt.io/>
- <https://download.qt.io/official_releases/qt/6.11/6.11.1/submodules/>
- <https://code.qt.io/cgit/>

发行包必须保持 Qt 为可替换的动态库，不得阻止用户为调试 Qt 修改而进行的反向工程，并应为实际发布的 Qt 二进制保留准确的对应源码获取方式。发布者应在发行前按 `docs/发布合规检查清单.md` 逐项核对。

## GCC / libstdc++ 运行库

`windeployqt --compiler-runtime` 会部署 GCC/MinGW 运行库，包括当前发行目录中的：

- `libgcc_s_seh-1.dll`
- `libstdc++-6.dll`

相关 GCC 库受 GPL 3.0 与 GCC Runtime Library Exception 3.1 约束。文本见：

- `licenses/GPL-3.0.txt`
- `licenses/GCC-RUNTIME-LIBRARY-EXCEPTION-3.1.txt`

## MinGW-w64 / winpthreads

当前发行目录还包含：

- `libwinpthread-1.dll`

本机 MinGW 13.1.0 Kit 随附的许可证文本见：

- `licenses/MINGW-W64-RUNTIME.txt`
- `licenses/WINPTHREADS.txt`

## Microsoft D3D Compiler

当前 `windeployqt` 输出包含 Microsoft 的 `D3Dcompiler_47.dll`。该文件不是 Qt 或 MemoAssistant 自有代码。发布者必须确认其复制与再分发符合用于构建/部署该文件的 Microsoft SDK 或可再发行组件条款；本仓库未复制 Microsoft 许可文本，也未对其再分发权作出额外声明。

## 发行包内容会变化

`windeployqt` 的输出可能因 Qt 版本、构建选项和插件发现结果而变化。每次发布都必须以最终 `dist/` 的实际文件为准重新核对第三方组件与许可证；不能仅依赖本文档的静态清单。

本文档不是法律意见。若计划向组织外发布，请由软件权利人或合规负责人确认最终许可方案。
