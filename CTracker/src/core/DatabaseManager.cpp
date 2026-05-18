#include "core/DatabaseManager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QSet>
#include <QTimer>

// ============================================================
//  DatabaseManager.cpp  �  Implementation
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

// Private constructor � nothing special to do here
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

    // -- Step 1: create all unified tables (idempotent) --------
    if (!createTables()) return false;

    // -- Step 2: seed defaults --------------------------------
    if (!seedDefaults()) return false;

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

void DatabaseManager::beginBatchUpdate() {
    m_batchUpdateMode = true;
    m_pendingDataChanged = false;
}

void DatabaseManager::endBatchUpdate() {
    m_batchUpdateMode = false;
    if (m_pendingDataChanged) {
        m_pendingDataChanged = false;
        emitDataChanged();
    }
}

void DatabaseManager::emitDataChanged() {
    if (m_batchUpdateMode) {
        m_pendingDataChanged = true;
        return;
    }

    if (m_dataChangedQueued) {
        return;
    }

    m_dataChangedQueued = true;
    QTimer::singleShot(0, this, [this]() {
        m_dataChangedQueued = false;
        emit dataChanged();
    });
}

// ============================================================
//  createTables()
//
//  Runs CREATE TABLE IF NOT EXISTS for all 4 tables.
//  This is idempotent � safe to call every time the app starts.
//  If tables already exist, SQLite does nothing.
// ============================================================
bool DatabaseManager::createTables()
{
    qDebug() << "[DB] Creating tables if they do not exist...";

    // -- Categories -------------------------------------------
    // A short colour-tagged label attached to a CoursesProjects row.
    // Name is UNIQUE so the user can't make two "Algorithms" pills.
    bool ok = executeQuery(R"(
        CREATE TABLE IF NOT EXISTS Categories (
            ID        INTEGER PRIMARY KEY AUTOINCREMENT,
            Name      TEXT    NOT NULL UNIQUE,
            Color     TEXT    NOT NULL,
            CreatedAt TEXT    DEFAULT CURRENT_TIMESTAMP
        )
    )");
    if (!ok) return false;

    // -- CoursesProjects --------------------------------------
    // Stores both courses AND projects. The "Type" column tells us which.
    ok = executeQuery(R"(
        CREATE TABLE IF NOT EXISTS CoursesProjects (
            ID         INTEGER PRIMARY KEY AUTOINCREMENT,
            Name       TEXT    NOT NULL,
            Type       TEXT    NOT NULL CHECK(Type IN ('Course', 'Project')),
            CreatedAt  TEXT    DEFAULT CURRENT_TIMESTAMP,
            UpdatedAt  TEXT    DEFAULT CURRENT_TIMESTAMP,
            CategoryID INTEGER NULL REFERENCES Categories(ID) ON DELETE SET NULL,
            Status     TEXT    NOT NULL DEFAULT 'active' CHECK(Status IN ('active','paused','completed'))
        )
    )");
    if (!ok) return false;

    // -- Units ------------------------------------------------
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

    // -- SessionsTasks ----------------------------------------
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

    // -- ActivityLog ------------------------------------------
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

    // -- ProjectMeta ------------------------------------------
    // 1-to-1 sidecar for the Project flavour of CoursesProjects.
    // ProjectID is BOTH the PK and the FK  natural one-row-per-project.
    // ON DELETE CASCADE: when the project disappears, its meta does too.
    ok = executeQuery(R"(
        CREATE TABLE IF NOT EXISTS ProjectMeta (
            ProjectID   INTEGER PRIMARY KEY,
            Description TEXT,
            Priority    TEXT    CHECK(Priority IN ('high','medium','low')),
            Deadline    TEXT,
            TeamJson    TEXT,
            LinksJson   TEXT,
            FOREIGN KEY (ProjectID) REFERENCES CoursesProjects(ID) ON DELETE CASCADE
        )
    )");
    if (!ok) return false;

    // -- Todos ------------------------------------------------
    // Standalone todo list, not tied to a course/project.
    ok = executeQuery(R"(
        CREATE TABLE IF NOT EXISTS Todos (
            ID          INTEGER PRIMARY KEY AUTOINCREMENT,
            Title       TEXT    NOT NULL,
            Completed   INTEGER NOT NULL DEFAULT 0,
            Priority    TEXT    CHECK(Priority IN ('high','medium','low')),
            CreatedAt   TEXT    DEFAULT CURRENT_TIMESTAMP,
            CompletedAt TEXT
        )
    )");
    if (!ok) return false;

    // -- PomodoroSessions -------------------------------------
    // Each completed pomodoro becomes one row. CourseID is nullable
    // (a session can be "free" / uncategorised) and SET NULL on
    // course-delete so we keep the history but lose the link.
    ok = executeQuery(R"(
        CREATE TABLE IF NOT EXISTS PomodoroSessions (
            ID              INTEGER PRIMARY KEY AUTOINCREMENT,
            CourseID        INTEGER,
            DurationMinutes INTEGER NOT NULL,
            CompletedAt     TEXT    NOT NULL DEFAULT CURRENT_TIMESTAMP,
            Mode            TEXT    NOT NULL CHECK(Mode IN ('work','break')),
            FOREIGN KEY (CourseID) REFERENCES CoursesProjects(ID) ON DELETE SET NULL
        )
    )");
    if (!ok) return false;

    // -- CalendarDayDetails -----------------------------------
    // Per-day notes/todos/completed bundles, keyed by date string.
    // Date is TEXT in ISO 8601 (YYYY-MM-DD) so lexical order == chronological.
    ok = executeQuery(R"(
        CREATE TABLE IF NOT EXISTS CalendarDayDetails (
            Date          TEXT PRIMARY KEY,
            TodoJson      TEXT,
            CompletedJson TEXT,
            Notes         TEXT
        )
    )");
    if (!ok) return false;

    // -- Settings ---------------------------------------------
    // Generic key/value store. Typed accessors (Task 4.8) wrap it
    // so the UI never sees raw strings.
    ok = executeQuery(R"(
        CREATE TABLE IF NOT EXISTS Settings (
            Key   TEXT PRIMARY KEY,
            Value TEXT NOT NULL
        )
    )");
    if (!ok) return false;

    // -- Indexes ----------------------------------------------
    ok = executeQuery("CREATE INDEX IF NOT EXISTS idx_activitylog_date "
                      "ON ActivityLog(DATE(Timestamp))");
    if (!ok) return false;
    ok = executeQuery("CREATE INDEX IF NOT EXISTS idx_pomodoro_date "
                      "ON PomodoroSessions(DATE(CompletedAt))");
    if (!ok) return false;
    ok = executeQuery("CREATE INDEX IF NOT EXISTS idx_todos_completed "
                      "ON Todos(Completed)");
    if (!ok) return false;

    qDebug() << "[DB] All tables ready.";
    return true;
}

