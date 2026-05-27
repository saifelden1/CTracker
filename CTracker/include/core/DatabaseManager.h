#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QVariantMap>
#include <QList>
#include <QSet>
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
    
    // ---- Batch update mode (prevents signal spam) ----
    void beginBatchUpdate();  // Temporarily block dataChanged signals
    void endBatchUpdate();    // Re-enable and emit one dataChanged

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

    // ---- Phase 10: Projects tasks board ----
    // Status writer also keeps CurrentProgress in sync (todo=0, done=100,
    // in_progress/review left at the current value or bumped to 1 if 0)
    // so the existing overall-progress ring stays correct.
    bool setSessionTaskStatus     (int sessionId, const QString& status);
    bool setSessionTaskDueDate    (int sessionId, const QDate&   dueDate);
    bool setSessionTaskDescription(int sessionId, const QString& description);
    // All tasks of a project, with their owning unit name attached.
    QList<SessionTaskData> fetchTasksForProject(int projectId);
    // Single task fetch (used by TaskDetailDialog so it doesn't rely on
    // the board's in-memory list). Returns id == -1 if not found.
    SessionTaskData getSessionTask(int sessionId);

    // ---- Activity log operations ----
    int  logActivity(int itemId, int oldVal, int newVal,
                     const QString& type,
                     const QDateTime& ts = QDateTime::currentDateTime());
    QList<ActivityLogEntry> getActivityLog(const QDate& from, const QDate& to);
    QList<ActivityLogEntry> getActivityLogForItem(int itemId);

    // ============================================================
    //  Phase 4 — Extended v2 API
    //
    //  Everything below was added once the v2 schema landed (Phase 2).

    // ---- Test accessor (for unit tests only) ----
    QSqlDatabase database() const { return m_database; }
    //  Each block maps 1:1 to one Phase-4 task in tasks.md so the
    //  surface is easy to audit against the spec.
    //
    //  Convention: every mutating call emits dataChanged() on success
    //  so any subscribed view refreshes itself. Reads never emit.
    // ============================================================

    // ---- Categories (Task 4.2) ----
    int  addCategory(const QString& name, const QColor& color);
    bool renameCategory(int id, const QString& newName);
    bool setCategoryColor(int id, const QColor& color);
    bool removeCategory(int id);                                   // NULLs dependent entities
    QList<CategoryData> fetchAllCategories();                      // includes entityCount
    bool assignCategory(int entityId, int categoryId);             // categoryId == -1 clears

    // ---- Course status (Task 4.3) ----
    bool    setCourseStatus(int courseId, const QString& status);
    QString getCourseStatus(int courseId);

    // ---- ProjectMeta (Task 4.4) ----
    bool             upsertProjectMeta(const ProjectMetaData& meta);
    ProjectMetaData  getProjectMeta(int projectId);                // defaults if no row
    bool setProjectPriority(int projectId, const QString& priority);
    bool setProjectDeadline(int projectId, const QDate& deadline);
    bool setProjectTeam    (int projectId, const QStringList& team);
    bool setProjectLinks   (int projectId, const QList<ProjectMetaData::Link>& links);

    // ---- Todos (Task 4.5) ----
    int  addTodo(const QString& title, const QString& priority = "medium");
    bool toggleTodoCompleted(int id);
    bool setTodoPriority(int id, const QString& priority);
    bool removeTodo(int id);
    QList<TodoData> fetchActiveTodos();
    QList<TodoData> fetchCompletedTodos();
    int  countCompletedTodosOn(const QDate& date);

    // ---- Pomodoro sessions (Task 4.6) ----
    int  insertPomodoroSession(int courseId, int durationMin, const QString& mode);
    QList<PomodoroSessionData> fetchRecentSessions(int limit);
    QList<PomodoroSessionData> fetchSessionsOn(const QDate& date);
    int  totalMinutesOn(const QDate& date);                        // sum of work minutes

    // ---- Calendar day details (Task 4.7) ----
    CalendarDayData getDay(const QDate& date);                     // empty struct if missing
    bool            upsertDay(const CalendarDayData& data);
    QSet<QDate>     datesWithContent(const QDate& from, const QDate& to);

    // ---- Settings k/v + typed wrappers (Task 4.8) ----
    QString getSetting(const QString& key, const QString& defaultValue = {});
    bool    setSetting(const QString& key, const QString& value);
    int     getSettingInt (const QString& key, int  defaultValue = 0);
    bool    getSettingBool(const QString& key, bool defaultValue = false);

    ProfileData     getProfile();
    bool            setProfile(const ProfileData& profile);
    PreferencesData getPreferences();
    bool            setPreferences(const PreferencesData& prefs);

    // ---- Persisted pomodoro timer state (Task 4.9) ----
    PomodoroTimerState getPomodoroState();
    bool               savePomodoroState(const PomodoroTimerState& state);

    // ---- Clear all data (destructive reset) ----
    bool clearAllData();   // DELETE all rows, reset sequences, re-seed defaults

    // ---- Unit activity (Task 7.5a — UnitCard subtitle) ----
    // Returns MAX(ActivityLog.Timestamp) across every session belonging
    // to the unit. Returns an invalid QDateTime when the unit has no
    // activity yet. Read-only; does not emit dataChanged.
    QDateTime lastActivityForUnit(int unitId);

signals:
    // Emitted whenever data changes so the UI can refresh itself
    void dataChanged();
    // Emitted on any database error with a human-readable message
    void databaseError(const QString& errorMessage);

private:
    // Private constructor — only instance() can create this object
    explicit DatabaseManager(QObject* parent = nullptr);
    
    // Helper to emit dataChanged respecting batch mode
    void emitDataChanged();
    ~DatabaseManager() = default;

    // Disable copy — a Singleton must never be copied
    DatabaseManager(const DatabaseManager&)            = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    // ---- Internal helpers ----
    bool createTables();          // Creates all 10 unified schema tables
    bool seedDefaults();          // Default categories + settings

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
    
    // Batch update mode flag
    bool m_batchUpdateMode = false;
    bool m_pendingDataChanged = false;
    bool m_dataChangedQueued = false;

    // Static pointer — holds the one instance
    static DatabaseManager* s_instance;
};
