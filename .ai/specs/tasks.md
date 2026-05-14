# Project Tasks: CTracker
**Stack:** C++17 | Qt 6 (Core/Widgets/Sql/Charts/Svg) | SQLite | CMake ≥ 3.20
**Architecture:** Model–View | Signal-Slot | Dark Industrial Theme | Feature-grouped source tree

> **UI Strategy (hybrid):**
> - **Qt Designer (`.ui` files):** static layouts only — `MainWindow` frame, `SettingsView`, `AnalyticsView` skeleton.
> - **Pure C++:** every custom widget (`CircularProgressBar`, `ContributionHeatmap`, `CalendarWidget`, `PomodoroTimerWidget`, …).
>
> **Learning approach:** every task is explained before it is written. No black boxes.
>
> **Visual reference vs. implementation (non-negotiable):**
> - The `Design/` folder is a **React + TypeScript + Tailwind** prototype used **only** as a visual/interaction reference.
> - **Every task below is implemented in Qt 6 / C++17 / QSS.** No JSX/Tailwind/Radix/Recharts code is ported; their Qt-native equivalents are used.
> - SVG icons exported from the design source are reused; everything else is re-implemented in C++.
>
> **Plan shape:** this file replaces the old "Phase 1–9 + Expansion" split. The two passes are merged into a single linear sequence so each component is built once, in its final v2 form. Tasks already finished against the old plan are kept ticked.

---

## Phase 0: Source-Tree Reorganization (feature-grouped layout)

Goal: move every existing `.h`/`.cpp` from the flat `include/` and `src/` into feature folders, so future widgets/views land in the right place from day one.

### Task 0.1: Create feature folders
- [ ] `include/core/`, `include/shared/`, `include/courses/`, `include/projects/`, `include/todos/`, `include/pomodoro/`, `include/analytics/`, `include/calendar/`, `include/settings/`
- [ ] `src/` mirrors the same set 1:1
- [ ] Add a `.gitkeep` to any folder that is initially empty (todos, pomodoro, calendar, settings)

### Task 0.2: Move existing files
| File | New folder |
|------|------------|
| DataStructures.h, DatabaseManager.h/.cpp, DataImporter.h/.cpp, DataExporter.h/.cpp | `core/` |
| CircularProgressBar.h/.cpp, MainWindow.h/.cpp, SideNavigationBar.h/.cpp | `shared/` |
| EntityCard.h/.cpp, UnitExpandableWidget.h/.cpp, SessionTaskRow.h/.cpp, EntityDetailView.h/.cpp, CourseDetailView.h (alias header) | `courses/` |
| ProjectDetailView.h (alias header) | `projects/` |
| ContributionHeatmap.h/.cpp, ActivityLogModel.h/.cpp, AnalyticsView.h/.cpp | `analytics/` |
| HomeDashboard.h/.cpp | `shared/` (it becomes the dashboard hub) |
| SettingsView.h/.cpp | `settings/` |
| main.cpp | stays under `src/` root |

- [ ] Move each file with `git mv` so blame history is preserved
- [ ] Replace `#include "FooBar.h"` with `#include "feature/FooBar.h"` everywhere

### Task 0.3: Update CMakeLists.txt
- [ ] Rewrite the `SOURCES` / `HEADERS` lists to reflect the new folder paths
- [ ] Keep `include/` as the single include root so files reference each other as `core/DatabaseManager.h`
- [ ] Verify `cmake --build build` succeeds from a clean configure

### Task 0.4: Spec write-back
- [ ] Update `.ai/specs/design.md` "Components and Interfaces" to show each component's owning folder
- [ ] Note the convention rule in the design doc: *one folder per top-level feature; `core/` for data layer; `shared/` for cross-feature widgets and chrome*

---

## Phase 1: Project Initialization & Build System (already complete)

### Task 1.1: Repository & Directory Structure — DONE
- [x] All directories created and committed

### Task 1.2: CMake Build System — DONE (will be extended in Task 8.4)
- [x] Base CMake setup with Qt6 Core/Widgets/Sql

