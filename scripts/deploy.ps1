#Requires -RunAsAdministrator
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug'
)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'config.ps1')

$RebootRequiredExitCodes = @{ devcon = 1; pnputil = 3010 }

function Invoke-Tool([string]$Name, [string]$Exe, [string[]]$Arguments) {
    & $Exe @Arguments
    if ($LASTEXITCODE -eq $RebootRequiredExitCodes[$Name]) {
        Write-Warning "$Name requests a reboot to complete: $($Arguments -join ' ')"
    } elseif ($LASTEXITCODE -ne 0) {
        throw "$Name failed with exit code ${LASTEXITCODE}: $($Arguments -join ' ')"
    }
}

function Get-DriverDevices {
    Get-PnpDevice | Where-Object { $_.HardwareID -contains $HardwareId }
}

function Get-DriverInfPath {
    $inf = Join-Path (Get-PackageDir $Configuration) "$DriverName.inf"
    if (-not (Test-Path $inf)) { throw "Driver package not found: $inf. Run driver\scripts\build.ps1 -Configuration $Configuration first." }
    $inf
}

function Remove-DriverDevices {
    foreach ($device in Get-DriverDevices) {
        Invoke-Tool pnputil pnputil.exe '/remove-device', $device.InstanceId
    }
}

function Remove-StagedDriverPackages {
    $staged = Get-ChildItem "$env:windir\INF\oem*.inf" | Select-String -SimpleMatch $HardwareId -List
    foreach ($oemInf in $staged) {
        Invoke-Tool pnputil pnputil.exe '/delete-driver', $oemInf.Filename, '/uninstall', '/force'
    }
}

function Reset-JoystickCalibration {
    $calibration = "HKCU:\System\CurrentControlSet\Control\MediaProperties\PrivateProperties\DirectInput\$HidVidPid\Calibration"
    if (Test-Path -LiteralPath $calibration) {
        Remove-Item -LiteralPath $calibration -Recurse
    }
}

function Install-DriverDevice([string]$InfPath) {
    Invoke-Tool devcon $DevconExe 'install', $InfPath, $HardwareId
}

$inf = Get-DriverInfPath
Remove-DriverDevices
Remove-StagedDriverPackages
Reset-JoystickCalibration
Install-DriverDevice $inf
Get-DriverDevices | Format-Table Status, Class, FriendlyName, InstanceId -AutoSize