// ============================================================
//  executeQuery()  �  Write operations (INSERT/UPDATE/DELETE)
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
//  executeSelectQuery()  �  Read operations (SELECT)
//
//  Returns a list of rows; each row is a QVariantMap of col ? value.
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

// -- Transaction helpers --------------------------------------
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
    emitDataChanged();
    return newId;
}

int DatabaseManager::addCourse(const QString& name)  { return addEntity(name, "Course");  }
int DatabaseManager::addProject(const QString& name) { return addEntity(name, "Project"); }

bool DatabaseManager::removeCourse(int id)  {
    // Phase 4.9 hygiene: if the persisted pomodoro timer was attached
    // to *this* course, clear that link before deletion so the timer
    // does not try to resume against a course that no longer exists.
    // (Pomodoro session rows are handled by the FK SET NULL rule on
    // PomodoroSessions.CourseID; the timer state lives in Settings
    // k/v, which has no FK, so we clear it manually here.)
    if (getSettingInt("pomodoro.state.courseId", -1) == id) {
        setSetting("pomodoro.state.courseId", "-1");
    }
    bool ok = executeQuery("DELETE FROM CoursesProjects WHERE ID = :id AND Type = 'Course'",
                           {{":id", id}});
    if (ok) emitDataChanged();
    return ok;
}

bool DatabaseManager::removeProject(int id) {
    bool ok = executeQuery("DELETE FROM CoursesProjects WHERE ID = :id AND Type = 'Project'",
                           {{":id", id}});
    if (ok) emitDataChanged();
    return ok;
}

bool DatabaseManager::renameEntity(int entityId, const QString& type, const QString& newName) {
    bool ok = executeQuery(
        "UPDATE CoursesProjects SET Name = :name, UpdatedAt = CURRENT_TIMESTAMP WHERE ID = :id AND Type = :type",
        {{":name", newName}, {":id", entityId}, {":type", type}}
    );
    if (ok) emitDataChanged();
    return ok;
}

bool DatabaseManager::renameCourse(int courseId, const QString& newName) {
    return renameEntity(courseId, "Course", newName);
}

bool DatabaseManager::renameProject(int projectId, const QString& newName) {
    return renameEntity(projectId, "Project", newName);
}

// ============================================================
//  Entity ? EntityData mapping (Task 3.2)
//
//  v1 used `SELECT * FROM CoursesProjects`. v2 needs four more
//  fields on every row: CategoryID, Status, CategoryName, CategoryColor.
//  The first two live on CoursesProjects itself (added in Phase 2.10);
//  the last two come from a LEFT JOIN onto Categories so each entity
//  carries its colour + label pre-resolved.
//
//  LEFT JOIN (not INNER): an entity may have CategoryID = NULL �
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
static EntityData rowToEntity(const QVariantMap& row, DatabaseManager* db) {
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
    
    // Calculate overall progress by averaging all session/task progress values
    e.overallProgress = 0;
    if (db && e.id >= 0) {
        const QList<UnitData> units = db->getUnitsForParent(e.id);
        int sum = 0;
        int count = 0;
        for (const UnitData& u : units) {
            const QList<SessionTaskData> sessions = db->getSessionTasksForUnit(u.id);
            for (const SessionTaskData& s : sessions) {
                sum += s.progress;
                ++count;
            }
        }
        e.overallProgress = (count == 0) ? 0 : (sum / count);
    }
    
    return e;
}

