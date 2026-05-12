# Project Tasks: CTracker
**Stack:** C++17 | Qt 6 | SQLite | CMake  
**Architecture:** Model-View | Signal-Slot | Dark Industrial Theme

> **UI Strategy:** Hybrid approach.
> - **Qt Designer (`.ui` files):** Used for static layouts (MainWindow frame, Settings, AnalyticsView skeleton)
> - **Pure C++ Code:** Used for all custom widgets (`CircularProgressBar`, `ContributionHeatmap`, `SessionTaskRow`, `UnitExpandableWidget`, `EntityCard`)
> 
> **Learning Approach:** Every task is explained before it is written. No black boxes.

---

## Phase 1: Project Initialization & Build System

### Task 1.1: Repository & Directory Structure
- [x] Create root project directory: `CTracker/`
- [x] Create `src/` directory for all source files
- [x] Create `include/` directory for all header files
- [x] Create `assets/styles/` directory for QSS files
- [x] Create `assets/icons/` directory for navigation icons
- [x] Create `tests/` directory for unit tests
- [x] Create `resources/` directory for Qt resource files (`.qrc`)
- [x] Create `.gitignore` with CMake build artifacts and OS files excluded

### Task 1.2: CMake Build System
- [x] Create root `CMakeLists.txt` with `cmake_minimum_required(VERSION 3.20)`
- [x] Set project name and `CMAKE_CXX_STANDARD 17`
- [x] Add `find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql)`
- [x] Define `SOURCES` variable listing all `.cpp` files
- [x] Define `HEADERS` variable listing all `.h` files
- [x] Add `qt_add_executable(CTracker ...)` target
- [x] Link Qt6::Core, Qt6::Widgets, Qt6::Sql to the target
- [x] Enable `CMAKE_AUTOMOC ON`, `CMAKE_AUTORCC ON`, `CMAKE_AUTOUIC ON`
- [x] Add a `tests/CMakeLists.txt` sublisting with GTest or QtTest
- [x] Create a `build/` directory and verify `cmake ..` runs without errors

### Task 1.3: Qt Resource File
- [x] Create `resources/resources.qrc`
- [x] Add QSS dark theme file path to `.qrc`
- [x] Add placeholder icon paths to `.qrc`
- [x] Include the `.qrc` in `CMakeLists.txt` via `qt_add_resources`

---

## Phase 2: Database Layer (`DatabaseManager`)

### Task 2.1: Singleton Setup
- [x] Create `include/DatabaseManager.h`
- [x] Declare `DatabaseManager` as `QObject` singleton with `static instance()` method
- [x] Declare private constructor to prevent direct instantiation
- [x] Declare `bool initialize(const QString& dbPath = {})` method
- [x] Declare `bool isOpen() const` and `void close()` methods
- [x] Declare private `QSqlDatabase m_database` member
- [x] Create `src/DatabaseManager.cpp` and implement `instance()` with static pointer
- [x] Implement `initialize()`: open/create `ctracker.db` in the app data path

### Task 2.2: Schema Creation
- [x] Implement private `bool createTables()` in `DatabaseManager`
- [x] Write `CREATE TABLE IF NOT EXISTS CoursesProjects` SQL with all columns
- [x] Write `CREATE TABLE IF NOT EXISTS Units` SQL with `FOREIGN KEY ... CASCADE DELETE`
- [x] Write `CREATE TABLE IF NOT EXISTS SessionsTasks` SQL with `CHECK(CurrentProgress BETWEEN 0 AND 100)`
- [x] Write `CREATE TABLE IF NOT EXISTS ActivityLog` SQL with `FOREIGN KEY ... CASCADE DELETE`
- [x] Enable SQLite foreign key support: `PRAGMA foreign_keys = ON`
- [x] Call `createTables()` inside `initialize()` after opening the DB
- [x] Test: Manually inspect the generated `.db` file with a SQLite viewer

### Task 2.3: Query Helper Methods
- [x] Implement private `bool executeQuery(const QString& sql, const QVariantMap& params)` using `QSqlQuery`
- [x] Implement private `QList<QVariantMap> executeSelectQuery(const QString& sql, const QVariantMap& params)`
- [x] Use named placeholders (`:param`) in all queries to prevent SQL injection
- [x] Handle and emit `databaseError(const QString& error)` signal on failure
- [x] Implement `beginTransaction()`, `commitTransaction()`, `rollbackTransaction()` wrappers

