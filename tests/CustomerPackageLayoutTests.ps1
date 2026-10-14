param([Parameter(Mandatory=$true)][string]$PackageRoot)
$ErrorActionPreference='Stop'
function Assert-True([bool]$Condition,[string]$Message){if(!$Condition){throw "ASSERT FAILED: $Message"}}

$rootItems=@(Get-ChildItem -LiteralPath $PackageRoot -Force)
$expectedRoot=@('01_Cubase与Nuendo专用版','02_通用VST3版','01_先看这里_版本选择说明.txt','02_授权与安装说明.txt','03_各宿主安装与使用说明.txt')
Assert-True ($rootItems.Count -eq $expectedRoot.Count) 'customer root must contain only two product folders and three guides'
foreach($name in $expectedRoot){Assert-True (Test-Path -LiteralPath (Join-Path $PackageRoot $name)) "missing root item: $name"}

$cubase=Join-Path $PackageRoot '01_Cubase与Nuendo专用版'
$cubaseFiles=@(Get-ChildItem -LiteralPath $cubase -Force)
$expectedCubase=@('安装.cmd','Piano Auto Sustain Cubase.dll','Install-Cubase.ps1','License-Activation.ps1')
Assert-True ($cubaseFiles.Count -eq $expectedCubase.Count) 'Cubase folder contains unnecessary files'
foreach($name in $expectedCubase){Assert-True (Test-Path -LiteralPath (Join-Path $cubase $name) -PathType Leaf) "missing Cubase file: $name"}

$vst3=Join-Path $PackageRoot '02_通用VST3版'
$expectedVst3=@('安装.cmd','卸载.cmd','Piano Auto Sustain.vst3','Install-VST3.ps1','License-Activation.ps1','SHA256SUMS.txt')
$vst3Items=@(Get-ChildItem -LiteralPath $vst3 -Force)
Assert-True ($vst3Items.Count -eq $expectedVst3.Count) 'VST3 folder contains unnecessary files'
foreach($name in $expectedVst3){Assert-True (Test-Path -LiteralPath (Join-Path $vst3 $name)) "missing VST3 item: $name"}

$guide=Get-Content -Raw -LiteralPath (Join-Path $PackageRoot '03_各宿主安装与使用说明.txt')
foreach($hostName in @('Cubase','Nuendo','REAPER','Studio One','Ableton Live','FL Studio','CC64')){
 Assert-True ($guide.Contains($hostName)) "host guide is missing: $hostName"
}
Assert-True (!$guide.Contains('测试包不含商业授权')) 'customer guide must not describe this as an unlicensed test package'
Write-Output 'PASS: customer package is minimal and contains all host guides'
