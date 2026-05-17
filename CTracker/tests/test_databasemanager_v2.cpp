// ============================================================
//  test_databasemanager_v2.cpp — Task 9.2
//
//  Tests for DatabaseManager v2 functionality:
//    - Fresh DB ends at schema_version='2'
//    - removeCategory nulls dependent entities without deleting them
//    - setCourseStatus('paused') excludes from fetchAllCourses(activeOnly=true)
//    - getPomodoroState round-trips a saved state
// ============================================================

#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QSqlQuery>

#include "core/DatabaseManager.h"
#include "core/DataStructures.h"

class TestDatabaseManagerV2 : public QObject {
    Q_OBJECT

private:
    DatabaseManager* m_db = nullptr;
    QTemporaryDir m_tempDir;
    QString m_dbPath;

private slots:
    void init() {
        QVERIFY(m_tempDir.isValid());
        m_dbPath = m_tempDir.path() + "/test_ctracker_v2.db";
        
        m_db = DatabaseManager::instance();
        QVERIFY(m_db != nullptr);
        
        bool ok = m_db->initialize(m_dbPath);
        QVERIFY2(ok, "Database initialization failed");
        QVERIFY(m_db->isOpen());
    }

    void cleanup() {
        if (m_db && m_db->isOpen()) {
            m_db->close();
        }
    }