### Task 1.3: Qt Resource File — DONE
- [x] `resources/resources.qrc` registered

---

## Phase 2: Database Layer (`core/DatabaseManager`) — v2 schema in one shot

> Rationale for the merge: every Phase-7 view (HomeDashboard stats row, paused-course filter, TodoView, PomodoroView, CalendarDayDetails) depends on the v2 tables. Building v1-only and migrating later would force two passes over `DatabaseManager`.

### Task 2.1: Singleton Setup — DONE
- [x] `core/DatabaseManager.h` with `static instance()` and private ctor
- [x] `initialize()`, `isOpen()`, `close()`
- [x] `m_database` member
- [x] Implementation opens/creates `ctracker.db` in `QStandardPaths::AppLocalDataLocation`

### Task 2.2: v1 Schema (CoursesProjects / Units / SessionsTasks / ActivityLog) — DONE
- [x] All four tables created with `IF NOT EXISTS`
- [x] `PRAGMA foreign_keys = ON`
- [x] CASCADE DELETE on every FK
- [x] `CurrentProgress` `CHECK` constraint

### Task 2.3: Query helpers + transactions — DONE
- [x] `executeQuery`, `executeSelectQuery` with named placeholders
- [x] `databaseError(QString)` signal
- [x] `beginTransaction`/`commitTransaction`/`rollbackTransaction`

### Task 2.4 → 2.7: Entity / Unit / Session-Task / ActivityLog CRUD — DONE
- [x] All v1 CRUD methods implemented and emitting `dataChanged()`

### Task 2.8: Schema v2 — Migration & versioning
- [ ] Create `SchemaInfo(Key TEXT PRIMARY KEY, Value TEXT NOT NULL)` on `initialize()` if missing
- [ ] Seed `('schema_version','1')` on existing v1 dbs; seed `('schema_version','2')` on fresh installs
- [ ] Implement `int currentSchemaVersion()`
- [ ] Implement `bool migrate(int from, int to)` dispatching ordered migrations inside a single transaction
- [ ] Migration `1 → 2` is idempotent: column-exists checks before each `ALTER`

### Task 2.9: New v2 tables (created idempotently)
- [ ] `Categories(ID, Name UNIQUE, Color TEXT NOT NULL, CreatedAt)`
- [ ] `ProjectMeta(ProjectID PK → CoursesProjects, Description, Priority CHECK IN ('high','medium','low'), Deadline, TeamJson, LinksJson)` — `ON DELETE CASCADE`
- [ ] `Todos(ID, Title, Completed INT, Priority, CreatedAt, CompletedAt)`
- [ ] `PomodoroSessions(ID, CourseID NULL → CoursesProjects ON DELETE SET NULL, DurationMinutes, CompletedAt, Mode CHECK IN ('work','break'))`
- [ ] `CalendarDayDetails(Date TEXT PRIMARY KEY, TodoJson, CompletedJson, Notes)`
- [ ] `Settings(Key TEXT PRIMARY KEY, Value TEXT NOT NULL)`
- [ ] Indexes: `idx_activitylog_date`, `idx_pomodoro_date`, `idx_todos_completed`

### Task 2.10: `CoursesProjects` column additions
- [ ] `CategoryID INTEGER NULL REFERENCES Categories(ID) ON DELETE SET NULL`
- [ ] `Status TEXT NOT NULL DEFAULT 'active' CHECK(Status IN ('active','paused','completed'))`
- [ ] Both guarded by a `pragma_table_info`-driven column-exists check

### Task 2.11: Seed defaults
- [ ] Five default `Categories`: Algorithms `#10b981`, Web Development `#3b82f6`, Machine Learning `#8b5cf6`, Systems `#f59e0b`, Security `#ec4899`
- [ ] Default `Settings`: `pomodoro.workMinutes=25`, `pomodoro.breakMinutes=5`, `notifications.enabled=1`, `sound.enabled=1`, `courses.autoPauseDays=30`

---

## Phase 3: Data Structures (`core/DataStructures.h`)

> All shared POD structs live in one header so any feature folder can include it without back-references.

