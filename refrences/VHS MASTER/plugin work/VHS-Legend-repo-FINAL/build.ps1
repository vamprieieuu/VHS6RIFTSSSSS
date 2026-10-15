param(
  [string]$SdkRoot = $env:AE_SDK_ROOT
)

$ErrorActionPreference = "Stop"

if (-not $SdkRoot) {
  throw "Provide AE_SDK_ROOT, e.g. .\build.ps1 -SdkRoot 'D:\AfterEffectsSDK'"
}

cmake -S . -B build -G "Visual Studio 17 2022" -A x64 "-DAE_SDK_ROOT=$SdkRoot"
cmake --build build --config Release --parallel

Write-Host ""
Write-Host "Generated AEX files:"
Get-ChildItem -Path build -Recurse -Filter *.aex | Select-Object FullName