### Task 2.4: Entity (Course/Project) CRUD
- [x] Implement `int addCourse(const QString& name)` — returns new ID or -1 on failure
- [x] Implement `int addProject(const QString& name)` — returns new ID or -1 on failure
- [x] Implement `bool removeCourse(int courseId)` — CASCADE deletes Units/Sessions
- [x] Implement `bool removeProject(int projectId)` — CASCADE deletes Units/Sessions
- [x] Implement `bool renameCourse(int courseId, const QString& newName)`
- [x] Implement `bool renameProject(int projectId, const QString& newName)`
- [x] Implement `QList<EntityData> fetchAllCourses()`
- [x] Implement `QList<EntityData> fetchAllProjects()`
- [x] Implement `QList<EntityData> fetchAllEntities()` (courses + projects combined)
- [x] Emit `dataChanged()` signal at the end of each successful write operation

### Task 2.5: Unit CRUD
- [x] Implement `int addUnit(int parentId, const QString& name)`
- [x] Implement `bool removeUnit(int unitId)` — CASCADE deletes Sessions/Tasks
- [x] Implement `bool renameUnit(int unitId, const QString& newName)`
- [x] Implement `QList<UnitData> getUnitsForParent(int parentId)`

### Task 2.6: Session/Task CRUD
- [x] Implement `int addSessionTask(int unitId, const QString& name, int progress = 0)`
- [x] Implement `bool removeSessionTask(int sessionId)`
- [x] Implement `bool renameSessionTask(int sessionId, const QString& newName)`
- [x] Implement `bool updateSessionTaskProgress(int sessionId, int progress)` inside a transaction
- [x] Implement `int getSessionTaskProgress(int sessionId)` helper
- [x] Implement `QList<SessionTaskData> getSessionTasksForUnit(int unitId)`

### Task 2.7: Activity Log Operations
- [x] Implement `int logActivity(int itemId, int oldVal, int newVal, const QString& type, const QDateTime& ts)`
- [x] Guard: if `oldVal == newVal`, do NOT insert — return -1
- [x] Calculate and store `ProgressDelta = ABS(newVal - oldVal)` in the INSERT
- [x] Implement `QList<ActivityLogEntry> getActivityLog(const QDate& from, const QDate& to)`
- [x] Implement `QList<ActivityLogEntry> getActivityLogForItem(int itemId)`

---

## Phase 3: Data Structures

### Task 3.1: Define Shared Data Structs
- [x] Create `include/DataStructures.h`
- [x] Define `struct EntityData { int id; QString name; QString type; int overallProgress; }`
- [x] Define `struct UnitData { int id; int parentId; QString name; }`
- [x] Define `struct SessionTaskData { int id; int unitId; QString name; int progress; }`
- [x] Define `struct ActivityLogEntry { int id; int itemId; QDateTime timestamp; int progressDelta; int oldValue; int newValue; QString type; }`
- [x] Define `struct HeatmapDataPoint { QDate date; int totalProgress; int activityCount; int intensityLevel; }`

---

## Phase 4: Models

### Task 4.1: ActivityLogModel
- [x] Create `include/ActivityLogModel.h` inheriting `QAbstractTableModel`
- [x] Declare columns: `TimestampCol`, `ItemNameCol`, `TypeCol`, `ProgressChangeCol`
- [x] Implement `rowCount()`, `columnCount()`, `data()`, `headerData()` overrides
- [x] Implement `setFilterDateRange(from, to)` and `setFilterItemType(type)`
- [x] Implement `clearFilters()` and `refresh()`
- [x] Implement `QMap<QDate, int> getDailyProgressTotals(from, to)`
- [x] Implement `QMap<QDate, int> getDailyActivityCounts(from, to)`

