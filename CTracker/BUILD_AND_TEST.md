# Quick Start: Build and Test CTracker

## Step 1: Close Running Application
**IMPORTANT**: Close any running CTracker.exe instance to avoid "Permission denied" errors during build.

## Step 2: Build the Project

```powershell
cd C:\Users\01226\Desktop\MySelf\CTracker\build
cmake --build . --config Release
```

## Step 3: Run Tests

```powershell
cd C:\Users\01226\Desktop\MySelf\CTracker
.\run-tests.ps1
```

Or run specific tests:

```powershell
.\run-tests.ps1 -TestName test_databasemanager_v1
.\run-tests.ps1 -TestName test_databasemanager_v2
.\run-tests.ps1 -TestName test_models
.\run-tests.ps1 -TestName test_widgets
```

## Step 4: Test the Application

After tests pass, manually test:
1. Launch CTracker.exe
2. Add a course → should NOT crash
3. Add a unit → should NOT crash
4. Add sessions → should NOT crash
5. Expand unit → sessions should display
6. Collapse unit → sessions should hide
7. Modify progress → should NOT crash
8. Delete items → should NOT crash

## What Was Fixed

### Critical Bug #1: Infinite Recursion
- **Symptom**: App crashed after any add/remove/modify operation
- **Cause**: `DatabaseManager::emitDataChanged()` called itself
- **Fix**: Changed to `emit dataChanged()`

### Bug #2: Unit Expansion
- **Symptom**: Units wouldn't expand/collapse properly
- **Cause**: Backwards logic in `setExpanded()`
- **Fix**: Simplified early return logic

### Bug #3: Memory Safety
- **Symptom**: Potential crashes from dangling signals
- **Cause**: Signals not disconnected before widget deletion
- **Fix**: Added `disconnect()` calls

## What Was Added

### Test Suite (37 tests total):
- **8 tests** - DatabaseManager v1 (core CRUD)
- **7 tests** - DatabaseManager v2 (extended features)
- **7 tests** - Models (import/export/aggregation)
- **15 tests** - Widgets (UI behavior)

### Infrastructure:
- `run-tests.ps1` - Automated test runner
- `tests/README.md` - Comprehensive test documentation
- Updated `tests/CMakeLists.txt` - All tests configured

## Expected Results

✅ All tests should pass
✅ Application should not crash during normal operations
✅ Units should expand/collapse smoothly
✅ Progress updates should work correctly

## If Tests Fail

1. Run with verbose output:
   ```powershell
   .\run-tests.ps1 -Verbose
   ```

2. Check specific failing test:
   ```powershell
   .\run-tests.ps1 -TestName <test_name>
   ```

3. Review test output for details

## Next Steps After Testing

Once tests pass and manual testing confirms stability:
1. Continue with remaining bug fixes
2. Add more edge case tests
3. Implement Phase 8 remaining tasks (styling, assets)
4. Prepare for production deployment

---

**Status**: Ready to build and test
**Action Required**: Close CTracker.exe, then run build command
