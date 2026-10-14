$ErrorActionPreference = 'Stop'
$scriptPath = Join-Path (Split-Path $PSScriptRoot -Parent) 'scripts\License-Activation.ps1'
. $scriptPath

function Assert-True([bool]$Condition, [string]$Message) {
    if (!$Condition) { throw "ASSERT FAILED: $Message" }
}

$root = Join-Path $env:TEMP ('pas-license-test-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $root | Out-Null
try {
    $guid = '12345678-1234-1234-1234-123456789ABC'
    $bytes = [Text.Encoding]::UTF8.GetBytes("AMU_PIANO_AUTO_SUSTAIN|$guid")
    $expected = -join ([Security.Cryptography.SHA256]::Create().ComputeHash($bytes) | ForEach-Object { $_.ToString('X2') })
    $machine = Get-AmuMachineHash -MachineGuidOverride $guid
    Assert-True ($machine -eq $expected) 'machine hash must be uppercase SHA-256 of PRODUCT_ID|MachineGuid'

    $licensePath = Join-Path $root 'license.dat'
    $key = 'LC-TEST-ABCD-EFGH-IJKL'
    # The CI/agent token has no loaded Windows user profile, so DPAPI itself
    # cannot run here. Inject reversible byte transforms while exercising the
    # same save/reload/validation paths used by production.
    $protect = { param([byte[]]$Bytes) [byte[]]($Bytes | ForEach-Object { $_ -bxor 0xA5 }) }
    $unprotect = { param([byte[]]$Bytes) [byte[]]($Bytes | ForEach-Object { $_ -bxor 0xA5 }) }
    $transport = {
        param($Uri, $Body)
        [pscustomobject]@{
            valid = $true
            message = '激活成功'
            license = [pscustomobject]@{
                licenseKey = $Body.licenseKey
                productId = $Body.productId
                machineHash = $Body.machineHash
            }
        }
    }
    Invoke-AmuActivation -LicenseKey $key -MachineHash $machine -LicensePath $licensePath -Transport $transport -ProtectBytes $protect -UnprotectBytes $unprotect | Out-Null
    Assert-True (Test-Path -LiteralPath $licensePath) 'successful activation must save a local license'
    $stored = Get-Content -Raw -LiteralPath $licensePath
    Assert-True (!$stored.Contains($key)) 'saved license must not expose the activation key in plaintext'
    Assert-True (Test-AmuLocalLicense -LicensePath $licensePath -MachineHash $machine -UnprotectBytes $unprotect) 'saved license must reload on the same machine'
    Assert-True (!(Test-AmuLocalLicense -LicensePath $licensePath -MachineHash ('0' * 64) -UnprotectBytes $unprotect)) 'copied license must fail on another machine'

    $badPath = Join-Path $root 'bad.dat'
    $badTransport = {
        param($Uri, $Body)
        [pscustomobject]@{
            valid = $true
            license = [pscustomobject]@{
                licenseKey = $Body.licenseKey
                productId = 'ANOTHER_PRODUCT'
                machineHash = $Body.machineHash
            }
        }
    }
    $threw = $false
    try { Invoke-AmuActivation -LicenseKey $key -MachineHash $machine -LicensePath $badPath -Transport $badTransport -ProtectBytes $protect -UnprotectBytes $unprotect | Out-Null } catch { $threw = $true }
    Assert-True $threw 'mismatched server response must be rejected'
    Assert-True (!(Test-Path -LiteralPath $badPath)) 'mismatched server response must not create a license file'

    $calls = 0
    $unusedTransport = { param($Uri, $Body) $script:calls++; throw 'transport should not be called' }
    $reused = Ensure-AmuLicense -LicensePath $licensePath -MachineHash $machine -Quiet -Transport $unusedTransport -ProtectBytes $protect -UnprotectBytes $unprotect
    Assert-True $reused 'upgrade must reuse an existing valid license'
    Assert-True ($calls -eq 0) 'upgrade reuse must not call the activation service'

    $installer = Get-Content -Raw -LiteralPath (Join-Path (Split-Path $PSScriptRoot -Parent) 'scripts\Install-Cubase.ps1')
    Assert-True ($installer.Contains('License-Activation.ps1')) 'installer must load the activation helper'
    Assert-True ($installer.Contains('Ensure-AmuLicense')) 'installer must require authorization'
    Assert-True ($installer.IndexOf('Ensure-AmuLicense') -lt $installer.IndexOf('Copy-Item -LiteralPath $source')) 'authorization must happen before installing the DLL'

    $vstInstaller = Get-Content -Raw -LiteralPath (Join-Path (Split-Path $PSScriptRoot -Parent) 'scripts\Install-VST3.ps1')
    Assert-True ($vstInstaller.Contains('License-Activation.ps1')) 'VST3 installer must load the activation helper'
    Assert-True ($vstInstaller.Contains('Ensure-AmuLicense')) 'VST3 installer must require authorization'
    Assert-True ($vstInstaller.IndexOf('Ensure-AmuLicense') -lt $vstInstaller.IndexOf('Copy-Item -LiteralPath $source')) 'VST3 authorization must happen before installing files'

    Write-Output 'PASS: machine hash, encrypted save/reload, response validation, upgrade reuse'
} finally {
    if (Test-Path -LiteralPath $root) { Remove-Item -LiteralPath $root -Recurse -Force }
}