### Task 4.2: JSON Data Importer
- [x] Create `include/DataImporter.h` and `src/DataImporter.cpp`
- [x] Implement `bool importFromFile(const QString& filePath)`
- [x] Validate: file opens, is valid JSON, is a JSON object
- [x] Validate: `version`, `type`, `name`, and `units` fields exist
- [x] Validate: `type` is `"course"` or `"project"` — error otherwise
- [x] Wrap entire import in a single DB transaction (atomic)
- [x] For each unit, call `addUnit()`; for each session, call `addSessionTask()`
- [x] Clamp all `progress` values to [0, 100]
- [x] Log warnings for invalid entries but continue importing valid ones
- [x] Emit `importCompleted(int entityId)` signal on success

### Task 4.3: JSON Data Exporter
- [x] Create `include/DataExporter.h` and `src/DataExporter.cpp`
- [x] Implement `bool exportToFile(const QString& filePath)`
- [x] Fetch all entities from `DatabaseManager`
- [x] For each entity: build `QJsonObject` with `version`, `type`, `name`, `units`
- [x] For each unit: build nested `QJsonObject` with `name` and `sessions` array
- [x] For each session: build `QJsonObject` with `name` and `progress`
- [x] Write `QJsonDocument` to file with UTF-8 encoding
- [x] Return `false` and display error if file cannot be opened for writing

---

## Phase 5: Custom Widgets

### Task 5.1: CircularProgressBar
- [ ] Create `include/CircularProgressBar.h` inheriting `QWidget`
- [ ] Declare `Q_PROPERTY` for `progress`, `lineWidth`, `backgroundColor`, `progressColor`
- [ ] Implement `sizeHint()` returning `QSize(100, 100)`
- [ ] Override `paintEvent(QPaintEvent*)` with `QPainter` + `Antialiasing` hint
- [ ] Implement `drawBackground()`: draw full 360° arc in `m_backgroundColor`
- [ ] Implement `drawProgress()`: draw arc from 90° for `progress * 3.6` degrees clockwise
- [ ] Implement `drawText()`: draw centered `"%1%"` text inside the ring
- [ ] Implement `setProgress(int value)` — clamp to [0,100], call `update()`
- [ ] Test: Display widget in a standalone `QDialog` with sliders to verify rendering

### Task 5.2: ContributionHeatmap
- [ ] Create `include/ContributionHeatmap.h` inheriting `QWidget`
- [ ] Define inner structs `DayData` and `Cell`
- [ ] Declare constants: `COLS = 53`, `ROWS = 7`, `CELL_SIZE = 12`, `CELL_SPACING = 3`
- [ ] Implement `setData(const QMap<QDate, DayData>&)` and `setYear(int)`
- [ ] Implement `calculateCells()`: compute `QRect` for each of the 371 cells based on week/day position
- [ ] Override `paintEvent()`: call `drawLabels()`, then loop and `fillRect()` for each cell
- [ ] Implement `getIntensityColor(int intensity)`: return 5-level GitHub green gradient
- [ ] Implement `drawLabels()`: month labels on top, day-of-week labels on left
- [ ] Override `mouseMoveEvent()`: find hovered cell using `getCellAtPosition()`, show `QToolTip`
- [ ] Override `leaveEvent()`: hide tooltip on mouse leave
- [ ] Implement year navigation with `setYear()` triggering full repaint

### Task 5.3: SessionTaskRow
- [ ] Create `include/SessionTaskRow.h` inheriting `QWidget`
- [ ] Setup layout: `QLabel` (name) | `QSlider` (0-100) | `QLabel` (percentage)
- [ ] Override `mouseDoubleClickEvent()` on name label to switch to `QLineEdit` for inline editing
- [ ] Connect `QLineEdit::editingFinished` to revert to label display
- [ ] Validate: if new name is empty/whitespace, revert to previous name
- [ ] Connect `QSlider::sliderPressed` to record `m_oldProgress`
- [ ] Connect `QSlider::sliderReleased` to emit `progressChanged(oldValue, newValue)` only if value changed
- [ ] Connect `QSlider::valueChanged` to update the percentage `QLabel` in real time
- [ ] Implement `setProgress(int)` and `setName(const QString&)` for external updates

