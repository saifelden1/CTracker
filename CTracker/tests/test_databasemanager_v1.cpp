// ============================================================
//  test_databasemanager_v1.cpp — Task 9.1
//
//  Tests for DatabaseManager v1 functionality:
//    - initialize() creates all base tables
//    - addCourse / addProject return valid IDs
//    - addUnit fails on invalid parent
//    - updateSessionTaskProgress out-of-range is rejected
//    - Cascade delete: deleting a Course removes Units & Sessions
//    - logActivity is NOT called when oldValue == newValue
// ============================================================

#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QSqlQuery>
#include <QSqlError>

#include "core/DatabaseManager.h"
#include "core/DataStructures.h"

class TestDatabaseManagerV1 : public QObject {
    Q_OBJECT

private:
    DatabaseManager* m_db = nullptr;
    QTemporaryDir m_tempDir;
    QString m_dbPath;

private slots:
    // Called before each test function
    void init() {
        QVERIFY(m_tempDir.isValid());
        m_dbPath = m_tempDir.path() + "/test_ctracker.db";
        
        m_db = DatabaseManager::instance();
        QVERIFY(m_db != nullptr);
        
        bool ok = m_db->initialize(m_dbPath);
        QVERIFY2(ok, "Database initialization failed");
        QVERIFY(m_db->isOpen());
    }

    // Called after each test function
    void cleanup() {
        if (m_db && m_db->isOpen()) {
            m_db->close();
        }
    }

    // ---- Test 1: initialize() creates all base tables ----
    void test_initialize_creates_tables() {
        // Verify that all required tables exist
        QStringList requiredTables = {
            "CoursesProjects",
            "Units",
            "SessionsTasks",
            "ActivityLog",
            "Categories",
            "ProjectMeta",
            "Todos",
            "PomodoroSessions",
            "CalendarDayDetails",
            "Settings"
        };

        QSqlQuery query(m_db->database());
        query.exec("SELECT name FROM sqlite_master WHERE type='table'");
        
        QStringList actualTables;
        while (query.next()) {
            actualTables << query.value(0).toString();
        }

        for (const QString& table : requiredTables) {
            QVERIFY2(actualTables.contains(table), 
                     qPrintable(QString("Missing table: %1").arg(table)));
        }
    }

    // ---- Test 2: addCourse / addProject return valid IDs ----
    void test_addCourse_returns_valid_id() {
        int courseId = m_db->addCourse("Test Course");
        QVERIFY(courseId > 0);
        
        // Verify it was actually inserted
        QList<EntityData> courses = m_db->fetchAllCourses();
        bool found = false;
        for (const EntityData& e : courses) {
            if (e.id == courseId && e.name == "Test Course") {
                found = true;
                QCOMPARE(e.type, QString("Course"));
                break;
            }
        }
        QVERIFY(found);
    }

    void test_addProject_returns_valid_id() {
        int projectId = m_db->addProject("Test Project");
        QVERIFY(projectId > 0);
        
        // Verify it was actually inserted
        QList<EntityData> projects = m_db->fetchAllProjects();
        bool found = false;
        for (const EntityData& e : projects) {
            if (e.id == projectId && e.name == "Test Project") {
                found = true;
                QCOMPARE(e.type, QString("Project"));
                break;
            }
        }
        QVERIFY(found);
    }

    // ---- Test 3: addUnit fails on invalid parent ----
    void test_addUnit_fails_on_invalid_parent() {
        int invalidParentId = 99999;
        int unitId = m_db->addUnit(invalidParentId, "Invalid Unit");
        
        // Should return -1 or fail due to foreign key constraint
        QVERIFY(unitId < 0);
    }

    // ---- Test 4: updateSessionTaskProgress out-of-range is rejected ----
    void test_updateSessionTaskProgress_rejects_out_of_range() {
        // Create a course, unit, and session
        int courseId = m_db->addCourse("Test Course");
        QVERIFY(courseId > 0);
        
        int unitId = m_db->addUnit(courseId, "Test Unit");
        QVERIFY(unitId > 0);
        
        int sessionId = m_db->addSessionTask(unitId, "Test Session", 50);
        QVERIFY(sessionId > 0);
        
        // Try to set progress to -10 (should fail or clamp to 0)
        bool result = m_db->updateSessionTaskProgress(sessionId, -10);
        
        // Verify progress is clamped or rejected
        QList<SessionTaskData> sessions = m_db->getSessionTasksForUnit(unitId);
        QCOMPARE(sessions.size(), 1);
        QVERIFY(sessions[0].progress >= 0);
        QVERIFY(sessions[0].progress <= 100);
        
        // Try to set progress to 150 (should fail or clamp to 100)
        result = m_db->updateSessionTaskProgress(sessionId, 150);
        
        sessions = m_db->getSessionTasksForUnit(unitId);
        QCOMPARE(sessions.size(), 1);
        QVERIFY(sessions[0].progress >= 0);
        QVERIFY(sessions[0].progress <= 100);
    }

