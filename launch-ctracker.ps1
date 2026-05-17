# Launch CTracker Application

Write-Host "Launching CTracker..." -ForegroundColor Cyan

if (-not (Test-Path "CTracker/build/CTracker.exe")) {
    Write-Host "Error: CTracker.exe not found!" -ForegroundColor Red
    Write-Host "Please build the project first:" -ForegroundColor Yellow
    Write-Host "  cmake -B CTracker/build -S CTracker" -ForegroundColor White
    Write-Host "  cmake --build CTracker/build" -ForegroundColor White
    exit 1
}

# Check if Qt DLLs are present
if (-not (Test-Path "CTracker/build/Qt6Core.dll")) {
    Write-Host ""
    Write-Host "Warning: Qt DLLs not found in build directory!" -ForegroundColor Yellow
    Write-Host "The application may fail to start." -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Run this first to deploy Qt DLLs:" -ForegroundColor Cyan
    Write-Host "  .\deploy-qt-dlls.ps1" -ForegroundColor White
    Write-Host ""
    $response = Read-Host "Continue anyway? (y/n)"
    if ($response -ne "y") {
        exit 0
    }
}

# Launch the application
Write-Host ""
Write-Host "Starting CTracker..." -ForegroundColor Green
Start-Process "CTracker/build/CTracker.exe"

Write-Host "CTracker launched!" -ForegroundColor Green
Write-Host ""
Write-Host "Database location: $env:LOCALAPPDATA\CTracker\ctracker.db" -ForegroundColor Gray
Write-Host ""
Write-Host "If the app doesn't start, run: .\deploy-qt-dlls.ps1" -ForegroundColor Yellow
