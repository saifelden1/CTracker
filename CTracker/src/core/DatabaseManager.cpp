#include "core/DatabaseManager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

// ============================================================
//  DatabaseManager.cpp  —  Implementation
// ============================================================

// Initialize the static instance pointer to nullptr (no object yet)
DatabaseManager* DatabaseManager::s_instance = nullptr;

// ============================================================
//  Singleton: instance()
//
//  The first time you call this, it creates the one-and-only
//  DatabaseManager object. Every subsequent call returns the
//  same object.
// ============================================================
DatabaseManager* DatabaseManager::instance()
{
    if (!s_instance) {
        s_instance = new DatabaseManager();
    }
    return s_instance;
}

// Private constructor — nothing special to do here
DatabaseManager::DatabaseManager(QObject* parent)
    : QObject(parent)
{}

// ============================================================
//  initialize()
//
//  Opens (or creates) the SQLite database file.
//  If dbPath is empty, we store the DB in the OS app-data folder.
//  e.g. on Windows: C:\Users\<name>\AppData\Local\CTracker\ctracker.db
// ============================================================
bool DatabaseManager::initialize(const QString& dbPath)
{
    QString path = dbPath;

    if (path.isEmpty()) {
        // Ask Qt for the right folder to store user data
        QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dataDir);               // Create folder if it doesn't exist
        path = dataDir + "/ctracker.db";
    }

    qDebug() << "[DB] Opening database at:" << path;

    // "QSQLITE" tells Qt to use the built-in SQLite driver.
    // Guard against double-registration so calling initialize() twice
    // (e.g. from tests) reuses the existing connection instead of warning.
    const QString connName = "CTrackerConnection";
    if (QSqlDatabase::contains(connName)) {
        m_database = QSqlDatabase::database(connName);
    } else {
        m_database = QSqlDatabase::addDatabase("QSQLITE", connName);
    }
    m_database.setDatabaseName(path);

    if (!m_database.open()) {
        qWarning() << "[DB] Failed to open:" << m_database.lastError().text();
        emit databaseError("Failed to open database: " + m_database.lastError().text());
        return false;
    }

    qDebug() << "[DB] Connection opened successfully.";

    // Enable foreign key enforcement (SQLite disables it by default!)
    executeQuery("PRAGMA foreign_keys = ON");

    // ── Step 1: build the v1 schema (idempotent) ─────────────
    if (!createTables()) return false;

    // ── Step 2: bring schema up to the latest version ────────
    // currentSchemaVersion() returns 0 on a fresh install (no SchemaInfo
    // row yet) OR on an older v1 database. Either way, migrate(→2) is the
    // single code path that brings us to v2.
    const int kTargetVersion = 2;
    int v = currentSchemaVersion();
    if (v < kTargetVersion) {
        qDebug() << "[DB] Schema version" << v << "→" << kTargetVersion;
        if (!migrate(v, kTargetVersion)) {
            qWarning() << "[DB] Migration failed.";
            return false;
        }
    } else {
        qDebug() << "[DB] Schema already at version" << v;
    }

    return true;
}

bool DatabaseManager::isOpen() const
{
    return m_database.isOpen();
}

void DatabaseManager::close()
{
    if (m_database.isOpen()) {
        m_database.close();
        qDebug() << "[DB] Connection closed.";
    }
}

