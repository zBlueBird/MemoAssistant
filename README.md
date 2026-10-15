# MemoAssistant（备忘录助手）

面向 Windows 11 的轻量桌面备忘录工具，使用 C++20、Qt 6 Widgets 和 SQLite 开发。

## 已实现功能

- 始终置顶的圆形悬浮球
- 拖动、屏幕边缘吸附及位置持久化
- 点击悬浮球展开备忘录，可按日期查看历史记录并一键回到今天
- 鼠标离开面板后延迟收起，`Esc` 可立即收起
- 创建、编辑、完成和删除备忘录
- 可选的单次提醒时间
- SQLite 本地持久化（WAL 模式）
- Windows 托盘通知及程序常驻
- 托盘/悬浮球右键菜单
- 登录 Windows 后自动启动开关
- 单实例运行；重复启动会唤醒已有实例
- 多显示器可见区域约束
- 数据层自动测试

完整产品和技术方案见 [`docs/完整方案.md`](docs/完整方案.md)，实机与干净机器验证结果记录在 [`docs/验收记录.md`](docs/验收记录.md)。

## 环境要求

- Windows 11 x64
- Qt 6.11.1 MinGW 64-bit（至少 Qt 6.5）
- CMake 3.21+
- Ninja
- Qt 模块：Core、Gui、Widgets、Sql、Network、Test

本机已验证的 Kit：

```text
Qt 6.11.1 MinGW 64-bit
MinGW GCC/G++ 13.1.0
CMake 3.30.5
Ninja 1.12.1
```

## 在 Qt Creator 中打开

1. 启动 Qt Creator。
2. 选择“打开项目”，打开根目录中的 `CMakeLists.txt`。
3. 选择 `Desktop Qt 6.11.1 MinGW 64-bit` Kit。
4. 构建并运行 `MemoAssistant`。

## 命令行构建

在已配置 Qt/MinGW 环境的终端中执行：

```powershell
$env:PATH = "D:\software\QtCreator\Tools\mingw1310_64\bin;D:\software\QtCreator\Tools\Ninja;$env:PATH"
& "D:\software\QtCreator\Tools\CMake_64\bin\cmake.exe" -S . -B build -G Ninja `
  -DCMAKE_PREFIX_PATH="D:/software/QtCreator/6.11.1/mingw_64" `
  -DCMAKE_CXX_COMPILER="D:/software/QtCreator/Tools/mingw1310_64/bin/g++.exe" `
  -DCMAKE_BUILD_TYPE=Debug
& "D:\software\QtCreator\Tools\CMake_64\bin\cmake.exe" --build build
& "D:\software\QtCreator\Tools\CMake_64\bin\ctest.exe" --test-dir build --output-on-failure
```

## 数据位置

应用数据使用 `QStandardPaths::AppLocalDataLocation`，本机通常为：

```text
%LOCALAPPDATA%\MemoAssistant\MemoAssistant\memo.db
```

配置通过 `QSettings` 保存。备忘录内容不会上传网络。

## 发布

使用发布脚本从干净目录构建、部署运行库、复制许可证并生成 SHA-256 清单：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1
```

如本机 Qt 工具链不在默认位置，可通过 `-QtDirectory`、`-MinGwBinDirectory`、`-CMake` 和 `-NinjaDirectory` 覆盖。脚本会校验关键运行库，并在部署后移除除 `qsqlite.dll` 外未使用的 SQL 驱动，避免把数据库客户端插件带入发行物。

部署完成后确认包含：

- `platforms/qwindows.dll`
- `sqldrivers/qsqlite.dll`
- Qt Core、Gui、Widgets、Sql、Network DLL
- MinGW 运行库

### 发布许可材料

当前构建动态链接 Qt，并按 Qt 的 LGPL 3.0 许可选项准备第三方材料。发布脚本会将声明和许可证目录复制到 `dist/`，随后按照 [`docs/发布合规检查清单.md`](docs/发布合规检查清单.md) 核对最终 `dist/` 的实际文件。尤其需要确认：

- Qt DLL 保持独立且可由用户替换，不得静态链接或施加阻止替换的技术措施；
- 随包保留 `THIRD_PARTY_NOTICES.md`、LGPL/GPL 正文、MinGW/GCC runtime 声明及对应 Qt SPDX 清单；
- 为所分发的 Qt 二进制落实 LGPL 对对应源码与应用重链接/替换能力的要求；
- 对 `windeployqt` 新增的每个 DLL/插件重新检查许可证，而不是假设静态清单始终完整；
- 在无 Qt SDK 的干净 Windows 11 环境验证发行包。

第三方组件与材料来源详见 [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md)。**MemoAssistant 自有代码和资源目前尚未声明许可证，正式对外发布前必须由权利人作出明确选择。**

### 安装包

仓库提供 Inno Setup 工程 `installer/MemoAssistant.iss`。安装 Inno Setup 6 后，在生成 `dist/` 的基础上执行：

```powershell
iscc.exe installer\MemoAssistant.iss
```

安装程序输出到 `installer-output/`。当前开发机未安装 Inno Setup，因此只完成了工程文件，尚未实际编译安装包。

## 操作说明

- **单击悬浮球**：展开/收起面板
- **日期栏 `‹` / `›`**：查看前一天或后一天（不允许超过今天）
- **日期栏“今天”**：从历史日期回到今天
- **拖动悬浮球**：移动，松手后吸附最近屏幕边缘
- **右键悬浮球**：显示快捷菜单
- **面板右上角 `+`**：创建备忘录
- **勾选记录**：标记完成或重新打开
- **铅笔/垃圾桶图标**：编辑或删除
- **鼠标离开面板**：约 700ms 后自动收起
- **托盘菜单 → 退出**：真正退出应用

## 当前提醒限制

第一版会同时请求 Windows 托盘通知并显示非模态的应用内置顶提醒，避免通知权限或专注助手导致提醒静默丢失。只有程序正在运行时才能按时提醒；若程序关闭，下一次启动会补查数据库中未发送且已到期的提醒。
