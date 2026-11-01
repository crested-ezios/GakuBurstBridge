# 版本与组件关系

| 项目 | 当前关系 |
| --- | --- |
| 平台 | Windows x64；原生实现显式要求 64 位指针。 |
| GakuMotion | 加载库并提供角色姿态、生命周期及后端选项，须使用其发行说明配套的 Bridge。 |
| GakuLoader | 是 GakuMotion 的宿主；Bridge 不直接依赖 Loader SDK，也不作为插件扫描目标。 |
| Bridge C ABI | `GBB_API_VERSION=2`，调用方同时校验函数表与每种结构的 size/version。 |
| 当前参考 profile | `gakumas-burst-20261008-windows-x64`，含 SSE/AVX 参考入口和 ABI 偏移；该日期不是游戏发行版本号。 |
| 工具依赖 | Python 仅维护工具需要，MSVC/SDK 仅源码构建需要；普通用户运行不需要它们。 |

支持范围由 [NativeProfile.hpp](../src/NativeProfile.hpp)、[QuartzProfile.hpp](../src/QuartzProfile.hpp)及运行时元数据/入口校验决定。[参考 JSON](../profiles/supported-build-20261008.json)用于工具对照，改 JSON 不改变 DLL。

本仓尚无可下载的配套产品包，以及完整的“GakuMotion × Bridge × 游戏客户端”发行矩阵。配套包需要列明组件版本/提交、文件位置、支持范围和 SHA256；不能把任何一次测试 DLL 当成正式兼容发行。

可明确复核的代码基线为 `11a771b71ff4d87228a2bf0dc0857db604a616d4`，已独立本地 clone 构建，见[验证记录](VALIDATION.md)。它不扩大到未经记录的新客户端、角色或服装，也不改变用户已有的 GakuMotion 游戏内验收结论。
