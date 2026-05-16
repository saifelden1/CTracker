#pragma once

#include "courses/EntityDetailView.h"

class QLabel;
class QPushButton;

// ============================================================
//  CourseDetailView — Task 7.5
//
//  Extends EntityDetailView with course-specific UI:
//    - Pause/Resume toggle button (binds to setCourseStatus)
//    - Status badge ("Active" / "Paused" / "Completed") in title bar
//    - Session count subtitle
//
//  The base class provides the scroll area of UnitExpandableWidget
//  items, add-unit / add-session / delete buttons, and the overall
//  progress ring. This class only adds the course-specific chrome.
// ============================================================
class CourseDetailView : public EntityDetailView {
    Q_OBJECT
public:
    explicit CourseDetailView(QWidget* parent = nullptr);

    void loadEntity(int entityId) override;

    // Convenience alias used by MainWindow wiring (mirrors design.md).
    void loadCourse(int courseId) { loadEntity(courseId); }

protected slots:
    void onDataChanged() override;

private slots:
    void onStatusToggleClicked();

private:
    void setupCourseChrome();   // inserts course-specific widgets into m_titleLayout
    void refreshStatusUI();

    QString m_courseStatus = "active";

    QPushButton* m_statusToggleBtn = nullptr;
    QLabel*      m_statusBadge     = nullptr;
    QLabel*      m_sessionCountLabel = nullptr;
};
