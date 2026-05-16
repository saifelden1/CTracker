# CTracker Design Document (Compact)

> **Implementation**: Qt 6 + C++17 + QSS. React prototype in `Design/` is visual reference only.
> **Source tree**: Feature-grouped folders under `include/` and `src/`: `core/`, `shared/`, `courses/`, `projects/`, `todos/`, `pomodoro/`, `analytics/`, `calendar/`, `settings/`

## Architecture Overview

**Pattern**: Model-View with Signal-Slot
**Database**: SQLite with DatabaseManager singleton
**Theme**: Dark industrial (#1a1d24 bg, #10b981 primary green)

### Key Components

| Component | Folder | Purpose |
|-----------|--------|---------|
| MainWindow | shared/ | App container, navigation coordinator |
| SideNavigationBar | shared/ | 256px fixed nav bar |
| HomeDashboard | shared/ | Stats cards + entity grid |
| EntityCard | courses/ | 160×180px card with CircularProgressBar |
| CircularProgressBar | shared/ | Custom QPainter arc widget |
| ContributionHeatmap | analytics/ | 53×7 GitHub-style grid |
| UnitExpandableWidget | courses/ | Collapsible unit with sessions |
| SessionTaskRow | courses/ | Name + progress slider (0-100) |
| DatabaseManager | core/ | Singleton SQLite access layer |
| CoursesView | courses/ | Responsive grid with filter bar |
| ProjectsView | projects/ | Project grid with metadata |

## Database Schema

```sql
-- CoursesProjects: Both courses and projects
CREATE TABLE CoursesProjects (
    ID INTEGER PRIMARY KEY,
    Name TEXT NOT NULL,
    Type TEXT CHECK(Type IN ('Course', 'Project')),
    CategoryID INTEGER,  -- FK to Categories, ON DELETE SET NULL
    Status TEXT DEFAULT 'active'  -- 'active' | 'paused' | 'completed'
);

-- Units: Subdivisions of courses/projects
CREATE TABLE Units (
    ID INTEGER PRIMARY KEY,
    ParentID INTEGER NOT NULL,  -- FK to CoursesProjects, CASCADE
    Name TEXT NOT NULL
);

-- SessionsTasks: Terminal leaves with progress
CREATE TABLE SessionsTasks (
    ID INTEGER PRIMARY KEY,
    UnitID INTEGER NOT NULL,  -- FK to Units, CASCADE
    Name TEXT NOT NULL,
    CurrentProgress INTEGER DEFAULT 0 CHECK(CurrentProgress BETWEEN 0 AND 100)
);

-- ActivityLog: Time-series progress changes
CREATE TABLE ActivityLog (
    ID INTEGER PRIMARY KEY,
    ItemID INTEGER NOT NULL,  -- FK to SessionsTasks, CASCADE
    Timestamp TEXT DEFAULT CURRENT_TIMESTAMP,
    OldValue INTEGER,
    NewValue INTEGER,
    ProgressDelta INTEGER,
    Type TEXT CHECK(Type IN ('Course', 'Project'))
);

-- Categories: Color-coded tags
CREATE TABLE Categories (
    ID INTEGER PRIMARY KEY,
    Name TEXT UNIQUE NOT NULL,
    Color TEXT NOT NULL  -- Hex color
);

-- ProjectMeta: Project-specific metadata
CREATE TABLE ProjectMeta (
    ProjectID INTEGER PRIMARY KEY,  -- FK to CoursesProjects
    Description TEXT,
    Priority TEXT DEFAULT 'medium',  -- 'high' | 'medium' | 'low'
    Deadline TEXT,  -- ISO date
    Team TEXT,  -- JSON array
    Links TEXT  -- JSON array of {label, url}
);

-- Todos: Standalone tasks
CREATE TABLE Todos (
    ID INTEGER PRIMARY KEY,
    Title TEXT NOT NULL,
    Completed INTEGER DEFAULT 0,
    Priority TEXT DEFAULT 'medium',
    CreatedAt TEXT DEFAULT CURRENT_TIMESTAMP,
    CompletedAt TEXT
);

-- PomodoroSessions: Completed focus sessions
CREATE TABLE PomodoroSessions (
    ID INTEGER PRIMARY KEY,
    CourseID INTEGER,  -- FK to CoursesProjects, nullable
    DurationMinutes INTEGER NOT NULL,
    CompletedAt TEXT DEFAULT CURRENT_TIMESTAMP,
    Mode TEXT DEFAULT 'work'  -- 'work' | 'break'
);

-- CalendarDayDetails: Per-day notes and todos
CREATE TABLE CalendarDayDetails (
    Date TEXT PRIMARY KEY,  -- ISO date
    ToDo TEXT,  -- JSON array
    Completed TEXT,  -- JSON array
    Notes TEXT
);

-- Settings: Key-value config
CREATE TABLE Settings (
    Key TEXT PRIMARY KEY,
    Value TEXT
);
```

## Data Structures (core/DataStructures.h)

```cpp
struct EntityData {
    int id = -1;
    QString name;
    QString type;  // "Course" | "Project"
    int overallProgress = 0;
    int categoryId = -1;
    QString status = "active";
    QString categoryName;
    QColor categoryColor;
};

struct UnitData {
    int id = -1;
    int parentId = -1;
    QString name;
};

struct SessionTaskData {
    int id = -1;
    int unitId = -1;
    QString name;
    int progress = 0;
};

struct CategoryData {
    int id = -1;
    QString name;
    QColor color;
    int entityCount = 0;
};

struct ProjectMetaData {
    struct Link { QString label; QString url; };
    int projectId = -1;
    QString description;
    QString priority = "medium";
    QDate deadline;
    QStringList team;
    QList<Link> links;
};

struct CourseFilter {
    QString search;
    int categoryId = -1;
    QString status = "all";
};

struct ProjectFilter {
    QString search;
    QString priority = "all";
    QString status = "all";
};
```

## Key Algorithms

### Overall Progress Calculation
```
overallProgress = SUM(all session progress) / COUNT(all sessions)
If no sessions: return 0
```

### Heatmap Intensity (Home page)
```
Bucket daily total into 5 levels:
  0: no activity
  1: 1 item
  2: 2-3 items
  3: 4-6 items
  4: 7+ items
```

### Heatmap Intensity (Analytics page)
```
intensity = floor((dailyTotal / maxDailyTotal) * 4)
Bounded to [0, 4]
```

### Progress Update Flow
```
1. User releases slider
2. SessionTaskRow emits progressChanged(old, new)
3. UnitExpandableWidget → DatabaseManager::updateSessionTaskProgress()
4. Database UPDATE + INSERT ActivityLog
5. Emit dataChanged() signal
6. All views refresh
```

## UI Patterns

### Responsive Grid (CoursesView, ProjectsView)
- < 500px: 1 column
- < 700px: 2 columns
- < 1000px: 3 columns
- ≥ 1000px: 4 columns

### Filter Pattern
- Search input (debounced 200ms)
- Collapsible filter panel
- Active filter badges with × remove
- "Clear all" link

### Progress Controls
- **Circular**: QPainter arc, 0-360°, progress * 3.6 = degrees
- **Slider**: QSlider 0-100, press/release events for old/new values
- **Horizontal**: QProgressBar 0-100

### Empty States
- No entities: "Add your first..." with action button
- No filter results: "No results found" with contextual message

## Design Tokens (for QSS)

```css
/* Colors */
--background: #1a1d24
--elevated: #1f2229
--surface: #252932
--surface-hover: #2d323d
--sidebar: #16181d
--text: #e4e6eb
--text-muted: #9ca3af
--primary: #10b981
--primary-hover: #059669
--border: #2d323d
--success: #10b981
--warning: #f59e0b
--error: #ef4444

/* Spacing (px) */
xs=4, sm=8, md=16, lg=24, xl=32, 2xl=48

/* Radius (px) */
sm=4, md=6, lg=8, xl=12

/* Typography */
Base: 14px, Weights: 400/500/600
```

## Signal/Slot Patterns

### Navigation
```cpp
SideNavigationBar::navigationRequested(int) 
  → MainWindow::onNavigationButtonClicked(int)
  → QStackedWidget::setCurrentIndex(int)
```

### Data Changes
```cpp
DatabaseManager::dataChanged()
  → CoursesView::onDataChanged()
  → refreshCards()
```

### Card Selection
```cpp
EntityCard::clicked(int entityId, EntityType type)
  → CoursesView::onCardClicked(...)
  → emit courseSelected(int) or projectSelected(int)
  → MainWindow wires to detail view navigation
```

### Progress Updates
```cpp
SessionTaskRow: slider released
  → emit progressChanged(old, new)
  → UnitExpandableWidget::onSliderReleased()
  → DatabaseManager::updateSessionTaskProgress()
  → emit dataChanged()
```

## Error Handling

| Error | Response |
|-------|----------|
| DB connection failure | Error dialog + exit(1) |
| Schema corruption | Offer to recreate DB |
| Import parse error | Skip invalid, log warnings, continue |
| Progress update failure | Revert slider, show notification |
| Widget render failure | Catch exception, render placeholder |

## Testing Strategy

1. **Unit Tests**: DatabaseManager CRUD, progress calculations, filter logic
2. **Widget Tests**: CircularProgressBar rendering, slider events, card interactions
3. **Integration Tests**: Full progress update flow, import/export round-trip
4. **Manual Tests**: Responsive layouts, theme consistency, keyboard navigation

---

**For detailed component interfaces, see full design.md. For visual reference, see Design/VISUAL_REFERENCE.md.**
