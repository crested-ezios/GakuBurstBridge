# 维护契约

先读 [集成契约](INTEGRATION.md) 和 [C ABI](../include/GakuBurstBridge.h)。本页仅保留可迁移的维护规则，不包含实际游戏输入、录制、反汇编报告或旧验收日志。

## 边界

- 未确认客户端兼容时保持 fallback；不放宽 ABI、入口、身份或字节门禁来消除报错。
- `src/NativeProfile.hpp` 为运行时权威；`profiles/supported-build-20261008.json` 只供工具对照，修改 JSON 不会改变 DLL。
- RVA、少量 opcode/调用参数模式、字段偏移和客户端 hash 是兼容检查事实；没有随仓库提供原始游戏 DLL、模型或真实动作。
- 支持范围变化需来源明确的只读证据、代码审阅和获授权的另行验证。本地合成测试不等于新客户端实机认可。

## 可独立运行的工具

```powershell
python tools/inspect_burst.py --help
python tools/inspect_metadata.py --help
python -m unittest discover -s tools/tests
```

工具仅接受调用方显式提供的输入，静态读取 PE/JSON 后写入显式 output，不 LoadLibrary、不连接游戏，也不批准写入或注入。本仓合成测试不需要提供任何游戏文件。

## 版本与回退

C ABI size/version、主线程与 worker 边界、固定输入快照、generation、双 Actor 指纹、单写入者和 fallback 规则保留。原生模块和 trampoline 在进程结束前不卸载；shutdown 仅取消未来输入与回调。

开发者构建和合成测试遵循根 [README](../README.md)。依赖来源、原始许可及修改范围见 [SOURCE_REVIEW.md](../SOURCE_REVIEW.md) 与 [MinHook 上游记录](../deps/minhook/UPSTREAM.md)。对外发布、游戏部署和真实运行验证不由这些脚本执行。
