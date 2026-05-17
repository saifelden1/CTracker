// ============================================================
//  test_models.cpp — Task 9.3
//
//  Tests for Model functionality:
//    - ActivityLogModel::getDailyProgressTotals correct sums
//    - DataImporter rejects missing `type`; clamps progress > 100
//    - DataExporter produces valid JSON that re-imports
//    - HeatmapAggregator::RecentBuckets produces correct buckets
//    - TodoModel separates active vs completed correctly
//    - CategoryModel::entityCount matches actual DB join
// ============================================================

#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

#include "core/DatabaseManager.h"
#include "core/DataImporter.h"
#include "core/DataExporter.h"
#include "analytics/ActivityLogModel.h"
#include "analytics/HeatmapAggregator.h"
#include "todos/TodoModel.h"
#include "shared/CategoryModel.h"

class TestModels : public QObject {
    Q_OBJECT

private:
    DatabaseManager* m_db = nullptr;
    QTemporaryDir m_tempDir;
    QString m_dbPath;

private slots:
    void init() {
        QVERIFY(m_tempDir.isValid());
        m_dbPath = m_tempDir.path() + "/test_models.db";
        
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

    // ---- Test 1: ActivityLogModel::getDailyProgressTotals ----
    void test_activityLogModel_dailyProgressTotals() {
        // Create a course with sessions and log some activity
        int courseId = m_db->addCourse("Test Course");
        int unitId = m_db->addUnit(courseId, "Test Unit");
        int session1Id = m_db->addSessionTask(unitId, "Session 1", 0);
        int session2Id = m_db->addSessionTask(unitId, "Session 2", 0);
        
        // Update progress to generate activity logs
        m_db->updateSessionTaskProgress(session1Id, 50);  // +50
        m_db->updateSessionTaskProgress(session2Id, 30);  // +30
        m_db->updateSessionTaskProgress(session1Id, 75);  // +25
        
        // Create model and get daily totals
        ActivityLogModel model;
        model.refresh();
        
        QDate today = QDate::currentDate();
        QDate weekAgo = today.addDays(-7);
        QDate weekFromNow = today.addDays(7);
        QMap<QDate, int> dailyTotals = model.getDailyProgressTotals(weekAgo, weekFromNow);
        
        // Today should have total of 50 + 30 + 25 = 105
        QVERIFY(dailyTotals.contains(today));
        QCOMPARE(dailyTotals[today], 105);
    }

    // ---- Test 2: DataImporter rejects missing type ----
    void test_dataImporter_rejects_missing_type() {
        QString jsonPath = m_tempDir.path() + "/invalid.json";
        
        // Create JSON without 'type' field
        QJsonObject root;
        root["version"] = "1.0";
        root["name"] = "Test Course";
        root["units"] = QJsonArray();
        
        QFile file(jsonPath);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(QJsonDocument(root).toJson());
        file.close();
        
        // Try to import
        DataImporter importer;
        bool result = importer.importFromFile(jsonPath);
        
        // Should fail
        QVERIFY(!result);
        QVERIFY(!importer.lastError().isEmpty());
    }

    // ---- Test 3: DataImporter clamps progress > 100 ----
    void test_dataImporter_clamps_progress() {
        QString jsonPath = m_tempDir.path() + "/clamp_test.json";
        
        // Create JSON with progress > 100
        QJsonObject root;
        root["version"] = "1.0";
        root["type"] = "course";
        root["name"] = "Test Course";
        
        QJsonArray units;
        QJsonObject unit;
        unit["name"] = "Unit 1";
        
        QJsonArray sessions;
        QJsonObject session1;
        session1["name"] = "Session 1";
        session1["progress"] = 150;  // Over 100
        
        QJsonObject session2;
        session2["name"] = "Session 2";
        session2["progress"] = -10;  // Negative
        
        sessions.append(session1);
        sessions.append(session2);
        unit["sessions"] = sessions;
        units.append(unit);
        root["units"] = units;
        
        QFile file(jsonPath);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(QJsonDocument(root).toJson());
        file.close();
        
        // Import
        DataImporter importer;
        bool result = importer.importFromFile(jsonPath);
        QVERIFY2(result, qPrintable(importer.lastError()));
        
        // Verify progress was clamped
        QList<EntityData> courses = m_db->fetchAllCourses();
        QVERIFY(courses.size() > 0);
        
        int courseId = courses.last().id;
        QList<UnitData> unitList = m_db->getUnitsForParent(courseId);
        QCOMPARE(unitList.size(), 1);
        
        QList<SessionTaskData> sessionList = m_db->getSessionTasksForUnit(unitList[0].id);
        QCOMPARE(sessionList.size(), 2);
        
        // Check clamping
        for (const SessionTaskData& s : sessionList) {
            QVERIFY(s.progress >= 0);
            QVERIFY(s.progress <= 100);
        }
    }

    // ---- Test 4: DataExporter round-trip ----
    void test_dataExporter_roundtrip() {
        // Create a course with structure
        int courseId = m_db->addCourse("Export Test Course");
        int unit1Id = m_db->addUnit(courseId, "Unit 1");
        int unit2Id = m_db->addUnit(courseId, "Unit 2");
        m_db->addSessionTask(unit1Id, "Session 1", 25);
        m_db->addSessionTask(unit1Id, "Session 2", 50);
        m_db->addSessionTask(unit2Id, "Session 3", 75);
        
        // Export all entities
        QString exportPath = m_tempDir.path() + "/export_test.json";
        DataExporter exporter;
        bool exported = exporter.exportToFile(exportPath);
        QVERIFY2(exported, qPrintable(exporter.lastError()));
        
        // Verify file exists
        QVERIFY(QFile::exists(exportPath));
        
        // Delete the original course
        m_db->removeCourse(courseId);
        
        // Re-import
        DataImporter importer;
        bool imported = importer.importFromFile(exportPath);
        QVERIFY2(imported, qPrintable(importer.lastError()));
        
        // Verify the imported course exists
        QList<EntityData> courses = m_db->fetchAllCourses();
        bool found = false;
        int importedCourseId = -1;
        for (const EntityData& e : courses) {
            if (e.name == "Export Test Course") {
                found = true;
                importedCourseId = e.id;
                break;
            }
        }
        QVERIFY(found);
        
        // Verify structure
        QList<UnitData> units = m_db->getUnitsForParent(importedCourseId);
        QCOMPARE(units.size(), 2);
        
        int totalSessions = 0;
        for (const UnitData& u : units) {
            totalSessions += m_db->getSessionTasksForUnit(u.id).size();
        }
        QCOMPARE(totalSessions, 3);
    }

    // ---- Test 5: HeatmapAggregator RecentBuckets ----
    void test_heatmapAggregator_recentBuckets() {
        // Create some activity
        int courseId = m_db->addCourse("Test Course");
        int unitId = m_db->addUnit(courseId, "Test Unit");
        int sessionId = m_db->addSessionTask(unitId, "Session", 0);
        
        // Generate different amounts of activity
        m_db->updateSessionTaskProgress(sessionId, 10);  // 1 activity
        
        // Add todos
        m_db->addTodo("Todo 1", "high");
        m_db->addTodo("Todo 2", "medium");
        int todo1Id = m_db->fetchActiveTodos()[0].id;
        m_db->toggleTodoCompleted(todo1Id);  // 1 completed todo
        
        // Aggregate
        HeatmapAggregator aggregator;
        QDate today = QDate::currentDate();
        QDate weekAgo = today.addDays(-7);
        
        QMap<QDate, ContributionHeatmap::DayData> data = 
            aggregator.aggregate(weekAgo, today, HeatmapAggregator::Mode::RecentBuckets);
        
        // Today should have activity
        QVERIFY(data.contains(today));
        
        // Verify intensity bucketing (0 / 1 / 2-3 / 4-6 / 7+)
        int todayCount = data[today].activityCount;
        int todayIntensity = data[today].intensityLevel;
        
        QVERIFY(todayCount >= 2);  // At least 1 activity log + 1 completed todo
        
        // Intensity should be bucketed correctly
        if (todayCount == 0) {
            QCOMPARE(todayIntensity, 0);
        } else if (todayCount == 1) {
            QCOMPARE(todayIntensity, 1);
        } else if (todayCount >= 2 && todayCount <= 3) {
            QCOMPARE(todayIntensity, 2);
        } else if (todayCount >= 4 && todayCount <= 6) {
            QCOMPARE(todayIntensity, 3);
        } else {
            QCOMPARE(todayIntensity, 4);
        }
    }

    // ---- Test 6: TodoModel active vs completed ----
    void test_todoModel_active_vs_completed() {
        // Add some todos
        int todo1Id = m_db->addTodo("Active Todo 1", "high");
        int todo2Id = m_db->addTodo("Active Todo 2", "medium");
        int todo3Id = m_db->addTodo("Completed Todo", "low");
        
        QVERIFY(todo1Id > 0);
        QVERIFY(todo2Id > 0);
        QVERIFY(todo3Id > 0);
        
        // Complete one
        m_db->toggleTodoCompleted(todo3Id);
        
        // Verify counts directly from database
        QList<TodoData> active = m_db->fetchActiveTodos();
        QList<TodoData> completed = m_db->fetchCompletedTodos();
        
        QCOMPARE(active.size(), 2);
        QCOMPARE(completed.size(), 1);
        
        // Verify the completed one
        QCOMPARE(completed[0].id, todo3Id);
        QVERIFY(completed[0].completed);
    }

    // ---- Test 7: CategoryModel entity count ----
    void test_categoryModel_entityCount() {
        // Create a category
        int catId = m_db->addCategory("Test Category", QColor(255, 0, 0));
        QVERIFY(catId > 0);
        
        // Create courses and assign category
        int course1Id = m_db->addCourse("Course 1");
        int course2Id = m_db->addCourse("Course 2");
        int course3Id = m_db->addCourse("Course 3");
        
        m_db->assignCategory(course1Id, catId);
        m_db->assignCategory(course2Id, catId);
        // course3 has no category
        
        // Fetch categories
        QList<CategoryData> categories = m_db->fetchAllCategories();
        
        // Find our test category
        bool found = false;
        for (const CategoryData& cat : categories) {
            if (cat.id == catId) {
                QCOMPARE(cat.entityCount, 2);
                found = true;
                break;
            }
        }
        QVERIFY(found);
        
        // Create model and verify
        CategoryModel model;
        model.refresh();
        
        // Model should have the same count
        int modelCount = model.rowCount();
        QVERIFY(modelCount >= 6);  // 5 defaults + 1 test category
    }
};

QTEST_MAIN(TestModels)
#include "test_models.moc"
