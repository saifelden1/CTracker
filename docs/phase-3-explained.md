# Phase 3 — Data Structures, Explained

> **Audience.** Same template as the earlier docs: written so a student
> can follow each decision end-to-end and replicate the work themselves.
>
> **What Phase 3 is.** Define the C++ POD structs that everything else
> in the project will pass around — DB → models → views. *No behaviour,
> no business logic, no I/O.* These structs are the project's shared
> vocabulary. After this phase, every later component knows what shape
> a "Course", a "Todo", a "Pomodoro session" has, and can implement
> against that shape without re-deriving it.
>
> **One file, one responsibility.** Every struct lives in
> `include/core/DataStructures.h`. That single header is included
> downstream by every feature folder. No struct ever escapes into a
> feature-specific header where another feature would have to depend
> on `courses/SomethingPrivate.h` to get at a Pomodoro shape — that
> would create back-references between sibling features, which the
> project's folder convention is meant to prevent.

---

## 0. State before Phase 3 started

The audit at the top of this session showed:

| Task | Status going in | Evidence |
|---|---|---|
| 3.1 v1 structs (`EntityData`, `UnitData`, `SessionTaskData`, `ActivityLogEntry`, `HeatmapDataPoint`) | ✅ already done | `DataStructures.h` lines 18–58 (the original file) |
| 3.2 Extend `EntityData` with 4 v2 fields | ❌ pending | header had no `categoryId`/`status`/`categoryName`/`categoryColor`; `rowToEntity` didn't read them |
| 3.3 Seven v2 structs | ❌ pending | none of `CategoryData`/`ProjectMetaData`/`TodoData`/`PomodoroSessionData`/`CalendarDayData`/`AnalyticsSummary` existed in code |
| 3.4 Five filter/state structs | ❌ pending | none of `CourseFilter`/`ProjectFilter`/`PomodoroTimerState`/`ProfileData`/`PreferencesData` existed in code |
| 3.5 Spec write-back | ✅ already done | `design.md` already carried the full struct catalogue at lines 1858–1953 |

So the work for this phase was: extend one existing struct, add eleven
new structs, update three SQL fetchers to populate the extended fields,
verify the build, tick the boxes.

---

## 1. What was already there (Phase 3.1 — recap)

The v1 catalogue, exactly as it shipped in the first three commits:

```cpp
struct EntityData {       // a Course or a Project
    int id; QString name; QString type; QDateTime createdAt;
    int overallProgress = 0;   // derived, not stored
};
struct UnitData       { int id; int parentId; QString name; };
struct SessionTaskData{ int id; int unitId; QString name; int progress; };
struct ActivityLogEntry { int id; int itemId; QDateTime timestamp;
                          int oldValue; int newValue; int progressDelta;
                          QString type; };
struct HeatmapDataPoint { QDate date; int totalProgress; int activityCount;
                          int intensityLevel; };
```

Two patterns worth pointing out before we add to them:

1. **Plain C++, no `QObject`, no virtuals.** Structs are values. You
   can put them in a `QList`, copy them, return them by value, store
   them as members. The cheap copy is a deliberate choice — it lets
   any layer that needs to subscribe to "the data right now" snapshot
   it without lifetime management gymnastics.
2. **Every "FK to parent" is captured as an `int id` member named after
   the parent.** `parentId` in `UnitData` is `CoursesProjects.ID`;
   `unitId` in `SessionTaskData` is `Units.ID`. No pointers between
   structs — relationships are owned by the database, not the in-memory
   graph. A struct never holds a dangling reference because it never
   holds a reference at all.

---

## 2. Task 3.2 — Extending `EntityData` for v2

### Why

Phase 2 added two new columns to `CoursesProjects`:

- `CategoryID INTEGER NULL REFERENCES Categories(ID) ON DELETE SET NULL`
- `Status TEXT NOT NULL DEFAULT 'active' CHECK(Status IN ('active','paused','completed'))`

Plus a whole `Categories` table. Every UI that shows a course or
project (the EntityCard, the courses grid, the filter bar) needs to
know **the category's name and colour**, not just its numeric ID.
Forcing each view to issue a follow-up `SELECT name, color FROM Categories
WHERE id = ?` would mean N+1 queries per render — bad.

The cure: **resolve the join inside the data layer** so each
`EntityData` arrives with its category pre-attached.

### The new fields

