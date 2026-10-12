#Requires -RunAsAdministrator
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

$cert = Get-SigningCertificate
if (-not $cert) { $cert = New-SigningCertificate }
Grant-MachineTrust $cert
Write-Host "Signing certificate: $($cert.Subject) [$($cert.Thumbprint)], exported to $CertFile"
