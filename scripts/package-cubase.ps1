param([Parameter(Mandatory=$true)][string]$NativeBuild,[Parameter(Mandatory=$true)][string]$DeliveryRoot)
$ErrorActionPreference='Stop'
$repo=(Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$release=Join-Path $DeliveryRoot '01_发布包'
$name='Piano Auto Sustain 1.0 Cubase Windows x64 授权版'
$package=Join-Path $release $name
if(Test-Path -LiteralPath $package){throw 'Package already exists; inspect before replacing.'}
New-Item -ItemType Directory -Path $package -Force | Out-Null
$binary=Join-Path $NativeBuild 'Piano Auto Sustain Cubase.dll'
if(!(Test-Path -LiteralPath $binary)){$binary=Join-Path $NativeBuild 'Release\Piano Auto Sustain Cubase.dll'}
if(!(Test-Path -LiteralPath $binary)){throw 'Release plugin DLL was not found.'}
Copy-Item -LiteralPath $binary -Destination $package
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'Install-Cubase.ps1') -Destination $package
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'License-Activation.ps1') -Destination $package
@'
@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Install-Cubase.ps1"
'@ | Set-Content -LiteralPath (Join-Path $package '安装.cmd') -Encoding ASCII
Copy-Item -LiteralPath (Join-Path $repo 'docs\QUICKSTART-zh.md') -Destination (Join-Path $package '使用说明.md')
Copy-Item -LiteralPath (Join-Path $repo 'docs\QUICKSTART-en.md') -Destination (Join-Path $package 'Quick Start.md')
Copy-Item -LiteralPath (Join-Path $repo 'docs\VALIDATION-1.0.0.md') -Destination (Join-Path $package '验证范围.md')
Copy-Item -LiteralPath (Join-Path $repo 'licenses') -Destination (Join-Path $package 'licenses') -Recurse
@'
钢琴自动延音踏板 1.0（Cubase / Windows x64）授权说明

产品编号：AMU_PIANO_AUTO_SUSTAIN
授权服务器：https://license.amumusic.com

安装步骤：
1. 保存工程并完全退出 Cubase / Nuendo。
2. 双击“安装.cmd”。
3. 首次安装时输入本产品的授权码，显示激活成功后继续安装。
4. 重新启动 Cubase，在 MIDI 插入中选择 Piano Auto Sustain Cubase。

授权规则：
- 一个授权码首次激活后绑定当前 Windows 电脑。
- 同一台电脑以后安装本产品的新版本，通常无需更换授权码，会自动读取原授权。
- 更换电脑或重装 Windows 后机器标识可能变化，需要在授权管理中心先解绑，再重新激活。
- 授权文件保存在当前 Windows 用户的本地应用数据目录，并使用 Windows DPAPI 加密。
- 授权无效时插件仍可安全加载并原样传递 MIDI，但不会自动生成延音踏板事件。

发码备注建议：钢琴自动延音踏板 1.0 / 客户名称 / 发码日期
'@ | Set-Content -LiteralPath (Join-Path $package '授权说明.txt') -Encoding UTF8
$hashes=Get-ChildItem -LiteralPath $package -File -Recurse | ForEach-Object {"$((Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLower())  $($_.FullName.Substring($package.Length+1))"}
Set-Content -LiteralPath (Join-Path $package 'SHA256SUMS.txt') -Value $hashes -Encoding UTF8
Compress-Archive -LiteralPath $package -DestinationPath (Join-Path $release "$name.zip") -CompressionLevel Optimal
Write-Output $package