QList<EntityData> DatabaseManager::fetchAllEntities() {
    QList<EntityData> list;
    auto rows = executeSelectQuery(
        QString(kEntitySelectSql) + "ORDER BY cp.CreatedAt ASC");
    for (const auto& row : rows) list.append(rowToEntity(row, this));
    return list;
}

QList<EntityData> DatabaseManager::fetchAllCourses() {
    QList<EntityData> list;
    auto rows = executeSelectQuery(
        QString(kEntitySelectSql) +
        "WHERE cp.Type = 'Course' ORDER BY cp.CreatedAt ASC");
    for (const auto& row : rows) list.append(rowToEntity(row, this));
    return list;
}

QList<EntityData> DatabaseManager::fetchAllProjects() {
    QList<EntityData> list;
    auto rows = executeSelectQuery(
        QString(kEntitySelectSql) +
        "WHERE cp.Type = 'Project' ORDER BY cp.CreatedAt ASC");
    for (const auto& row : rows) list.append(rowToEntity(row, this));
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
    emitDataChanged();
    return newId;
}

bool DatabaseManager::removeUnit(int unitId) {
    bool ok = executeQuery("DELETE FROM Units WHERE ID = :id", {{":id", unitId}});
    if (ok) emitDataChanged();
    return ok;
}

bool DatabaseManager::renameUnit(int unitId, const QString& newName) {
    bool ok = executeQuery(
        "UPDATE Units SET Name = :name WHERE ID = :id",
        {{":name", newName}, {":id", unitId}}
    );
    if (ok) emitDataChanged();
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
    emitDataChanged();
    return newId;
}

bool DatabaseManager::removeSessionTask(int sessionId) {
    bool ok = executeQuery("DELETE FROM SessionsTasks WHERE ID = :id", {{":id", sessionId}});
    if (ok) emitDataChanged();
    return ok;
}

bool DatabaseManager::renameSessionTask(int sessionId, const QString& newName) {
    bool ok = executeQuery(
        "UPDATE SessionsTasks SET Name = :name WHERE ID = :id",
        {{":name", newName}, {":id", sessionId}}
    );
    if (ok) emitDataChanged();
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
    emitDataChanged();
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

bool DatabaseManager::seedDefaults()
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

// ============================================================
//  clearAllData — destructive reset
//
//  Deletes every row from every table, resets auto-increment
//  sequences, then re-seeds the default categories & settings
//  so the app is in a clean-but-valid state.
// ============================================================
bool DatabaseManager::clearAllData()
{
    beginBatchUpdate();

    static const char* kTables[] = {
        "SessionsTasks",
        "Units",
        "ActivityLog",
        "PomodoroSessions",
        "Todos",
        "CalendarDayDetails",
        "ProjectMeta",
        "CoursesProjects",
        "Categories",
        "Settings",
    };

    for (const char* tbl : kTables) {
        if (!executeQuery(QString("DELETE FROM %1").arg(tbl))) {
            endBatchUpdate();
            return false;
        }
    }

    // Reset auto-increment sequences so IDs start from 1 again
    if (!executeQuery("DELETE FROM sqlite_sequence")) {
        endBatchUpdate();
        return false;
    }

    // Re-seed default categories and settings
    if (!seedDefaults()) {
        endBatchUpdate();
        return false;
    }

    qDebug() << "[DB] All data cleared, defaults re-seeded.";
    endBatchUpdate();   // emits one dataChanged()
    return true;
}

// ============================================================
//  Phase 4 — Extended v2 API
//
//  Every method below sits on top of the v2 schema built in
//  Phase 2 and the v2 structs added in Phase 3. They are the
//  only entry point future widgets/views use to talk to the DB.
//  Reads never emit; writes emitDataChanged() on success so
//  subscribed views can refresh themselves.
// ============================================================

// ── Small JSON helpers, file-local ─────────────────────────────
//
// We store list-valued columns (team, links, todos, completed) as
// JSON text. Two reasons for that, instead of a side table:
//   1. They are *always read together* with their owning row, and
//      never queried (you never "find me all projects where a team
//      member is Alice"). Flattening avoids an unnecessary join.
//   2. The lists are bounded and tiny (members of a team, links on
//      a project, todos on a day). JSON is cheap at this size.
//
// We round-trip explicitly so an empty string in the DB becomes an
// empty QStringList, and a missing/malformed array never crashes —
// it just returns empty.
static QString stringListToJson(const QStringList& list) {
    QJsonArray arr;
    for (const QString& s : list) arr.append(s);
    return QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact));
}

static QStringList jsonToStringList(const QString& text) {
    QStringList out;
    if (text.isEmpty()) return out;
    QJsonParseError err{};
    auto doc = QJsonDocument::fromJson(text.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isArray()) return out;
    const auto arr = doc.array();
    for (const auto& v : arr) if (v.isString()) out << v.toString();
    return out;
}