// ============================================================
//  createTables()
//
//  Runs CREATE TABLE IF NOT EXISTS for all 4 tables.
//  This is idempotent — safe to call every time the app starts.
//  If tables already exist, SQLite does nothing.
// ============================================================
bool DatabaseManager::createTables()
{
    qDebug() << "[DB] Creating tables if they do not exist...";

    // ── Table 1: CoursesProjects ─────────────────────────────
    // Stores both courses AND projects. The "Type" column tells us which.
    bool ok = executeQuery(R"(
        CREATE TABLE IF NOT EXISTS CoursesProjects (
            ID        INTEGER PRIMARY KEY AUTOINCREMENT,
            Name      TEXT    NOT NULL,
            Type      TEXT    NOT NULL CHECK(Type IN ('Course', 'Project')),
            CreatedAt TEXT    DEFAULT CURRENT_TIMESTAMP,
            UpdatedAt TEXT    DEFAULT CURRENT_TIMESTAMP
        )
    )");
    if (!ok) return false;

    // ── Table 2: Units ───────────────────────────────────────
    // Each Unit belongs to one Course or Project.
    // ON DELETE CASCADE: if a Course is deleted, all its Units auto-delete.
    ok = executeQuery(R"(
        CREATE TABLE IF NOT EXISTS Units (
            ID        INTEGER PRIMARY KEY AUTOINCREMENT,
            ParentID  INTEGER NOT NULL,
            Name      TEXT    NOT NULL,
            CreatedAt TEXT    DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (ParentID) REFERENCES CoursesProjects(ID) ON DELETE CASCADE
        )
    )");
    if (!ok) return false;

    // ── Table 3: SessionsTasks ───────────────────────────────
    // Each Session/Task belongs to one Unit.
    // CHECK constraint: progress can only be 0..100.
    ok = executeQuery(R"(
        CREATE TABLE IF NOT EXISTS SessionsTasks (
            ID              INTEGER PRIMARY KEY AUTOINCREMENT,
            UnitID          INTEGER NOT NULL,
            Name            TEXT    NOT NULL,
            CurrentProgress INTEGER DEFAULT 0
                CHECK(CurrentProgress >= 0 AND CurrentProgress <= 100),
            CreatedAt       TEXT    DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (UnitID) REFERENCES Units(ID) ON DELETE CASCADE
        )
    )");
    if (!ok) return false;

    // ── Table 4: ActivityLog ─────────────────────────────────
    // Every time a slider is moved, one row is written here.
    // This is the raw data that powers the Contribution Heatmap.
    ok = executeQuery(R"(
        CREATE TABLE IF NOT EXISTS ActivityLog (
            ID            INTEGER PRIMARY KEY AUTOINCREMENT,
            ItemID        INTEGER NOT NULL,
            Timestamp     TEXT    NOT NULL DEFAULT CURRENT_TIMESTAMP,
            OldValue      INTEGER,
            NewValue      INTEGER,
            ProgressDelta INTEGER,
            Type          TEXT    NOT NULL CHECK(Type IN ('Course', 'Project')),
            FOREIGN KEY (ItemID) REFERENCES SessionsTasks(ID) ON DELETE CASCADE
        )
    )");
    if (!ok) return false;

    qDebug() << "[DB] All tables ready.";
    return true;
}

// ============================================================
//  executeQuery()  —  Write operations (INSERT/UPDATE/DELETE)
//
//  Using named placeholders (:param) prevents SQL injection.
//  Example call:
//    executeQuery("INSERT INTO Units (ParentID, Name) VALUES (:pid, :name)",
//                 {{":pid", 1}, {":name", "Arrays"}});
// ============================================================
bool DatabaseManager::executeQuery(const QString& sql, const QVariantMap& params)
{
    QSqlQuery query(m_database);
    query.prepare(sql);

    // Bind each parameter value to its placeholder
    for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
        query.bindValue(it.key(), it.value());
    }

    if (!query.exec()) {
        QString err = query.lastError().text();
        qWarning() << "[DB] Query failed:" << err << "\nSQL:" << sql;
        emit databaseError(err);
        return false;
    }
    return true;
}

// ============================================================
//  executeSelectQuery()  —  Read operations (SELECT)
//
//  Returns a list of rows; each row is a QVariantMap of col → value.
// ============================================================
QList<QVariantMap> DatabaseManager::executeSelectQuery(const QString& sql,
                                                        const QVariantMap& params)
{
    QList<QVariantMap> results;
    QSqlQuery query(m_database);
    query.prepare(sql);

    for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
        query.bindValue(it.key(), it.value());
    }

    if (!query.exec()) {
        qWarning() << "[DB] Select failed:" << query.lastError().text();
        emit databaseError(query.lastError().text());
        return results;
    }

    QSqlRecord record = query.record();
    while (query.next()) {
        QVariantMap row;
        for (int i = 0; i < record.count(); ++i) {
            row[record.fieldName(i)] = query.value(i);
        }
        results.append(row);
    }
    return results;
}