```cpp
struct EntityData {
    int       id              = -1;
    QString   name;
    QString   type;
    QDateTime createdAt;
    int       overallProgress = 0;

    // v2
    int       categoryId      = -1;       // -1 = uncategorised
    QString   status          = "active"; // matches DB CHECK
    QString   categoryName;               // empty when categoryId == -1
    QColor    categoryColor;              // invalid() when categoryId == -1
};
```

Every field defaults to a sane "empty" value:

- `id = -1` is the project-wide convention for "no record".
- `status = "active"` matches the column's `DEFAULT 'active'` — so a
  default-constructed `EntityData` is shaped like a brand-new row.
- `categoryId = -1`, `categoryName = ""`, `QColor()` (which is
  `isValid() == false`) — three coherent signals that this entity has
  no category attached.

The UI's branching becomes one check:

```cpp
if (entity.categoryId >= 0) {
    // show the CategoryPill (Task 6.7) with entity.categoryName + entity.categoryColor
}
```

### The new include: `<QColor>`

`QColor` lives in **`Qt6::Gui`**, not `Qt6::Core`. We added the include
to the header and made the `Gui` module *explicit* in both
`CMakeLists.txt`:

```cmake
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql Test)
…
target_link_libraries(CTracker PRIVATE
    Qt6::Core
    Qt6::Gui          # QColor and friends — pulled in by core/DataStructures.h
    Qt6::Widgets
    Qt6::Sql
)
```

Why bother — wasn't `Qt6::Widgets` already pulling `Qt6::Gui` in
transitively? Yes. But making it explicit:

- Survives a future refactor where someone removes `Qt6::Widgets` from
  a non-GUI target (e.g. the test target, which only needs Core/Sql/Test
  + Gui for `QColor`).
- Documents intent for the next reader.
- Is free.

We also added `Qt6::Gui` to `tests/CMakeLists.txt`. The test source
list is currently empty (Phase 9.1 hasn't started), but the moment
someone writes `#include "core/DataStructures.h"` in a test, that test
will fail to link without it. We fixed it preemptively.

### Updating the three `fetchAll*` queries

The original code was:

```cpp
auto rows = executeSelectQuery("SELECT * FROM CoursesProjects ORDER BY CreatedAt ASC");
```

This is now insufficient — we need to join `Categories` so each row
carries `Color` and `Name` for the joined category. Two pitfalls:

1. **`SELECT *` is ambiguous after the join.** Both `CoursesProjects.Name`
   and `Categories.Name` exist. `QVariantMap row["Name"]` would lose to
   whichever came last in the join order — silent data corruption.
2. **An entity may not have a category.** `CategoryID` can be `NULL`.
   `INNER JOIN` would drop that row from the result; we'd lose the
   entity. **We need `LEFT JOIN`.**

The new SELECT is built once and shared by all three fetchers via a
file-scope constant:

```cpp
static const char* kEntitySelectSql =
    "SELECT cp.ID          AS ID,            "
    "       cp.Name        AS Name,          "
    "       cp.Type        AS Type,          "
    "       cp.CreatedAt   AS CreatedAt,     "
    "       cp.CategoryID  AS CategoryID,    "
    "       cp.Status      AS Status,        "
    "       cat.Name       AS CategoryName,  "
    "       cat.Color      AS CategoryColor  "
    "FROM   CoursesProjects cp                "
    "LEFT   JOIN Categories cat ON cat.ID = cp.CategoryID ";
```

Three things to notice:

- **Aliases everywhere.** `cp.Name AS Name`, `cat.Name AS CategoryName`.
  No ambiguity, and the column names in the result match exactly the
  field names in `EntityData`.
- **Single source of truth.** All three fetchers (`fetchAllEntities`,
  `fetchAllCourses`, `fetchAllProjects`) append a `WHERE`/`ORDER BY` to
  the same base string. Adding a new joined field in v3 is a one-line
  diff here, not three.
- **The trailing space.** Each line in the multi-line string literal
  ends with a space so concatenation with `"WHERE cp.Type = 'Course'"`
  doesn't smash `Categories` and `WHERE` together. (Small detail, real
  bug if forgotten.)

### Updating `rowToEntity`

The mapper learns to read four more columns:

```cpp
static EntityData rowToEntity(const QVariantMap& row) {
    EntityData e;
    e.id        = row["ID"].toInt();
    e.name      = row["Name"].toString();
    e.type      = row["Type"].toString();
    e.createdAt = QDateTime::fromString(row["CreatedAt"].toString(), Qt::ISODate);

    const QVariant catId = row.value("CategoryID");
    e.categoryId    = catId.isNull() ? -1 : catId.toInt();
    e.status        = row.value("Status",       "active").toString();
    e.categoryName  = row.value("CategoryName", QString()).toString();
    e.categoryColor = QColor(row.value("CategoryColor", QString()).toString());
    return e;
}
```

