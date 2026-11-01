# 安装、升级与卸载

Bridge 只在选择实验性原生物理时需要。默认动作播放和内建弹簧物理无需本库，普通用户也无需 Python 或编译工具。

## 获取匹配组件

先按 GakuMotion 对应版本说明安装 GakuLoader 和 GakuMotion。Bridge 不独立注册为 Loader 插件，不能代替它们。

本仓目前提供源码，尚无可下载、已标明匹配 GakuMotion 和游戏客户端版本的产品包。请使用作者提供的配套组合，或按[开发者指南](DEVELOPMENT.md)构建。构建通过、文件名相同或 ABI v2 相同都不能证明任意 DLL 可以互换，详见[兼容说明](COMPATIBILITY.md)。

## 安装

1. 正常退出游戏。
2. 将配套 `GakuBurstBridge.dll` 放入游戏目录的 `gakumas-local/GakuMotion/`；没有此目录时可创建。
3. 重新启动游戏，在 GakuMotion“设置”页选择实验性原生物理。
4. 查看实际后端和状态：`pending` 是发现中，不表示注入已成功。失败时保持内建物理，按[排障页](TROUBLESHOOTING.md)检查 detail。

```text
游戏目录/
└─ gakumas-local/
   ├─ plugins/                 已有 GakuMotion 等插件
   └─ GakuMotion/
      └─ GakuBurstBridge.dll   可选伴生库
```

不要放进 `plugins/` 根目录，也不要将其改名为 `version.dll`。

## 升级与回退

正常退出游戏，保存原 DLL 和对应版本说明，再换成新的配套 DLL。回退时同样在退出后恢复保存版本。发行说明要求配套更新 GakuMotion 时应一起更新，不能只凭文件名判断兼容。

运行中可以切回内建物理，但不会卸载已装钩子的 DLL。磁盘文件替换和重新选择模式不能实现同进程换版；新 DLL 需要新进程加载。

## 卸载

在 GakuMotion 中切回内建物理或关闭物理，正常退出游戏，然后移走 `gakumas-local/GakuMotion/GakuBurstBridge.dll`。重新启动后使用内建物理。无需删除动作、音乐、曲库或 GakuMotion 配置，也不需要移除 Loader 和其它插件。

本仓构建/测试脚本不执行这些安装、替换或卸载操作。