// ── Transaction helpers ──────────────────────────────────────
bool DatabaseManager::beginTransaction()    { return m_database.transaction(); }
bool DatabaseManager::commitTransaction()   { return m_database.commit(); }
bool DatabaseManager::rollbackTransaction() { return m_database.rollback(); }

// ============================================================
//  Entity CRUD
// ============================================================

// Shared private helper used by addCourse() and addProject()
int DatabaseManager::addEntity(const QString& name, const QString& type)
{
    if (!executeQuery(
            "INSERT INTO CoursesProjects (Name, Type) VALUES (:name, :type)",
            {{":name", name}, {":type", type}})) {
        return -1;
    }

    // Get the ID of the row we just inserted
    QList<QVariantMap> rows = executeSelectQuery("SELECT last_insert_rowid() AS id");
    if (rows.isEmpty()) return -1;

    int newId = rows.first()["id"].toInt();
    qDebug() << "[DB] Added" << type << "'" << name << "' with ID" << newId;
    emit dataChanged();
    return newId;
}

int DatabaseManager::addCourse(const QString& name)  { return addEntity(name, "Course");  }
int DatabaseManager::addProject(const QString& name) { return addEntity(name, "Project"); }

bool DatabaseManager::removeCourse(int id)  {
    bool ok = executeQuery("DELETE FROM CoursesProjects WHERE ID = :id AND Type = 'Course'",
                           {{":id", id}});
    if (ok) emit dataChanged();
    return ok;
}

bool DatabaseManager::removeProject(int id) {
    bool ok = executeQuery("DELETE FROM CoursesProjects WHERE ID = :id AND Type = 'Project'",
                           {{":id", id}});
    if (ok) emit dataChanged();
    return ok;
}

bool DatabaseManager::renameEntity(int entityId, const QString& type, const QString& newName) {
    bool ok = executeQuery(
        "UPDATE CoursesProjects SET Name = :name, UpdatedAt = CURRENT_TIMESTAMP WHERE ID = :id AND Type = :type",
        {{":name", newName}, {":id", entityId}, {":type", type}}
    );
    if (ok) emit dataChanged();
    return ok;
}

bool DatabaseManager::renameCourse(int courseId, const QString& newName) {
    return renameEntity(courseId, "Course", newName);
}

bool DatabaseManager::renameProject(int projectId, const QString& newName) {
    return renameEntity(projectId, "Project", newName);
}

// ============================================================
//  Entity → EntityData mapping (Task 3.2)
//
//  v1 used `SELECT * FROM CoursesProjects`. v2 needs four more
//  fields on every row: CategoryID, Status, CategoryName, CategoryColor.
//  The first two live on CoursesProjects itself (added in Phase 2.10);
//  the last two come from a LEFT JOIN onto Categories so each entity
//  carries its colour + label pre-resolved.
//
//  LEFT JOIN (not INNER): an entity may have CategoryID = NULL —
//  it's uncategorised. We must still return that row, just with
//  empty categoryName and invalid categoryColor.
//
//  Why explicit columns instead of SELECT *: CoursesProjects.Name
//  and Categories.Name would collide; aliasing forces clarity.
// ============================================================

// One source of truth for the join. All three fetchers share it so a
// future column addition only has to change one place.
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

// Helper to convert a joined QVariantMap row → EntityData struct.
static EntityData rowToEntity(const QVariantMap& row) {
    EntityData e;
    e.id        = row["ID"].toInt();
    e.name      = row["Name"].toString();
    e.type      = row["Type"].toString();
    e.createdAt = QDateTime::fromString(row["CreatedAt"].toString(), Qt::ISODate);

    // v2: a NULL CategoryID becomes the -1 sentinel; everything else
    // tracks the joined Categories row when present, sensible empties
    // when not. QColor("") is invalid — that's the right "no colour"
    // value for the UI to test against with isValid().
    const QVariant catId = row.value("CategoryID");
    e.categoryId    = catId.isNull() ? -1 : catId.toInt();
    e.status        = row.value("Status",       "active").toString();
    e.categoryName  = row.value("CategoryName", QString()).toString();
    e.categoryColor = QColor(row.value("CategoryColor", QString()).toString());
    return e;
}

