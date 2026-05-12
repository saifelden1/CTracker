#include "DatabaseManager.h"

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

    // Create all tables if they don't exist yet
    return createTables();
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

// Helper to convert a QVariantMap row → EntityData struct
static EntityData rowToEntity(const QVariantMap& row) {
    EntityData e;
    e.id        = row["ID"].toInt();
    e.name      = row["Name"].toString();
    e.type      = row["Type"].toString();
    e.createdAt = QDateTime::fromString(row["CreatedAt"].toString(), Qt::ISODate);
    return e;
}

QList<EntityData> DatabaseManager::fetchAllEntities() {
    QList<EntityData> list;
    auto rows = executeSelectQuery("SELECT * FROM CoursesProjects ORDER BY CreatedAt ASC");
    for (const auto& row : rows) list.append(rowToEntity(row));
    return list;
}

QList<EntityData> DatabaseManager::fetchAllCourses() {
    QList<EntityData> list;
    auto rows = executeSelectQuery(
        "SELECT * FROM CoursesProjects WHERE Type = 'Course' ORDER BY CreatedAt ASC");
    for (const auto& row : rows) list.append(rowToEntity(row));
    return list;
}

QList<EntityData> DatabaseManager::fetchAllProjects() {
    QList<EntityData> list;
    auto rows = executeSelectQuery(
        "SELECT * FROM CoursesProjects WHERE Type = 'Project' ORDER BY CreatedAt ASC");
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