### Task 3.1: v1 structs — DONE
- [x] `EntityData`, `UnitData`, `SessionTaskData`, `ActivityLogEntry`, `HeatmapDataPoint`

### Task 3.2: Extend `EntityData` for v2
- [ ] Add fields: `int categoryId = -1`, `QString status = "active"`, `QString categoryName`, `QColor categoryColor`
- [ ] Update every `SELECT *` building an `EntityData` to populate these (LEFT JOIN to `Categories`)
- [ ] Update the spec's "Component 4: EntityCard" interface to show the new fields

### Task 3.3: New v2 structs
- [ ] `struct CategoryData { int id; QString name; QColor color; int entityCount = 0; }`
- [ ] `struct ProjectMetaData::Link { QString label; QString url; }`
- [ ] `struct ProjectMetaData { int projectId; QString description; QString priority; QDate deadline; QStringList team; QList<Link> links; }`
- [ ] `struct TodoData { int id; QString title; bool completed; QString priority; QDateTime createdAt; QDateTime completedAt; }`
- [ ] `struct PomodoroSessionData { int id; int courseId; QString courseName; int durationMinutes; QDateTime completedAt; QString mode; }`
- [ ] `struct CalendarDayData { QDate date; QStringList todo; QStringList completed; QString notes; bool hasContent() const; }`
- [ ] `struct AnalyticsSummary { int currentStreakDays; int longestStreakDays; int monthHoursStudied; double avgSessionsPerDay7d; double weekOverWeekPct; }`

### Task 3.4: NEW — Filter / state structs (gap I found in the design)
- [ ] `struct CourseFilter { QString search; int categoryId = -1; QString status = "all"; }` — used by `courses/CoursesFilterBar` and any future model adapter (promoted out of the widget so models can share it)
- [ ] `struct ProjectFilter { QString search; QString priority = "all"; QString status = "all"; }` — analogue for `projects/ProjectsView`
- [ ] `struct PomodoroTimerState { enum Mode { Work, Break }; enum State { Idle, Running, Paused }; Mode mode = Work; State state = Idle; int courseId = -1; int totalSeconds = 25*60; int remainingSeconds = 25*60; QDateTime startedAt; }` — lets `PomodoroView` persist timer state through navigation and restore on app restart (currently the React design re-creates timer state in-memory only; we want better)
- [ ] `struct ProfileData { QString name; QString email; QString goals; }`
- [ ] `struct PreferencesData { int workMinutes; int breakMinutes; bool notifications; bool sound; int autoPauseDays; }` — typed wrapper over the `Settings` k/v table consumed by `settings/SettingsView`

### Task 3.5: Spec write-back
- [ ] Mirror Tasks 3.2–3.4 into `design.md` "Expansion: New Data Structures" section, adding the four new structs flagged above

---

## Phase 4: Extended `DatabaseManager` API

### Task 4.1: v1 API surface — DONE
- [x] Entity / Unit / Session-Task / ActivityLog CRUD

### Task 4.2: Category CRUD
- [ ] `int addCategory(const QString& name, const QColor& color)`
- [ ] `bool renameCategory(int id, const QString& newName)`
- [ ] `bool setCategoryColor(int id, const QColor& color)`
- [ ] `bool removeCategory(int id)` — sets `CategoryID = NULL` on dependent entities
- [ ] `QList<CategoryData> fetchAllCategories()` — `LEFT JOIN CoursesProjects` to compute `entityCount`
- [ ] `bool assignCategory(int entityId, int categoryId)` (`categoryId = -1` clears)

### Task 4.3: Course status (active/paused/completed)
- [ ] `bool setCourseStatus(int courseId, const QString& status)`
- [ ] `QString getCourseStatus(int courseId)`
- [ ] `fetchAllCourses()`/`fetchAllProjects()` updated to include `status`, `categoryId`, `categoryName`, `categoryColor` via `LEFT JOIN Categories`

### Task 4.4: ProjectMeta CRUD
- [ ] `bool upsertProjectMeta(const ProjectMetaData& meta)`
- [ ] `ProjectMetaData getProjectMeta(int projectId)` — returns defaults if no row exists
- [ ] Convenience setters: `setProjectPriority`, `setProjectDeadline`, `setProjectTeam`, `setProjectLinks`