static QString linksToJson(const QList<ProjectMetaData::Link>& links) {
    QJsonArray arr;
    for (const auto& l : links) {
        QJsonObject o;
        o["label"] = l.label;
        o["url"]   = l.url;
        arr.append(o);
    }
    return QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact));
}

static QList<ProjectMetaData::Link> jsonToLinks(const QString& text) {
    QList<ProjectMetaData::Link> out;
    if (text.isEmpty()) return out;
    QJsonParseError err{};
    auto doc = QJsonDocument::fromJson(text.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isArray()) return out;
    const auto arr = doc.array();
    for (const auto& v : arr) {
        if (!v.isObject()) continue;
        const auto o = v.toObject();
        ProjectMetaData::Link link;
        link.label = o.value("label").toString();
        link.url   = o.value("url").toString();
        out.append(link);
    }
    return out;
}

// ============================================================
//  Task 4.2 — Categories
// ============================================================
int DatabaseManager::addCategory(const QString& name, const QColor& color)
{
    if (!executeQuery(
            "INSERT INTO Categories (Name, Color) VALUES (:n, :c)",
            {{":n", name}, {":c", color.name()}})) {
        return -1;
    }
    auto rows = executeSelectQuery("SELECT last_insert_rowid() AS id");
    if (rows.isEmpty()) return -1;
    int newId = rows.first()["id"].toInt();
    qDebug() << "[DB] Added Category" << name << "with ID" << newId;
    emitDataChanged();
    return newId;
}

bool DatabaseManager::renameCategory(int id, const QString& newName)
{
    bool ok = executeQuery(
        "UPDATE Categories SET Name = :n WHERE ID = :id",
        {{":n", newName}, {":id", id}}
    );
    if (ok) emitDataChanged();
    return ok;
}

bool DatabaseManager::setCategoryColor(int id, const QColor& color)
{
    bool ok = executeQuery(
        "UPDATE Categories SET Color = :c WHERE ID = :id",
        {{":c", color.name()}, {":id", id}}
    );
    if (ok) emitDataChanged();
    return ok;
}

bool DatabaseManager::removeCategory(int id)
{
    // FK is ON DELETE SET NULL on CoursesProjects.CategoryID, so any
    // dependent entities lose their category but survive. SQLite does
    // that automatically when foreign_keys = ON (set in initialize()).
    bool ok = executeQuery("DELETE FROM Categories WHERE ID = :id", {{":id", id}});
    if (ok) emitDataChanged();
    return ok;
}

QList<CategoryData> DatabaseManager::fetchAllCategories()
{
    // entityCount comes from a LEFT JOIN so a freshly-created category
    // with zero entities still appears in the list (count = 0).
    QList<CategoryData> list;
    auto rows = executeSelectQuery(R"(
        SELECT c.ID    AS ID,
               c.Name  AS Name,
               c.Color AS Color,
               COUNT(cp.ID) AS EntityCount
        FROM   Categories c
        LEFT   JOIN CoursesProjects cp ON cp.CategoryID = c.ID
        GROUP  BY c.ID
        ORDER  BY c.Name ASC
    )");
    for (const auto& row : rows) {
        CategoryData c;
        c.id          = row["ID"].toInt();
        c.name        = row["Name"].toString();
        c.color       = QColor(row["Color"].toString());
        c.entityCount = row["EntityCount"].toInt();
        list.append(c);
    }
    return list;
}

bool DatabaseManager::assignCategory(int entityId, int categoryId)
{
    // categoryId == -1 means "clear the category". SQL needs NULL,
    // not 0. We pass a typed null QVariant for that path.
    QVariant catParam = (categoryId < 0)
        ? QVariant(QMetaType(QMetaType::Int))   // NULL of type INT
        : QVariant(categoryId);

    bool ok = executeQuery(
        "UPDATE CoursesProjects SET CategoryID = :cat WHERE ID = :id",
        {{":cat", catParam}, {":id", entityId}}
    );
    if (ok) emitDataChanged();
    return ok;
}

// ============================================================
//  Task 4.3 — Course status
//
//  The CoursesProjects.Status column was added in Phase 2.10 with
//  a CHECK constraint. We do a soft pre-check here so callers get
//  a clear error rather than a constraint violation buried in a
//  generic SQL message.
// ============================================================
bool DatabaseManager::setCourseStatus(int courseId, const QString& status)
{
    static const QSet<QString> kValid = {"active", "paused", "completed"};
    if (!kValid.contains(status)) {
        qWarning() << "[DB] setCourseStatus: invalid value" << status;
        emit databaseError("Invalid course status: " + status);
        return false;
    }
    bool ok = executeQuery(
        "UPDATE CoursesProjects SET Status = :s WHERE ID = :id",
        {{":s", status}, {":id", courseId}}
    );
    if (ok) emitDataChanged();
    return ok;
}

QString DatabaseManager::getCourseStatus(int courseId)
{
    auto rows = executeSelectQuery(
        "SELECT Status FROM CoursesProjects WHERE ID = :id",
        {{":id", courseId}}
    );
    if (rows.isEmpty()) return QString();
    return rows.first()["Status"].toString();
}

