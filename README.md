# GakuBurstBridge

[安装与卸载指南](docs/INSTALLATION.md) · [使用排障](docs/TROUBLESHOOTING.md) · [版本兼容说明](docs/COMPATIBILITY.md) · [开发者指南](docs/DEVELOPMENT.md)

## 这是什么

GakuBurstBridge 是 [GakuMotion](https://github.com/crested-ezios/GakuMotion) 的可选 Windows x64 原生物理伴生库。它将动作播放时捕获的角色身体姿态传递给已确认的 Unity Burst Swing 动画流，让官方头发和裙摆解算读取动作输入。具体角色、服装和客户端的视觉结果需分别验证。

当前 GakuMotion 默认优先尝试原生物理；本伴生库缺失或未就绪时使用内置弹簧物理。用户也可显式选择内置后端，常规动作、表情与镜头播放不要求安装 Bridge。

## 准备工作与依赖

### 前置要求
- 已安装好 [GakuLoader](https://github.com/crested-ezios/GakuLoader) 插件加载器（游戏根目录下的 `version.dll`）。
- 已安装好 [GakuMotion](https://github.com/crested-ezios/GakuMotion) 动作播放插件（`gakumas-local/plugins/GakuMotion.dll`）。
- 配套版本的 `GakuBurstBridge.dll`。日常游玩使用预编译 DLL 时，无需安装 Visual Studio 或 Python。

### 文件放置路径

退出游戏后，将 `GakuBurstBridge.dll` 放置在如下特定目录中：

```text
<游戏根目录>/
├── gakumas.exe
├── version.dll                              # GakuLoader 核心
└── gakumas-local/
    ├── GakuMotion/
    │   └── GakuBurstBridge.dll              # 物理伴生库放置在此处
    └── plugins/
        ├── GakuMotion.dll                   # 动作播放插件
        └── GakuMotion.VmdImporter.Native.dll # VMD 导入器
```

> **重要放置提示**：
> 1. `GakuBurstBridge.dll` **切勿**放置在 `gakumas-local/plugins/` 目录中。它不是一个独立的 GakuLoader 插件，而是 GakuMotion 的私有伴生库；若放错目录将被加载器忽略。
> 2. `GakuBurstBridge.dll` **切勿**重命名为 `version.dll` 去替换根目录文件。

## 使用方法与快捷键

安装完成后，在游戏内按以下步骤启用：

1. 启动游戏并进入“偶像摄影”。
2. 按 **Ctrl+M** 唤出 GakuMotion 控制面板。
3. 切换至“设置”页，在“物理引擎”中选择“原生物理（默认，实验性）”。选择后端不等于注入成功。
4. 结合 GakuMotion 的状态诊断与宿主日志确认实际结果：
   - `active`：物理桥已正常挂钩，正在向官方 Burst 动画流传递骨骼姿态。
   - `pending`：物理桥已加载，正在等待场景准入或骨骼准备就绪。
   - 未安装或发生异常时：GakuMotion 会自动安全回退到内建的弹簧物理，保障动作正常播放。

## 常见问题与排查

1. **设置中选择原生物理后为什么不生效**：
   - 确认 DLL 文件是否放在了 `gakumas-local/GakuMotion/GakuBurstBridge.dll`，而不是 `plugins/` 目录。
   - 检查 `gakumas-local/gakuloader.log` 日志文件，搜索 `BurstBridge` 查看加载日志与初始化状态。
2. **角色头发或服装出现异常拉伸或抖动**：
   - 当动作位移幅度过大或帧率发生剧烈波动时，官方物理求解器可能产生瞬间抖动。
   - 可以在 GakuMotion 设置中随时将次级物理后端切回“内建弹簧物理”。
3. **如何彻底卸载**：
   - 完全退出游戏后，直接删除 `gakumas-local/GakuMotion/GakuBurstBridge.dll` 即可。GakuMotion 会自动继续使用内建物理。模组不支持在游戏运行中途热卸载。
4. **更多排障细节**：
   - 详细的诊断方法与故障排查请参阅 [使用排障指南](docs/TROUBLESHOOTING.md)。

## 源码构建

### 编译环境
- 操作系统：Windows 10 / 11 64 位
- 编译器：Visual Studio 2022（或更高版本，包含 MSVC C++20 与 Windows 10/11 SDK）
- 仓库内已自带所需的 MinHook 源码，无需额外拉取外部子模块。

### 编译命令
在 `GakuBurstBridge` 根目录下运行 PowerShell：

```powershell
# 编译 Release（产物输出为 build/GakuBurstBridge.dll）
powershell -NoProfile -File build.ps1

# 编译并运行合成测试套件（覆盖 ABI、Core、Quartz 与 Sleeve 测试）
powershell -NoProfile -File build.ps1 -RunTests
```

编译完成后，手动将 `build/GakuBurstBridge.dll` 复制到游戏目录的 `gakumas-local/GakuMotion/` 文件夹中即可。

### 开发者文档与契约
- [文档索引导航](docs/README.md)
- [C ABI v2 接口定义](include/GakuBurstBridge.h) 与 [集成契约](docs/INTEGRATION.md)
- [客户端更新维护指南](docs/MAINTENANCE_SOP.md)
- [构建验证记录](docs/VALIDATION.md)
- 源码遵循 [MIT 许可证](LICENSE)，第三方通知见 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)，依赖来源记录见 [MinHook 上游说明](deps/minhook/UPSTREAM.md) 与 [源码审计说明](SOURCE_REVIEW.md)。
