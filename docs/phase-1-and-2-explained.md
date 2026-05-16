# Phases 1 & 2 — Build System and Database Layer, Explained

> **Audience.** Same as `phase-0-explained.md` — written so a student
> can read it end-to-end and reproduce the work themselves. The C++ and
> SQL knowledge required is moderate: you've used `std::string` and
> written `SELECT … WHERE` queries; you don't need to have done SQLite
> migrations before.
>
> **What these phases are.**
>
> - **Phase 1** is the *project skeleton*: CMake build, Qt6 hookup,
>   resources file, directory tree. This was done in the very first
>   commit; we won't re-derive it from zero, we'll explain why each
>   piece is shaped the way it is.
> - **Phase 2** is the *data layer*: a singleton `DatabaseManager` that
>   owns the one SQLite connection, four v1 tables (Courses/Projects,
>   Units, Sessions/Tasks, ActivityLog), and — newly added in this
>   session — the v2 *migration system* that grows the schema to six
>   more tables plus two new columns, all idempotently.

---

## Phase 1 — Build System and Project Skeleton

### 1.1 Why CMake, why Qt 6

We chose CMake (≥ 3.20) because:

1. Qt 6 ships first-class CMake integration (`qt_add_executable`,
   `AUTOMOC`, `AUTORCC`, `AUTOUIC`). The legacy `qmake` is on its way
   out.
2. CMake handles cross-platform build orchestration so you can
   eventually open the same source tree in Visual Studio, Qt Creator,
   CLion, or VS Code without per-IDE config drift.
3. The build is reproducible — `cmake -S . -B build` from a clean
   checkout always produces the same binary.

Qt 6 modules we link:

| Module | What it gives us |
|---|---|
| `Qt6::Core` | `QObject`, `QString`, `QDate`, signals/slots, the event loop. |
| `Qt6::Widgets` | Every UI class (`QMainWindow`, `QWidget`, `QLayout`, …). |
| `Qt6::Sql` | `QSqlDatabase`, `QSqlQuery`, the bundled SQLite driver. |
| `Qt6::Test` | (tests target only) `QtTest` framework for Phase 9. |

Two more — `Qt6::Charts` and `Qt6::Svg` — are scheduled for Phase 8 when
the analytics view starts plotting curves and rendering Lucide icons.

### 1.2 What `AUTOMOC` / `AUTORCC` / `AUTOUIC` actually do

Qt uses a tiny preprocessor of its own (the **Meta-Object Compiler,
moc**) to make `signals`, `slots`, and `Q_OBJECT` work. Without moc, the
class macros are just empty `extern "C"` no-ops.

- **`set(CMAKE_AUTOMOC ON)`** tells CMake: scan every header listed in
  `HEADERS`; if you find `Q_OBJECT`, run `moc` on it and link the
  generated `moc_<Class>.cpp` into the target. **This is why
  registering headers in `HEADERS` matters** — a header that's not in
  the list won't be moc'd, and the resulting class won't work as a
  QObject. (Phase 0 surfaced one such silent omission.)
- **`set(CMAKE_AUTORCC ON)`** processes `.qrc` resource files (image
  embedding, icons, QSS stylesheets) at build time.
- **`set(CMAKE_AUTOUIC ON)`** processes `.ui` files from Qt Designer
  (static layouts for `MainWindow`, `SettingsView`, …) into header
  files that your C++ can `#include`.

You enable all three once at the top of `CMakeLists.txt` and forget
about them.

### 1.3 The directory tree

```
CTracker/
├── CMakeLists.txt
├── include/        # public headers (feature-grouped after Phase 0)
├── src/            # implementations (feature-grouped after Phase 0)
├── resources/
│   └── resources.qrc       # Qt resource manifest
├── assets/
│   ├── icons/              # SVGs, populated in Phase 8.4
│   └── styles/             # dark-industrial.qss, Phase 8
├── tests/
│   └── CMakeLists.txt      # each test_*.cpp → one executable
└── build/          # generated, gitignored
```