QList<EntityData> DatabaseManager::fetchAllEntities() {
    QList<EntityData> list;
    auto rows = executeSelectQuery(
        QString(kEntitySelectSql) + "ORDER BY cp.CreatedAt ASC");
    for (const auto& row : rows) list.append(rowToEntity(row));
    return list;
}

QList<EntityData> DatabaseManager::fetchAllCourses() {
    QList<EntityData> list;
    auto rows = executeSelectQuery(
        QString(kEntitySelectSql) +
        "WHERE cp.Type = 'Course' ORDER BY cp.CreatedAt ASC");
    for (const auto& row : rows) list.append(rowToEntity(row));
    return list;
}

QList<EntityData> DatabaseManager::fetchAllProjects() {
    QList<EntityData> list;
    auto rows = executeSelectQuery(
        QString(kEntitySelectSql) +
        "WHERE cp.Type = 'Project' ORDER BY cp.CreatedAt ASC");
    for (const auto& row : rows) list.append(rowToEntity(row));
    return list;
}

// ============================================================
//  Unit CRUD
// ============================================================
int DatabaseManager::addUnit(int parentId, const QString& name) {
    if (!executeQuery(
            "INSERT INTO Units (ParentID, Name) VALUES (:pid, :name)",
            {{":pid", parentId}, {":name", name}})) {
        return -1;
    }
    auto rows = executeSelectQuery("SELECT last_insert_rowid() AS id");
    if (rows.isEmpty()) return -1;
    int newId = rows.first()["id"].toInt();
    qDebug() << "[DB] Added Unit '" << name << "' under parent" << parentId;
    emit dataChanged();
    return newId;
}

bool DatabaseManager::removeUnit(int unitId) {
    bool ok = executeQuery("DELETE FROM Units WHERE ID = :id", {{":id", unitId}});
    if (ok) emit dataChanged();
    return ok;
}

bool DatabaseManager::renameUnit(int unitId, const QString& newName) {
    bool ok = executeQuery(
        "UPDATE Units SET Name = :name WHERE ID = :id",
        {{":name", newName}, {":id", unitId}}
    );
    if (ok) emit dataChanged();
    return ok;
}

QList<UnitData> DatabaseManager::getUnitsForParent(int parentId) {
    QList<UnitData> list;
    auto rows = executeSelectQuery(
        "SELECT * FROM Units WHERE ParentID = :pid ORDER BY ID ASC",
        {{":pid", parentId}}
    );
    for (const auto& row : rows) {
        UnitData u;
        u.id       = row["ID"].toInt();
        u.parentId = row["ParentID"].toInt();
        u.name     = row["Name"].toString();
        list.append(u);
    }
    return list;
}

// ============================================================
//  Session / Task CRUD
// ============================================================
int DatabaseManager::addSessionTask(int unitId, const QString& name, int progress) {
    // Clamp progress to [0, 100] just in case
    progress = qBound(0, progress, 100);
    if (!executeQuery(
            "INSERT INTO SessionsTasks (UnitID, Name, CurrentProgress) VALUES (:uid, :name, :prog)",
            {{":uid", unitId}, {":name", name}, {":prog", progress}})) {
        return -1;
    }
    auto rows = executeSelectQuery("SELECT last_insert_rowid() AS id");
    if (rows.isEmpty()) return -1;
    int newId = rows.first()["id"].toInt();
    qDebug() << "[DB] Added Session/Task '" << name << "' under unit" << unitId;
    emit dataChanged();
    return newId;
}

bool DatabaseManager::removeSessionTask(int sessionId) {
    bool ok = executeQuery("DELETE FROM SessionsTasks WHERE ID = :id", {{":id", sessionId}});
    if (ok) emit dataChanged();
    return ok;
}

bool DatabaseManager::renameSessionTask(int sessionId, const QString& newName) {
    bool ok = executeQuery(
        "UPDATE SessionsTasks SET Name = :name WHERE ID = :id",
        {{":name", newName}, {":id", sessionId}}
    );
    if (ok) emit dataChanged();
    return ok;
}