### Task 4.5: Todo CRUD
- [ ] `int addTodo(const QString& title, const QString& priority)`
- [ ] `bool toggleTodoCompleted(int id)`
- [ ] `bool setTodoPriority(int id, const QString& priority)`
- [ ] `bool removeTodo(int id)`
- [ ] `QList<TodoData> fetchActiveTodos()` / `fetchCompletedTodos()`
- [ ] `int countCompletedTodosOn(const QDate& date)`

### Task 4.6: Pomodoro Sessions
- [ ] `int insertPomodoroSession(int courseId, int durationMin, const QString& mode)`
- [ ] `QList<PomodoroSessionData> fetchRecentSessions(int limit)`
- [ ] `QList<PomodoroSessionData> fetchSessionsOn(const QDate& date)`
- [ ] `int totalMinutesOn(const QDate& date)`

### Task 4.7: Calendar Day Details
- [ ] `CalendarDayData getDay(const QDate& date)` — empty struct if missing
- [ ] `bool upsertDay(const CalendarDayData& data)`
- [ ] `QSet<QDate> datesWithContent(const QDate& from, const QDate& to)`

### Task 4.8: Settings k/v + typed accessors
- [ ] `QString getSetting(const QString& key, const QString& defaultValue = {})`
- [ ] `bool setSetting(const QString& key, const QString& value)`
- [ ] Convenience: `int getSettingInt`, `bool getSettingBool`
- [ ] Typed wrappers: `ProfileData getProfile()` / `bool setProfile(const ProfileData&)`; same for `PreferencesData`

### Task 4.9: Persisted Pomodoro timer state (NEW — gap-fill)
- [ ] `PomodoroTimerState getPomodoroState()` (reads from `Settings` via reserved keys `pomodoro.state.*`)
- [ ] `bool savePomodoroState(const PomodoroTimerState&)`
- [ ] Cleared on `reset()` and on entity deletion that nukes `CourseID`

---

## Phase 5: Models (`analytics/`, `todos/`, `shared/`)

### Task 5.1: ActivityLogModel (`analytics/`) — DONE
- [x] Inherits `QAbstractTableModel`, columns `Timestamp / ItemName / Type / ProgressChange`
- [x] `setFilterDateRange`, `setFilterItemType`, `clearFilters`, `refresh`
- [x] `getDailyProgressTotals`, `getDailyActivityCounts`

### Task 5.2: DataImporter — DONE
- [x] JSON parse + atomic transaction + clamp progress to [0,100]

### Task 5.3: DataExporter — DONE
- [x] JSON export round-trips with importer

### Task 5.4: CategoryModel (`shared/`)
- [ ] `QAbstractListModel` exposing `(id, name, color, entityCount)` via roles
- [ ] `refresh()` slot wired to `DatabaseManager::dataChanged`

### Task 5.5: TodoModel (`todos/`)
- [ ] Two filtered views (active vs. completed) from a single underlying list
- [ ] Signals: `activeCountChanged(int)`, `completedCountChanged(int)`
- [ ] Auto-refresh on `dataChanged`

### Task 5.6: HeatmapAggregator (`analytics/`)
- [ ] `QMap<QDate, ContributionHeatmap::DayData> aggregate(from, to, Mode)`
- [ ] `Mode::RecentBuckets`: `count = #ActivityLog + #completed Todos + #completed Pomodoro`; intensity bucketed `0 / 1 / 2-3 / 4-6 / 7+`
- [ ] `Mode::NormalizedRange`: `intensity = floor((count / max) * 4)` bounded `[0,4]`
- [ ] Re-aggregates on `dataChanged` from ActivityLog / Todos / PomodoroSessions

### Task 5.7: AnalyticsSummary computer (`analytics/`)
- [ ] Free function (or static method) producing an `AnalyticsSummary` from `DatabaseManager` queries
- [ ] Implements `dayStreak()`, `monthHoursStudied()`, `avgSessionsPerDay(7)`, `weekOverWeekPct()`