The `assets/` folder is the *source-of-truth* for design files (raw
SVGs, the QSS stylesheet). `resources/resources.qrc` is the manifest
that tells Qt which of those files to embed inside the binary so the
app can load them via `:/icons/foo.svg` at runtime, with no external
file dependency.

### 1.4 Verifying Phase 1 from scratch

```bash
cd CTracker
rm -rf build
cmake -S . -B build -G "Ninja" \
    -DCMAKE_PREFIX_PATH="C:/Qt/6.7.2/mingw_64"   # adjust to your install
cmake --build build
./build/CTracker.exe
```

Three things to confirm:

1. **Configure succeeds.** If you see *"Could not find Qt6Config.cmake"*,
   your `CMAKE_PREFIX_PATH` is wrong.
2. **Build is clean.** A clean rebuild should produce N/N targets (12
   after Phase 0/2; will grow as new files are added) and link
   `CTracker.exe`.
3. **App launches.** The window pops up — even if it's mostly stubs.
   This is also when the database gets created (more on that below).

---

## Phase 2 — Database Layer (`core/DatabaseManager`)

This is the most opinionated part of the project. Every assumption is
recorded in code with a comment; this doc explains the **why** behind
those comments at a deeper level.

### 2.1 Why Singleton?

`DatabaseManager` is the **only** place in the app that talks to SQLite.
There is exactly one instance for the entire process lifetime.

Reasoning:

