# 构建验证记录

此页将[开发者指南](DEVELOPMENT.md)中的普通命令与实际结果对应；用户安装入口为[安装指南](INSTALLATION.md)。

## 独立本地 clone

验证代码为 `11a771b71ff4d87228a2bf0dc0857db604a616d4`。使用 `git clone --no-local` 从本地冻结仓建立独立 Git 对象，无 alternates 或对象硬链接；源与 clone 的 45 个 Git blobs、44 个 payload SHA256 和清单一致。

这证明独立的本地 clone，不冒充 GitHub 下载或托管远端 clone。当时11a771b基线的清单 SHA256 为 `f1e56c9eafa78593eb0261b4c0621c9aceab0ff8cf6c8216ac44b131917d3724`。

| 命令/检查 | 结果 |
| --- | --- |
| `powershell -NoProfile -File build.ps1 -RunTests` | 退出 0；DLL 构建、ABI、Core 通过，Quartz 379 项、Sleeve 25 项通过。 |
| `python -m unittest discover -s tools/tests` | 14/14 通过。 |
| 两个 Python 工具 `--help` | 在独立源码构建中通过。 |
| 构建后冻结检查 | payload/manifest 不变，源和 clone 工作区干净。 |

平台为 Windows x64/MSVC，Python 标准库测试。MinHook 既有 C4310、C4244、C4201、C4701 警告仍存在，结果不是零警告；临时文件限定在新 clone 的 build。

该次 DLL 的观察 SHA256 为 `9b040b47c0bc77729775b559cbc3ae69fff2bc1d22abebe5154675be5554b4bc`。它没有作为产品包发布，不是普通用户下载版本。

## 本轮文档维护

本轮恢复产品首页、安装/卸载、开发与维护文档。`src/`、`include/`、`tests/`、Python 实现、profile、vendor 和 `build.ps1` 保持上述基线字节，因此沿用固定代码已有构建结果，不声称重跑整套测试。新文档命令、链接和清单检查另行记录。

构建与合成检查不扩大为新的原生客户端兼容或视觉结论。用户已通过的 GakuMotion 游戏内验收维持原结论，无需因这次文档维护重做。

维护指南中的 PE/slot、probe+index、ABI JSON、native trace 四条只读 CLI 已在新合成 PE/JSON 上执行。PE 两条按预期返回 2（合成文件不匹配客户端 hash，但参考布局/入口检查和报告成功）；metadata 两条返回 0。没有读取或加载真实游戏输入。

当前文档增加了页面，因此完整源码清单的文件数和哈希会随之更新；这不改变固定编译输入。新清单中的 `documentation_maintenance` 记录本次范围与沿用测试的原因。