---

## Phase 6: Custom Widgets

### Task 6.1: CircularProgressBar (`shared/`) — DONE
- [x] Custom-painted ring, Q_PROPERTYs, `setProgress` clamp + change-suppression

### Task 6.2: ContributionHeatmap (`analytics/`) — DONE
- [x] 53×7 grid, hover tooltip, year navigation, GitHub green ramp

### Task 6.3: SessionTaskRow (`courses/`) — DONE
- [x] `QStackedWidget` label↔edit swap, slider press/release semantics

### Task 6.4: UnitExpandableWidget (`courses/`) — DONE
- [x] Expand/collapse header, child `SessionTaskRow` management, overall-progress computation

### Task 6.5: EntityCard (`courses/`) — DONE for v1; needs v2 additions
- [x] CircularProgressBar + name + type badge, fixed 160×180 px
- [ ] **v2 addition:** embedded `CategoryPill` slot (top-left) — show only when `categoryId >= 0`
- [ ] **v2 addition:** "Paused" status badge (top-right) when `status == "paused"`, muted color
- [ ] Spec write-back: add the two new sub-elements to design.md "Component 4: EntityCard"

### Task 6.6: StatsCard (`shared/`)
- [ ] `QFrame` with icon (top-right) + title + big value + subtitle + optional badge
- [ ] `lg` border-radius, surface background, hover shadow
- [ ] API: `setTitle`, `setValue`, `setSubtitle`, `setIcon`, `setBadgeText`

### Task 6.7: CategoryPill (`shared/`)
- [ ] Small horizontal pill: 8 px colored dot + name
- [ ] Background = category color at 15% alpha; `sm` radius
- [ ] `setCategory(const CategoryData&)` / `clearCategory()`

### Task 6.8: CalendarWidget (`calendar/`)
- [ ] Inherit `QWidget` (do **not** use `QCalendarWidget` — we need custom indicators)
- [ ] Header `◀ Month Year ▶`, 7×6 grid with single-letter day labels (S M T W T F S)
- [ ] Current day highlighted in primary accent ring
- [ ] Indicator dot under every date in `m_indicatorDates`
- [ ] Signals: `dateClicked(QDate)`, `monthChanged(int year, int month)`
- [ ] Keyboard arrow navigation

### Task 6.9: DayDetailsPanel (`calendar/`)
- [ ] Header: selected date + close `×` button → emits `closed()`
- [ ] Three stacked sections: To Do (list + add input), Completed (strikethrough list), Notes (`QTextEdit`)
- [ ] Empty state when `clear()` is called
- [ ] Signals: `todoAdded(date, text)`, `todoToggled(date, idx, completed)`, `notesChanged(date, text)`

### Task 6.10: TodoRow (`todos/`)
- [ ] Layout: checkbox | title | priority badge | trash button
- [ ] Completed state = 60% opacity + strikethrough; hover bg = surface-hover
- [ ] Signals: `completedToggled(int id, bool)`, `deleteRequested(int id)`

### Task 6.11: PomodoroTimerWidget (`pomodoro/`)
- [ ] Embeds `CircularProgressBar` sized 256 px
- [ ] Drives a `QTimer` at 1 Hz; computes `remainingSeconds` and the ring %
- [ ] Center label shows `MM:SS`
- [ ] API: `setMode`, `setWorkDurationMinutes`, `setBreakDurationMinutes`, `start`, `pause`, `resume`, `reset`
- [ ] Holds a `PomodoroTimerState` mirror in memory; emits `tick`, `stateChanged`, `completed`, `modeChanged`
- [ ] On `completed`, emits + auto-switches mode + resets
- [ ] Restores from `DatabaseManager::getPomodoroState()` on construction (NEW — enables cross-session resume)

### Task 6.12: ProjectCard (`projects/`)
- [ ] Layout: name (semibold) + description (2-line truncate) + priority badge + deadline badge + horizontal progress bar + "X/Y tasks" + team-icon + member count
- [ ] `setDeadline()` computes days-left and applies red ≤ 3 d / amber ≤ 7 d / gray otherwise
- [ ] `mousePressEvent` emits `clicked(int projectId)`