    // ---- Test 5: Cascade delete ----
    void test_cascade_delete_course_removes_units_and_sessions() {
        // Create a course with units and sessions
        int courseId = m_db->addCourse("Test Course");
        QVERIFY(courseId > 0);
        
        int unit1Id = m_db->addUnit(courseId, "Unit 1");
        int unit2Id = m_db->addUnit(courseId, "Unit 2");
        QVERIFY(unit1Id > 0);
        QVERIFY(unit2Id > 0);
        
        int session1Id = m_db->addSessionTask(unit1Id, "Session 1", 0);
        int session2Id = m_db->addSessionTask(unit1Id, "Session 2", 0);
        int session3Id = m_db->addSessionTask(unit2Id, "Session 3", 0);
        QVERIFY(session1Id > 0);
        QVERIFY(session2Id > 0);
        QVERIFY(session3Id > 0);
        
        // Verify they exist
        QList<UnitData> units = m_db->getUnitsForParent(courseId);
        QCOMPARE(units.size(), 2);
        
        QList<SessionTaskData> sessions1 = m_db->getSessionTasksForUnit(unit1Id);
        QList<SessionTaskData> sessions2 = m_db->getSessionTasksForUnit(unit2Id);
        QCOMPARE(sessions1.size(), 2);
        QCOMPARE(sessions2.size(), 1);
        
        // Delete the course
        bool deleted = m_db->removeCourse(courseId);
        QVERIFY(deleted);
        
        // Verify units are gone
        units = m_db->getUnitsForParent(courseId);
        QCOMPARE(units.size(), 0);
        
        // Verify sessions are gone (cascade delete)
        sessions1 = m_db->getSessionTasksForUnit(unit1Id);
        sessions2 = m_db->getSessionTasksForUnit(unit2Id);
        QCOMPARE(sessions1.size(), 0);
        QCOMPARE(sessions2.size(), 0);
    }

    // ---- Test 6: logActivity is NOT called when oldValue == newValue ----
    void test_no_activity_log_when_progress_unchanged() {
        // Create a course, unit, and session
        int courseId = m_db->addCourse("Test Course");
        int unitId = m_db->addUnit(courseId, "Test Unit");
        int sessionId = m_db->addSessionTask(unitId, "Test Session", 50);
        
        // Get initial activity log count for a wide date range
        QDate today = QDate::currentDate();
        QDate weekAgo = today.addDays(-7);
        QDate weekFromNow = today.addDays(7);
        QList<ActivityLogEntry> initialLogs = m_db->getActivityLog(weekAgo, weekFromNow);
        int initialCount = initialLogs.size();
        
        // Update progress to the same value (50 -> 50)
        m_db->updateSessionTaskProgress(sessionId, 50);
        
        // Verify no new activity log entry was created
        QList<ActivityLogEntry> afterLogs = m_db->getActivityLog(weekAgo, weekFromNow);
        QCOMPARE(afterLogs.size(), initialCount);
    }

    // ---- Additional test: Verify progress is actually updated ----
    void test_updateSessionTaskProgress_updates_correctly() {
        int courseId = m_db->addCourse("Test Course");
        int unitId = m_db->addUnit(courseId, "Test Unit");
        int sessionId = m_db->addSessionTask(unitId, "Test Session", 0);
        
        // Update progress to 75
        bool result = m_db->updateSessionTaskProgress(sessionId, 75);
        QVERIFY(result);
        
        // Verify the update
        QList<SessionTaskData> sessions = m_db->getSessionTasksForUnit(unitId);
        QCOMPARE(sessions.size(), 1);
        QCOMPARE(sessions[0].progress, 75);
        
        // Verify activity log was created
        QDate today = QDate::currentDate();
        QDate weekAgo = today.addDays(-7);
        QDate weekFromNow = today.addDays(7);
        QList<ActivityLogEntry> logs = m_db->getActivityLog(weekAgo, weekFromNow);
        QVERIFY(logs.size() > 0);
        
        // Find the log entry for this session
        bool foundLog = false;
        for (const ActivityLogEntry& log : logs) {
            if (log.itemId == sessionId) {
                QCOMPARE(log.oldValue, 0);
                QCOMPARE(log.newValue, 75);
                QCOMPARE(log.progressDelta, 75);
                foundLog = true;
                break;
            }
        }
        QVERIFY(foundLog);
    }
};

QTEST_MAIN(TestDatabaseManagerV1)
#include "test_databasemanager_v1.moc"
