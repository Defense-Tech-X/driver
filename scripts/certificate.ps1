. (Join-Path $PSScriptRoot 'config.ps1')

function Get-SigningCertificate {
    Get-ChildItem Cert:\CurrentUser\My |
        Where-Object { $_.Subject -eq $CertSubject -and $_.HasPrivateKey } |
        Sort-Object NotAfter -Descending |
        Select-Object -First 1
}
