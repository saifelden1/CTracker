# Deploy Qt DLLs to CTracker build directory

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "Qt DLL Deployment Script" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""

# Find Qt installation
Write-Host "Searching for Qt installation..." -ForegroundColor Yellow

# Common Qt installation paths - check E: drive first, then C:
$qtPaths = @(
    "$env:QTDIR",
    # E: drive paths (user's Qt location)
    "E:\Qt\6.8.0\msvc2022_64",
    "E:\Qt\6.7.0\msvc2022_64",
    "E:\Qt\6.6.0\msvc2022_64",
    "E:\Qt\6.5.0\msvc2022_64",
    "E:\Qt\6.4.0\msvc2022_64",
    "E:\Qt\6.8.0\mingw_64",
    "E:\Qt\6.7.0\mingw_64",
    "E:\Qt\6.6.0\mingw_64",
    # C: drive paths
    "C:\Qt\6.8.0\msvc2022_64",
    "C:\Qt\6.7.0\msvc2022_64",
    "C:\Qt\6.6.0\msvc2022_64",
    "C:\Qt\6.5.0\msvc2022_64",
    "C:\Qt\6.4.0\msvc2022_64",
    "C:\Qt\6.8.0\mingw_64",
    "C:\Qt\6.7.0\mingw_64",
    "C:\Qt\6.6.0\mingw_64",
    "$env:ProgramFiles\Qt\6.8.0\msvc2022_64",
    "$env:ProgramFiles\Qt\6.7.0\msvc2022_64"
)

$qtDir = $null
foreach ($path in $qtPaths) {
    if ($path -and (Test-Path "$path\bin\Qt6Core.dll")) {
        $qtDir = $path
        Write-Host "Found Qt at: $qtDir" -ForegroundColor Green
        break
    }
}

# If not found in common paths, scan E: and C: drives
if (-not $qtDir) {
    Write-Host "Not found in common paths. Scanning E: and C: drives..." -ForegroundColor Yellow
    
    $drivesToScan = @("E:", "C:")
    foreach ($drive in $drivesToScan) {
        if (Test-Path $drive) {
            Write-Host "  Scanning $drive..." -ForegroundColor Gray
            
            # Look for Qt folder
            $qtFolders = Get-ChildItem -Path "$drive\" -Directory -Filter "Qt*" -ErrorAction SilentlyContinue
            foreach ($qtFolder in $qtFolders) {
                # Look for version folders
                $versionFolders = Get-ChildItem -Path $qtFolder.FullName -Directory -ErrorAction SilentlyContinue | Where-Object { $_.Name -match '^\d+\.\d+\.\d+$' }
                foreach ($versionFolder in $versionFolders) {
                    # Look for compiler folders
                    $compilerFolders = Get-ChildItem -Path $versionFolder.FullName -Directory -ErrorAction SilentlyContinue | Where-Object { $_.Name -match 'msvc|mingw' }
                    foreach ($compilerFolder in $compilerFolders) {
                        if (Test-Path "$($compilerFolder.FullName)\bin\Qt6Core.dll") {
                            $qtDir = $compilerFolder.FullName
                            Write-Host "  Found Qt at: $qtDir" -ForegroundColor Green
                            break
                        }
                    }
                    if ($qtDir) { break }
                }
                if ($qtDir) { break }
            }
            if ($qtDir) { break }
        }
    }
}

if (-not $qtDir) {
    Write-Host "Qt installation not found!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Please specify your Qt installation path:" -ForegroundColor Yellow
    Write-Host "Example: C:\Qt\6.8.0\msvc2022_64" -ForegroundColor Gray
    Write-Host ""
    $qtDir = Read-Host "Enter Qt path"
    
    if (-not (Test-Path "$qtDir\bin\Qt6Core.dll")) {
        Write-Host "Invalid Qt path!" -ForegroundColor Red
        exit 1
    }
}

# Check if windeployqt exists
$windeployqt = "$qtDir\bin\windeployqt.exe"
if (Test-Path $windeployqt) {
    Write-Host ""
    Write-Host "Using windeployqt (automatic deployment)..." -ForegroundColor Cyan
    
    # Run windeployqt
    & $windeployqt "CTracker\build\CTracker.exe" --no-translations --no-system-d3d-compiler --no-opengl-sw
    
    Write-Host ""
    Write-Host "Deployment complete!" -ForegroundColor Green
    Write-Host "Try launching: .\CTracker\build\CTracker.exe" -ForegroundColor Yellow
} else {
    Write-Host ""
    Write-Host "windeployqt not found. Using manual DLL copy..." -ForegroundColor Yellow
    
    # Required Qt DLLs
    $requiredDlls = @(
        "Qt6Core.dll",
        "Qt6Gui.dll",
        "Qt6Widgets.dll",
        "Qt6Sql.dll",
        "Qt6Charts.dll"
    )
    
    # Copy DLLs
    Write-Host ""
    Write-Host "Copying Qt DLLs..." -ForegroundColor Cyan
    foreach ($dll in $requiredDlls) {
        $source = "$qtDir\bin\$dll"
        $dest = "CTracker\build\$dll"
        
        if (Test-Path $source) {
            Copy-Item $source $dest -Force
            Write-Host "  Copied: $dll" -ForegroundColor Green
        } else {
            Write-Host "  Warning: $dll not found" -ForegroundColor Yellow
        }
    }
    
    # Copy platform plugin
    Write-Host ""
    Write-Host "Copying platform plugins..." -ForegroundColor Cyan
    $platformDir = "CTracker\build\platforms"
    if (-not (Test-Path $platformDir)) {
        New-Item -ItemType Directory -Path $platformDir | Out-Null
    }
    
    $qwindows = "$qtDir\plugins\platforms\qwindows.dll"
    if (Test-Path $qwindows) {
        Copy-Item $qwindows "$platformDir\qwindows.dll" -Force
        Write-Host "  Copied: qwindows.dll" -ForegroundColor Green
    }
    
    # Copy SQL drivers
    Write-Host ""
    Write-Host "Copying SQL drivers..." -ForegroundColor Cyan
    $sqlDriverDir = "CTracker\build\sqldrivers"
    if (-not (Test-Path $sqlDriverDir)) {
        New-Item -ItemType Directory -Path $sqlDriverDir | Out-Null
    }
    
    $qsqlite = "$qtDir\plugins\sqldrivers\qsqlite.dll"
    if (Test-Path $qsqlite) {
        Copy-Item $qsqlite "$sqlDriverDir\qsqlite.dll" -Force
        Write-Host "  Copied: qsqlite.dll" -ForegroundColor Green
    }
    
    Write-Host ""
    Write-Host "Manual deployment complete!" -ForegroundColor Green
    Write-Host "Try launching: .\CTracker\build\CTracker.exe" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "Deployment Summary" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "Qt Path: $qtDir" -ForegroundColor Gray
Write-Host "Build Dir: CTracker\build" -ForegroundColor Gray
Write-Host ""
Write-Host "To launch the app:" -ForegroundColor Yellow
Write-Host "  .\CTracker\build\CTracker.exe" -ForegroundColor White
Write-Host ""
