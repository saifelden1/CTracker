# CTracker Build and Run unified script

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "CTracker Build and Run" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""

# 1. Build the application
Write-Host "Building CTracker..." -ForegroundColor Cyan
cmake -B CTracker/build -S CTracker
if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake configuration failed!" -ForegroundColor Red
    exit $LASTEXITCODE
}

cmake --build CTracker/build
if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    exit $LASTEXITCODE
}
Write-Host "Build completed successfully." -ForegroundColor Green
Write-Host ""

# 2. Extract Qt deploy function
Write-Host "Deploying Qt DLLs..." -ForegroundColor Cyan
$qtPaths = @(
    "$env:QTDIR",
    "E:\Qt\6.8.0\msvc2022_64",
    "E:\Qt\6.7.0\msvc2022_64",
    "E:\Qt\6.8.0\mingw_64",
    "E:\Qt\6.7.0\mingw_64",
    "C:\Qt\6.8.0\msvc2022_64",
    "C:\Qt\6.7.0\msvc2022_64",
    "C:\Qt\6.8.0\mingw_64",
    "C:\Qt\6.7.0\mingw_64",
    "$env:ProgramFiles\Qt\6.8.0\msvc2022_64"
)

$qtDir = $null
foreach ($path in $qtPaths) {
    if ($path -and (Test-Path "$path\bin\Qt6Core.dll")) {
        $qtDir = $path
        Write-Host "Found Qt at: $qtDir" -ForegroundColor Green
        break
    }
}

if ($qtDir) {
    $windeployqt = "$qtDir\bin\windeployqt.exe"
    if (Test-Path $windeployqt) {
        Write-Host "Running windeployqt..." -ForegroundColor Yellow
        & $windeployqt "CTracker\build\CTracker.exe" --no-translations --no-system-d3d-compiler --no-opengl-sw
        
        $platformDir = "CTracker\build\platforms"
        if (-not (Test-Path $platformDir)) {
            New-Item -ItemType Directory -Path $platformDir | Out-Null
        }
        foreach ($platformPlugin in @("qoffscreen.dll", "qminimal.dll")) {
            $source = "$qtDir\plugins\platforms\$platformPlugin"
            if (Test-Path $source) {
                Copy-Item $source "$platformDir\$platformPlugin" -Force
            }
        }
    } else {
        Write-Host "Skipping auto-deployment as windeployqt wasn't found." -ForegroundColor Yellow
    }
} else {
    Write-Host "Qt installation not found for DLL deployment. You might need to manually set it up if the app fails to start." -ForegroundColor Yellow
}
Write-Host ""

# 3. Launch application
Write-Host "Starting CTracker..." -ForegroundColor Green
$exePath = "CTracker/build/CTracker.exe"
if (Test-Path $exePath) {
    Start-Process $exePath
    Write-Host "CTracker launched successfully!" -ForegroundColor Green
} else {
    Write-Host "Error: CTracker executable not found at $exePath" -ForegroundColor Red
}