### Task 6.13: CoursesFilterBar (`courses/`)
- [ ] Layout: search `QLineEdit` (left) | Filter toggle button + "Add New" button (right)
- [ ] Collapsible filter panel: Category dropdown + Status dropdown
- [ ] Active-filter badges with `×` to remove + "Clear all" link
- [ ] Emits `filterChanged(CourseFilter)` (debounced 200 ms on search edits) and `addNewRequested()`

---

## Phase 7: Views & Navigation

### Task 7.1: SideNavigationBar (`shared/`)
- [ ] 256 px fixed width, header (CTracker logo + name), 7 nav buttons (Home, Courses, Projects, To-Do, Pomodoro, Analytics, Settings), footer (user profile chip)
- [ ] `setActiveButton(int)`, `navigationRequested(int)` signal
- [ ] Active button uses left accent border in primary green

### Task 7.2: HomeDashboard (`shared/`) — final v2 form (no v1 throwaway)
- [ ] Top row: 3 × `StatsCard` (Active Courses w/ paused badge, Projects w/ due-soon sub-count, Completion Rate)
- [ ] Lower section: `CalendarWidget` (left) + stacked `DayDetailsPanel` & `ContributionHeatmap` (right)
- [ ] Heatmap configured in `RecentBuckets` mode for last 12 weeks (84 days)
- [ ] Calendar `dateClicked` → `DatabaseManager::getDay` → `DayDetailsPanel::showDay`
- [ ] Subscribes to `dataChanged` for live refresh of every section

### Task 7.3: CoursesView (`courses/`)
- [ ] Embeds `CoursesFilterBar` at top
- [ ] Responsive `QGridLayout` of `EntityCard` widgets (1-4 cols based on width)
- [ ] Applies `CourseFilter` (search/category/status) in real time
- [ ] EmptyState widget when no results
- [ ] "Add New" → `EntityCreateDialog` (Task 7.10) → on accept, navigate to detail view

### Task 7.4: ProjectsView (`projects/`)
- [ ] Same filter pattern but with `ProjectFilter` and `ProjectCard` grid
- [ ] Fetch via `fetchAllProjects()` + `getProjectMeta` per project (consider batching to one JOIN later)
- [ ] "Add New" → `EntityCreateDialog` in project-only mode

### Task 7.5: CourseDetailView (`courses/`) — supersedes the old skeletal `EntityDetailView`
- [ ] Title bar: entity name + Pause/Resume toggle (binds to `setCourseStatus`) + Add/Delete unit buttons + overall progress `CircularProgressBar`
- [ ] `QScrollArea` of `UnitExpandableWidget` items
- [ ] `loadCourse(int courseId)` fetches units/sessions, populates
- [ ] Add Unit → `QInputDialog` → `addUnit`
- [ ] Add Session-Task within a unit → `addSessionTask`
- [ ] Wire `UnitExpandableWidget::sessionTaskProgressChanged` → `updateSessionTaskProgress`
- [ ] Back button → emits `backRequested()`

### Task 7.6: ProjectDetailView (`projects/`)
- [ ] Mirrors `CourseDetailView` shell
- [ ] Left column: task checklist; right column: project info (deadline, team, links opening in default browser via `QDesktopServices::openUrl`)
- [ ] Status / priority / deadline badges in header
- [ ] Shares a base class with `CourseDetailView` for the common scaffolding (optional refactor)

### Task 7.7: TodoView (`todos/`)
- [ ] Header: title + 2 stat boxes (active count, completed count)
- [ ] Add-task input with Enter-to-submit and `+` button
- [ ] Two sections (Active / Completed) of `TodoRow`s
- [ ] Within each section: priority high → medium → low

