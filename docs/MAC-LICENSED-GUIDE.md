# Piano Auto Sustain 1.0 · Mac 使用说明

本插件处理 MIDI 音符并生成延音踏板 CC64，自身没有钢琴音色。需要另加载钢琴音源。Mac 包含 VST3 和 AU MIDI FX；Logic 使用 AU，Cubase 使用 VST3。Universal 构建包含 Intel 与 Apple Silicon，最低构建目标 macOS 11；实际支持范围以随包真机验收记录为准。

## 安装、升级

1. 保存工程并完全退出 Logic、Cubase、Studio One、REAPER 等宿主。
2. 将 ZIP 完整解压到本地文件夹，不要在压缩包里运行，不要只复制 Install.command。
3. 双击 Install.command。安装器核对包内 SHA-256、检查插件代码签名，两个格式都准备成功后才替换旧版。失败会尝试恢复旧版，并显示保留备份的位置。若提示恢复不完整，请保留该文件夹联系销售方。
4. 首次脚本启动若被系统拦截，在 Finder 中右键选择“打开”；如仍被拦截，检查“系统设置 → 隐私与安全性”的相关提示。安装脚本只有被系统允许启动后才能处理插件的隔离属性。不要关闭整个系统的安全机制。
5. 重开宿主并重新扫描插件。安装路径只影响当前 macOS 用户：
   - VST3：`~/Library/Audio/Plug-Ins/VST3/Piano Auto Sustain.vst3`
   - Logic AU：`~/Library/Audio/Plug-Ins/Components/Piano Auto Sustain MIDI FX.component`

当前采用 ad-hoc 签名，未使用 Apple Developer ID 公证。不能保证每种 macOS/宿主的首次加载行为完全相同。遇到拦截请提供原始提示、系统版本和宿主版本。脚本不会保证绕过宿主的库校验。包中不需要额外动态 libsodium 安装。

升级保留原插件备份，也保留授权文件和工程。不要同时在系统 `/Library/Audio/Plug-Ins` 与用户目录放置同名旧版，以免宿主扫到错误副本。

## 激活

1. 在宿主里加载插件，点击主界面右上角的授权状态。
2. 输入购买的授权码，点击“激活”。首次需允许宿主访问 `https://license.amumusic.com`。
3. 等状态显示“已授权”。同一 Mac、同一系统用户下，AU 与 VST3 共用这次激活，其他已打开实例约两秒后识别。
4. 之后可断网使用。授权保存于 `~/Library/Application Support/AmuPianoAutoSustain/license.dat`，无需手动移动。

没有试用模式。未激活时音符、您已有的踏板数据照常通过，但不会生成自动延音。每个授权码在服务端只能绑定一台电脑，Windows/Mac 占用同一名额。显示“已绑定其他电脑”时联系销售方申请换机，不要反复重装。该永久离线方案的人工解绑只释放服务端名额，无法立即远程作废旧电脑上已保存的离线授权。

## Logic Pro：一条软件乐器轨

1. 建立软件乐器轨。在通道条 **Instrument/乐器** 插槽加载钢琴音源，并确认直接弹琴能出声。
2. 在乐器上方的 **MIDI FX** 插槽选择 Amu music 的 **Piano Auto Sustain MIDI FX**。不要放进 Audio FX，也不要找 VST3。
3. 激活本插件，保持“启用自动踏板”打开，输出模式使用“CC64 踏板”。选择该轨并开启录音准备，试弹低音与和弦，查看踏板动画及 CC64 计数。
4. 要把自动踏板写入录音：点本插件 MIDI FX 插槽右侧的箭头菜单，选 **Record MIDI to Track Here**，确认黄色三角标记在本插件下面。
5. 开始录制新片段，演奏完成松键后等尾音释放再停止。在钢琴卷帘的控制器/MIDI Draw 中选择 **Sustain / CC64**，应有高低变化，而非只有力度柱。
6. 回放已含 CC64 的片段时旁路本插件，确认仍有延音。这样验证数据确实写入了片段。

若版本里没有上述录制菜单，不能仅凭听见延音认定已录入；请提供具体 Logic 版本再核对支持方式。旧片段不会因为打开菜单自动补写踏板。Apple 官方操作参考：<https://support.apple.com/guide/logicpro/record-midi-to-track-lgcp423beea5/mac>。

## Cubase VST3：保留两条轨，只录一条

1. 处理轨加载 Piano Auto Sustain，输入指定实体键盘。打开监听，关闭录音准备。
2. 钢琴轨加载 Kontakt 等钢琴音源。顶部 MIDI 输入选 **Piano Auto Sustain - MIDI Output**，不要选 All MIDI Inputs。打开监听与录音准备。
3. 钢琴轨不再加载 Windows Cubase 专用 MIDI 插入。录音只在钢琴轨产生音符和 CC64。
4. 录后在钢琴轨控制器栏检查 CC64；播放该片段直接进入钢琴音源，不要移回处理轨。

## Studio One / Fender Studio Pro VST3

处理轨：乐器选择 Piano Auto Sustain，MIDI 输入选择实体键盘；只开监听，不开录音准备。
钢琴轨：乐器选择钢琴音源，MIDI 输入选择 Piano Auto Sustain / MIDI Output 1；打开监听和录音准备。
注意轨道名称不决定插件和路由，请看实际下拉框。两轨都录音会留下重复片段。设置成功后保存工程模板，下次无需重新连接。

## REAPER VST3

同一轨的 FX 链先放 Piano Auto Sustain，再放钢琴音源；指定键盘输入并启用监听。需要录下处理后的 MIDI 时选择 **Record: output (MIDI)**。先录短片段确认 CC64，再正式录制。回放已处理 MIDI 时旁路延音插件。也可采用处理轨发送 MIDI 至独立钢琴轨的方式，接收轨录制处理后 MIDI。

## 常见问题

- 无声音：插件本身不发声。先旁路它，确认钢琴音源加载、输出设备和监听正常，再核对 MIDI 输入来源。
- 有声音无自动延音：确认已授权、自动踏板启用、未旁路、输出模式 CC64、音源支持踏板；查看“诊断”的音符和 CC64 计数。
- 能听见延音但 CC64 空白：检查宿主录制点。Logic 的标记必须在插件后；两轨方案只录钢琴接收轨。
- 重复音符：不要让钢琴轨同时接收实体键盘与处理轨，不要两条轨同时录制。
- 持续响：点“释放踏板”，检查重复路由/循环路由；录完等尾音释放再停止。
- 授权目录不可写：确认使用自己的 macOS 用户登录，磁盘有空间，用户 Library 目录可写；不要用 sudo 启动宿主。
- 激活服务未就绪/验签失败：保留状态提示联系销售方，不要删除原有有效授权。升级签名服务未部署或公钥不匹配可能导致此错误。
- Logic 不显示：在插件管理器里查找本 AU 并重置/重新扫描；必要时重新登录系统。提供 auval 日志给开发者，不要随意清空全部宿主设置。

## 卸载

退出所有宿主后运行 Uninstall.command。它将这两个插件移到隐藏备份目录，保留授权和工程。换机绑定由销售方处理，删除本地文件不会自动解绑。

合作咨询微信：Amumusic1005
