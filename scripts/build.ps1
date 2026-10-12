param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',
    [switch]$Rebuild
)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'ewdk.ps1')
. (Join-Path $PSScriptRoot 'certificate.ps1')

function Get-SigningThumbprint {
    $cert = Get-SigningCertificate
    if (-not $cert) { throw "Signing certificate '$CertSubject' not found. Run driver\scripts\setup-machine.ps1 first." }
    $cert.Thumbprint
}

function Invoke-DriverBuild {
    $target = if ($Rebuild) { 'Rebuild' } else { 'Build' }
    Invoke-EwdkCommand "msbuild `"$DriverProj`" -t:$target -p:Configuration=$Configuration -p:Platform=$Platform -nologo -v:minimal"
}

function Invoke-CatalogSigning([string]$Thumbprint) {
    $catalog = Join-Path (Get-PackageDir $Configuration) "$DriverName.cat"
    if ((Get-AuthenticodeSignature $catalog).SignerCertificate.Thumbprint -eq $Thumbprint) { return }
    & $SignToolExe sign /q /fd sha256 /sha1 $Thumbprint $catalog
    if ($LASTEXITCODE -ne 0) { throw "signtool failed with exit code $LASTEXITCODE" }
    Write-Host "Signed $catalog"
}

$thumbprint = Get-SigningThumbprint
Invoke-DriverBuild
Invoke-CatalogSigning $thumbprint
Write-Host "Driver package: $(Get-PackageDir $Configuration)"