// ============================================================
//  Task 4.4 — ProjectMeta
//
//  The table uses ProjectID as both PK and FK to CoursesProjects.
//  That makes "upsert" the natural verb: there is at most one meta
//  row per project. INSERT OR REPLACE keys on the PK.
// ============================================================
bool DatabaseManager::upsertProjectMeta(const ProjectMetaData& meta)
{
    bool ok = executeQuery(R"(
        INSERT OR REPLACE INTO ProjectMeta
            (ProjectID, Description, Priority, Deadline, TeamJson, LinksJson)
        VALUES
            (:pid, :desc, :prio, :deadline, :team, :links)
    )", {
        {":pid",      meta.projectId},
        {":desc",     meta.description},
        {":prio",     meta.priority},
        {":deadline", meta.deadline.isValid() ? QVariant(meta.deadline.toString(Qt::ISODate))
                                              : QVariant()},
        {":team",     stringListToJson(meta.team)},
        {":links",    linksToJson(meta.links)},
    });
    if (ok) emitDataChanged();
    return ok;
}

ProjectMetaData DatabaseManager::getProjectMeta(int projectId)
{
    // Defaults: if no row exists the caller gets a well-formed empty
    // struct with projectId stamped, so the UI can edit then upsert
    // without needing to special-case "missing".
    ProjectMetaData m;
    m.projectId = projectId;

    auto rows = executeSelectQuery(
        "SELECT * FROM ProjectMeta WHERE ProjectID = :pid",
        {{":pid", projectId}}
    );
    if (rows.isEmpty()) return m;

    const auto& row = rows.first();
    m.description = row["Description"].toString();
    m.priority    = row["Priority"].toString();
    if (m.priority.isEmpty()) m.priority = "medium";

    const QString deadlineStr = row["Deadline"].toString();
    if (!deadlineStr.isEmpty()) m.deadline = QDate::fromString(deadlineStr, Qt::ISODate);

    m.team  = jsonToStringList(row["TeamJson"].toString());
    m.links = jsonToLinks    (row["LinksJson"].toString());
    return m;
}

// Convenience setters — each one read-modify-writes the row so an
// individual field can be edited without rebuilding the whole struct.
// They share the upsert path so behaviour is identical to a full
// upsertProjectMeta() call from the caller's perspective.
bool DatabaseManager::setProjectPriority(int projectId, const QString& priority)
{
    ProjectMetaData m = getProjectMeta(projectId);
    m.priority = priority;
    return upsertProjectMeta(m);
}

bool DatabaseManager::setProjectDeadline(int projectId, const QDate& deadline)
{
    ProjectMetaData m = getProjectMeta(projectId);
    m.deadline = deadline;
    return upsertProjectMeta(m);
}

bool DatabaseManager::setProjectTeam(int projectId, const QStringList& team)
{
    ProjectMetaData m = getProjectMeta(projectId);
    m.team = team;
    return upsertProjectMeta(m);
}

bool DatabaseManager::setProjectLinks(int projectId, const QList<ProjectMetaData::Link>& links)
{
    ProjectMetaData m = getProjectMeta(projectId);
    m.links = links;
    return upsertProjectMeta(m);
}

// ============================================================
//  Task 4.5 — Todos
// ============================================================
int DatabaseManager::addTodo(const QString& title, const QString& priority)
{
    if (!executeQuery(R"(
        INSERT INTO Todos (Title, Completed, Priority) VALUES (:t, 0, :p)
    )", {{":t", title}, {":p", priority}})) {
        return -1;
    }
    auto rows = executeSelectQuery("SELECT last_insert_rowid() AS id");
    if (rows.isEmpty()) return -1;
    int newId = rows.first()["id"].toInt();
    emitDataChanged();
    return newId;
}

bool DatabaseManager::toggleTodoCompleted(int id)
{
    // Two-step so we can flip the stored bit and stamp CompletedAt
    // atomically. Wrapping in a transaction keeps a crash mid-toggle
    // from leaving the row in a half-state (Completed=1 but no timestamp).
    auto rows = executeSelectQuery(
        "SELECT Completed FROM Todos WHERE ID = :id",
        {{":id", id}}
    );
    if (rows.isEmpty()) return false;
    const bool currentlyCompleted = rows.first()["Completed"].toInt() != 0;
    const bool nextCompleted      = !currentlyCompleted;

    beginTransaction();
    bool ok = executeQuery(R"(
        UPDATE Todos
        SET    Completed   = :c,
               CompletedAt = CASE WHEN :c = 1 THEN CURRENT_TIMESTAMP ELSE NULL END
        WHERE  ID = :id
    )", {{":c", nextCompleted ? 1 : 0}, {":id", id}});

    if (!ok) { rollbackTransaction(); return false; }
    commitTransaction();
    emitDataChanged();
    return true;
}

bool DatabaseManager::setTodoPriority(int id, const QString& priority)
{
    bool ok = executeQuery(
        "UPDATE Todos SET Priority = :p WHERE ID = :id",
        {{":p", priority}, {":id", id}}
    );
    if (ok) emitDataChanged();
    return ok;
}

