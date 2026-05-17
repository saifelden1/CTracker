# ============================================================
#  run-tests.ps1 - CTracker Test Runner
#
#  Builds and runs all CTracker tests using CTest.
#  Usage:
#    .\run-tests.ps1
#    .\run-tests.ps1 -Verbose
#    .\run-tests.ps1 -TestName test_databasemanager_v1
# ============================================================

param(
    [string]$BuildDir = "build",
    [string]$Config = "Release",
    [switch]$Verbose,
    [string]$TestName = ""
)

$ErrorActionPreference = "Stop"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  CTracker Test Suite" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

if (-not (Test-Path $BuildDir)) {
    Write-Host "Error: Build directory '$BuildDir' not found!" -ForegroundColor Red
    Write-Host "Please run cmake to configure the project first." -ForegroundColor Yellow
    exit 1
}

Push-Location $BuildDir

try {
    Write-Host "[1/3] Building tests..." -ForegroundColor Green
    $buildArgs = @("--build", ".", "--config", $Config, "--target", "all")

    $buildOutput = & cmake @buildArgs 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Build failed!" -ForegroundColor Red
        Write-Host $buildOutput
        exit 1
    }

    Write-Host "Build successful!" -ForegroundColor Green
    Write-Host ""

    Write-Host "[2/3] Running tests..." -ForegroundColor Green
    Write-Host ""

    $ctestArgs = @("-C", $Config, "--output-on-failure")

    if ($Verbose) {
        $ctestArgs += "--verbose"
    }

    if ($TestName -ne "") {
        Write-Host "Running specific test: $TestName" -ForegroundColor Yellow
        $ctestArgs += @("-R", $TestName)
    }

    & ctest @ctestArgs
    $testResult = $LASTEXITCODE

    Write-Host ""
    Write-Host "[3/3] Test Summary" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Cyan

    if ($testResult -eq 0) {
        Write-Host "OK: All tests passed!" -ForegroundColor Green
    } else {
        Write-Host "FAIL: Some tests failed!" -ForegroundColor Red
        Write-Host ""
        Write-Host "To see detailed output, run:" -ForegroundColor Yellow
        Write-Host "  .\run-tests.ps1 -Verbose" -ForegroundColor Yellow
        Write-Host ""
        Write-Host "To run a specific test:" -ForegroundColor Yellow
        Write-Host "  .\run-tests.ps1 -TestName test_databasemanager_v1" -ForegroundColor Yellow
    }

    Write-Host "========================================" -ForegroundColor Cyan

    exit $testResult
} finally {
    Pop-Location
}