### Task 7.8: PomodoroView (`pomodoro/`)
- [ ] Two-column layout
- [ ] **Left:** mode toggle (Work / Break) + `PomodoroTimerWidget` + Settings card (course dropdown, work duration {15/20/25/30/45/50}, break duration {5/10/15})
- [ ] **Right:** Today's Progress card (sessions + minutes from `totalMinutesOn(today)`) + Recent Sessions card (last 5)
- [ ] On `completed(Work)` → `insertPomodoroSession` + auto-switch + reset (handled inside the widget; view persists the session row)
- [ ] Settings widgets disabled while timer is running
- [ ] On view construct, restore `PomodoroTimerState` from DB so cross-navigation/cross-launch resume works

### Task 7.9: AnalyticsView (`analytics/`) — final v2 form (no v1 throwaway)
- [ ] Top row: 4 × `StatsCard` (Day Streak, Total Hours, Avg Sessions/Day, Week Comparison)
- [ ] 5 chart widgets via `QChartView`:
  - Progress over time (`QLineSeries`, 8 weeks)
  - Study hours/week (`QBarSeries`, 8 weeks)
  - Course progress breakdown (custom horizontal-bar list using `CategoryData::color`)
  - Time distribution (`QPieSeries` over Pomodoro minutes per course)
  - Weekly activity pattern (`QBarSeries`, Mon-Sun)
- [ ] Dark-themed `QChart` palette, custom tooltips, `NoRubberBand`, antialiased
- [ ] `ContributionHeatmap` repositioned below charts in `NormalizedRange` mode, configurable year nav

### Task 7.10: EntityCreateDialog (`shared/`)
- [ ] Modes: Course-or-Project (from `CoursesView`) vs. Project-only (from `ProjectsView`)
- [ ] Validates Name not empty/whitespace → enables OK
- [ ] On accept, calls `addCourse`/`addProject` and (for projects) `upsertProjectMeta` with description/priority/deadline

### Task 7.11: SettingsView (`settings/`)
- [ ] Replace the minimal v1 view with 5 cards: Profile, Preferences, Categories, Data Management, About
- [ ] **Profile:** Name, Email, Study Goals — bound via `getProfile/setProfile`
- [ ] **Preferences:** pomodoro durations, notifications, sound, autoPauseDays — via `getPreferences/setPreferences`
- [ ] **Categories:** list with edit/add + color picker dialog; uses `CategoryModel`
- [ ] **Data Management:** Export, Import, Clear All Data (destructive red, confirmation dialog)
- [ ] **About:** version, build, license

### Task 7.12: MainWindow (`shared/`)
- [ ] `QStackedWidget` with all 7 pages in order: Home (0), Courses (1), Projects (2), To-Do (3), Pomodoro (4), Analytics (5), Settings (6)
- [ ] Plus 2 hidden detail pages (CourseDetail, ProjectDetail) reached via card clicks
- [ ] Wire `SideNavigationBar::navigationRequested` → `setCurrentIndex`
- [ ] Wire `HomeDashboard::courseSelected` → `CourseDetailView::loadCourse` → switch view
- [ ] Same for `projectSelected`
- [ ] `loadStyleSheet(":/styles/dark-industrial.qss")`
- [ ] Min size 1280×800; persist geometry via `QSettings`

---

## Phase 8: Styling, Assets & Application Wiring

### Task 8.1: Base theme (`assets/styles/dark-industrial.qss`)
- [ ] Apply canonical palette tokens (matching CLAUDE.md §5 and Req 15.5):
  - Background `#1a1d24` / elevated `#1f2229` / surface `#252932` / surface-hover `#2d323d`
  - Sidebar `#16181d`; text `#e4e6eb` / muted `#9ca3af` / subtle `#6b7280`
  - Primary accent `#10b981` / hover `#059669` / muted `#064e3b`
  - Borders `#2d323d` / strong `#404854`
  - Status: success `#10b981`, warning `#f59e0b`, error `#ef4444`, info `#3b82f6`
- [ ] Spacing constants used inline (QSS has no `var`): xs=4, sm=8, md=16, lg=24, xl=32, 2xl=48 px
- [ ] Border-radius constants: sm=4, md=6, lg=8, xl=12 px
- [ ] Base font 14 px, weights 400/500/600