Three nuances:

- **`catId.isNull()` check.** `QVariant().toInt()` would return `0`, not
  `-1`. We need the explicit null check to map SQL NULL → our `-1`
  sentinel.
- **`row.value(key, default)`.** The two-argument `QVariantMap::value`
  returns the default when the key is missing — not when the value is
  null. Useful because it makes the mapper resilient if a future
  schema change drops the column entirely.
- **`QColor(QString())`** constructs an invalid `QColor`. The UI checks
  `entity.categoryColor.isValid()` to decide whether to render a pill.
  Same idea as the `-1` sentinel for `categoryId`, but in `QColor`'s
  natural vocabulary.

---

## 3. Task 3.3 — The seven v2 POD structs

Each struct mirrors one of the v2 tables (or, for `AnalyticsSummary`,
a derived view). All POD, all default-initialised.

### `CategoryData`

```cpp
struct CategoryData {
    int     id          = -1;
    QString name;
    QColor  color;
    int     entityCount = 0;
};
```

`entityCount` is **not stored**. It's computed by `fetchAllCategories`
(Task 4.2) via:

```sql
SELECT cat.*, COUNT(cp.ID) AS entityCount
FROM Categories cat
LEFT JOIN CoursesProjects cp ON cp.CategoryID = cat.ID
GROUP BY cat.ID;
```

The UI shows "Algorithms · 7 courses" without ever doing a second
query. Same N+1 avoidance trick as `EntityData`.

### `ProjectMetaData` (with nested `Link`)

```cpp
struct ProjectMetaData {
    struct Link {
        QString label;
        QString url;
    };

    int             projectId = -1;
    QString         description;
    QString         priority   = "medium";
    QDate           deadline;
    QStringList     team;
    QList<Link>     links;
};
```

Why nest `Link` instead of declaring it at file scope? Because nothing
else uses it. Pulling `struct Link` into the global namespace would
pollute autocomplete in every translation unit that includes this
header. Nesting keeps the symbol scoped to where it's meaningful:
`ProjectMetaData::Link`.

