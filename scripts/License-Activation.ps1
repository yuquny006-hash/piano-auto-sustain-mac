$script:AmuProductId = 'AMU_PIANO_AUTO_SUSTAIN'
$script:AmuActivationUrl = 'https://license.amumusic.com/api/activate'

function Get-AmuDefaultLicensePath {
    Join-Path ([Environment]::GetFolderPath('LocalApplicationData')) 'AmuPianoAutoSustain\license.dat'
}

function Get-AmuMachineHash {
    param([string]$MachineGuidOverride)
    $machineGuid = $MachineGuidOverride
    if ([string]::IsNullOrWhiteSpace($machineGuid)) {
        $machineGuid = (Get-ItemProperty -LiteralPath 'HKLM:\SOFTWARE\Microsoft\Cryptography' -Name MachineGuid -ErrorAction Stop).MachineGuid
    }
    if ([string]::IsNullOrWhiteSpace($machineGuid)) { throw '无法读取本机设备标识。' }
    $inputBytes = [Text.Encoding]::UTF8.GetBytes("$script:AmuProductId|$machineGuid")
    $sha = [Security.Cryptography.SHA256]::Create()
    try { return -join ($sha.ComputeHash($inputBytes) | ForEach-Object { $_.ToString('X2') }) }
    finally { $sha.Dispose() }
}

function Protect-AmuLicenseRecord {
    param(
        [Parameter(Mandatory=$true)][string]$LicenseKey,
        [Parameter(Mandatory=$true)][string]$MachineHash,
        [Parameter(Mandatory=$true)][string]$LicensePath,
        [scriptblock]$ProtectBytes
    )
    Add-Type -AssemblyName System.Security
    $record = "$script:AmuProductId|$LicenseKey|$MachineHash"
    $plain = [Text.Encoding]::UTF8.GetBytes($record)
    if ($ProtectBytes) { [byte[]]$encrypted = & $ProtectBytes $plain }
    else { $encrypted = [Security.Cryptography.ProtectedData]::Protect($plain, $null, [Security.Cryptography.DataProtectionScope]::CurrentUser) }
    $parent = Split-Path -Parent $LicensePath
    if (!(Test-Path -LiteralPath $parent)) { New-Item -ItemType Directory -Path $parent -Force | Out-Null }
    [IO.File]::WriteAllText($LicensePath, [Convert]::ToBase64String($encrypted), [Text.Encoding]::ASCII)
}

function Test-AmuLocalLicense {
    param(
        [string]$LicensePath = (Get-AmuDefaultLicensePath),
        [string]$MachineHash = (Get-AmuMachineHash),
        [scriptblock]$UnprotectBytes
    )
    try {
        if (!(Test-Path -LiteralPath $LicensePath) -or [string]::IsNullOrWhiteSpace($MachineHash)) { return $false }
        Add-Type -AssemblyName System.Security
        $encoded = [IO.File]::ReadAllText($LicensePath).Trim()
        $encrypted = [Convert]::FromBase64String($encoded)
        if ($UnprotectBytes) { [byte[]]$plain = & $UnprotectBytes $encrypted }
        else { $plain = [Security.Cryptography.ProtectedData]::Unprotect($encrypted, $null, [Security.Cryptography.DataProtectionScope]::CurrentUser) }
        $parts = [Text.Encoding]::UTF8.GetString($plain).Split('|')
        return $parts.Count -eq 3 -and $parts[0] -eq $script:AmuProductId -and ![string]::IsNullOrWhiteSpace($parts[1]) -and $parts[2] -eq $MachineHash
    } catch { return $false }
}

function Invoke-AmuActivation {
    param(
        [Parameter(Mandatory=$true)][string]$LicenseKey,
        [string]$MachineHash = (Get-AmuMachineHash),
        [string]$LicensePath = (Get-AmuDefaultLicensePath),
        [scriptblock]$Transport,
        [scriptblock]$ProtectBytes,
        [scriptblock]$UnprotectBytes
    )
    $key = $LicenseKey.Trim().ToUpperInvariant()
    if ([string]::IsNullOrWhiteSpace($key)) { throw '请输入授权码。' }
    $body = [ordered]@{ licenseKey = $key; productId = $script:AmuProductId; machineHash = $MachineHash }
    try {
        if ($Transport) { $response = & $Transport $script:AmuActivationUrl $body }
        else {
            $json = $body | ConvertTo-Json -Compress
            $response = Invoke-RestMethod -Uri $script:AmuActivationUrl -Method Post -ContentType 'application/json; charset=utf-8' -Body ([Text.Encoding]::UTF8.GetBytes($json)) -TimeoutSec 20
        }
    } catch {
        throw ('无法连接授权服务器：' + $_.Exception.Message)
    }
    if (!$response.valid) { throw $(if ($response.message) { [string]$response.message } else { '授权码无效。' }) }
    $license = $response.license
    if (!$license -or [string]$license.licenseKey -ne $key -or [string]$license.productId -ne $script:AmuProductId -or [string]$license.machineHash -ne $MachineHash) {
        throw '授权服务器返回的数据与本机或本产品不匹配。'
    }
    Protect-AmuLicenseRecord -LicenseKey $key -MachineHash $MachineHash -LicensePath $LicensePath -ProtectBytes $ProtectBytes
    if (!(Test-AmuLocalLicense -LicensePath $LicensePath -MachineHash $MachineHash -UnprotectBytes $UnprotectBytes)) { throw '本机授权文件写入后校验失败。' }
    return $true
}

function Ensure-AmuLicense {
    param(
        [string]$LicensePath = (Get-AmuDefaultLicensePath),
        [string]$MachineHash = (Get-AmuMachineHash),
        [switch]$Quiet,
        [scriptblock]$Transport,
        [scriptblock]$ProtectBytes,
        [scriptblock]$UnprotectBytes
    )
    if (Test-AmuLocalLicense -LicensePath $LicensePath -MachineHash $MachineHash -UnprotectBytes $UnprotectBytes) { return $true }
    if ($Quiet) { throw '本机尚未激活，请重新运行安装并输入授权码。' }
    Add-Type -AssemblyName Microsoft.VisualBasic
    $key = [Microsoft.VisualBasic.Interaction]::InputBox(
        "请输入【钢琴自动延音踏板】的授权码。`r`n产品编号：$script:AmuProductId`r`n授权成功后，今后同一台电脑升级无需换码。",
        '钢琴自动延音踏板 1.0 授权',
        ''
    )
    if ([string]::IsNullOrWhiteSpace($key)) { throw '已取消授权，软件未安装。' }
    return Invoke-AmuActivation -LicenseKey $key -MachineHash $MachineHash -LicensePath $LicensePath -Transport $Transport -ProtectBytes $ProtectBytes -UnprotectBytes $UnprotectBytes
}
