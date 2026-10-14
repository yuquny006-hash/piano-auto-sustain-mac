# Mac 授权源码交接 · 2026-10-14

这是新增授权后的源码，不能把之前编译的无授权 TEST 二进制直接放进授权文件夹发客户。必须用此源码重新编译。

先由服务器管理员部署 Mac 签名服务，拿到64字符大写十六进制**公钥**，另存为本目录 `mac-public-key.txt`。不要把服务器私钥发给朋友。当前没有生产公钥，构建脚本会明确停止，不会生成假授权版。

Mac 编译朋友：安装 Xcode、CMake、Python3；在工程目录运行：

```bash
bash scripts/build-mac-licensed.sh
```

脚本自动使用随包 libsodium 1.0.20 源码构建 Universal 静态库，获取 JUCE 8.0.12，编译 VST3/AU，运行验签与处理器测试，检查架构与 ad-hoc 签名并输出 Licensed-CANDIDATE ZIP。首次需要网络下载 JUCE。不要使用旧 build-portable.sh 代替本次授权构建入口。

脚本不自动覆盖已装插件。构建后退出宿主，运行候选包 Install.command，再按 `docs/MAC-ACCEPTANCE.md` 测试。使用说明在 `docs/MAC-LICENSED-GUIDE.md`；同样的关键激活与Logic录制说明已加入插件“说明书”。

本次本地验证：Windows 回归与跨语言签名验证通过；服务器使用临时数据库测试通过；安装事务通过故障注入测试。Mac编译、IOPlatformUUID、AU验证、Gatekeeper和宿主实机尚未验证。

该包只含源码、构建依赖与测试夹具，没有生产私钥、客户授权码或客户数据库。测试夹具公钥仅验证测试数据，不是生产公钥。不要向客户分发源码包。