bool DatabaseManager::removeTodo(int id)
{
    bool ok = executeQuery("DELETE FROM Todos WHERE ID = :id", {{":id", id}});
    if (ok) emitDataChanged();
    return ok;
}

static TodoData rowToTodo(const QVariantMap& row) {
    TodoData t;
    t.id        = row["ID"].toInt();
    t.title     = row["Title"].toString();
    t.completed = row["Completed"].toInt() != 0;
    t.priority  = row["Priority"].toString();
    if (t.priority.isEmpty()) t.priority = "medium";
    t.createdAt   = QDateTime::fromString(row["CreatedAt"].toString(),   Qt::ISODate);
    t.completedAt = QDateTime::fromString(row["CompletedAt"].toString(), Qt::ISODate);
    return t;
}

QList<TodoData> DatabaseManager::fetchActiveTodos()
{
    // Ordering by priority text relies on it being a closed set
    // ('high','medium','low'). We compute an explicit rank in the
    // SQL so 'high' beats 'medium' regardless of alphabetical order.
    QList<TodoData> list;
    auto rows = executeSelectQuery(R"(
        SELECT *,
               CASE Priority
                   WHEN 'high'   THEN 0
                   WHEN 'medium' THEN 1
                   WHEN 'low'    THEN 2
                   ELSE 3
               END AS PriRank
        FROM   Todos
        WHERE  Completed = 0
        ORDER  BY PriRank ASC, CreatedAt DESC
    )");
    for (const auto& row : rows) list.append(rowToTodo(row));
    return list;
}

QList<TodoData> DatabaseManager::fetchCompletedTodos()
{
    QList<TodoData> list;
    auto rows = executeSelectQuery(R"(
        SELECT * FROM Todos
        WHERE  Completed = 1
        ORDER  BY CompletedAt DESC
    )");
    for (const auto& row : rows) list.append(rowToTodo(row));
    return list;
}

int DatabaseManager::countCompletedTodosOn(const QDate& date)
{
    auto rows = executeSelectQuery(R"(
        SELECT COUNT(*) AS N
        FROM   Todos
        WHERE  Completed = 1
          AND  DATE(CompletedAt) = :d
    )", {{":d", date.toString(Qt::ISODate)}});
    return rows.isEmpty() ? 0 : rows.first()["N"].toInt();
}

// ============================================================
//  Task 4.6 — Pomodoro sessions
// ============================================================
int DatabaseManager::insertPomodoroSession(int courseId, int durationMin, const QString& mode)
{
    // courseId == -1 → free session, store as NULL so the SET NULL
    // FK rule has nothing to point at and the column reads as
    // "unattached" everywhere downstream.
    QVariant courseParam = (courseId < 0)
        ? QVariant(QMetaType(QMetaType::Int))
        : QVariant(courseId);

    if (!executeQuery(R"(
        INSERT INTO PomodoroSessions (CourseID, DurationMinutes, Mode)
        VALUES (:cid, :dur, :mode)
    )", {{":cid", courseParam}, {":dur", durationMin}, {":mode", mode}})) {
        return -1;
    }
    auto rows = executeSelectQuery("SELECT last_insert_rowid() AS id");
    if (rows.isEmpty()) return -1;
    int newId = rows.first()["id"].toInt();
    emitDataChanged();
    return newId;
}

// One source of truth for the join used by both list fetchers — same
// pattern as kEntitySelectSql above. CourseName comes from a LEFT
// JOIN since CourseID may be NULL.
static const char* kPomodoroSelectSql =
    "SELECT p.ID              AS ID,             "
    "       p.CourseID        AS CourseID,       "
    "       cp.Name           AS CourseName,     "
    "       p.DurationMinutes AS DurationMinutes,"
    "       p.CompletedAt     AS CompletedAt,    "
    "       p.Mode            AS Mode            "
    "FROM   PomodoroSessions p                    "
    "LEFT   JOIN CoursesProjects cp ON cp.ID = p.CourseID ";

static PomodoroSessionData rowToPomodoro(const QVariantMap& row) {
    PomodoroSessionData s;
    s.id              = row["ID"].toInt();
    const QVariant cid = row.value("CourseID");
    s.courseId        = cid.isNull() ? -1 : cid.toInt();
    s.courseName      = row.value("CourseName", QString()).toString();
    s.durationMinutes = row["DurationMinutes"].toInt();
    s.completedAt     = QDateTime::fromString(row["CompletedAt"].toString(), Qt::ISODate);
    s.mode            = row["Mode"].toString();
    return s;
}

QList<PomodoroSessionData> DatabaseManager::fetchRecentSessions(int limit)
{
    QList<PomodoroSessionData> list;
    auto rows = executeSelectQuery(
        QString(kPomodoroSelectSql) +
        "ORDER BY p.CompletedAt DESC LIMIT :n",
        {{":n", limit}}
    );
    for (const auto& row : rows) list.append(rowToPomodoro(row));
    return list;
}