### Task 8.2: Component-specific styling
- [ ] `EntityCard` (lg radius, surface bg, hover border in accent)
- [ ] `StatsCard` (lg radius, surface bg, hover shadow)
- [ ] `CategoryPill` (15% alpha bg, sm radius)
- [ ] Priority badges (high=red, medium=amber, low=gray)
- [ ] Deadline badges (mapping per Task 6.12)
- [ ] Pomodoro mode toggle (active=primary, inactive=muted)
- [ ] `QSlider` groove/handle, `QPushButton` hover/pressed, chart container frames

### Task 8.3: Typography
- [ ] Load Inter (or Roboto) via `QFontDatabase::addApplicationFont`
- [ ] Apply via `QApplication::setFont`

### Task 8.4: Charts & SVG — CMake + assets
- [ ] Add `Qt6::Charts` and `Qt6::Svg` to `find_package` + `target_link_libraries`
- [ ] Export Lucide-style SVG icons (`home`, `book-open`, `folder-kanban`, `check-square`, `timer`, `bar-chart-3`, `settings`, `plus`, `search`, `filter`, `x`, `chevron-left/right`, …) to `assets/icons/lucide/`
- [ ] Register in `resources/resources.qrc`
- [ ] Add every new header/source from Phases 5–7 to `SOURCES`/`HEADERS`

### Task 8.5: main.cpp + error handling
- [ ] (existing) `QApplication`, app name/version, `DatabaseManager::initialize` + exit-1 on failure, show `MainWindow`, `app.exec()`, close DB on exit
- [ ] Connect `DatabaseManager::databaseError(QString)` to a global `QMessageBox::critical`
- [ ] On `updateSessionTaskProgress` DB failure, signal back to revert the slider
- [ ] `DataImporter` parse errors → `qWarning()` + skip + continue

---

## Phase 9: Testing

### Task 9.1: DatabaseManager v1 tests
- [ ] `initialize()` creates all base tables
- [ ] `addCourse` / `addProject` return valid IDs
- [ ] `addUnit` fails on invalid parent
- [ ] `updateSessionTaskProgress` out-of-range is rejected
- [ ] Cascade delete: deleting a Course removes Units & Sessions
- [ ] `logActivity` is NOT called when `oldValue == newValue`

### Task 9.2: DatabaseManager v2 tests
- [ ] Fresh DB ends at `schema_version='2'`
- [ ] v1 sample DB migrates to v2 without data loss
- [ ] `migrate()` is idempotent (running twice produces no errors)
- [ ] `removeCategory` nulls dependent entities without deleting them
- [ ] `setCourseStatus('paused')` excludes the course from `fetchAllCourses(activeOnly=true)`
- [ ] `getPomodoroState` round-trips a saved state

### Task 9.3: Model tests
- [ ] `ActivityLogModel::getDailyProgressTotals` correct sums
- [ ] `DataImporter` rejects missing `type`; clamps progress > 100
- [ ] `DataExporter` produces valid JSON that re-imports
- [ ] `HeatmapAggregator::RecentBuckets` produces correct buckets for known counts
- [ ] `TodoModel` separates active vs completed correctly
- [ ] `CategoryModel::entityCount` matches actual DB join

### Task 9.4: Widget tests
- [ ] `CircularProgressBar::setProgress(-1)` → 0; `setProgress(150)` → 100
- [ ] `UnitExpandableWidget::calculateOverallProgress()` returns 0 with no children
- [ ] `PomodoroTimerWidget` ticks down once per second (use `QSignalSpy` + 3 s wait)
- [ ] Auto-switch from Work → Break inserts a session row
- [ ] Pause + resume preserves remaining time

---

# Implementation Order Summary

1. **Phase 0** — folder reorg + CMake update.
2. **Phase 2.8–2.11** + **Phase 3.2–3.4** + **Phase 4.2–4.9** — schema v2, new structs, new DB API.
3. **Phase 5.4–5.7** — new models (Category, Todo, Heatmap, AnalyticsSummary).
4. **Phase 6.5(v2 add-ons)–6.13** — remaining custom widgets.
5. **Phase 7** — every view built once in its final form.
6. **Phase 8** — styling, charts/svg CMake, error wiring.
7. **Phase 9** — full test pass.