### Task 5.4: UnitExpandableWidget
- [ ] Create `include/UnitExpandableWidget.h` inheriting `QWidget`
- [ ] Setup header: expand/collapse `QPushButton` + unit name `QLabel` + overall progress `QLabel`
- [ ] Setup collapsible `QWidget` containing a `QVBoxLayout` for `SessionTaskRow` children
- [ ] Implement `setExpanded(bool)`: show/hide the content widget, update button arrow icon
- [ ] Implement `addSessionTask(int id, const QString& name, int progress)`: create `SessionTaskRow`, add to map and layout
- [ ] Implement `removeSessionTask(int id)`: find in map, delete widget, remove from layout
- [ ] Implement `calculateOverallProgress()`: average of all child slider values
- [ ] Connect each child's `progressChanged` to `calculateOverallProgress()` and update parent label
- [ ] Emit `sessionTaskProgressChanged(sessionId, oldValue, newValue)` to propagate to views

### Task 5.5: EntityCard
- [ ] Create `include/EntityCard.h` inheriting `QFrame`
- [ ] Define `enum class EntityType { Course, Project }`
- [ ] Setup layout: `CircularProgressBar` (center) + `QLabel` name (bottom) + `QLabel` type badge
- [ ] Override `mousePressEvent()` to emit `clicked(entityId, type)`
- [ ] Override `enterEvent()` / `leaveEvent()` to animate a subtle border/shadow highlight
- [ ] Implement `setProgress(int)` delegating to `CircularProgressBar::setProgress()`
- [ ] Apply fixed card size (e.g., 160x180px) so grid layout is uniform

---

## Phase 6: Views & Navigation

### Task 6.1: SideNavigationBar
- [ ] Create `include/SideNavigationBar.h` inheriting `QWidget`
- [ ] Create 5 `QPushButton` icon buttons: Home, Courses, Projects, Analytics, Settings
- [ ] Arrange in a `QVBoxLayout` with a `QSpacerItem` at the bottom
- [ ] Implement `setActiveButton(int index)`: toggle active CSS class/property on buttons
- [ ] Connect each button `clicked` to emit `navigationRequested(int pageIndex)` signal
- [ ] Use fixed width (e.g., 60px) for the sidebar

### Task 6.2: HomeDashboard
- [ ] Create `include/HomeDashboard.h` inheriting `QWidget`
- [ ] Create a `QScrollArea` wrapping a `QWidget` with a `QGridLayout` (3 columns)
- [ ] Implement `loadEntities()`: fetch all from `DatabaseManager`, call `createCards()`
- [ ] Implement `createCards()`: instantiate `EntityCard` for each entity, connect `clicked` signals
- [ ] Implement `clearCards()`: delete all existing card widgets before reload
- [ ] Connect `DatabaseManager::dataChanged()` to `onDataChanged()` slot for live updates
- [ ] Emit `courseSelected(int id)` / `projectSelected(int id)` when cards are clicked

### Task 6.3: CourseDetailView & ProjectDetailView
- [ ] Create `include/CourseDetailView.h` inheriting `QWidget`
- [ ] Add a title bar with entity name, overall progress `CircularProgressBar`, and Add/Delete buttons
- [ ] Add a `QScrollArea` wrapping a `QVBoxLayout` for `UnitExpandableWidget` items
- [ ] Implement `loadCourse(int courseId)`: fetch units and sessions, populate widget list
- [ ] Connect Add Unit button to a `QInputDialog` → `DatabaseManager::addUnit()`
- [ ] Connect Add Session button within a unit to `DatabaseManager::addSessionTask()`
- [ ] Connect `UnitExpandableWidget::sessionTaskProgressChanged` → `DatabaseManager::updateSessionTaskProgress()`
- [ ] Create `ProjectDetailView` mirroring `CourseDetailView` (can share a base class)

### Task 6.4: AnalyticsView
- [ ] Create `include/AnalyticsView.h` inheriting `QWidget`
- [ ] Add year navigation buttons (previous/next) and a year label
- [ ] Embed `ContributionHeatmap` widget
- [ ] Implement `loadYear(int year)`: fetch data from `ActivityLogModel::getDailyProgressTotals()`, call `heatmap.setData()`
- [ ] Add a legend row below the heatmap explaining the 5 intensity levels

### Task 6.5: SettingsView
- [ ] Create `include/SettingsView.h` inheriting `QWidget`
- [ ] Add "Import Data" button → opens `QFileDialog` → calls `DataImporter::importFromFile()`
- [ ] Add "Export Data" button → opens `QFileDialog` → calls `DataExporter::exportToFile()`
- [ ] Add "Database Location" read-only label showing path to `.db` file

