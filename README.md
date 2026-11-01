# GakuBurstBridge

GakuMotion 的可选 Windows x64 原生物理伴生库，通过稳定 C ABI v2 在已确认的官方 Burst Swing 入口前交接身体姿态。普通动作播放不需要本库；缺失、门禁拒绝或初始化失败时由 GakuMotion 使用 fallback。

本目录为独立、本地、无远端的源码 staging 仓。源码、合成测试、只读维护工具以及固定版本 MinHook vendor 均随目录提供，不依赖其他工作区或子模块下载。现有源码目录可直接构建；如需传递源码，可在本仓构建后执行 `git archive --format=zip --output=build/GakuBurstBridge-source.zip HEAD`，接收者解压后从根目录构建。归档仅含已提交源码，不含历史仓 Git 对象、缓存或二进制。

## 构建与合成测试

需要 Windows x64、Visual Studio 2022 或更新版本的 MSVC C++ x64/x86 工具链及 Windows 10/11 SDK。脚本通过 Visual Studio Installer 的 vswhere 自行定位工具链，不要求 Developer PowerShell。MinHook 已 vendor，无需 git submodule 命令。该 vendor 冻结来源仓实际使用的本地变体，并非声明上游提交的原样副本；具体差异与来源限制见上游记录。Python 3 只用于只读工具与对应测试，不参与 DLL 运行。

在源码根目录运行：

```powershell
powershell -NoProfile -File build.ps1
```

产物为 `build/GakuBurstBridge.dll`，脚本只构建，不部署。建议在本次新建的隔离临时目录运行合成测试：

```powershell
$bridgeTestTemp = Join-Path (Get-Location) ('build/test-temp-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $bridgeTestTemp -Force | Out-Null
$env:TEMP = $bridgeTestTemp
$env:TMP = $bridgeTestTemp
powershell -NoProfile -File build.ps1 -RunTests
python -m unittest discover -s tools/tests
```

C ABI、Core、Quartz、Sleeve 测试使用合成对象、受控 PE 内存和回调，不启动游戏。ABI 测试加载本次构建 DLL，并在上述 TEMP 里创建再删除 `GakuBurstBridge-abi-traces.json`。Python 工具测试只生成合成 PE/JSON。

## 使用与集成

本库不是 GakuLoader 插件。配套发布包中的 DLL 位置是游戏目录下 `gakumas-local/GakuMotion/GakuBurstBridge.dll`；普通用户应使用与 GakuMotion 配套的版本。源码 staging 不包含安装器或部署操作。

- [C ABI](include/GakuBurstBridge.h) 与[线程、所有权和降级契约](docs/INTEGRATION.md)。
- [维护入口](docs/MAINTENANCE_SOP.md)；JSON profile 是只读参考，运行时权威是 `src/NativeProfile.hpp`。
- [来源审阅](SOURCE_REVIEW.md)、[逐文件来源与 SHA256](SOURCE_PROVENANCE.json)、[固定上游版本](deps/minhook/UPSTREAM.md)。
- [项目 MIT 许可](LICENSE)、[第三方通知](THIRD_PARTY_NOTICES.md)和 [staging NOTICE](NOTICE)。

Native 模式必须显式选择，pending 代表尚未发现支持入口，不能表示注入成功。BeforeQuartz 与 Sleeve 诊断默认关闭。已安装钩子的 DLL 固定在进程内直到退出，不支持热卸载。合成测试不证明新的游戏客户端、角色或服装已通过视觉验收。