int DatabaseManager::getSessionTaskProgress(int sessionId) {
    auto rows = executeSelectQuery(
        "SELECT CurrentProgress FROM SessionsTasks WHERE ID = :id",
        {{":id", sessionId}}
    );
    if (rows.isEmpty()) return -1;
    return rows.first()["CurrentProgress"].toInt();
}

bool DatabaseManager::updateSessionTaskProgress(int sessionId, int progress) {
    int oldValue = getSessionTaskProgress(sessionId);
    if (oldValue < 0) return false;  // Session not found

    // Guard: if nothing changed, do nothing
    if (oldValue == progress) return true;

    // Clamp to valid range
    progress = qBound(0, progress, 100);

    beginTransaction();

    bool ok = executeQuery(
        "UPDATE SessionsTasks SET CurrentProgress = :prog WHERE ID = :id",
        {{":prog", progress}, {":id", sessionId}}
    );
    if (!ok) { rollbackTransaction(); return false; }

    // Determine entity type (Course or Project) for the activity log
    auto typeRows = executeSelectQuery(R"(
        SELECT cp.Type FROM CoursesProjects cp
        JOIN Units u ON u.ParentID = cp.ID
        JOIN SessionsTasks st ON st.UnitID = u.ID
        WHERE st.ID = :id
    )", {{":id", sessionId}});

    QString entityType = typeRows.isEmpty() ? "Course" : typeRows.first()["Type"].toString();

    // Log the activity
    logActivity(sessionId, oldValue, progress, entityType);

    commitTransaction();
    emit dataChanged();
    return true;
}

QList<SessionTaskData> DatabaseManager::getSessionTasksForUnit(int unitId) {
    QList<SessionTaskData> list;
    auto rows = executeSelectQuery(
        "SELECT * FROM SessionsTasks WHERE UnitID = :uid ORDER BY ID ASC",
        {{":uid", unitId}}
    );
    for (const auto& row : rows) {
        SessionTaskData s;
        s.id       = row["ID"].toInt();
        s.unitId   = row["UnitID"].toInt();
        s.name     = row["Name"].toString();
        s.progress = row["CurrentProgress"].toInt();
        list.append(s);
    }
    return list;
}

// ============================================================
//  Activity Log
// ============================================================
int DatabaseManager::logActivity(int itemId, int oldVal, int newVal,
                                  const QString& type, const QDateTime& ts)
{
    // Guard: no-op if nothing changed
    if (oldVal == newVal) return -1;

    int delta = qAbs(newVal - oldVal);

    executeQuery(R"(
        INSERT INTO ActivityLog (ItemID, Timestamp, OldValue, NewValue, ProgressDelta, Type)
        VALUES (:iid, :ts, :old, :new, :delta, :type)
    )", {
        {":iid",   itemId},
        {":ts",    ts.toString(Qt::ISODate)},
        {":old",   oldVal},
        {":new",   newVal},
        {":delta", delta},
        {":type",  type}
    });

    auto rows = executeSelectQuery("SELECT last_insert_rowid() AS id");
    return rows.isEmpty() ? -1 : rows.first()["id"].toInt();
}

static ActivityLogEntry rowToLogEntry(const QVariantMap& row) {
    ActivityLogEntry e;
    e.id            = row["ID"].toInt();
    e.itemId        = row["ItemID"].toInt();
    e.timestamp     = QDateTime::fromString(row["Timestamp"].toString(), Qt::ISODate);
    e.oldValue      = row["OldValue"].toInt();
    e.newValue      = row["NewValue"].toInt();
    e.progressDelta = row["ProgressDelta"].toInt();
    e.type          = row["Type"].toString();
    return e;
}

QList<ActivityLogEntry> DatabaseManager::getActivityLog(const QDate& from, const QDate& to) {
    QList<ActivityLogEntry> list;
    auto rows = executeSelectQuery(R"(
        SELECT * FROM ActivityLog
        WHERE DATE(Timestamp) BETWEEN :from AND :to
        ORDER BY Timestamp ASC
    )", {{":from", from.toString(Qt::ISODate)}, {":to", to.toString(Qt::ISODate)}});
    for (const auto& row : rows) list.append(rowToLogEntry(row));
    return list;
}