### Task 6.6: MainWindow
- [ ] Create `include/MainWindow.h` inheriting `QMainWindow`
- [ ] Instantiate `SideNavigationBar` and dock it to the left
- [ ] Instantiate `QStackedWidget` and add all 5 views as pages
- [ ] Connect `SideNavigationBar::navigationRequested(int)` → `QStackedWidget::setCurrentIndex(int)`
- [ ] Connect `HomeDashboard::courseSelected(int)` → `CourseDetailView::loadCourse(int)` → navigate to course page
- [ ] Connect `HomeDashboard::projectSelected(int)` → `ProjectDetailView::loadProject(int)` → navigate to project page
- [ ] Implement `loadStyleSheet()`: read `:/styles/dark-industrial.qss` and apply via `setStyleSheet()`
- [ ] Set minimum window size (e.g., 900x600)

---

## Phase 7: Styling (Dark Industrial QSS)

### Task 7.1: Base Theme
- [ ] Create `assets/styles/dark-industrial.qss`
- [ ] Set `QMainWindow` background to `#0d1117`
- [ ] Style `SideNavigationBar` background to `#161b22`, buttons to flat/borderless
- [ ] Style active nav button with a left accent border in `#39d353` (GitHub green)
- [ ] Style `QScrollArea` and `QWidget` backgrounds consistently dark

### Task 7.2: Widget Styling
- [ ] Style `EntityCard` (QFrame): `background: #161b22; border-radius: 8px; border: 1px solid #30363d`
- [ ] Style `EntityCard` hover: `border: 1px solid #39d353`
- [ ] Style `QSlider` groove and handle to match dark theme
- [ ] Style `QLabel` text to use `#c9d1d9` (GitHub text color)
- [ ] Style `QPushButton` with hover and pressed states

### Task 7.3: Typography
- [ ] Load a custom font (e.g., Inter or Roboto) via `QFontDatabase::addApplicationFont()`
- [ ] Apply the font globally via `QApplication::setFont()`

---

## Phase 8: Application Entry Point & Error Handling

### Task 8.1: main.cpp
- [ ] Create `src/main.cpp`
- [ ] Instantiate `QApplication`
- [ ] Set application name, version, and organization
- [ ] Call `DatabaseManager::instance()->initialize()` — exit with code 1 on failure
- [ ] Create and show `MainWindow`
- [ ] Enter `app.exec()` event loop
- [ ] On exit, call `DatabaseManager::instance()->close()`

### Task 8.2: Error Handling
- [ ] Connect `DatabaseManager::databaseError(QString)` to a global `QMessageBox::critical()` slot
- [ ] In `updateSessionTaskProgress`: on DB failure, emit signal to revert slider to old value
- [ ] In `CircularProgressBar::paintEvent`: wrap in try/catch, render placeholder on exception
- [ ] In `DataImporter`: for parse errors, log to `qWarning()` with details and continue

---

## Phase 9: Testing

### Task 9.1: DatabaseManager Tests
- [ ] Test `initialize()` creates all 4 tables correctly
- [ ] Test `addCourse()` returns a valid ID > 0
- [ ] Test `addUnit()` fails gracefully with invalid parent ID
- [ ] Test `updateSessionTaskProgress()` with value outside [0,100] is rejected
- [ ] Test cascade delete: deleting a Course removes all its Units and Sessions
- [ ] Test `logActivity()` is NOT called when `oldValue == newValue`

### Task 9.2: Model Tests
- [ ] Test `ActivityLogModel::getDailyProgressTotals()` returns correct sums
- [ ] Test `DataImporter` rejects JSON with missing `type` field
- [ ] Test `DataImporter` clamps progress values > 100 to 100
- [ ] Test `DataExporter` produces valid JSON that can be re-imported

### Task 9.3: Widget Tests
- [ ] Test `CircularProgressBar::setProgress(-1)` clamps to 0
- [ ] Test `CircularProgressBar::setProgress(150)` clamps to 100
- [ ] Test `UnitExpandableWidget::calculateOverallProgress()` returns 0 with no children
