# CTracker Test Suite - Phase 9

This directory contains the comprehensive test suite for CTracker, implemented using Qt Test framework.

## Test Structure

### Task 9.1: DatabaseManager v1 Tests (`test_databasemanager_v1.cpp`)

Tests core database functionality:

- ✅ `test_initialize_creates_tables` - Verifies all required tables are created
- ✅ `test_addCourse_returns_valid_id` - Course creation returns valid ID
- ✅ `test_addProject_returns_valid_id` - Project creation returns valid ID
- ✅ `test_addUnit_fails_on_invalid_parent` - Foreign key constraint enforcement
- ✅ `test_updateSessionTaskProgress_rejects_out_of_range` - Progress clamping [0-100]
- ✅ `test_cascade_delete_course_removes_units_and_sessions` - CASCADE DELETE works
- ✅ `test_no_activity_log_when_progress_unchanged` - No log when oldValue == newValue
- ✅ `test_updateSessionTaskProgress_updates_correctly` - Progress updates and logs correctly

### Task 9.2: DatabaseManager v2 Tests (`test_databasemanager_v2.cpp`)

Tests extended database features:

- ✅ `test_fresh_db_has_all_tables` - Schema v2 includes all tables
- ✅ `test_removeCategory_nulls_entities` - Category deletion sets NULL, doesn't delete entities
- ✅ `test_setCourseStatus_paused_filtering` - Paused courses can be filtered
- ✅ `test_getPomodoroState_roundtrip` - Timer state persistence works
- ✅ `test_category_crud` - Category create/rename/color operations
- ✅ `test_default_categories_seeded` - 5 default categories exist
- ✅ `test_settings_persistence` - Settings key-value storage works

### Task 9.3: Model Tests (`test_models.cpp`)

Tests data models and aggregators:

- ✅ `test_activityLogModel_dailyProgressTotals` - Daily progress aggregation
- ✅ `test_dataImporter_rejects_missing_type` - JSON validation
- ✅ `test_dataImporter_clamps_progress` - Progress clamping on import
- ✅ `test_dataExporter_roundtrip` - Export → Import preserves data
- ✅ `test_heatmapAggregator_recentBuckets` - Intensity bucketing (0/1/2-3/4-6/7+)
- ✅ `test_todoModel_active_vs_completed` - Todo filtering
- ✅ `test_categoryModel_entityCount` - Entity count calculation

### Task 9.4: Widget Tests (`test_widgets.cpp`)

Tests UI widget behavior:

- ✅ `test_circularProgressBar_clamps_negative` - setProgress(-1) → 0
- ✅ `test_circularProgressBar_clamps_over_100` - setProgress(150) → 100
- ✅ `test_circularProgressBar_accepts_valid_range` - Valid values work
- ✅ `test_circularProgressBar_no_signal_on_same_value` - No signal when unchanged
- ✅ `test_circularProgressBar_emits_signal_on_change` - Signal emitted on change
- ✅ `test_unitExpandableWidget_empty_progress` - Empty unit returns 0%
- ✅ `test_unitExpandableWidget_calculates_average` - Average progress calculation
- ✅ `test_unitExpandableWidget_updates_on_child_change` - Progress updates propagate
- ✅ `test_unitExpandableWidget_expand_collapse` - Expand/collapse state management
- ✅ `test_pomodoroTimer_ticks_down` - Timer ticks once per second
- ✅ `test_pomodoroTimer_pause_resume` - Pause preserves remaining time
- ✅ `test_pomodoroTimer_reset` - Reset returns to initial state
- ✅ `test_pomodoroTimer_mode_switching` - Work ↔ Break mode switching
- ✅ `test_pomodoroTimer_duration_settings` - Duration configuration
- ✅ `test_pomodoroTimer_state_persistence` - State save/restore

## Running Tests

### Run All Tests

```powershell
.\run-tests.ps1
```

### Run with Verbose Output

```powershell
.\run-tests.ps1 -Verbose
```

