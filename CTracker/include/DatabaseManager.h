#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QVariantMap>
#include <QList>
#include <QDate>
#include <QDateTime>
#include <QString>

#include "DataStructures.h"

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
    bool createTables();

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
