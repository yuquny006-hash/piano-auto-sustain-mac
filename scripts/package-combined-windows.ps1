param(
 [string]$OutputRoot='E:\软件制作\钢琴自动延音踏板vst插件\01_发布包'
)
$ErrorActionPreference='Stop'
$project=Split-Path $PSScriptRoot -Parent
$build='E:\Codex\2026-09-05\new-chat\work\piano-auto-sustain-vst3-build'
$cubaseSource='E:\Codex\2026-09-05\new-chat\delivery\01_发布包\Piano Auto Sustain 1.0 Cubase Windows x64 授权版'
$vst3Source=Join-Path $build 'BassPedal_artefacts\Release\VST3\Piano Auto Sustain.vst3'
$nativeBinary='E:\Codex\2026-09-05\new-chat\work\piano-auto-sustain-build-release\Piano Auto Sustain Cubase.dll'

$cubaseFiles=@('安装.cmd','Piano Auto Sustain Cubase.dll','Install-Cubase.ps1','License-Activation.ps1')
$required=@($vst3Source,$nativeBinary)
$required += $cubaseFiles | ForEach-Object { Join-Path $cubaseSource $_ }
foreach($item in $required){
 if(!(Test-Path -LiteralPath $item)){throw "Missing package input: $item"}
}

