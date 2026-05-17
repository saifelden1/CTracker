# CTracker Quick Start Guide

## Problem: "Qt6Gui.dll was not found"

This is a **Qt deployment issue**. The Qt DLLs need to be copied to your build directory.

---

## Solution: Deploy Qt DLLs

### Step 1: Run the Deployment Script

```powershell
.\deploy-qt-dlls.ps1
```

**What this does:**
- Automatically finds your Qt installation
- Copies all required Qt DLLs to `CTracker/build/`
- Copies platform plugins (qwindows.dll)
- Copies SQL drivers (qsqlite.dll)

### Step 2: Launch the Application

```powershell
.\launch-ctracker.ps1
```

Or directly:
```powershell
.\CTracker\build\CTracker.exe
```

---

## If Deployment Script Can't Find Qt

The script will ask you to enter your Qt path manually.

### Find Your Qt Installation:

**Common locations:**
- `C:\Qt\6.8.0\msvc2022_64`
- `C:\Qt\6.7.0\msvc2022_64`
- `C:\Qt\6.6.0\mingw_64`

**How to find it:**
1. Open Qt Creator
2. Go to: Tools → Options → Kits
3. Look at the "Qt version" path
4. The path should end with something like `msvc2022_64` or `mingw_64`

**Example:**
```
If Qt Creator shows: C:\Qt\6.8.0\msvc2022_64\bin\qmake.exe
Your Qt path is:     C:\Qt\6.8.0\msvc2022_64
```

---

## Alternative: Add Qt to System PATH

If you prefer, you can add Qt's bin directory to your system PATH:

1. Open System Properties → Environment Variables
2. Edit the `Path` variable
3. Add: `C:\Qt\6.8.0\msvc2022_64\bin` (adjust to your Qt version)
4. Restart PowerShell
5. Run: `.\CTracker\build\CTracker.exe`

---

## What Gets Deployed

### Required DLLs:
- `Qt6Core.dll` - Core Qt functionality
- `Qt6Gui.dll` - GUI components
- `Qt6Widgets.dll` - Widget toolkit
- `Qt6Sql.dll` - Database support
- `Qt6Charts.dll` - Charts for analytics

### Required Plugins:
- `platforms/qwindows.dll` - Windows platform integration
- `sqldrivers/qsqlite.dll` - SQLite database driver

### Optional (for full deployment):
- `styles/` - Additional widget styles
- `imageformats/` - Image format plugins
- `iconengines/` - Icon rendering

---

## Troubleshooting

### Error: "Cannot find Qt6Core.dll"
**Solution:** Run `.\deploy-qt-dlls.ps1`

### Error: "This application failed to start because no Qt platform plugin could be initialized"
**Solution:** The `platforms/qwindows.dll` is missing
```powershell
# Manually copy it:
Copy-Item "C:\Qt\6.8.0\msvc2022_64\plugins\platforms\qwindows.dll" "CTracker\build\platforms\"
```

### Error: "Driver not loaded" (SQLite)
**Solution:** The SQL driver is missing
```powershell
# Manually copy it:
Copy-Item "C:\Qt\6.8.0\msvc2022_64\plugins\sqldrivers\qsqlite.dll" "CTracker\build\sqldrivers\"
```

### App launches but crashes immediately
**Check:**
1. Qt version matches your compiler (MSVC vs MinGW)
2. All DLLs are in the build directory
3. Run from PowerShell to see error messages:
   ```powershell
   cd CTracker\build
   .\CTracker.exe
   ```

### "VCRUNTIME140.dll not found"
**Solution:** Install Visual C++ Redistributable
- Download from: https://aka.ms/vs/17/release/vc_redist.x64.exe
- Install and restart

---

## Complete Workflow

### First Time Setup:
```powershell
# 1. Deploy Qt DLLs
.\deploy-qt-dlls.ps1

# 2. Launch the app
.\launch-ctracker.ps1
```

### After Rebuilding:
```powershell
# Rebuild
cmake --build CTracker/build --clean-first

# DLLs are already there, just launch
.\launch-ctracker.ps1
```

### Clean Deployment (if something breaks):
```powershell
# Remove all DLLs
Remove-Item CTracker\build\*.dll
Remove-Item CTracker\build\platforms -Recurse -Force
Remove-Item CTracker\build\sqldrivers -Recurse -Force

# Re-deploy
.\deploy-qt-dlls.ps1

# Launch
.\launch-ctracker.ps1
```

---

## Verification

After running `deploy-qt-dlls.ps1`, check that these files exist:

```powershell
# Check DLLs
Test-Path CTracker\build\Qt6Core.dll
Test-Path CTracker\build\Qt6Gui.dll
Test-Path CTracker\build\Qt6Widgets.dll
Test-Path CTracker\build\Qt6Sql.dll
Test-Path CTracker\build\Qt6Charts.dll

# Check plugins
Test-Path CTracker\build\platforms\qwindows.dll
Test-Path CTracker\build\sqldrivers\qsqlite.dll
```

All should return `True`.

---

## Next Steps

Once the app launches successfully:

1. **Test the UI** - See `VERIFICATION_GUIDE.md` for detailed testing
2. **Create sample data** - Add courses, projects, todos
3. **Explore features** - Try all 7 views
4. **Check the database** - Located at `%LOCALAPPDATA%\CTracker\ctracker.db`

---

## Summary

**The issue:** Windows needs Qt DLLs in the same directory as the executable.

**The fix:** Run `.\deploy-qt-dlls.ps1` once to copy all required files.

**Then:** Launch with `.\launch-ctracker.ps1` or directly run `.\CTracker\build\CTracker.exe`

**Need help?** Check `VERIFICATION_GUIDE.md` for detailed testing instructions.