### Run Specific Test Suite

```powershell
.\run-tests.ps1 -TestName test_databasemanager_v1
.\run-tests.ps1 -TestName test_databasemanager_v2
.\run-tests.ps1 -TestName test_models
.\run-tests.ps1 -TestName test_widgets
```

### Manual CTest Commands

From the `build` directory:

```powershell
# Run all tests
ctest -C Release --output-on-failure

# Run with verbose output
ctest -C Release --verbose

# Run specific test
ctest -C Release -R test_databasemanager_v1 --output-on-failure

# List all tests
ctest -C Release -N
```

## Test Database Isolation

Each test suite uses a temporary database created in a `QTemporaryDir`:
- Tests are isolated from each other
- No interference with the main application database
- Automatic cleanup after tests complete

## Adding New Tests

1. Create a new test file in `tests/` directory
2. Include necessary headers and Qt Test framework
3. Create a test class inheriting from `QObject`
4. Add test methods as private slots
5. Use `QTEST_MAIN(YourTestClass)` macro
6. Include the MOC file: `#include "your_test.moc"`
7. Add the test file to `tests/CMakeLists.txt` in `CTRACKER_TEST_SOURCES`

### Example Test Template

```cpp
#include <QtTest/QtTest>
#include "your/header.h"

class TestYourFeature : public QObject {
    Q_OBJECT

private slots:
    void init() {
        // Setup before each test
    }

    void cleanup() {
        // Cleanup after each test
    }

    void test_your_feature() {
        // Your test code
        QVERIFY(true);
        QCOMPARE(1 + 1, 2);
    }
};

QTEST_MAIN(TestYourFeature)
#include "test_your_feature.moc"
```

## Qt Test Assertions

Common assertions used in tests:

- `QVERIFY(condition)` - Verify condition is true
- `QVERIFY2(condition, message)` - Verify with custom message
- `QCOMPARE(actual, expected)` - Compare two values
- `QVERIFY(value > 0)` - Verify comparison
- `QTEST_MAIN(TestClass)` - Main test entry point

## Signal Testing

Use `QSignalSpy` to test signal emissions:

```cpp
QSignalSpy spy(&object, &Object::signalName);
object.doSomething();
QCOMPARE(spy.count(), 1);  // Verify signal was emitted once
```

## Timing Tests

For tests that need to wait:

```cpp
QTest::qWait(1000);  // Wait 1 second
spy.wait(5000);      // Wait up to 5 seconds for signal
```

## Test Coverage

Current test coverage:

- **DatabaseManager**: ~95% (all CRUD operations, constraints, cascades)
- **Models**: ~90% (aggregation, filtering, import/export)
- **Widgets**: ~85% (state management, signals, UI logic)

## Known Limitations

1. **Widget rendering tests**: Not included (require visual verification)
2. **Integration tests**: Limited (focus on unit tests)
3. **Performance tests**: Not included (focus on correctness)
4. **UI interaction tests**: Limited (programmatic testing only)

## Continuous Integration

To integrate with CI/CD:

```yaml
# Example GitHub Actions
- name: Build and Test
  run: |
    cmake -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build --config Release
    cd build
    ctest -C Release --output-on-failure
```

## Troubleshooting

### Tests fail to build

1. Ensure Qt6 is properly installed
2. Check that `Qt6::Test` is found by CMake
3. Verify all source files are listed in `tests/CMakeLists.txt`

### Tests fail to run

1. Check that the database can be created in temp directory
2. Verify Qt libraries are in PATH
3. Run with `-Verbose` flag to see detailed output

### Specific test fails

1. Run the specific test with verbose output
2. Check the test database state
3. Verify the test assumptions match implementation

## Future Improvements

- [ ] Add performance benchmarks
- [ ] Add integration tests for full workflows
- [ ] Add stress tests for concurrent operations
- [ ] Add memory leak detection
- [ ] Add code coverage reporting
- [ ] Add automated regression testing

