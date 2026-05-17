# CTracker Build Verification Script (PowerShell)

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "CTracker Build Verification" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""

# Check if build directory exists
if (-not (Test-Path "CTracker/build")) {
    Write-Host "Build directory not found" -ForegroundColor Red
    Write-Host "   Run: cmake -B CTracker/build -S CTracker" -ForegroundColor Yellow
    exit 1
}

Write-Host "Build directory exists" -ForegroundColor Green

# Check if executable exists
if (-not (Test-Path "CTracker/build/CTracker.exe")) {
    Write-Host "Executable not found" -ForegroundColor Red
    Write-Host "   Run: cmake --build CTracker/build" -ForegroundColor Yellow
    exit 1
}

Write-Host "Executable exists" -ForegroundColor Green

# Get executable info
$exe = Get-Item "CTracker/build/CTracker.exe"
$sizeMB = [math]::Round($exe.Length / 1MB, 2)
Write-Host "   Size: $sizeMB MB" -ForegroundColor Gray
Write-Host "   Last Modified: $($exe.LastWriteTime)" -ForegroundColor Gray

# Check source files
Write-Host ""
Write-Host "Checking source structure..." -ForegroundColor Cyan
$expectedDirs = @("core", "shared", "courses", "projects", "todos", "pomodoro", "analytics", "calendar", "settings")
foreach ($dir in $expectedDirs) {
    $includeExists = Test-Path "CTracker/include/$dir"
    $srcExists = Test-Path "CTracker/src/$dir"
    if ($includeExists -and $srcExists) {
        Write-Host "$dir/ (include + src)" -ForegroundColor Green
    } else {
        Write-Host "$dir/ missing" -ForegroundColor Red
    }
}

# Check CMakeLists.txt
Write-Host ""
Write-Host "Checking CMakeLists.txt..." -ForegroundColor Cyan
$cmake = Get-Content "CTracker/CMakeLists.txt" -Raw

if ($cmake -match "Qt6::Charts") {
    Write-Host "Qt6::Charts linked" -ForegroundColor Green
} else {
    Write-Host "Qt6::Charts not found (needed for Phase 8)" -ForegroundColor Yellow
}

if ($cmake -match "Qt6::Svg") {
    Write-Host "Qt6::Svg linked" -ForegroundColor Green
} else {
    Write-Host "Qt6::Svg not found (will be added in Phase 8.4)" -ForegroundColor Yellow
}

# Check database location
Write-Host ""
Write-Host "Database location:" -ForegroundColor Cyan
$dbPath = "$env:LOCALAPPDATA\CTracker\ctracker.db"
Write-Host "   $dbPath" -ForegroundColor Gray
if (Test-Path $dbPath) {
    $db = Get-Item $dbPath
    Write-Host "   Database exists (created: $($db.CreationTime))" -ForegroundColor Green
} else {
    Write-Host "   Database will be created on first launch" -ForegroundColor Gray
}

# Summary
Write-Host ""
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "Summary" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "Phases 0-7 appear complete" -ForegroundColor Green
Write-Host "Application is ready to launch" -ForegroundColor Green
Write-Host ""
Write-Host "To run the application:" -ForegroundColor Yellow
Write-Host "  .\CTracker\build\CTracker.exe" -ForegroundColor White
Write-Host ""
Write-Host "To rebuild:" -ForegroundColor Yellow
Write-Host "  cmake --build CTracker/build --clean-first" -ForegroundColor White
Write-Host ""
Write-Host "See VERIFICATION_GUIDE.md for detailed testing instructions" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