QList<ActivityLogEntry> DatabaseManager::getActivityLogForItem(int itemId) {
    QList<ActivityLogEntry> list;
    auto rows = executeSelectQuery(
        "SELECT * FROM ActivityLog WHERE ItemID = :id ORDER BY Timestamp ASC",
        {{":id", itemId}}
    );
    for (const auto& row : rows) list.append(rowToLogEntry(row));
    return list;
}

// ============================================================
//  Schema versioning (Task 2.8)
// ============================================================
//
//  Why a SchemaInfo table?
//  -----------------------
//  Databases live for years across many releases of an app. A schema
//  change (new column, new table) must be applied in-place without
//  destroying the user's existing data. We therefore stamp the DB
//  with a small version number and gate each set of changes behind
//  that number. Every release knows: "I expect version N — if I find
//  N-1, run migration; if I find N, do nothing; if I find N+1, refuse
//  because the user opened a future DB with an old binary."
//
//  Idempotence is the contract: calling migrate() twice must be safe.
//  We achieve it with:
//    • CREATE TABLE IF NOT EXISTS         (tables)
//    • CREATE INDEX IF NOT EXISTS         (indexes)
//    • columnExists() guard before ALTER  (columns)
//    • INSERT OR IGNORE                   (seed rows, keyed on UNIQUE)
//
//  The whole 1 → 2 transition runs inside a single transaction so a
//  crash midway leaves the DB at exactly the pre-migration version.
// ============================================================

// pragma_table_info returns one row per column of `table`. We scan for
// the requested column name. Used to keep ALTER TABLE idempotent.
bool DatabaseManager::columnExists(const QString& table, const QString& column)
{
    // pragma_table_info() is a SQLite table-valued function. We cannot
    // bind the table name as a placeholder (it's a SQL identifier, not
    // a value), so we splice it directly — safe because callers pass
    // hard-coded table names, never user input.
    auto rows = executeSelectQuery(
        QString("SELECT name FROM pragma_table_info('%1')").arg(table)
    );
    for (const auto& row : rows) {
        if (row["name"].toString().compare(column, Qt::CaseInsensitive) == 0) {
            return true;
        }
    }
    return false;
}

int DatabaseManager::currentSchemaVersion()
{
    // If SchemaInfo doesn't exist yet (fresh install, or pre-versioning
    // v1 database), treat that as version 0 — migrate() will create
    // the table and stamp the right number.
    auto exists = executeSelectQuery(
        "SELECT name FROM sqlite_master WHERE type='table' AND name='SchemaInfo'"
    );
    if (exists.isEmpty()) return 0;

    auto rows = executeSelectQuery(
        "SELECT Value FROM SchemaInfo WHERE Key = 'schema_version'"
    );
    if (rows.isEmpty()) return 0;
    return rows.first()["Value"].toInt();
}

bool DatabaseManager::migrate(int from, int to)
{
    if (from == to) return true;
    if (from > to) {
        qWarning() << "[DB] Refusing to downgrade schema from"
                   << from << "to" << to;
        return false;
    }

    // Single transaction — all-or-nothing.
    if (!beginTransaction()) {
        qWarning() << "[DB] Could not begin migration transaction.";
        return false;
    }

    // Always make sure SchemaInfo exists first; its mere presence is
    // how currentSchemaVersion() distinguishes "fresh DB" from "v1 DB
    // built before we had versioning".
    bool ok = executeQuery(R"(
        CREATE TABLE IF NOT EXISTS SchemaInfo (
            Key   TEXT PRIMARY KEY,
            Value TEXT NOT NULL
        )
    )");
    if (!ok) { rollbackTransaction(); return false; }

    // ── 0 / 1 → 2  (single hop) ──────────────────────────────
    // We treat 0 (no version row) and 1 (legacy v1 DB) identically:
    // both need every v2 step applied. Each step is itself idempotent
    // so re-running here is harmless.
    if (from < 2 && to >= 2) {
        if (!createV2Tables())  { rollbackTransaction(); return false; }
        if (!addV2Columns())    { rollbackTransaction(); return false; }
        if (!seedV2Defaults())  { rollbackTransaction(); return false; }
    }

    // Stamp the new version (INSERT OR REPLACE = upsert on PK).
    ok = executeQuery(
        "INSERT OR REPLACE INTO SchemaInfo (Key, Value) VALUES ('schema_version', :v)",
        {{":v", QString::number(to)}}
    );
    if (!ok) { rollbackTransaction(); return false; }

    if (!commitTransaction()) {
        qWarning() << "[DB] Migration commit failed.";
        return false;
    }

    qDebug() << "[DB] Migration" << from << "→" << to << "complete.";
    emit dataChanged();
    return true;
}