$stage=Join-Path $env:TEMP ('pas-customer-'+[guid]::NewGuid().ToString('N'))
$customerName='Piano Auto Sustain 1.0 Windows x64 客户授权完整包'
$customer=Join-Path $stage $customerName
$cubase=Join-Path $customer '01_Cubase与Nuendo专用版'
$vst3=Join-Path $customer '02_通用VST3版'
try {
 New-Item -ItemType Directory -Path $cubase,$vst3 -Force | Out-Null
 foreach($name in $cubaseFiles){
  $packageInput=if($name -eq 'Piano Auto Sustain Cubase.dll'){$nativeBinary}else{Join-Path $project ('scripts\'+$name)}
  if($name -eq '安装.cmd'){$packageInput=Join-Path $cubaseSource $name}
  Copy-Item -LiteralPath $packageInput -Destination $cubase -Force
 }
 Copy-Item -LiteralPath $vst3Source -Destination $vst3 -Recurse -Force
 Copy-Item -LiteralPath (Join-Path $project 'scripts\Install-VST3.ps1') -Destination $vst3
 Copy-Item -LiteralPath (Join-Path $project 'scripts\License-Activation.ps1') -Destination $vst3
 Set-Content -LiteralPath (Join-Path $vst3 '安装.cmd') -Encoding Ascii -Value "@echo off`r`npowershell.exe -NoProfile -ExecutionPolicy Bypass -File `"%~dp0Install-VST3.ps1`"`r`npause"
 Set-Content -LiteralPath (Join-Path $vst3 '卸载.cmd') -Encoding Ascii -Value "@echo off`r`npowershell.exe -NoProfile -ExecutionPolicy Bypass -File `"%~dp0Install-VST3.ps1`" -Action Uninstall`r`npause"

 $vstManifest=Get-ChildItem -LiteralPath (Join-Path $vst3 'Piano Auto Sustain.vst3') -File -Recurse | Sort-Object FullName | ForEach-Object {
  $relative=$_.FullName.Substring($vst3.Length+1).Replace('\','/')
  '{0}  {1}' -f (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant(),$relative
 }
 Set-Content -LiteralPath (Join-Path $vst3 'SHA256SUMS.txt') -Value $vstManifest -Encoding UTF8

 @'
版本选择说明

1. Cubase 或 Nuendo 用户：进入“01_Cubase与Nuendo专用版”，双击“安装.cmd”。
2. REAPER、Studio One、Ableton Live、FL Studio 等支持 VST3 的宿主：进入“02_通用VST3版”，双击“安装.cmd”。
3. 同一台电脑只需成功激活一次；两个版本共用本机授权，不需要重复使用新码。
4. 只安装自己宿主需要的版本即可；需要同时使用多种宿主时，可以安装两个版本。
5. 详细的宿主加载、路由和录制方法，请查看“03_各宿主安装与使用说明.txt”。
'@ | Set-Content -LiteralPath (Join-Path $customer '01_先看这里_版本选择说明.txt') -Encoding UTF8

 @'
授权与安装说明

产品编号：AMU_PIANO_AUTO_SUSTAIN
授权网站：https://license.amumusic.com/

安装步骤：
1. 根据“01_先看这里_版本选择说明”进入对应文件夹。
2. 双击“安装.cmd”，输入卖家提供的授权码。
3. 显示激活成功后，安装程序会自动安装插件。
4. 重启音乐宿主并重新扫描插件。

授权规则：
- 授权绑定当前 Windows 用户和当前电脑。
- 同一台电脑升级软件时会自动复用原授权，不需要更换授权码。
- 更换电脑、重装系统或更换 Windows 用户后，可能需要联系卖家解绑或重新授权。
- 如果提示无法连接授权服务器，请检查网络、系统时间、防火墙，并确认能打开授权网站。
'@ | Set-Content -LiteralPath (Join-Path $customer '02_授权与安装说明.txt') -Encoding UTF8

 @'
各宿主安装与使用说明

一、先了解插件用途
Piano Auto Sustain 是 MIDI 处理插件，本身不发出钢琴声音。正确链路是：
键盘或 MIDI 片段 → Piano Auto Sustain → 钢琴音源 → 音频输出。
插件默认生成延音踏板控制信息 CC64。若需要把效果录进工程，请确认宿主录制的是插件处理后的 MIDI 输出。

二、Cubase / Nuendo（专用版）
1. 退出宿主，进入“01_Cubase与Nuendo专用版”，双击“安装.cmd”。
2. 重启宿主，建立乐器轨并加载钢琴音源。
3. 在该轨的 MIDI Inserts（MIDI 插入效果）中加载 Piano Auto Sustain Cubase。
4. 需要录制生成的踏板信息时，把插件放在 MIDI 插入录制分界线以上，或启用将 MIDI 插入效果输出录入轨道的功能。
5. 录制后打开控制器编辑区，选择 CC64，检查是否出现踏板曲线。
说明：Cubase 15 已实机验证。部分较旧版本或 AI/Elements 版本可能没有完整的 MIDI Inserts 功能。

三、REAPER（通用 VST3 版）
1. 进入“02_通用VST3版”，双击“安装.cmd”，然后在 REAPER 中重新扫描 VST3。
2. 建立 MIDI 处理轨，启用录音和输入监听，先插入 Piano Auto Sustain。
3. 在插件之后加载钢琴音源；也可以把处理轨的 MIDI 发送到另一条钢琴音源轨。
4. 要录制处理结果，选择 Record: output (MIDI)，录制后在事件/控制器视图检查 CC64。
5. 播放已经录好的处理后 MIDI 时，可旁路本插件，避免重复生成 CC64。

四、Studio One（通用 VST3 版）
1. 安装后让 Studio One 重新扫描 VST3。
2. 建立两条乐器轨：第一条加载 Piano Auto Sustain，第二条加载钢琴音源。
3. 钢琴轨的键盘图标行选钢琴音源，箭头图标行选 Piano Auto Sustain / MIDI Output 1。处理轨的键盘图标行选本插件，箭头图标行选实体键盘。
4. 两条轨监听都打开，只打开钢琴轨的录音准备，处理轨关闭录音准备；这样只录出一份音符与 CC64。
5. 录完在钢琴轨的控制器区检查 Sustain / CC64。录音留在钢琴轨直接回放，处理轨不要留重复片段。保存为模板，下次直接录制。
验证：Fender Studio Pro 8 已由用户确认发声、延音及录入 CC64；其他 Studio One 版本需实际测试。

Cubase 15 使用通用 VST3 的两轨方案
1. 处理乐器轨加载 Piano Auto Sustain，输入选实体键盘，例如 iRig KEYS Pro；监听打开，录音准备关闭。
2. 钢琴乐器轨加载 Kontakt 等钢琴音源；顶部 MIDI 输入选插件分组中的 Piano Auto Sustain - MIDI Output，监听与录音准备打开。
3. 钢琴轨不要再添加专用 MIDI 插入版。只在钢琴轨录制，随后在键盘编辑器查看 CC64。
4. 回放片段留在钢琴轨，不移回处理轨；保存模板以复用设置。
验证：Cubase 15 两轨路由已由用户确认可用，录制 CC64 应在控制器通道核对。

五、Ableton Live（通用 VST3 版）
1. 安装后在设置中启用并重新扫描 VST3。
2. 建立一条处理用 MIDI 轨并加载 Piano Auto Sustain。
3. 再建立一条加载钢琴音源的 MIDI 轨，把 MIDI From 设为处理轨或插件输出，并将 Monitor 设为 In。
4. 在接收轨上录制，打开 MIDI Clip 的 Envelope/控制器区域检查 CC64。

六、FL Studio（通用 VST3 版）
1. 安装后运行插件扫描并加载 Piano Auto Sustain。
2. 在插件 Wrapper 的 MIDI 设置中指定 Output Port。
3. 在钢琴音源 Wrapper 中把 Input Port 设置成相同端口。
4. 先确认钢琴能发声、延音有效，再录制并检查 CC64 数据。
5. 如果当前 FL Studio 或 Wrapper 版本没有暴露插件 MIDI 输出，则该版本无法按此方式路由。

七、其他 VST3 宿主
宿主必须同时支持“加载 MIDI 效果/处理插件”和“把插件 MIDI 输出路由到乐器”。通用原则仍是：
MIDI 输入 → Piano Auto Sustain → 钢琴音源，并录制处理后的 MIDI 输出。

八、常见问题
- 没有声音：本插件不含钢琴音色，请确认后方已经加载钢琴音源，并且 MIDI 输出已正确路由。
- 没有延音：确认插件已启用、输出已送到音源，并检查 CC64 模式。
- 延音重复或异常：不要让已包含 CC64 的录音再次经过插件处理；回放时可旁路插件。
- 找不到插件：重启宿主，确认已启用 VST3 扫描并执行完整重新扫描。

验证状态：Cubase 15 专用 MIDI 插入已完成用户实机验证；Fender Studio Pro 8 两轨路由和 CC64 已由用户确认，Cubase 15 VST3 两轨路由已确认。REAPER、Ableton Live、FL Studio 及其他版本仍需实际测试。
'@ | Set-Content -LiteralPath (Join-Path $customer '03_各宿主安装与使用说明.txt') -Encoding UTF8

 $customerDir=Join-Path $OutputRoot '01_发给客户_授权版'
 New-Item -ItemType Directory -Path $customerDir -Force | Out-Null
 $customerZip=Join-Path $customerDir ($customerName+'.zip')
 Compress-Archive -LiteralPath $customer -DestinationPath $customerZip -CompressionLevel Optimal -Force
 Write-Output $customerZip
} finally {
 $tempRoot=[IO.Path]::GetFullPath($env:TEMP).TrimEnd('\')+'\'
 $resolvedStage=[IO.Path]::GetFullPath($stage)
 if(!$resolvedStage.StartsWith($tempRoot,[StringComparison]::OrdinalIgnoreCase) -or !(Split-Path $resolvedStage -Leaf).StartsWith('pas-customer-')){throw 'Unsafe temporary cleanup target'}
 if(Test-Path -LiteralPath $resolvedStage){Remove-Item -LiteralPath $resolvedStage -Recurse -Force}
}
