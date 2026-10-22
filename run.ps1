<#

Build & Run file for Dense Engine

Requirements:

- Windows 10 (version 1809) or later
- GPU with DirectX Raytracing support (DXR Tier 1.0, feature level 12_1)
- PowerShell
- CMake >= 3.25
- Microsoft Visual Studio 2022 (C++ workload)
- dxc (DirectX Shader Compiler)

Setup:

- Clone with submodules: git clone --recursive https://github.com/EmilDimov93/Dense-Engine.git

Usage:

- Open PowerShell in the repo root
- Run: ./run.ps1

#>

$config = "Release"

if (-not (Test-Path -Path "build/CMakeCache.txt")) {
    Write-Host "Build folder not configured. Configuring..." -ForegroundColor Cyan

    cmake -B build -S . -A x64
    if ($LASTEXITCODE -ne 0) { exit 1 }
}

Write-Host "Building..." -ForegroundColor Blue

cmake --build build --config $config
if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed" -ForegroundColor Red
    exit 1
}

Write-Host "Running..." -ForegroundColor Green

Push-Location "build/$config"
try {
    ./DenseEngine.exe
}
finally {
    Pop-Location
}

Write-Host "Finished" -ForegroundColor Magenta