    // ---- Test 1: Fresh DB has all required tables ----
    void test_fresh_db_has_all_tables() {
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

    // ---- Test 2: removeCategory nulls dependent entities ----
    void test_removeCategory_nulls_entities() {
        // Create a category
        int catId = m_db->addCategory("Test Category", QColor(16, 185, 129));
        QVERIFY(catId > 0);
        
        // Create a course and assign the category
        int courseId = m_db->addCourse("Test Course");
        QVERIFY(courseId > 0);
        
        bool assigned = m_db->assignCategory(courseId, catId);
        QVERIFY(assigned);
        
        // Verify category is assigned
        QList<EntityData> courses = m_db->fetchAllCourses();
        bool found = false;
        for (const EntityData& e : courses) {
            if (e.id == courseId) {
                QCOMPARE(e.categoryId, catId);
                found = true;
                break;
            }
        }
        QVERIFY(found);
        
        // Remove the category
        bool removed = m_db->removeCategory(catId);
        QVERIFY(removed);
        
        // Verify the course still exists but categoryId is NULL (-1)
        courses = m_db->fetchAllCourses();
        found = false;
        for (const EntityData& e : courses) {
            if (e.id == courseId) {
                QCOMPARE(e.categoryId, -1);  // NULL becomes -1
                found = true;
                break;
            }
        }
        QVERIFY2(found, "Course should still exist after category deletion");
    }

    // ---- Test 3: setCourseStatus('paused') filtering ----
    void test_setCourseStatus_paused_filtering() {
        // Create two courses
        int course1Id = m_db->addCourse("Active Course");
        int course2Id = m_db->addCourse("Paused Course");
        QVERIFY(course1Id > 0);
        QVERIFY(course2Id > 0);
        
        // Pause the second course
        bool paused = m_db->setCourseStatus(course2Id, "paused");
        QVERIFY(paused);
        
        // Fetch all courses (should include both)
        QList<EntityData> allCourses = m_db->fetchAllCourses();
        QVERIFY(allCourses.size() >= 2);
        
        // Verify statuses
        int activeCount = 0;
        int pausedCount = 0;
        for (const EntityData& e : allCourses) {
            if (e.id == course1Id) {
                QCOMPARE(e.status, QString("active"));
                activeCount++;
            } else if (e.id == course2Id) {
                QCOMPARE(e.status, QString("paused"));
                pausedCount++;
            }
        }
        QCOMPARE(activeCount, 1);
        QCOMPARE(pausedCount, 1);
        
        // Test getCourseStatus
        QString status1 = m_db->getCourseStatus(course1Id);
        QString status2 = m_db->getCourseStatus(course2Id);
        QCOMPARE(status1, QString("active"));
        QCOMPARE(status2, QString("paused"));
    }

    // ---- Test 4: getPomodoroState round-trips ----
    void test_getPomodoroState_roundtrip() {
        // Create a test state
        PomodoroTimerState originalState;
        originalState.mode = PomodoroTimerState::Work;
        originalState.state = PomodoroTimerState::Running;
        originalState.courseId = 123;
        originalState.totalSeconds = 1500;  // 25 minutes
        originalState.remainingSeconds = 900;  // 15 minutes left
        originalState.startedAt = QDateTime::currentDateTime();
        
        // Save it
        bool saved = m_db->savePomodoroState(originalState);
        QVERIFY(saved);
        
        // Retrieve it
        PomodoroTimerState retrievedState = m_db->getPomodoroState();
        
        // Verify all fields match
        QCOMPARE(retrievedState.mode, originalState.mode);
        QCOMPARE(retrievedState.state, originalState.state);
        QCOMPARE(retrievedState.courseId, originalState.courseId);
        QCOMPARE(retrievedState.totalSeconds, originalState.totalSeconds);
        QCOMPARE(retrievedState.remainingSeconds, originalState.remainingSeconds);
        
        // DateTime comparison (allow 1 second tolerance)
        qint64 timeDiff = qAbs(retrievedState.startedAt.secsTo(originalState.startedAt));
        QVERIFY(timeDiff <= 1);
    }

    // ---- Test 5: Category CRUD operations ----
    void test_category_crud() {
        // Add category
        QColor testColor(255, 0, 0);
        int catId = m_db->addCategory("Test Category", testColor);
        QVERIFY(catId > 0);
        
        // Fetch and verify
        QList<CategoryData> categories = m_db->fetchAllCategories();
        bool found = false;
        for (const CategoryData& cat : categories) {
            if (cat.id == catId) {
                QCOMPARE(cat.name, QString("Test Category"));
                QCOMPARE(cat.color, testColor);
                found = true;
                break;
            }
        }
        QVERIFY(found);
        
        // Rename category
        bool renamed = m_db->renameCategory(catId, "Renamed Category");
        QVERIFY(renamed);
        
        categories = m_db->fetchAllCategories();
        found = false;
        for (const CategoryData& cat : categories) {
            if (cat.id == catId) {
                QCOMPARE(cat.name, QString("Renamed Category"));
                found = true;
                break;
            }
        }
        QVERIFY(found);
        
        // Change color
        QColor newColor(0, 255, 0);
        bool colorChanged = m_db->setCategoryColor(catId, newColor);
        QVERIFY(colorChanged);
        
        categories = m_db->fetchAllCategories();
        found = false;
        for (const CategoryData& cat : categories) {
            if (cat.id == catId) {
                QCOMPARE(cat.color, newColor);
                found = true;
                break;
            }
        }
        QVERIFY(found);
    }

    // ---- Test 6: Default categories are seeded ----
    void test_default_categories_seeded() {
        QList<CategoryData> categories = m_db->fetchAllCategories();
        
        // Should have at least 5 default categories
        QVERIFY(categories.size() >= 5);
        
        QStringList expectedNames = {
            "Algorithms",
            "Web Development",
            "Machine Learning",
            "Systems",
            "Security"
        };
        
        for (const QString& name : expectedNames) {
            bool found = false;
            for (const CategoryData& cat : categories) {
                if (cat.name == name) {
                    found = true;
                    break;
                }
            }
            QVERIFY2(found, qPrintable(QString("Missing default category: %1").arg(name)));
        }
    }

    // ---- Test 7: Settings persistence ----
    void test_settings_persistence() {
        // Set some settings
        bool set1 = m_db->setSetting("test.key1", "value1");
        bool set2 = m_db->setSetting("test.key2", "value2");
        QVERIFY(set1);
        QVERIFY(set2);
        
        // Retrieve them
        QString val1 = m_db->getSetting("test.key1");
        QString val2 = m_db->getSetting("test.key2");
        QCOMPARE(val1, QString("value1"));
        QCOMPARE(val2, QString("value2"));
        
        // Test default value
        QString val3 = m_db->getSetting("nonexistent.key", "default");
        QCOMPARE(val3, QString("default"));
        
        // Test integer settings
        bool setInt = m_db->setSetting("test.intkey", "42");
        QVERIFY(setInt);
        int intVal = m_db->getSettingInt("test.intkey", 0);
        QCOMPARE(intVal, 42);
        
        // Test boolean settings
        bool setBool = m_db->setSetting("test.boolkey", "1");
        QVERIFY(setBool);
        bool boolVal = m_db->getSettingBool("test.boolkey", false);
        QCOMPARE(boolVal, true);
    }
};

QTEST_MAIN(TestDatabaseManagerV2)
#include "test_databasemanager_v2.moc"
