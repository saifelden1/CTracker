# CTracker — Code Reference

> **Purpose.** Function-by-function catalogue of every class that
> exists in the codebase **today** (after Phases 0–3). Each entry
> answers three questions: *what does this do, when is it called, what
> does it return.* Use it as a lookup; you don't have to read it
> linearly.
>
> **Companion docs.**
> - `system-overview.html` — diagrams + status matrix.
> - `phase-0-explained.md` / `phase-1-and-2-explained.md` / `phase-3-explained.md` — narrative walk-throughs of how we got here.
>
> **What this doc is not.** It is not the spec. The spec lives at
> `.ai/specs/{requirements,design,tasks}.md` and is the source of
> truth for *what we will build*. This doc describes *what exists*.
>
> **Layout.** One section per file. Methods are grouped by visibility
> (`public` → `protected` → `private` → `signals` → `slots`) and
> annotated `[v1]` (original commits), `[v2]` (Phase 2 migration), or
> `[Phase 3]` (struct vocabulary).

---

## Table of contents

| Folder | File | Class |
|---|---|---|
| `core/` | [DataStructures.h](#datastructuresh) | (13 POD structs) |
| `core/` | [DatabaseManager.h](#databasemanager) | `DatabaseManager` |
| `core/` | [DataImporter.h](#dataimporter)        | `DataImporter`    |
| `core/` | [DataExporter.h](#dataexporter)        | `DataExporter`    |
| `shared/` | [MainWindow.h](#mainwindow)          | `MainWindow`      |
| `shared/` | [SideNavigationBar.h](#sidenavigationbar) | `SideNavigationBar` |
| `shared/` | [HomeDashboard.h](#homedashboard)    | `HomeDashboard`   |
| `shared/` | [CircularProgressBar.h](#circularprogressbar) | `CircularProgressBar` |
| `courses/` | [EntityCard.h](#entitycard)          | `EntityCard`      |
| `courses/` | [UnitExpandableWidget.h](#unitexpandablewidget) | `UnitExpandableWidget` |
| `courses/` | [SessionTaskRow.h](#sessiontaskrow)  | `SessionTaskRow`  |
| `courses/` | [EntityDetailView.h](#entitydetailview) | `EntityDetailView` |
| `courses/` | [CourseDetailView.h](#alias-headers) | (alias)           |
| `projects/`| [ProjectDetailView.h](#alias-headers) | (alias)          |
| `analytics/` | [ContributionHeatmap.h](#contributionheatmap) | `ContributionHeatmap` |
| `analytics/` | [ActivityLogModel.h](#activitylogmodel) | `ActivityLogModel` |
| `analytics/` | [AnalyticsView.h](#analyticsview) | `AnalyticsView`   |
| `settings/`| [SettingsView.h](#settingsview)      | `SettingsView`    |
| `src/` | [main.cpp](#maincpp)                    | (entry point)     |

---

## `main.cpp`

```cpp
int main(int argc, char* argv[]);
```

The program entry point. Tiny on purpose — every responsibility past
"start Qt and open the DB" lives elsewhere.

**What it does (in order):**

1. Construct a `QApplication` so Qt event handling, fonts, and the QSS
   engine are alive.
2. Set `applicationName` and `organizationName` to `"CTracker"`. These
   feed `QStandardPaths` (used by `DatabaseManager`) and `QSettings`
   (used for any future window-geometry persistence).
3. `DatabaseManager::instance()->initialize()` — open / create the DB,
   apply migrations. On failure: `QMessageBox::critical` and exit 1.
4. Construct `MainWindow` on the stack (RAII clean-up at scope exit).
5. `window.show()` then `app.exec()` (event loop). When `exec()` returns,
   close the DB explicitly and return the exit code.

**When called.** Once per process, by the OS.

---

## `DataStructures.h`

Thirteen plain-old-data structs, no methods (except one inline
`hasContent()`). Detailed exposition in
`phase-3-explained.md`; here's a one-line per struct reference.

### v1 structs `[v1]`

| Struct | Purpose | Maps to |
|---|---|---|
| `EntityData` | A course or a project | `CoursesProjects` + LEFT JOIN `Categories` |
| `UnitData` | A unit inside a course/project | `Units` |
| `SessionTaskData` | The leaf — what the slider acts on | `SessionsTasks` |
| `ActivityLogEntry` | One progress change | `ActivityLog` |
| `HeatmapDataPoint` | One day of activity | derived |

### v2 structs `[Phase 3]`

| Struct | Purpose | Maps to |
|---|---|---|
| `CategoryData` | Colour-tagged label | `Categories` (+ count via JOIN) |
| `ProjectMetaData` (with nested `Link`) | Project's deadline / priority / team / links | `ProjectMeta` (JSON cols → typed lists) |
| `TodoData` | One standalone todo | `Todos` |
| `PomodoroSessionData` | One completed pomodoro | `PomodoroSessions` + LEFT JOIN `CoursesProjects` |
| `CalendarDayData` | One day's notes/todos/completed | `CalendarDayDetails` (JSON cols → `QStringList`) |
| `AnalyticsSummary` | Streaks / hours / avg / WoW | derived, not stored |

### Filter / state structs `[Phase 3 — gap-fills]`

| Struct | Used by |
|---|---|
| `CourseFilter` | `CoursesFilterBar` ↔ `CoursesView` |
| `ProjectFilter` | `ProjectsView` |
| `PomodoroTimerState` (nested `Mode` / `State` enums) | `PomodoroTimerWidget` ↔ persisted in `Settings.pomodoro.state.*` |
| `ProfileData` | `SettingsView` Profile card |
| `PreferencesData` | `SettingsView` Preferences card |

### Helper method

`bool CalendarDayData::hasContent() const` — inline. Returns `true` if
any of `todo`, `completed`, `notes` is non-empty. Used by the
calendar-day indicator dot.

---

## `DatabaseManager`

The singleton that owns the one SQLite connection. Inherits `QObject`
for signals. Every CRUD method returns either a row id (positive on
success, `-1` on failure) or a `bool`. Errors are also reported via
the `databaseError(QString)` signal.

### Lifecycle `[v1]`

```cpp
static DatabaseManager* instance();
bool initialize(const QString& dbPath = QString());
bool isOpen() const;
void close();
```

- `instance()` — lazy singleton accessor. First call constructs;
  subsequent calls reuse.
- `initialize(path)` — open / create the DB. Empty `path` =
  `QStandardPaths::AppDataLocation/ctracker.db`. Pass `":memory:"` for
  tests. Idempotent re-call: reuses the existing Qt SQL connection.
  Calls `createTables()` then `migrate(currentSchemaVersion(), 2)`.
- `isOpen()` / `close()` — wrap `QSqlDatabase`.

### Entity CRUD `[v1]`

```cpp
int  addCourse(const QString& name);
int  addProject(const QString& name);
bool removeCourse(int courseId);
bool removeProject(int projectId);
bool renameCourse(int courseId, const QString& newName);
bool renameProject(int projectId, const QString& newName);
QList<EntityData> fetchAllEntities();
QList<EntityData> fetchAllCourses();
QList<EntityData> fetchAllProjects();
```

- Returns from `addCourse/Project` are the new `ID` (positive) or
  `-1` on failure.
- `fetchAll*` `[v2-updated in Phase 3]` — issue a `LEFT JOIN Categories`
  using a shared `kEntitySelectSql` constant; populate every field of
  `EntityData` including `categoryId`, `status`, `categoryName`,
  `categoryColor`.
- All mutators emit `dataChanged()` on success.

### Unit CRUD `[v1]`

```cpp
int  addUnit(int parentId, const QString& name);
bool removeUnit(int unitId);
bool renameUnit(int unitId, const QString& newName);
QList<UnitData> getUnitsForParent(int parentId);
```

`parentId` is the `CoursesProjects.ID`. Cascade delete handled by the
schema — deleting an entity removes its units automatically.

### Session/Task CRUD `[v1]`

```cpp
int  addSessionTask(int unitId, const QString& name, int progress = 0);
bool removeSessionTask(int sessionId);
bool renameSessionTask(int sessionId, const QString& newName);
bool updateSessionTaskProgress(int sessionId, int progress);
int  getSessionTaskProgress(int sessionId);
QList<SessionTaskData> getSessionTasksForUnit(int unitId);
```

`updateSessionTaskProgress` is the project's most-important write. Detail:

1. Read `oldValue = getSessionTaskProgress(sessionId)`. Bail with
   `true` (no-op) if `oldValue == progress` — prevents polluting
   `ActivityLog`.
2. Clamp to `[0, 100]`.
3. Open a transaction.
4. `UPDATE SessionsTasks SET CurrentProgress = ...`.
5. Resolve `entityType` ("Course" or "Project") via JOIN to
   `CoursesProjects`.
6. `logActivity(...)` — append one `ActivityLog` row.
7. `COMMIT`.
8. Emit `dataChanged()`.

If any step fails, the transaction rolls back and the slider's old
value is preserved on disk.

### Activity log `[v1]`

```cpp
int  logActivity(int itemId, int oldVal, int newVal,
                 const QString& type,
                 const QDateTime& ts = QDateTime::currentDateTime());
QList<ActivityLogEntry> getActivityLog(const QDate& from, const QDate& to);
QList<ActivityLogEntry> getActivityLogForItem(int itemId);
```

`logActivity` writes one row to `ActivityLog`, returning the new ID
(or `-1`). Guard: returns `-1` immediately if `oldVal == newVal`.

### Schema versioning `[v2 — Phase 2.8]`

```cpp
int  currentSchemaVersion();
bool migrate(int from, int to);
```

- `currentSchemaVersion()` — `0` if `SchemaInfo` doesn't exist;
  otherwise the stored integer.
- `migrate(from, to)` — wraps the whole 1→2 jump in a single
  transaction; calls `createV2Tables()`, `addV2Columns()`,
  `seedV2Defaults()`; stamps `SchemaInfo.schema_version`; rolls back
  on any error. Idempotent: re-runs are no-ops.

### Signals

```cpp
void dataChanged();
void databaseError(const QString& message);
```

- `dataChanged` — emitted after every successful mutation. Views and
  models connect to this to refresh.
- `databaseError` — emitted with a human-readable message on every SQL
  failure. `main.cpp` will eventually hook this to a global
  `QMessageBox::critical` (Phase 8.5).

### Private helpers (callable from inside the class only)

| Method | Purpose |
|---|---|
| `createTables()` | v1 schema (idempotent). |
| `createV2Tables()` | Six v2 tables + three indexes (idempotent). |
| `addV2Columns()` | Two `ALTER TABLE ... ADD COLUMN` on `CoursesProjects` guarded by `columnExists()`. |
| `seedV2Defaults()` | 5 default categories + 5 default settings via `INSERT OR IGNORE`. |
| `columnExists(table, column)` | `pragma_table_info` scan. |
| `executeQuery(sql, params)` | Bound-parameter `INSERT/UPDATE/DELETE` runner. |
| `executeSelectQuery(sql, params)` | Bound-parameter `SELECT` runner; returns `QList<QVariantMap>`. |
| `beginTransaction()` / `commitTransaction()` / `rollbackTransaction()` | Wrap `QSqlDatabase` calls. |
| `addEntity(name, type)` | Shared helper for `addCourse` / `addProject`. |
| `renameEntity(id, type, newName)` | Type-guarded rename. |

---

## `DataImporter`

Reads a JSON file and inserts a course/project (with units and
sessions) into the DB inside a single transaction.

```cpp
explicit DataImporter(QObject* parent = nullptr);
bool importFromFile(const QString& filePath);
QString lastError() const;
```

**Expected JSON shape:**

```json
{
  "version": 1,
  "type":    "course",
  "name":    "ROS 2",
  "units": [
    { "name": "Topics", "sessions": [{ "name": "Intro", "progress": 50 }] }
  ]
}
```

**Behaviour:**

- Returns `true` on full success. On failure, *no rows are written* —
  the transaction rolls back.
- Clamps any `progress` outside `[0, 100]` to that range; warns via
  `qWarning()` and continues.
- Rejects missing `type` outright.

**Signals**

- `importCompleted(int entityId)` — the new course/project ID.
- `importFailed(const QString& reason)` — human-readable error.

---

## `DataExporter`

Walks the entire DB and serialises every course + project (with their
units and sessions) into a single JSON file.

```cpp
bool exportToFile(const QString& filePath);
QString lastError() const;
```

**Output shape:**

```json
{
  "version": 1,
  "entities": [
    { "version": 1, "type": "course",  "name": "ROS 2",     "units": [...] },
    { "version": 1, "type": "project", "name": "Robot Arm", "units": [...] }
  ]
}
```

Round-trips with `DataImporter` — exporting and re-importing produces
equivalent data.

**Signals**

- `exportCompleted(const QString& filePath)`
- `exportFailed(const QString& reason)`

---

## `MainWindow`

The application shell. Owns the side navigation, the central
`QStackedWidget`, and the per-page child widgets. Loads the QSS theme.

```cpp
explicit MainWindow(QWidget* parent = nullptr);
```

### Stack page enum

```cpp
enum StackIndex {
    HomeStack      = 0,
    CourseStack    = 1,
    ProjectStack   = 2,
    AnalyticsStack = 3,
    SettingsStack  = 4,
};
```

Phase 7 will grow this to 9 by inserting `ToDoStack`, `PomodoroStack`,
`CoursesListStack`, and `ProjectsListStack` (the latter two replacing
the v1 direct-jump-to-detail behaviour).

### Private setup

| Method | Does |
|---|---|
| `setupUi()` | Creates the central widget, lays out sidebar + stack, instantiates the five page widgets. |
| `setupConnections()` | Wires sidebar `navigationRequested` and the two `HomeDashboard` signals (`courseSelected` / `projectSelected`) and the two detail-view `backRequested` signals. |
| `loadStyleSheet()` | Reads `:/styles/dark-industrial.qss` from the qrc and applies it. Empty/missing stylesheet is silently tolerated. |

### Private slots (event handlers)

| Slot | Triggered by | Effect |
|---|---|---|
| `onNavigationRequested(int pageIndex)` | `SideNavigationBar::navigationRequested` | Switches the stack. For Courses/Projects, jumps to the last-loaded detail view (or Home if nothing's loaded). |
| `onCourseSelected(int courseId)` | `HomeDashboard::courseSelected` | `CourseDetailView::loadCourse(id)`, switch stack to CourseStack, mark sidebar Courses as active. |
| `onProjectSelected(int projectId)` | `HomeDashboard::projectSelected` | Mirror of above for projects. |
| `onDetailBackRequested()` | Either detail view's `backRequested` | Switch back to HomeStack and mark Home active. |

---

## `SideNavigationBar`

256-px-wide column on the left edge. 5 buttons (Phase 7.1 will add 2
more for To-Do and Pomodoro).

```cpp
enum Page {
    HomePage = 0, CoursesPage, ProjectsPage,
    AnalyticsPage, SettingsPage, PageCount
};

void setActiveButton(int index);
int  activeButton() const;

signals:
void navigationRequested(int pageIndex);
```

- Buttons share a `QButtonGroup` so only one is checked at a time.
- Clicking a button emits `navigationRequested(index)`. `MainWindow`
  listens.
- `setActiveButton` is called by `MainWindow` after a navigation that
  *didn't* originate from a button (e.g. a card click on the home
  dashboard).

---

## `HomeDashboard`

The current Home page — a responsive `QGridLayout` of `EntityCard`s
showing every course and project, with an empty-state label when there
are none.

```cpp
void refreshCards();

signals:
void courseSelected(int courseId);
void projectSelected(int projectId);

public slots:
void onDataChanged();        // hooked to DatabaseManager::dataChanged
```

### Lifecycle

| Method | Does |
|---|---|
| `setupUi()` | Constructs the scroll area, the cards container, the grid layout, and an empty-state label. |
| `loadEntities()` | Fetches `fetchAllCourses()` + `fetchAllProjects()`, computes each one's `overallProgress`, calls `createCard` for each. |
| `createCard(id, name, type)` | Constructs an `EntityCard`, connects its `clicked` to emit `courseSelected` or `projectSelected`. |
| `clearCards()` | Deletes every card widget. |
| `computeOverallProgress(entityId) const` | Mean(progress) across every session/task of every unit of the entity. Returns 0 for empty entities. |
| `refreshCards()` | Equivalent to `clearCards()` + `loadEntities()`. |
| `onDataChanged()` | Slot: calls `refreshCards()`. |

> **Note (Phase 7.2 will rebuild this).** The v2 form adds a row of
> `StatsCard`s at the top and a calendar+heatmap pane below. Today's
> implementation is the v1 placeholder.

---

## `CircularProgressBar`

A 0..100% ring drawn with `QPainter`. Owns four `Q_PROPERTY`s for QSS
or `QPropertyAnimation` access.

```cpp
Q_PROPERTY(int    progress        READ progress  WRITE setProgress  NOTIFY progressChanged)
Q_PROPERTY(int    lineWidth       READ lineWidth WRITE setLineWidth)
Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)
Q_PROPERTY(QColor progressColor   READ progressColor   WRITE setProgressColor)

void setProgress(int value);          // clamped 0..100; emits progressChanged only if value differs
QSize sizeHint() const override;
QSize minimumSizeHint() const override;

protected:
void paintEvent(QPaintEvent*) override;   // antialiased arc rendering

signals:
void progressChanged(int value);
```

**`setProgress` invariants** (Task 9.4):

- `setProgress(-1)` ⇒ `progress() == 0`.
- `setProgress(150)` ⇒ `progress() == 100`.
- Calling with the current value is a no-op (no signal, no repaint).

---

## `EntityCard`

A fixed-size (160 × 180 px) card showing one course/project: its
ring, name, and a "Course"/"Project" type badge.

```cpp
enum class EntityType { Course, Project };

explicit EntityCard(int entityId, const QString& name,
                    EntityType type, int progress, QWidget* parent = nullptr);

int        entityId() const;
EntityType type()     const;
QString    name()     const;

void setProgress(int percentage);
void setName(const QString& name);

signals:
void clicked(int entityId, EntityType type);
```

Hover affordance via `enterEvent`/`leaveEvent` (border colour change).
Mouse click emits `clicked(id, type)`. **v2 add-ons (Task 6.5) pending:**
embedded `CategoryPill` slot and a "Paused" badge.

---

## `UnitExpandableWidget`

A collapsible block representing one `Unit`. Contains zero-or-more
`SessionTaskRow` children; their mean progress drives a small header
indicator.

```cpp
explicit UnitExpandableWidget(int unitId, const QString& name,
                              QWidget* parent = nullptr);

int     unitId()   const;
QString name()     const;
bool    isExpanded() const;

void setExpanded(bool expanded);
void setName(const QString& newName);

void addSessionTask(int sessionId, const QString& name, int progress);
void removeSessionTask(int sessionId);
void updateSessionTaskProgress(int sessionId, int progress);

int  calculateOverallProgress() const;     // mean(child progress); 0 if no children

signals:
void sessionTaskProgressChanged(int sessionId, int oldValue, int newValue);
void sessionTaskRenamed(int sessionId, const QString& newName);
void expandStateChanged(bool expanded);
```

**Internal slots wire children to outer signals:**

- `onExpandClicked()` toggles expansion, hides/shows the content
  widget, emits `expandStateChanged`.
- `onChildProgressChanged(old, new)` is fired by each `SessionTaskRow`;
  forwards as `sessionTaskProgressChanged(sessionId, old, new)`.
- `onChildNameChanged(newName)` likewise.

The widget owns its `SessionTaskRow`s in a `QMap<int, SessionTaskRow*>`
keyed by session ID for O(1) `update`/`remove`.

---

## `SessionTaskRow`

One row inside a `UnitExpandableWidget`. Contains a `QSlider` and a
name label that swaps to a `QLineEdit` in edit mode (double-click).

```cpp
explicit SessionTaskRow(int sessionId, const QString& name,
                        int progress, QWidget* parent = nullptr);

int     sessionId() const;
int     progress()  const;
QString name()      const;

void setProgress(int value);
void setName(const QString& newName);

signals:
void progressChanged(int oldValue, int newValue);
void nameChanged(const QString& newName);

protected:
bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
void onSliderPressed();
void onSliderReleased();
void onSliderValueChanged(int value);
void onNameEditingFinished();
```

**Slider semantics — important detail:**

- `valueChanged` fires continuously while dragging. We don't want to
  hit the DB on every pixel.
- The row tracks `m_pressedStartValue` in `onSliderPressed`.
- On `onSliderReleased`, it emits `progressChanged(oldValue, newValue)`
  *once* with the start-to-release delta.

The `QStackedWidget` swap between the name label and the inline edit
field is what gives the in-place rename UX.

---

## `EntityDetailView`

The shared base used by both `CourseDetailView` and
`ProjectDetailView` (today: alias headers; Phase 7.5/7.6: real
subclasses). Shows the entity title, an overall-progress
`CircularProgressBar`, add/delete buttons, and a scrolling list of
`UnitExpandableWidget`s.

```cpp
explicit EntityDetailView(EntityCard::EntityType type,
                          QWidget* parent = nullptr);

void loadEntity(int entityId);
int  currentEntityId() const;

signals:
void entityRemoved(int entityId);
void backRequested();

private slots:
void onAddUnitClicked();
void onAddSessionClicked();
void onDeleteEntityClicked();
void onSessionProgressChanged(int sessionId, int oldValue, int newValue);
void onSessionRenamed(int sessionId, const QString& newName);
void onDataChanged();
```

**Key flows:**

- `loadEntity(id)` — fetches the entity row, populates the title and
  the overall ring, calls `rebuildUnits()`.
- `rebuildUnits()` — `clearUnits()` then for each unit, creates a
  `UnitExpandableWidget`, populates its sessions, connects its
  signals.
- `onSessionProgressChanged` is the project's hot path — calls
  `DatabaseManager::updateSessionTaskProgress(id, newValue)`.
- `onDataChanged()` is a guard against stale UI after CRUD elsewhere.

The constructor takes an `EntityCard::EntityType` so the same class
can be instantiated for either kind of entity; the type drives
"Course"/"Project" label text and dispatches the right
`removeCourse` vs `removeProject` on delete.

---

## Alias headers

```cpp
// include/courses/CourseDetailView.h
#include "courses/EntityDetailView.h"
using CourseDetailView = EntityDetailView;

// include/projects/ProjectDetailView.h
#include "courses/EntityDetailView.h"
using ProjectDetailView = EntityDetailView;
```

Both are one-line aliases over `EntityDetailView`. The reason: every
call site reads as either `CourseDetailView` or `ProjectDetailView`,
clearer at intent than `EntityDetailView`. Phase 7.5/7.6 will replace
them with real subclasses that add type-specific buttons and panels.

---

## `ContributionHeatmap`

GitHub-style 53×7 grid of coloured cells, one cell per day of a year.
Reads pre-aggregated `DayData` from a `QMap<QDate, DayData>`.

```cpp
struct DayData {
    QDate date;
    int   totalProgress  = 0;
    int   activityCount  = 0;
    int   intensityLevel = 0;   // 0..4
};

explicit ContributionHeatmap(QWidget* parent = nullptr);

void setData(const QMap<QDate, DayData>& data);
void setYear(int year);
int  year() const;
void clearData();
QSize sizeHint() const override;

signals:
void dayHovered(const QDate& date, int totalProgress);

protected:
void paintEvent(QPaintEvent*) override;
void mouseMoveEvent(QMouseEvent*) override;
void leaveEvent(QEvent*) override;
void resizeEvent(QResizeEvent*) override;
```

**Grid constants** (matching the React reference):

```cpp
COLS = 53; ROWS = 7; CELL_SIZE = 12 px; CELL_SPACING = 3 px;
LEFT_PAD = 28;     // day-of-week labels
TOP_PAD  = 18;     // month labels
```

**Private:**

- `calculateCells()` — builds the `m_cells` list whenever year or
  geometry changes. Each `Cell` is `{ QRect, QDate, intensity }`.
- `drawLabels(painter)` — paints day-of-week column and month-name row.
- `getIntensityColor(intensity)` — 5-step green ramp.
- `getCellAtPosition(pos)` — used by mouse-move for hover tooltips.

---

## `ActivityLogModel`

`QAbstractTableModel` exposing `ActivityLog` rows + two daily-aggregate
helpers.

```cpp
enum Column {
    TimestampCol = 0, ItemNameCol, TypeCol, ProgressChangeCol, ColumnCount
};

explicit ActivityLogModel(QObject* parent = nullptr);

int      rowCount(...) const override;
int      columnCount(...) const override;
QVariant data(const QModelIndex& index, int role) const override;
QVariant headerData(int section, Qt::Orientation, int role) const override;

void setFilterDateRange(const QDate& from, const QDate& to);
void setFilterItemType(const QString& type);   // "Course"|"Project"|"" for all
void clearFilters();
void refresh();

QMap<QDate, int> getDailyProgressTotals(const QDate& from, const QDate& to) const;
QMap<QDate, int> getDailyActivityCounts(const QDate& from, const QDate& to) const;
```

**Behaviour:**

- Holds an in-memory `QList<ActivityLogEntry>` mirroring the current
  filter window.
- `refresh()` re-runs the query (via `DatabaseManager::getActivityLog`
  for the date range, or `getActivityLogForItem` for the item filter
  case) and rebuilds the cache.
- `itemName(int)` caches `SessionsTasks` name lookups per refresh —
  prevents N queries per repaint when the table view scrolls.

**Used by:** `AnalyticsView` (heatmap input and the table). Once
Phase 5.6's `HeatmapAggregator` lands, the heatmap will read from the
aggregator instead of directly from the model.

---

## `AnalyticsView`

Today: one widget — a `ContributionHeatmap` with previous/next-year
buttons. Phase 7.9 will add the four `StatsCard`s and five chart
widgets.

```cpp
explicit AnalyticsView(ActivityLogModel* model, QWidget* parent = nullptr);

void loadYear(int year);
int  currentYear() const;

public slots:
void onDataChanged();

private slots:
void onPreviousYear();
void onNextYear();
```

`loadYear(year)`:
1. Asks the model for daily progress totals & activity counts in
   `[YYYY-01-01, YYYY-12-31]`.
2. Bucketises each day's count into intensity 0–4
   (`0 / 1 / 2-3 / 4-6 / 7+`).
3. Builds `QMap<QDate, ContributionHeatmap::DayData>` and pushes via
   `heatmap->setData()`.

---

## `SettingsView`

Today: minimal — DB-path label + Import / Export buttons.
Phase 7.11 expands to Profile / Preferences / Categories / Data
Management / About.

```cpp
explicit SettingsView(QWidget* parent = nullptr);

private slots:
void onImportClicked();
void onExportClicked();

private:
void setupUi();
QString resolveDatabasePath() const;
```

`resolveDatabasePath()` recomputes
`QStandardPaths::AppDataLocation/ctracker.db` so the user sees where
their data lives. Read-only label.

`onImportClicked()` / `onExportClicked()` show a `QFileDialog`, hand
the path to a `DataImporter` / `DataExporter`, and surface success or
failure via `QMessageBox`.

---

# Signal/Slot connection map

A consolidated view of every cross-class signal/slot wire in the
running app today.

| Emitter (signal) | Listener (slot) | Owner of the connection |
|---|---|---|
| `SideNavigationBar::navigationRequested` | `MainWindow::onNavigationRequested` | `MainWindow::setupConnections` |
| `HomeDashboard::courseSelected`          | `MainWindow::onCourseSelected`      | `MainWindow::setupConnections` |
| `HomeDashboard::projectSelected`         | `MainWindow::onProjectSelected`     | `MainWindow::setupConnections` |
| `CourseDetailView::backRequested`        | `MainWindow::onDetailBackRequested` | `MainWindow::setupConnections` |
| `ProjectDetailView::backRequested`       | `MainWindow::onDetailBackRequested` | `MainWindow::setupConnections` |
| `EntityCard::clicked`                    | `HomeDashboard` lambdas → emit `courseSelected` / `projectSelected` | `HomeDashboard::createCard` |
| `DatabaseManager::dataChanged`           | `HomeDashboard::onDataChanged`      | `HomeDashboard::setupUi` |
| `DatabaseManager::dataChanged`           | `EntityDetailView::onDataChanged`   | `EntityDetailView` ctor |
| `DatabaseManager::dataChanged`           | `AnalyticsView::onDataChanged`      | `AnalyticsView` ctor |
| `SessionTaskRow::progressChanged`        | `UnitExpandableWidget::onChildProgressChanged` | `UnitExpandableWidget` per-row |
| `UnitExpandableWidget::sessionTaskProgressChanged` | `EntityDetailView::onSessionProgressChanged` | `EntityDetailView::rebuildUnits` |
| `SessionTaskRow::nameChanged`            | `UnitExpandableWidget::onChildNameChanged` | per-row |
| `UnitExpandableWidget::sessionTaskRenamed` | `EntityDetailView::onSessionRenamed` | per-unit |

---

# File index by line count

| File | LoC |
|---|---|
| `src/core/DatabaseManager.cpp` | 886 |
| `src/courses/EntityDetailView.cpp` | 287 |
| `include/core/DataStructures.h` | 243 |
| `src/analytics/ContributionHeatmap.cpp` | 169 |
| `src/core/DataImporter.cpp` | 156 |
| `src/analytics/ActivityLogModel.cpp` | 155 |
| `src/analytics/AnalyticsView.cpp` | 153 |
| `src/courses/UnitExpandableWidget.cpp` | 149 |
| `src/courses/SessionTaskRow.cpp` | 145 |
| `include/core/DatabaseManager.h` | 138 |
| `src/shared/MainWindow.cpp` | 128 |
| `src/shared/HomeDashboard.cpp` | 125 |
| `src/settings/SettingsView.cpp` | 124 |
| `src/shared/CircularProgressBar.cpp` | 105 |
| `src/courses/EntityCard.cpp` | 84 |
| `src/shared/SideNavigationBar.cpp` | 79 |
| `src/core/DataExporter.cpp` | 77 |
| **Total** | **~3,900 LoC** (incl. all headers) |

---

# How to use this reference

- **Looking up "what does method X do?"** — `Ctrl+F` for the method
  name; every public method is listed under its owner class.
- **Looking up "which class emits signal Y?"** — `Ctrl+F` for the
  signal name, or scan the *Signal/Slot connection map* table.
- **Wondering "where is this used at runtime?"** — the system overview
  HTML's sequence diagrams (slider-move flow, startup) trace the
  most-walked paths.
- **Trying to find which file owns class Z?** — the top-of-document
  index maps class → file.

For *why a method exists*, see the phase-N walk-through docs. This
reference deliberately doesn't repeat that material; it answers *what*
exists today, not *why* the design is shaped this way.
