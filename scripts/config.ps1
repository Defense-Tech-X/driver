$DriverName = 'VirtualDroneController'
$HardwareId = "root\$DriverName"
$Platform   = 'x64'

# Must match VDC_VENDOR_ID / VDC_PRODUCT_ID in driver\src\controller_protocol.h.
$HidVidPid  = 'VID_DEED&PID_FEED'

$EwdkRoot     = 'E:\'
$EwdkSdkVer   = '10.0.28000.0'
$EwdkKitsRoot = Join-Path $EwdkRoot 'Program Files\Windows Kits\10'
$DevconExe    = Join-Path $EwdkKitsRoot "Tools\$EwdkSdkVer\x64\devcon.exe"
$SignToolExe  = Join-Path $EwdkKitsRoot "bin\$EwdkSdkVer\x64\signtool.exe"

$DriverRoot  = Split-Path -Parent $PSScriptRoot
$ProjectRoot = Split-Path -Parent $DriverRoot
$DriverProj  = Join-Path $DriverRoot "src\$DriverName.vcxproj"
$CertsDir    = Join-Path $DriverRoot 'certs'
$CertFile    = Join-Path $CertsDir "$DriverName.cer"
$CertSubject = "CN=$DriverName Test Signing"

function Get-PackageDir([string]$Configuration) {
    Join-Path $DriverRoot "out\$Platform\$Configuration\$DriverName"
}