QList<PomodoroSessionData> DatabaseManager::fetchSessionsOn(const QDate& date)
{
    QList<PomodoroSessionData> list;
    auto rows = executeSelectQuery(
        QString(kPomodoroSelectSql) +
        "WHERE DATE(p.CompletedAt) = :d "
        "ORDER BY p.CompletedAt ASC",
        {{":d", date.toString(Qt::ISODate)}}
    );
    for (const auto& row : rows) list.append(rowToPomodoro(row));
    return list;
}

int DatabaseManager::totalMinutesOn(const QDate& date)
{
    // Only 'work' minutes count toward "time studied" — break minutes
    // are tracked separately and not surfaced as study time.
    auto rows = executeSelectQuery(R"(
        SELECT COALESCE(SUM(DurationMinutes), 0) AS Total
        FROM   PomodoroSessions
        WHERE  DATE(CompletedAt) = :d
          AND  Mode = 'work'
    )", {{":d", date.toString(Qt::ISODate)}});
    return rows.isEmpty() ? 0 : rows.first()["Total"].toInt();
}

// ============================================================
//  Task 4.7 — Calendar day details
// ============================================================
CalendarDayData DatabaseManager::getDay(const QDate& date)
{
    CalendarDayData d;
    d.date = date;

    auto rows = executeSelectQuery(
        "SELECT * FROM CalendarDayDetails WHERE Date = :d",
        {{":d", date.toString(Qt::ISODate)}}
    );
    if (rows.isEmpty()) return d;          // empty struct, valid date

    const auto& row = rows.first();
    d.todo      = jsonToStringList(row["TodoJson"].toString());
    d.completed = jsonToStringList(row["CompletedJson"].toString());
    d.notes     = row["Notes"].toString();
    return d;
}

bool DatabaseManager::upsertDay(const CalendarDayData& data)
{
    bool ok = executeQuery(R"(
        INSERT OR REPLACE INTO CalendarDayDetails
            (Date, TodoJson, CompletedJson, Notes)
        VALUES
            (:d, :todo, :done, :notes)
    )", {
        {":d",     data.date.toString(Qt::ISODate)},
        {":todo",  stringListToJson(data.todo)},
        {":done",  stringListToJson(data.completed)},
        {":notes", data.notes},
    });
    if (ok) emitDataChanged();
    return ok;
}

QSet<QDate> DatabaseManager::datesWithContent(const QDate& from, const QDate& to)
{
    // "Has content" = at least one of the three fields is non-empty.
    // We filter that in SQL so the caller doesn't have to fetch every
    // row in the range just to discard empties.
    QSet<QDate> set;
    auto rows = executeSelectQuery(R"(
        SELECT Date FROM CalendarDayDetails
        WHERE  Date BETWEEN :from AND :to
          AND ( COALESCE(TodoJson,      '') NOT IN ('', '[]')
             OR COALESCE(CompletedJson, '') NOT IN ('', '[]')
             OR COALESCE(Notes,         '') <> '' )
    )", {
        {":from", from.toString(Qt::ISODate)},
        {":to",   to.toString(Qt::ISODate)},
    });
    for (const auto& row : rows) {
        QDate d = QDate::fromString(row["Date"].toString(), Qt::ISODate);
        if (d.isValid()) set.insert(d);
    }
    return set;
}

// ============================================================
//  Task 4.8 — Settings k/v + typed wrappers
// ============================================================
QString DatabaseManager::getSetting(const QString& key, const QString& defaultValue)
{
    auto rows = executeSelectQuery(
        "SELECT Value FROM Settings WHERE Key = :k",
        {{":k", key}}
    );
    if (rows.isEmpty()) return defaultValue;
    return rows.first()["Value"].toString();
}

bool DatabaseManager::setSetting(const QString& key, const QString& value)
{
    // INSERT OR REPLACE: keyed on the PK so it acts as an upsert.
    // No dataChanged() spam from individual setting writes by default —
    // SettingsView batches them and emits once at the end. We keep
    // this honest by *not* emitting here. (If a future caller needs
    // a refresh, it can call dataChanged() explicitly.)
    return executeQuery(
        "INSERT OR REPLACE INTO Settings (Key, Value) VALUES (:k, :v)",
        {{":k", key}, {":v", value}}
    );
}

int DatabaseManager::getSettingInt(const QString& key, int defaultValue)
{
    const QString raw = getSetting(key, QString::number(defaultValue));
    bool ok = false;
    int v = raw.toInt(&ok);
    return ok ? v : defaultValue;
}

bool DatabaseManager::getSettingBool(const QString& key, bool defaultValue)
{
    const QString raw = getSetting(key, defaultValue ? "1" : "0");
    return raw == "1" || raw.compare("true", Qt::CaseInsensitive) == 0;
}

// ProfileData and PreferencesData are typed views over Settings. The
// key prefix groups them: profile.* and preferences.* (preferences
// reuse the same keys as the seeded user-facing pomodoro/sound/etc.
// rows so the wrapper does not duplicate state).
ProfileData DatabaseManager::getProfile()
{
    ProfileData p;
    p.name  = getSetting("profile.name");
    p.email = getSetting("profile.email");
    p.goals = getSetting("profile.goals");
    return p;
}

