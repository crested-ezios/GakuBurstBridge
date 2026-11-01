# GakuBurstBridge

GakuBurstBridge 是 [GakuMotion](https://github.com/crested-ezios/GakuMotion) 的可选 Windows x64 原生物理伴生库。它把已捕获的身体姿态交给支持的官方 Burst Swing 动画流，让官方头发和服装摆动读取动作输入，并保留原求解器和自然调用顺序。

GakuMotion 默认使用内建弹簧物理。仅使用动作播放时无需 Bridge、Visual Studio 或 Python；选择实验性原生物理时才加载本库，不可用时回退到内建物理。

## 安装与使用

先完成 GakuMotion 及其 [GakuLoader](https://github.com/crested-ezios/GakuLoader) 宿主的安装，再使用该版本配套的 `GakuBurstBridge.dll`，放到游戏目录的：

```text
gakumas-local/GakuMotion/GakuBurstBridge.dll
```

Bridge 不放在 `plugins/` 根目录，也不替换 Loader 的 `version.dll`。安装或替换前正常退出游戏；重新启动后，在 GakuMotion“设置”页选择实验性原生物理，并查看实际后端状态。`pending` 表示仍在等待支持入口。

**目前本仓还没有可直接下载、已标明与 GakuMotion 匹配版本的二进制产品包。** 本机构建的测试 DLL 没有作为正式发行包发布。用户可以继续使用默认内建物理，开发者可从源码构建。

[安装、升级与卸载](docs/INSTALLATION.md) · [使用排障](docs/TROUBLESHOOTING.md) · [版本与组件关系](docs/COMPATIBILITY.md)

## 从源码构建

需要 Windows x64、Visual Studio 2022 或更新版本的 MSVC C++ x64/x86 工具链及 Windows 10/11 SDK。取得完整源码后，在根目录运行：

```powershell
powershell -NoProfile -File build.ps1
```

输出 `build/GakuBurstBridge.dll`，脚本不会部署到游戏。MinHook 源码已随仓库提供，不需要子模块下载。

```powershell
powershell -NoProfile -File build.ps1 -RunTests
```

此命令同时运行 ABI、Core、Quartz、Sleeve 合成测试。隔离临时目录、可选 Python 测试及构建排障见[开发者指南](docs/DEVELOPMENT.md)。

## 接入与维护

- [文档导航](docs/README.md)：按用户、开发者和维护者组织入口。
- [C ABI v2](include/GakuBurstBridge.h)和[集成契约](docs/INTEGRATION.md)：线程、所有权、姿态提交及 fallback。
- [客户端更新维护指南](docs/MAINTENANCE_SOP.md)：PE、元数据、入口定位、回退与性能口径。
- [构建验证记录](docs/VALIDATION.md)：独立本地 clone 的实测结果及适用范围。

BeforeQuartz 和 Sleeve 诊断默认关闭。已安装钩子的 DLL 保留到进程退出，不支持热卸载。

项目采用 [MIT 许可](LICENSE)；第三方许可见 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)。依赖变体与来源资料保留在 [MinHook 上游说明](deps/minhook/UPSTREAM.md)和 [SOURCE_REVIEW.md](SOURCE_REVIEW.md)。