- A SQLite connection is a real, finite OS resource. Opening multiple
  connections to the same file works, but invites concurrency bugs
  (one connection sees a write the other doesn't until it COMMITs).
- The Qt model layer (`QAbstractTableModel`, etc.) expects a stable
  data source it can subscribe to. The `DatabaseManager::dataChanged`
  signal is the project's single source of refresh events; if there
  were multiple managers, each emitting their own version, the UI would
  fight itself.
- Testability is *not* sacrificed: the singleton can be initialized
  with an in-memory database (`:memory:`) for unit tests.

The pattern in code:

```cpp
class DatabaseManager : public QObject {
    Q_OBJECT
public:
    static DatabaseManager* instance();   // lazy, returns the one object
    // …public API…
private:
    DatabaseManager(QObject* parent = nullptr);     // private ctor
    ~DatabaseManager() = default;
    DatabaseManager(const DatabaseManager&)            = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
    static DatabaseManager* s_instance;
};
```

Deleting the copy ctor and assignment operator is critical — a
"singleton" you can accidentally `auto copy = *original;` isn't a
singleton anymore.

### 2.2 The v1 schema

Four tables, drawn in (verbal) ER form:

```
CoursesProjects (1) ─< (∞) Units (1) ─< (∞) SessionsTasks (1) ─< (∞) ActivityLog
```

Key shape decisions:

- **`CoursesProjects.Type CHECK IN ('Course','Project')`** — one table
  for two concepts that share 95% of their behaviour. The alternative
  (two parallel `Courses` + `Projects` tables) would force every query
  to be duplicated. The `Type` discriminator costs one byte per row and
  saves the duplication. Phase 2.10 will add a `Status` column with the
  same CHECK trick.
- **`ON DELETE CASCADE` everywhere down the tree.** Deleting a course
  must take its units, sessions, and activity log with it. The user's
  mental model is *"delete this course"*, not *"delete this course and
  remember to delete its 47 related rows yourself."*
- **`PRAGMA foreign_keys = ON`.** SQLite has foreign keys but disables
  enforcement by default — you have to opt in every connection. The
  manager does this in `initialize()`. If you forget, your CASCADE
  rules silently do nothing and orphan rows accumulate.
- **`CurrentProgress` `CHECK(>= 0 AND <= 100)`.** The slider widget
  also clamps, but the DB is the last line of defence. A bug elsewhere
  that sends `progress = 150` results in a clean SQL error rather than
  silently corrupt data.

### 2.3 Query helpers (`executeQuery` / `executeSelectQuery`)

Both helpers take a `QVariantMap params` of `{":placeholder", value}`
pairs. **All values from the UI go through this map** — never via
string concatenation. That is the project's single defence against SQL
injection. There's no SQL injection in a personal tracker per se, but
the discipline costs nothing and protects against e.g. an unlucky
course name containing a `'` character.

`executeSelectQuery` returns rows as `QList<QVariantMap>` keyed by column
name. Tradeoff: stringly-typed access vs. zero ceremony. For a project
this size, the simplicity wins; for a project ten times larger, you'd
generate typed row structs.

### 2.4 Transactions, and the slider example

The single most important DB write in the v1 app is **moving a session
slider**. From `updateSessionTaskProgress`:

```cpp
beginTransaction();
executeQuery("UPDATE SessionsTasks SET CurrentProgress = … WHERE ID = …");
// determine entity type via JOIN
logActivity(sessionId, oldValue, progress, entityType);
commitTransaction();
emit dataChanged();
```

Why a transaction? Because two writes happen: the *new progress value*
and the *activity log row*. If the second write failed and we'd already
committed the first, the heatmap (which reads from `ActivityLog`) would
permanently disagree with the slider. The transaction guarantees
both-or-neither.

A subtle guard sits at the top:

```cpp
if (oldValue == progress) return true;
```

Without this, releasing the slider on the same value you started with
would still write an `ActivityLog` row with a `ProgressDelta` of 0 —
polluting the heatmap with "activity" that was actually a no-op. The
test in Task 9.1 ("`logActivity` is NOT called when `oldValue ==
newValue`") protects this guard from regressing.

### 2.5 Where the database file lives

```cpp
QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
path = dataDir + "/ctracker.db";
```

`QStandardPaths::AppDataLocation` on Windows resolves to
`C:\Users\<you>\AppData\Local\<AppName>\`. We chose the OS-blessed
location instead of (say) the project directory because:

- Backup software (OneDrive, Time Machine) already knows to back up
  user-data directories.
- Multiple users on the same machine each get their own DB.
- Uninstall scripts know to clean it up.

For tests, we pass `":memory:"` instead; the DB lives in RAM and
vanishes when the connection closes.

---

## Phase 2.8 — Schema versioning (newly added)

This is the heart of what we just shipped. Every later phase that adds
a column or a table will use this same machinery.

### Why a `SchemaInfo` table?

Databases outlive any one release of an app. A user might install v1.0,
collect six months of data, then upgrade to v1.1 which expects two new
tables. v1.1 must:

1. Detect that the DB is older than it expects.
2. Apply *only* the diffs needed to bring it up to date — never
   recreate from scratch, that would destroy six months of work.
3. Refuse to run if the DB is *newer* than it understands (e.g. user
   downgraded by mistake), because writing to a future schema with old
   code corrupts everything.

The standard mechanism is a tiny `(Key, Value)` table that records the
current version number. Code reads it on startup, compares to a
compile-time `kTargetVersion`, and runs `migrate(from, to)` if
necessary.

```sql
CREATE TABLE SchemaInfo (
    Key   TEXT PRIMARY KEY,
    Value TEXT NOT NULL
);
INSERT INTO SchemaInfo VALUES ('schema_version', '2');
```

### `currentSchemaVersion()` — the careful early return

```cpp
int DatabaseManager::currentSchemaVersion() {
    auto exists = executeSelectQuery(
        "SELECT name FROM sqlite_master WHERE type='table' AND name='SchemaInfo'");
    if (exists.isEmpty()) return 0;
    auto rows = executeSelectQuery(
        "SELECT Value FROM SchemaInfo WHERE Key = 'schema_version'");
    if (rows.isEmpty()) return 0;
    return rows.first()["Value"].toInt();
}
```

Three return paths, all return `0` for "no version yet":

1. `SchemaInfo` doesn't exist (fresh install OR pre-versioning v1 DB).
2. `SchemaInfo` exists but the row isn't there (corruption / partial init).
3. The row exists and parses — return the number.

Returning `0` for both fresh and "legacy v1" is intentional — the
upgrade path is identical. The first time `migrate(0, 2)` runs, it
creates `SchemaInfo`, applies the v2 work, and stamps the row. Next
launch, `currentSchemaVersion()` returns `2` and `migrate()` is skipped.

### `migrate(from, to)` — the single-transaction contract

```cpp
bool DatabaseManager::migrate(int from, int to) {
    if (from == to) return true;
    if (from > to) { qWarning() << "refuse downgrade"; return false; }

    if (!beginTransaction()) return false;

    /* SchemaInfo first (it must exist before we stamp a version) */
    if (!executeQuery("CREATE TABLE IF NOT EXISTS SchemaInfo (…)"))
        { rollbackTransaction(); return false; }

    if (from < 2 && to >= 2) {
        if (!createV2Tables())  { rollbackTransaction(); return false; }
        if (!addV2Columns())    { rollbackTransaction(); return false; }
        if (!seedV2Defaults())  { rollbackTransaction(); return false; }
    }

    /* stamp version (INSERT OR REPLACE = upsert keyed on PK) */
    if (!executeQuery("INSERT OR REPLACE INTO SchemaInfo VALUES ('schema_version', :v)",
                       {{":v", QString::number(to)}}))
        { rollbackTransaction(); return false; }

    return commitTransaction();
}
```

Two correctness properties:

- **All-or-nothing.** Wrap every step in one transaction. If the process
  crashes between the column-add and the seed, the user reopens the app
  to a DB that's still at the old version — `migrate()` runs again
  cleanly, no half-applied state.
- **Strictly forward.** A downgrade attempt is rejected outright. We
  *could* implement reverse migrations later, but it's nontrivial
  (which seed rows do we delete? what about user-added categories that
  reference a column we'd be dropping?) and not worth the design cost
  for a personal tool.

### Idempotency — the contract that makes re-runs safe

"Idempotent" means: calling it twice has the same effect as calling it
once. For the migration to be safe, every SQL statement inside it must
be idempotent on its own:

| Operation | How we make it idempotent |
|---|---|
| `CREATE TABLE` | `CREATE TABLE IF NOT EXISTS …` |
| `CREATE INDEX` | `CREATE INDEX IF NOT EXISTS …` |
| `ALTER TABLE ADD COLUMN` | wrap in `if (!columnExists(table, col)) …` |
| `INSERT` seed row | `INSERT OR IGNORE` keyed on a UNIQUE column |
| Stamp version | `INSERT OR REPLACE` (upsert) |

`columnExists()` reads `pragma_table_info('CoursesProjects')`, which
returns one row per column. We scan for the column we're about to add.
SQLite has no `IF NOT EXISTS` for `ADD COLUMN`, so this is the
canonical pattern.

```cpp
bool DatabaseManager::columnExists(const QString& table, const QString& column) {
    auto rows = executeSelectQuery(
        QString("SELECT name FROM pragma_table_info('%1')").arg(table));
    for (const auto& row : rows)
        if (row["name"].toString().compare(column, Qt::CaseInsensitive) == 0)
            return true;
    return false;
}
```

**Why the table name is spliced into the SQL string** instead of bound
as a parameter: in SQL, identifiers (table/column names) are not
values, and `?`/`:placeholder` bindings only work for *values*. Since
we only call this with hard-coded table names from inside the manager,
not user input, splicing is safe. If you ever expose this helper to
external callers, validate the argument against a whitelist first.

---

## Phase 2.9 — The six v2 tables and three indexes

Each table is a `CREATE TABLE IF NOT EXISTS …` so the migration is safe
to re-run.

### `Categories`

```sql
CREATE TABLE Categories (
    ID        INTEGER PRIMARY KEY AUTOINCREMENT,
    Name      TEXT    NOT NULL UNIQUE,
    Color     TEXT    NOT NULL,
    CreatedAt TEXT    DEFAULT CURRENT_TIMESTAMP
);
```

A short colour-tagged label users can attach to a course/project.
`Name UNIQUE` prevents two "Algorithms" categories from coexisting and
also gives `INSERT OR IGNORE` (used by the seeder) a column to dedupe
on.

### `ProjectMeta`

```sql
CREATE TABLE ProjectMeta (
    ProjectID   INTEGER PRIMARY KEY,
    Description TEXT,
    Priority    TEXT    CHECK(Priority IN ('high','medium','low')),
    Deadline    TEXT,
    TeamJson    TEXT,
    LinksJson   TEXT,
    FOREIGN KEY (ProjectID) REFERENCES CoursesProjects(ID) ON DELETE CASCADE
);
```

A 1-to-1 sidecar. `ProjectID` is both the primary key and the foreign
key — there can be at most one meta row per project, enforced by the
PK. `ON DELETE CASCADE` is correct here (unlike Pomodoro below) because
project meta has no value once the project is gone.

`TeamJson` and `LinksJson` are TEXT columns holding JSON arrays. We
opted **against** normalizing to `ProjectTeam(ProjectID, MemberName)`
and `ProjectLinks(ProjectID, URL)` tables because:

- Both lists are bounded (handful of people, handful of links).
- They're always read together with the rest of the project meta.
- No queries filter on team-member-equals-X.

When all three of those conditions hold, JSON-in-TEXT is the right
tradeoff. If a query like "show me every project I'm on" becomes
important later, *that's* when normalization is worth doing.

### `Todos`

Standalone task list, not attached to courses or projects. Has its own
priority (using the same `CHECK IN ('high','medium','low')` vocabulary
as `ProjectMeta`).

### `PomodoroSessions`

```sql
CREATE TABLE PomodoroSessions (
    ID              INTEGER PRIMARY KEY AUTOINCREMENT,
    CourseID        INTEGER,
    DurationMinutes INTEGER NOT NULL,
    CompletedAt     TEXT    NOT NULL DEFAULT CURRENT_TIMESTAMP,
    Mode            TEXT    NOT NULL CHECK(Mode IN ('work','break')),
    FOREIGN KEY (CourseID) REFERENCES CoursesProjects(ID) ON DELETE SET NULL
);
```

Note `ON DELETE SET NULL`, **not** `CASCADE`. Deleting a course should
**not** wipe its pomodoro history — those minutes still happened. The
session row remains, with `CourseID` set to `NULL`. The analytics view
will display them as "Other".

This is the single most important schema decision in Phase 2.9. Getting
it wrong (CASCADE) would make the analytics permanently lossy after
the first course delete.

### `CalendarDayDetails`

```sql
CREATE TABLE CalendarDayDetails (
    Date          TEXT PRIMARY KEY,
    TodoJson      TEXT,
    CompletedJson TEXT,
    Notes         TEXT
);
```

One row per date. We use `TEXT` in ISO 8601 (`YYYY-MM-DD`) format
deliberately: it sorts correctly lexically, so `ORDER BY Date` gives
chronological order without conversion.

### `Settings`

```sql
CREATE TABLE Settings (
    Key   TEXT PRIMARY KEY,
    Value TEXT NOT NULL
);
```

Generic key/value store. Phase 4.8 will wrap this in typed accessors
(`getSetting(key, default)`, `getSettingInt(key)`, `setProfile(ProfileData)`)
so the UI never sees raw strings. The flexibility of TEXT-only storage
is worth more than the schema-level type checking we'd get from typed
columns — adding a new preference is a one-line code change, not a
schema migration.

### The three indexes

```sql
CREATE INDEX idx_activitylog_date  ON ActivityLog(DATE(Timestamp));
CREATE INDEX idx_pomodoro_date     ON PomodoroSessions(DATE(CompletedAt));
CREATE INDEX idx_todos_completed   ON Todos(Completed);
```

Each one targets a specific hot query:

- **`idx_activitylog_date`** — the contribution heatmap reads
  `ActivityLog` grouped by date. Without the index, every render
  full-scans the table. After a year of use that's ~10k rows; with the
  index it's ~365 lookups.
- **`idx_pomodoro_date`** — the pomodoro view shows "today's total
  minutes" — `WHERE DATE(CompletedAt) = :today`. Indexed lookup.
- **`idx_todos_completed`** — the todo view shows active and completed
  in separate sections, fetched via `WHERE Completed = 0` and `… = 1`.
  Two equality scans on an indexed boolean.

`CREATE INDEX IF NOT EXISTS` is the idempotent form.

---

## Phase 2.10 — Adding columns to `CoursesProjects`

Two ALTER TABLEs, each guarded:

```cpp
if (!columnExists("CoursesProjects", "CategoryID")) {
    executeQuery(
      "ALTER TABLE CoursesProjects ADD COLUMN CategoryID INTEGER NULL "
      "REFERENCES Categories(ID) ON DELETE SET NULL");
}

if (!columnExists("CoursesProjects", "Status")) {
    executeQuery(
      "ALTER TABLE CoursesProjects ADD COLUMN Status TEXT NOT NULL "
      "DEFAULT 'active' "
      "CHECK(Status IN ('active','paused','completed'))");
}
```

Two SQLite quirks worth knowing about:

1. **You can't `ALTER TABLE` an existing column to add a `REFERENCES`
   clause.** But you *can* add a new column with the `REFERENCES`
   inline, as we do here. (SQLite ≥ 3.6.19.) The FK constraint is
   stored as part of the new column's definition.
2. **`ALTER TABLE … ADD COLUMN` with a `NOT NULL` clause requires a
   non-NULL `DEFAULT`.** Otherwise existing rows would suddenly violate
   the NOT NULL constraint. Our `DEFAULT 'active'` satisfies this:
   every pre-existing course/project becomes `Status = 'active'`.

### Why `ON DELETE SET NULL` (and not CASCADE)?

Deleting a category must **not** delete the courses tagged with it.
The user's mental model: "I'm removing the 'Algorithms' label; my
algorithms course should stay, just untagged." SET NULL achieves
exactly that.

---

## Phase 2.11 — Seed defaults

Five categories with colours matching the design palette:

```cpp
static const Cat kCategories[] = {
    {"Algorithms",       "#10b981"},   // primary green
    {"Web Development",  "#3b82f6"},   // info blue
    {"Machine Learning", "#8b5cf6"},   // violet
    {"Systems",          "#f59e0b"},   // amber
    {"Security",         "#ec4899"},   // pink
};
for (const auto& c : kCategories)
    executeQuery("INSERT OR IGNORE INTO Categories (Name, Color) VALUES (:n, :c)", …);
```

`INSERT OR IGNORE` keyed on the `Name UNIQUE` constraint: if the user
already renamed "Algorithms" to "DSA" and we re-seed on a later launch,
we won't try to insert a duplicate. **But** if they *deleted* a default
category, we'll resurrect it next launch — that's an intentional
design call: the defaults are the floor, not the user's actual state.

Five default settings:

```
pomodoro.workMinutes  = 25
pomodoro.breakMinutes = 5
notifications.enabled = 1
sound.enabled         = 1
courses.autoPauseDays = 30
```

Same `INSERT OR IGNORE` pattern, keyed on the `Key` PK. If the user has
already changed `pomodoro.workMinutes` to 50, the seeder leaves it
alone.

---

## How Phase 2 ties together: the call graph on startup

```
main()
└── DatabaseManager::initialize()
    ├── open the SQLite file
    ├── PRAGMA foreign_keys = ON
    ├── createTables()                 ← v1 schema (idempotent)
    ├── currentSchemaVersion()         → 0 (fresh) or 1 (legacy v1) or 2 (up-to-date)
    └── if v < 2:  migrate(v, 2)
                   ├── BEGIN TRANSACTION
                   ├── CREATE TABLE IF NOT EXISTS SchemaInfo
                   ├── createV2Tables()      ← 6 tables + 3 indexes
                   ├── addV2Columns()        ← CategoryID + Status, guarded
                   ├── seedV2Defaults()      ← 5 cats + 5 settings, OR IGNORE
                   ├── stamp schema_version = 2
                   └── COMMIT
```

Two key properties of this pipeline:

1. **Re-entry is free.** If you delete `ctracker.db` and relaunch, you
   end at v2 with seeded defaults. If you keep an existing v1 DB and
   upgrade the binary, you also end at v2 — same data, more tables,
   more columns. If you launch a v2-aware binary against a v2 DB, it
   skips migration entirely. Three input states → one output state.
2. **All-or-nothing.** A failure anywhere inside `migrate()` rolls
   back. The DB never ends in a half-applied state.

---

## How to verify Phases 1 & 2

### Build verification (already done in this session)

```bash
cd CTracker
rm -rf build
cmake -S . -B build -G "Ninja" -DCMAKE_PREFIX_PATH="C:/Qt/6.7.2/mingw_64"
cmake --build build
```

Expected: clean compile, every target builds, `CTracker.exe` links.

### Runtime verification (deferred to Task 9.2)

The proper way to validate the migration is with `QtTest` against a
`:memory:` SQLite DB:

```cpp
// tests/test_databasemanager.cpp  (Phase 9.2, not yet written)
void TestDatabaseManager::testFreshInstallEndsAtV2() {
    DatabaseManager db;  // or use instance() with a fresh path
    QVERIFY(db.initialize(":memory:"));
    QCOMPARE(db.currentSchemaVersion(), 2);
}

void TestDatabaseManager::testIdempotency() {
    DatabaseManager db;
    db.initialize(":memory:");
    QVERIFY(db.migrate(1, 2));     // already at 2 — should no-op
    QVERIFY(db.migrate(2, 2));     // same — no-op
    QCOMPARE(db.currentSchemaVersion(), 2);
}

void TestDatabaseManager::testV1Upgrade() {
    // build a DB that mimics v1 (no SchemaInfo row), then initialize
    // and confirm it reaches v2 with all rows preserved.
}
```

These are the three tests every migration system needs:

1. **Fresh DB ends at the target version.**
2. **Running migrate twice is a no-op the second time.**
3. **An old DB is upgraded without data loss.**

Task 9.2 in `tasks.md` lists exactly these, plus three more
(`removeCategory` nulls dependent entities; `setCourseStatus('paused')`
hides the course from `fetchAllCourses(activeOnly=true)`;
`getPomodoroState` round-trips).

### Manual verification with the `sqlite3` CLI (optional)

If you want to peek at the DB after running the app once:

1. **Install the CLI** (it's *not* required by the app itself — Qt
   bundles its own SQLite engine; this is for ad-hoc inspection only).
   Grab `sqlite-tools-win-x64-*.zip` from <https://sqlite.org/download.html>
   and put `sqlite3.exe` somewhere on your `PATH`.
2. **Launch the app once** so the DB gets created.
3. Inspect:
   ```bash
   sqlite3 "%LOCALAPPDATA%\CTracker\ctracker.db"
   sqlite> .tables
   sqlite> SELECT * FROM SchemaInfo;
   sqlite> SELECT * FROM Categories;
   sqlite> SELECT * FROM Settings;
   sqlite> .schema CoursesProjects
   ```
   You should see all v1 + v2 tables, `schema_version = 2`, five seeded
   categories, five seeded settings, and `CoursesProjects` with the
   `CategoryID` and `Status` columns.

---

## Acceptance checklist (Phases 1 & 2)

- [ ] `cmake --build build` succeeds from a clean configure.
- [ ] `CTracker.exe` launches and `[DB] Connection opened successfully`
      appears in the console.
- [ ] On first launch, `[DB] Schema version 0 → 2` followed by
      `[DB] v2 tables ready.` and `[DB] v2 defaults seeded.` are logged.
- [ ] On second launch, `[DB] Schema already at version 2` is logged
      — no re-seed, no errors.
- [ ] `currentSchemaVersion()` returns `2` after init.
- [ ] (When the CLI is available) every table from Phase 2.2 + 2.9
      exists; `Settings` has 5 rows; `Categories` has 5 rows;
      `CoursesProjects` has `CategoryID` and `Status` columns.

---

## What this unlocks

Phase 2 done means the full data layer is **shape-complete**. Every
downstream phase (3 → structs that mirror these tables; 4 → API methods
that hit them; 5 → models that subscribe to `dataChanged`; 7 → views
that compose the models) can proceed without revisiting the schema.

The migration framework also means the next schema change is a
five-line diff: bump `kTargetVersion`, add a `from < 3 && to >= 3`
branch in `migrate()`, write the corresponding `createV3Tables()` /
`addV3Columns()` helpers. The user's data survives the upgrade
automatically.
