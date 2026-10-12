#Requires -RunAsAdministrator
param(
    # Only needed if a locally trusted certificate is not enough (e.g. future kernel-mode components).
    [switch]$EnableTestSigning
)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'certificate.ps1')

function New-SigningCertificate {
    New-SelfSignedCertificate -Type CodeSigningCert -Subject $CertSubject `
        -CertStoreLocation Cert:\CurrentUser\My -KeyExportPolicy Exportable `
        -HashAlgorithm sha256 -NotAfter (Get-Date).AddYears(10)
}

function Grant-MachineTrust($Certificate) {
    New-Item -ItemType Directory -Force $CertsDir | Out-Null
    Export-Certificate -Cert $Certificate -FilePath $CertFile -Force | Out-Null
    foreach ($store in 'Root', 'TrustedPublisher') {
        Import-Certificate -FilePath $CertFile -CertStoreLocation "Cert:\LocalMachine\$store" | Out-Null
    }
}

function Test-SecureBootEnabled {
    try { Confirm-SecureBootUEFI } catch { $false }
}

function Enable-TestSigning {
    if (Test-SecureBootEnabled) {
        throw 'Secure Boot is enabled, so test signing cannot be turned on. Disable Secure Boot in UEFI setup and run this script again.'
    }
    bcdedit /set testsigning on
    if ($LASTEXITCODE -ne 0) { throw "bcdedit failed with exit code $LASTEXITCODE" }
}

$cert = Get-SigningCertificate
if (-not $cert) { $cert = New-SigningCertificate }
Grant-MachineTrust $cert
Write-Host "Signing certificate: $($cert.Subject) [$($cert.Thumbprint)], exported to $CertFile"

if ($EnableTestSigning) {
    Enable-TestSigning
    Write-Host 'Test signing is enabled. Reboot the computer before the next deploy.'
}
