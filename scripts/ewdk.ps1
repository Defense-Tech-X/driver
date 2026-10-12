. (Join-Path $PSScriptRoot 'config.ps1')

function Assert-EwdkMounted {
    $setup = Join-Path $EwdkRoot 'BuildEnv\SetupBuildEnv.cmd'
    if (-not (Test-Path $setup)) {
        throw "EWDK not found at '$EwdkRoot'. Mount the ISO from '$ProjectRoot\ewdk' and set `$EwdkRoot in driver\scripts\config.ps1."
    }
    $setup
}

function Invoke-EwdkCommand([string]$Command) {
    $setup = Assert-EwdkMounted
    cmd /c "call `"$setup`" amd64 >nul && $Command"
    if ($LASTEXITCODE -ne 0) {
        throw "EWDK command failed with exit code ${LASTEXITCODE}: $Command"
    }
}