bool DatabaseManager::setProfile(const ProfileData& profile)
{
    beginTransaction();
    bool ok = setSetting("profile.name",  profile.name)
           && setSetting("profile.email", profile.email)
           && setSetting("profile.goals", profile.goals);
    if (!ok) { rollbackTransaction(); return false; }
    commitTransaction();
    emitDataChanged();
    return true;
}

PreferencesData DatabaseManager::getPreferences()
{
    PreferencesData p;
    p.workMinutes   = getSettingInt ("pomodoro.workMinutes",  25);
    p.breakMinutes  = getSettingInt ("pomodoro.breakMinutes", 5);
    p.notifications = getSettingBool("notifications.enabled", true);
    p.sound         = getSettingBool("sound.enabled",         true);
    p.autoPauseDays = getSettingInt ("courses.autoPauseDays", 30);
    return p;
}

bool DatabaseManager::setPreferences(const PreferencesData& prefs)
{
    beginTransaction();
    bool ok = setSetting("pomodoro.workMinutes",  QString::number(prefs.workMinutes))
           && setSetting("pomodoro.breakMinutes", QString::number(prefs.breakMinutes))
           && setSetting("notifications.enabled", prefs.notifications ? "1" : "0")
           && setSetting("sound.enabled",         prefs.sound         ? "1" : "0")
           && setSetting("courses.autoPauseDays", QString::number(prefs.autoPauseDays));
    if (!ok) { rollbackTransaction(); return false; }
    commitTransaction();
    emitDataChanged();
    return true;
}

// ============================================================
//  Task 4.9 — Persisted pomodoro timer state
//
//  We piggy-back on the Settings table under the reserved prefix
//  `pomodoro.state.*`. The seeded keys in Phase 2.11 used
//  `pomodoro.workMinutes` / `pomodoro.breakMinutes` (no `.state.`
//  segment) so there is no risk of collision.
// ============================================================
PomodoroTimerState DatabaseManager::getPomodoroState()
{
    PomodoroTimerState s;
    s.mode             = static_cast<PomodoroTimerState::Mode> (getSettingInt("pomodoro.state.mode",  0));
    s.state            = static_cast<PomodoroTimerState::State>(getSettingInt("pomodoro.state.state", 0));
    s.courseId         = getSettingInt("pomodoro.state.courseId",         -1);
    s.totalSeconds     = getSettingInt("pomodoro.state.totalSeconds",     25 * 60);
    s.remainingSeconds = getSettingInt("pomodoro.state.remainingSeconds", 25 * 60);
    const QString startedRaw = getSetting("pomodoro.state.startedAt");
    if (!startedRaw.isEmpty()) {
        s.startedAt = QDateTime::fromString(startedRaw, Qt::ISODate);
    }
    return s;
}

bool DatabaseManager::savePomodoroState(const PomodoroTimerState& state)
{
    beginTransaction();
    bool ok =
        setSetting("pomodoro.state.mode",             QString::number(static_cast<int>(state.mode)))
     && setSetting("pomodoro.state.state",            QString::number(static_cast<int>(state.state)))
     && setSetting("pomodoro.state.courseId",         QString::number(state.courseId))
     && setSetting("pomodoro.state.totalSeconds",     QString::number(state.totalSeconds))
     && setSetting("pomodoro.state.remainingSeconds", QString::number(state.remainingSeconds))
     && setSetting("pomodoro.state.startedAt",
                   state.startedAt.isValid() ? state.startedAt.toString(Qt::ISODate) : QString());
    if (!ok) { rollbackTransaction(); return false; }
    commitTransaction();
    // No dataChanged() — timer state changes shouldn't trigger UI-wide
    // refreshes. The widget owns its own tick signal.
    return true;
}

// ------------------------------------------------------------
//  lastActivityForUnit(unitId)  (Task 7.5a)
//
//  One indexed SELECT joining SessionsTasks → ActivityLog. Returns
//  the most recent ActivityLog.Timestamp across every session inside
//  this unit, or an invalid QDateTime when the unit has no activity.
//
//  Used by UnitCard to render "Last worked: 3 days ago" subtitles
//  inside the course-detail units grid.
// ------------------------------------------------------------
QDateTime DatabaseManager::lastActivityForUnit(int unitId)
{
    const QString sql = QStringLiteral(
        "SELECT MAX(a.Timestamp) AS last_ts "
        "  FROM ActivityLog a "
        "  JOIN SessionsTasks s ON s.ID = a.ItemID "
        " WHERE s.UnitID = :uid");
    const auto rows = executeSelectQuery(sql, { {":uid", unitId} });
    if (rows.isEmpty()) {
        return QDateTime();
    }
    const QVariant v = rows.first().value("last_ts");
    if (!v.isValid() || v.isNull()) {
        return QDateTime();
    }
    const QString iso = v.toString();
    if (iso.isEmpty()) {
        return QDateTime();
    }
    return QDateTime::fromString(iso, Qt::ISODate);
}