`team` and `links` mirror the `TeamJson` / `LinksJson` columns. The
DB stores JSON-in-TEXT for the reasons covered in
`phase-1-and-2-explained.md` § 2.9 ("Why JSON-in-TEXT for team and
links"). The struct exposes them as proper `QStringList` and `QList<Link>`
— JSON parsing happens in `DatabaseManager::getProjectMeta` (Task 4.4)
so neither the model nor the view ever sees JSON.

`deadline` is `QDate()` (invalid) when unset — same pattern as
`categoryColor`.

`priority` default `"medium"` is the most common case; the constructor
of a fresh `ProjectMetaData` matches what `addProject()` would seed
into the DB.

### `TodoData`

```cpp
struct TodoData {
    int       id        = -1;
    QString   title;
    bool      completed = false;
    QString   priority  = "medium";
    QDateTime createdAt;
    QDateTime completedAt;     // invalid() when completed == false
};
```

`completedAt` invalid when `completed` is false is a soft invariant —
not enforced at the type level, but documented and respected by the
CRUD method `toggleTodoCompleted()` (Task 4.5).

### `PomodoroSessionData`

```cpp
struct PomodoroSessionData {
    int       id              = -1;
    int       courseId        = -1;
    QString   courseName;          // resolved on read
    int       durationMinutes = 0;
    QDateTime completedAt;
    QString   mode            = "work";
};
```

Same join-pre-resolution pattern as `EntityData`: `courseName` is
populated by `fetchRecentSessions` (Task 4.6) via LEFT JOIN onto
`CoursesProjects`. `courseId == -1` is the canonical "uncategorised
session" value — the schema's `ON DELETE SET NULL` rule (Phase 2.9)
means deleting a course turns its old sessions into "free" sessions
that the UI labels as "Other".

### `CalendarDayData`

```cpp
struct CalendarDayData {
    QDate       date;
    QStringList todo;
    QStringList completed;
    QString     notes;

    bool hasContent() const {
        return !todo.isEmpty() || !completed.isEmpty() || !notes.isEmpty();
    }
};
```

This is the only struct in the file that carries a *method*. One-liner,
inlined in the header. The reason it exists at all: the
`CalendarWidget` (Task 6.8) shows an indicator dot under any date whose
day has *something* on it. Without `hasContent()`, every render of the
month view would write the same three-clause expression in two
different places (cell painting and tooltip generation), risking a
drift where one place forgets one of the three checks. A method keeps
the rule in one place.

Defining it in the header is right for two reasons:

1. It's one line — no linkage benefit to putting it in a `.cpp`.
2. There's no `.cpp` file for `DataStructures` anyway. The file is
   declaration-only by design.

### `AnalyticsSummary`

```cpp
struct AnalyticsSummary {
    int    currentStreakDays    = 0;
    int    longestStreakDays    = 0;
    int    monthHoursStudied    = 0;
    double avgSessionsPerDay7d  = 0.0;
    double weekOverWeekPct      = 0.0;
};
```

Not a DB row — a computed aggregate. The four `StatsCard` widgets at
the top of the analytics view (Task 7.9) each read one field. Returned
by value from a free function in the analytics folder (Task 5.7), so
the analytics view rebuilds the whole struct every time `dataChanged`
fires, ~milliseconds.

---

## 4. Task 3.4 — Filter / state structs (the gap-fill)

These don't correspond to a table — they exist because **models and
views need to share them**. The original React design treated filter
state as a private widget property; we promoted them out so a model
adapter or a save/restore mechanism can speak the same shape.

### `CourseFilter` and `ProjectFilter`

```cpp
struct CourseFilter {
    QString search;
    int     categoryId = -1;
    QString status     = "all";
};

struct ProjectFilter {
    QString search;
    QString priority = "all";
    QString status   = "all";
};
```

The convention: `-1` or `"all"` means "no filter applied on this axis".
Empty `search` means "no search filter". A default-constructed filter
matches every row — exactly the right behaviour for a freshly opened
view.

Why two structs instead of one big union? Because courses and projects
filter on different axes. Courses filter by `categoryId`; projects
filter by `priority`. Sharing a struct would force every UI to know
about every axis. Two clean structs keep each surface tight.

### `PomodoroTimerState` (nested enums)

```cpp
struct PomodoroTimerState {
    enum Mode  { Work, Break };
    enum State { Idle, Running, Paused };

    Mode      mode             = Work;
    State     state            = Idle;
    int       courseId         = -1;
    int       totalSeconds     = 25 * 60;
    int       remainingSeconds = 25 * 60;
    QDateTime startedAt;
};
```

This is the most interesting struct in Phase 3 because it's the only
one that captures *behavioural* state, not record state.

Why a struct (and not three loose member variables on `PomodoroTimerWidget`)?
**Cross-session resume.** The React reference design loses timer state
on navigation or app restart — close the app at minute 15 of a
25-minute pomodoro, you lose those 15 minutes. We do better: every
state change writes a snapshot to the `Settings` table under reserved
keys `pomodoro.state.*` (Task 4.9). On launch, `PomodoroTimerWidget`
reads the snapshot back and resumes where it left off.

For that to work, the *shape* of the state has to live in one place.
That place is this struct.

Two design details:

- **`startedAt` is `QDateTime`, not "seconds elapsed".** On resume,
  `remainingSeconds = totalSeconds - (now - startedAt)`. If the app was
  closed for 10 minutes during a Work session, the timer naturally
  fires "Pomodoro complete" on launch without any catch-up loop.
- **Nested enums.** `PomodoroTimerState::Mode::Work` reads better in
  call sites than a top-level `PomodoroMode::Work`. Same scoping
  argument as `ProjectMetaData::Link`.

### `ProfileData` and `PreferencesData`

```cpp
struct ProfileData {
    QString name;
    QString email;
    QString goals;
};

struct PreferencesData {
    int  workMinutes   = 25;
    int  breakMinutes  = 5;
    bool notifications = true;
    bool sound         = true;
    int  autoPauseDays = 30;
};
```

Typed wrappers over the generic `Settings` k/v table. The pattern is:

- `Settings` is a free-form `(Key TEXT, Value TEXT)` store — flexible.
- `ProfileData` / `PreferencesData` are typed views over a fixed set of
  keys — safe.

So `SettingsView` (Task 7.11) calls `getProfile()` which under the
hood does five `getSetting()` lookups and assembles a `ProfileData`.
The view never types a key string. The serialisation surface (which
keys go to which fields) is encapsulated in one method
(`DatabaseManager::getProfile` and its `setProfile` counterpart, Task
4.8).

Defaults on `PreferencesData` match the seeded defaults in Phase 2.11
exactly. A default-constructed `PreferencesData` represents what a
brand-new install would store.

---

## 5. Task 3.5 — Spec write-back

The `.ai/specs/design.md` "Expansion: New Data Structures" section
(lines 1858–1953) already carried the full struct catalogue *before*
this phase ran. It was written as part of the original v2 spec, and
the structs I added to `DataStructures.h` match those declarations
verbatim — same field names, same defaults, same nested types.

Two implications:

- **No `design.md` edits were needed this turn.** The spec was ahead
  of the code, the code caught up.
- **For Task 3.2 specifically** (the "show the new fields in
  Component 4: EntityCard" sub-checkbox), the design doc currently
  carries an *owning-folder* annotation on Component 4 but doesn't yet
  detail the EntityCard's v2 interface (CategoryPill slot, Paused
  badge). That's owned by **Task 6.5** ("v2 add-ons to EntityCard"),
  which will rewrite that section. Deferring the spec edit until then
  keeps each section honest: it'll describe the widget *as it actually
  exists*, not as it does today.

The `tasks.md` boxes for 3.2–3.5 are all ticked, with the deferral
explicitly noted.

---

## 6. Verification — what we ran, what we saw

```bash
cd CTracker
rm -rf build
cmake -S . -B build -G "Ninja" \
    -DCMAKE_PREFIX_PATH="C:/Qt/6.7.2/mingw_64" -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

Result: clean configure, **21/21 targets** compiled, `CTracker.exe`
linked. Every file in `include/` and `src/` that pulls in
`core/DataStructures.h` transitively continued to compile without
edits. The `Qt6::Gui` addition to both `CMakeLists.txt` files was the
only build-system change.

A note on the IDE: clangd flagged Unknown type name 'QString'/'QColor'/etc.
diagnostics while typing. Those come from the IDE not having a
`compile_commands.json` pointing at the build folder; the actual
`g++` / `ninja` toolchain has no such issue. If you want the IDE
warnings to clear:

```bash
cd CTracker
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      -DCMAKE_PREFIX_PATH="C:/Qt/6.7.2/mingw_64"
```

…and point your clangd extension at `build/compile_commands.json`.
The diagnostics are noise, not bugs — the real proof is the clean
21/21 build.

---

## 7. Acceptance checklist

- [ ] `include/core/DataStructures.h` contains all 12 v2 additions
      (extended `EntityData` + 7 v2 + 5 filter/state).
- [ ] Every new field has a sensible default — a default-constructed
      struct is a usable "empty" value.
- [ ] `core/DatabaseManager.cpp::rowToEntity` reads `CategoryID`,
      `Status`, `CategoryName`, `CategoryColor`.
- [ ] `fetchAllEntities`/`fetchAllCourses`/`fetchAllProjects` use a
      single shared `LEFT JOIN Categories` SELECT with aliased columns
      (no `SELECT *` collision).
- [ ] Both `CMakeLists.txt` files list `Qt6::Gui` explicitly.
- [ ] `cmake --build build` is clean.
- [ ] `tasks.md` Phase 3 boxes all ticked.

---

## 8. What this unlocks

Phase 3 done means **every later phase can speak in proper types**:

- **Phase 4 (DB API).** `addCategory(CategoryData)`, `getProjectMeta()
  → ProjectMetaData`, `fetchActiveTodos() → QList<TodoData>`,
  `getProfile() → ProfileData` — every signature is composed of these
  structs, no `QVariantMap` leaks past `DatabaseManager`.
- **Phase 5 (Models).** `CategoryModel` exposes `CategoryData` via Qt
  roles. `TodoModel` partitions `QList<TodoData>` by `.completed`.
  `HeatmapAggregator` consumes `ActivityLogEntry + TodoData +
  PomodoroSessionData` and emits `HeatmapDataPoint` per day.
- **Phase 6 (Widgets).** `EntityCard::setEntity(const EntityData&)`,
  `CategoryPill::setCategory(const CategoryData&)`,
  `TodoRow::TodoRow(const TodoData&)`, `DayDetailsPanel::showDay(const
  CalendarDayData&)`, `PomodoroTimerWidget` reads/writes
  `PomodoroTimerState`. Every widget API is a struct, never a tuple of
  primitives.
- **Phase 7 (Views).** `CoursesFilterBar::filterChanged(CourseFilter)`,
  `SettingsView` swaps `ProfileData` / `PreferencesData` with the DB,
  `PomodoroView` round-trips `PomodoroTimerState`.

That uniform vocabulary is what Phase 3 was for.
