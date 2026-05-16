#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QVariantMap>
#include <QList>
#include <QDate>
#include <QDateTime>
#include <QString>

#include "core/DataStructures.h"

// ============================================================
//  DatabaseManager.h
//
//  This class is the ONLY place in the app that talks to SQLite.
//  It is a Singleton — meaning there is exactly ONE instance of
//  it for the whole lifetime of the app.
//
//  WHY SINGLETON?
//  A database connection is an expensive resource. You don't want
//  10 different parts of the UI all opening their own connections.
//  The Singleton pattern guarantees one connection, shared by all.
// ============================================================

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    // ---- Singleton access ----
    // Call DatabaseManager::instance() from anywhere to get the one manager.
    static DatabaseManager* instance();

    // ---- Lifecycle ----
    bool initialize(const QString& dbPath = QString());
    bool isOpen() const;
    void close();

    // ---- Entity (Course / Project) operations ----
    int  addCourse(const QString& name);
    int  addProject(const QString& name);
    bool removeCourse(int courseId);
    bool removeProject(int projectId);
    bool renameCourse(int courseId, const QString& newName);
    bool renameProject(int projectId, const QString& newName);
    QList<EntityData> fetchAllEntities();
    QList<EntityData> fetchAllCourses();
    QList<EntityData> fetchAllProjects();

    // ---- Unit operations ----
    int  addUnit(int parentId, const QString& name);
    bool removeUnit(int unitId);
    bool renameUnit(int unitId, const QString& newName);
    QList<UnitData> getUnitsForParent(int parentId);

    // ---- Session / Task operations ----
    int  addSessionTask(int unitId, const QString& name, int progress = 0);
    bool removeSessionTask(int sessionId);
    bool renameSessionTask(int sessionId, const QString& newName);
    bool updateSessionTaskProgress(int sessionId, int progress);
    int  getSessionTaskProgress(int sessionId);
    QList<SessionTaskData> getSessionTasksForUnit(int unitId);

    // ---- Activity log operations ----
    int  logActivity(int itemId, int oldVal, int newVal,
                     const QString& type,
                     const QDateTime& ts = QDateTime::currentDateTime());
    QList<ActivityLogEntry> getActivityLog(const QDate& from, const QDate& to);
    QList<ActivityLogEntry> getActivityLogForItem(int itemId);

    // ---- Schema versioning (Task 2.8) ----
    //
    // The database carries its own version number in a tiny key/value
    // table called `SchemaInfo`. Whenever we ship a structural change
    // (new tables, new columns, new indexes) we bump the version and
    // implement the diff in `migrate()`.
    //
    // currentSchemaVersion() returns 0 if `SchemaInfo` does not exist
    // yet (fresh install) or the version is missing for any reason.
    //
    // migrate(from, to) is *idempotent*: re-running it must not fail
    // and must not corrupt data. It wraps every step in a single
    // transaction so a half-applied migration is impossible.
    int  currentSchemaVersion();
    bool migrate(int from, int to);

signals:
    // Emitted whenever data changes so the UI can refresh itself
    void dataChanged();
    // Emitted on any database error with a human-readable message
    void databaseError(const QString& errorMessage);

private:
    // Private constructor — only instance() can create this object
    explicit DatabaseManager(QObject* parent = nullptr);
    ~DatabaseManager() = default;

    // Disable copy — a Singleton must never be copied
    DatabaseManager(const DatabaseManager&)            = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    // ---- Internal helpers ----
    bool createTables();          // v1 schema (idempotent)
    bool createV2Tables();        // Task 2.9 — new tables + indexes (idempotent)
    bool addV2Columns();          // Task 2.10 — column additions on CoursesProjects
    bool seedV2Defaults();        // Task 2.11 — default categories + settings

    // pragma_table_info-based check used to keep `ALTER TABLE` idempotent.
    bool columnExists(const QString& table, const QString& column);

    // Execute a write query (INSERT / UPDATE / DELETE)
    // params = named placeholders, e.g. {":name", "ROS 2"}
    bool executeQuery(const QString& sql, const QVariantMap& params = {});

    // Execute a read query (SELECT) and return a list of rows.
    // Each row is a QVariantMap: column_name → value
    QList<QVariantMap> executeSelectQuery(const QString& sql,
                                          const QVariantMap& params = {});

    // Transaction helpers
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

    // Helper to add Course OR Project (shared logic)
    int addEntity(const QString& name, const QString& type);

    // Helper to rename Course OR Project (type-guarded so a Course id
    // can't rename a Project row and vice-versa)
    bool renameEntity(int entityId, const QString& type, const QString& newName);

    // The actual SQLite connection object
    QSqlDatabase m_database;

    // Static pointer — holds the one instance
    static DatabaseManager* s_instance;
};