// ============================================================
//  createV2Tables() — Task 2.9
//
//  Six new tables and three indexes. All idempotent.
// ============================================================
bool DatabaseManager::createV2Tables()
{
    qDebug() << "[DB] Creating v2 tables...";

    // ── Categories ───────────────────────────────────────────
    // A short colour-tagged label attached to a CoursesProjects row.
    // Name is UNIQUE so the user can't make two "Algorithms" pills.
    if (!executeQuery(R"(
        CREATE TABLE IF NOT EXISTS Categories (
            ID        INTEGER PRIMARY KEY AUTOINCREMENT,
            Name      TEXT    NOT NULL UNIQUE,
            Color     TEXT    NOT NULL,
            CreatedAt TEXT    DEFAULT CURRENT_TIMESTAMP
        )
    )")) return false;

    // ── ProjectMeta ──────────────────────────────────────────
    // 1-to-1 sidecar for the Project flavour of CoursesProjects.
    // ProjectID is BOTH the PK and the FK — natural one-row-per-project.
    // ON DELETE CASCADE: when the project disappears, its meta does too.
    if (!executeQuery(R"(
        CREATE TABLE IF NOT EXISTS ProjectMeta (
            ProjectID   INTEGER PRIMARY KEY,
            Description TEXT,
            Priority    TEXT    CHECK(Priority IN ('high','medium','low')),
            Deadline    TEXT,
            TeamJson    TEXT,
            LinksJson   TEXT,
            FOREIGN KEY (ProjectID) REFERENCES CoursesProjects(ID) ON DELETE CASCADE
        )
    )")) return false;

    // ── Todos ────────────────────────────────────────────────
    // Standalone todo list, not tied to a course/project.
    if (!executeQuery(R"(
        CREATE TABLE IF NOT EXISTS Todos (
            ID          INTEGER PRIMARY KEY AUTOINCREMENT,
            Title       TEXT    NOT NULL,
            Completed   INTEGER NOT NULL DEFAULT 0,
            Priority    TEXT    CHECK(Priority IN ('high','medium','low')),
            CreatedAt   TEXT    DEFAULT CURRENT_TIMESTAMP,
            CompletedAt TEXT
        )
    )")) return false;

    // ── PomodoroSessions ─────────────────────────────────────
    // Each completed pomodoro becomes one row. CourseID is nullable
    // (a session can be "free" / uncategorised) and SET NULL on
    // course-delete so we keep the history but lose the link.
    if (!executeQuery(R"(
        CREATE TABLE IF NOT EXISTS PomodoroSessions (
            ID              INTEGER PRIMARY KEY AUTOINCREMENT,
            CourseID        INTEGER,
            DurationMinutes INTEGER NOT NULL,
            CompletedAt     TEXT    NOT NULL DEFAULT CURRENT_TIMESTAMP,
            Mode            TEXT    NOT NULL CHECK(Mode IN ('work','break')),
            FOREIGN KEY (CourseID) REFERENCES CoursesProjects(ID) ON DELETE SET NULL
        )
    )")) return false;

    // ── CalendarDayDetails ───────────────────────────────────
    // Per-day notes/todos/completed bundles, keyed by date string.
    // Date is TEXT in ISO 8601 (YYYY-MM-DD) so lexical order == chronological.
    if (!executeQuery(R"(
        CREATE TABLE IF NOT EXISTS CalendarDayDetails (
            Date          TEXT PRIMARY KEY,
            TodoJson      TEXT,
            CompletedJson TEXT,
            Notes         TEXT
        )
    )")) return false;

    // ── Settings ─────────────────────────────────────────────
    // Generic key/value store. Typed accessors (Task 4.8) wrap it
    // so the UI never sees raw strings.
    if (!executeQuery(R"(
        CREATE TABLE IF NOT EXISTS Settings (
            Key   TEXT PRIMARY KEY,
            Value TEXT NOT NULL
        )
    )")) return false;

    // ── Indexes ──────────────────────────────────────────────
    // These speed up the three most frequent scans:
    //   • heatmap aggregation reads ActivityLog by date
    //   • pomodoro "today" totals scan PomodoroSessions by date
    //   • todo views partition by Completed flag
    if (!executeQuery("CREATE INDEX IF NOT EXISTS idx_activitylog_date "
                      "ON ActivityLog(DATE(Timestamp))")) return false;
    if (!executeQuery("CREATE INDEX IF NOT EXISTS idx_pomodoro_date "
                      "ON PomodoroSessions(DATE(CompletedAt))")) return false;
    if (!executeQuery("CREATE INDEX IF NOT EXISTS idx_todos_completed "
                      "ON Todos(Completed)")) return false;

    qDebug() << "[DB] v2 tables ready.";
    return true;
}

// ============================================================
//  addV2Columns() — Task 2.10
//
//  Adds two new columns to the existing CoursesProjects table:
//    • CategoryID — FK to Categories, SET NULL on category delete
//    • Status     — 'active' | 'paused' | 'completed'
//
//  SQLite cannot rename or drop columns easily, but `ALTER TABLE
//  ... ADD COLUMN` is fine and supports inline REFERENCES for new
//  columns (≥ 3.6.19). We guard each ADD with columnExists() so
//  re-running the migration is a no-op.
// ============================================================
bool DatabaseManager::addV2Columns()
{
    if (!columnExists("CoursesProjects", "CategoryID")) {
        if (!executeQuery(
            "ALTER TABLE CoursesProjects ADD COLUMN CategoryID INTEGER NULL "
            "REFERENCES Categories(ID) ON DELETE SET NULL"
        )) return false;
        qDebug() << "[DB] Added CoursesProjects.CategoryID";
    }

    if (!columnExists("CoursesProjects", "Status")) {
        // Note: SQLite's ALTER TABLE ADD COLUMN forbids a non-constant
        // default *with* a CHECK that references the column being added
        // in some old versions, but the literal default 'active' is fine.
        if (!executeQuery(
            "ALTER TABLE CoursesProjects ADD COLUMN Status TEXT NOT NULL "
            "DEFAULT 'active' "
            "CHECK(Status IN ('active','paused','completed'))"
        )) return false;
        qDebug() << "[DB] Added CoursesProjects.Status";
    }

    return true;
}

// ============================================================
//  seedV2Defaults() — Task 2.11
//
//  Five default categories (Name UNIQUE — INSERT OR IGNORE makes
//  re-runs harmless) and five default Settings rows.
// ============================================================
bool DatabaseManager::seedV2Defaults()
{
    struct Cat { const char* name; const char* color; };
    static const Cat kCategories[] = {
        {"Algorithms",       "#10b981"},
        {"Web Development",  "#3b82f6"},
        {"Machine Learning", "#8b5cf6"},
        {"Systems",          "#f59e0b"},
        {"Security",         "#ec4899"},
    };
    for (const auto& c : kCategories) {
        if (!executeQuery(
            "INSERT OR IGNORE INTO Categories (Name, Color) VALUES (:n, :c)",
            {{":n", c.name}, {":c", c.color}}
        )) return false;
    }

    struct Kv { const char* key; const char* value; };
    static const Kv kSettings[] = {
        {"pomodoro.workMinutes",   "25"},
        {"pomodoro.breakMinutes",  "5"},
        {"notifications.enabled",  "1"},
        {"sound.enabled",          "1"},
        {"courses.autoPauseDays",  "30"},
    };
    for (const auto& s : kSettings) {
        if (!executeQuery(
            "INSERT OR IGNORE INTO Settings (Key, Value) VALUES (:k, :v)",
            {{":k", s.key}, {":v", s.value}}
        )) return false;
    }

    qDebug() << "[DB] v2 defaults seeded.";
    return true;
}
