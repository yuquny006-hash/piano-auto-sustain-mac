# Mac 授权连接诊断编译交接（2026-10-15）

状态：诊断源码，尚未在 Mac 编译及 Logic 实机验收。不是已修复的正式客户包。

客户终端 health HTTP 200，空激活请求 HTTP 400 INVALID_REQUEST，说明终端能访问服务。不能证明 Logic/AU 宿主的网络请求成功，也不能证明签名公钥匹配。

本次保留原 JUCE 网络传输、HTTPS 校验、15 秒超时、禁止重定向和 Ed25519 验签。仅在独立 JUCE 副本的授权请求完成回调记录 NSError domain/code 及一层 underlying error domain/code；不记录授权码、机器码、请求和响应正文。连接失败标记 D1，其他未分类激活错误显示 HTTP 状态；授权状态中文使用显式 UTF-8。

## 给负责 Mac 编译的朋友

1. 解压源码，确认 mac-public-key.txt 保持原生产公钥，不要生成新密钥。
2. 安装 Xcode 命令行工具、CMake、Python 3、Git。
3. 终端进入源码目录，执行 `bash scripts/build-mac-licensed.sh`。
4. 脚本在新的 mac-build-* 目录中复制 JUCE_SOURCE_DIR（若已设置），否则下载 JUCE 8.0.12，再添加诊断。不会改共享 SDK。需要网络下载时由 Git 使用 HTTPS。
5. 输出位置见最后的 `Built candidate only` 提示。先在 Mac 验证 AU 扫描、界面、激活与离线重开，再给问题客户做定向诊断。
6. 同时发送源码中的 scripts/Collect-Logic-Activation-Logs.command。诊断标记 D1 可识别连接失败时是否加载新构建；不要混装同名旧 AU。

## 给客户的复测步骤

1. 保存工程，完全退出 Logic，安装朋友新编译的诊断包；必要时在 Logic 插件管理器重新扫描 Piano Auto Sustain MIDI FX。
2. 在软件乐器轨 MIDI FX 槽加载插件，下面 Instrument 槽加载钢琴。
3. 输入自己原有授权码尝试一次即可。无需重新购买或换码。真实激活成功会正常绑定该电脑。
4. 若失败，保留报错截图，15 分钟内双击 Collect-Logic-Activation-Logs.command，把桌面的 Piano-Auto-Sustain-Logic-*.txt 发回。
5. 若日志为空，不代表网络正常：确认新版本、刚刚重试激活及系统日志读取结果。不要关闭 Gatekeeper、SIP 或整个防火墙。

收集器本身不发送激活请求、不修改授权绑定、不更改系统配置；仅提取带 PAS_AUTH_NETWORK_V1 标记的诊断日志。它对当前旧包不会凭空产生原生错误记录。

## 开发者验收

- 用原失败环境取得 NSError，区分超时、DNS/TLS、系统网络拒绝等，再针对证据修复。
- HTTP 200 但验签失败：另核对服务端签名公钥、返回字段与客户端；不能通过跳过验签解决。
- 授权成功后完全重开 Logic，断网验证已保存凭证；检查音符与 CC64。
- 正式包仍需 Mac 真机验收。Windows 测试和脚本语法检查不等于 Mac 编译成功。
