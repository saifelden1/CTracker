#pragma once

#include "courses/EntityDetailView.h"

#include <QList>

class QLabel;
class QPushButton;
class QStackedWidget;
class QWidget;
class QGridLayout;
class QScrollArea;
class QResizeEvent;
class QMenu;
class UnitCard;
class UnitSessionsView;

// ============================================================
//  CourseDetailView — Task 7.5 + 7.5a (units-as-cards redesign)
//
//  Extends EntityDetailView with course-specific chrome:
//    - Pause/Resume toggle button (binds to setCourseStatus)
//    - Status badge ("Active" / "Paused" / "Completed")
//    - Session count subtitle below the title bar
//
//  Plus, the content area is rebuilt as an inner QStackedWidget:
//    - Page 0: responsive grid of UnitCard widgets + "+ New Unit" tile
//    - Page 1: UnitSessionsView focused on one unit
//
//  Clicking a unit card swaps to Page 1; the sub-view's backRequested
//  signal swaps back to Page 0. The base class's m_scrollArea +
//  m_unitsContainer are hidden — only ProjectDetailView still uses
//  them (it keeps the accordion shape for now).
// ============================================================
class CourseDetailView : public EntityDetailView {
    Q_OBJECT
public:
    explicit CourseDetailView(QWidget* parent = nullptr);

    void loadEntity(int entityId) override;

    // Convenience alias used by MainWindow wiring (mirrors design.md).
    void loadCourse(int courseId) { loadEntity(courseId); }

protected:
    void resizeEvent(QResizeEvent* event) override;

protected slots:
    void onDataChanged() override;

private slots:
    void onStatusToggleClicked();
    void onCategoryButtonClicked();
    void onUnitCardClicked(int unitId);
    void onAddUnitTileClicked();
    void onSubViewBackRequested();
    void onSubViewSessionProgressChanged(int sessionId, int oldValue, int newValue);
    void onSubViewSessionRenamed(int sessionId, const QString& newName);

private:
    void setupCourseChrome();
    void buildInnerStack();
    void refreshStatusUI();
    void refreshCategoryUI();

    // Units-grid management
    void rebuildUnitsGrid();
    void clearUnitCards();
    void updateUnitColumnCount();   // 1–4 based on width
    void showUnitsPage();
    void showSessionsPage(int unitId);

    QString m_courseStatus = "active";

    // Course chrome
    QPushButton* m_statusToggleBtn   = nullptr;
    QLabel*      m_statusBadge       = nullptr;
    QLabel*      m_sessionCountLabel = nullptr;

    // Category chrome
    QPushButton* m_categoryBtn       = nullptr;
    int          m_categoryId        = -1;

    // Inner stack (Page 0 = grid, Page 1 = sessions view)
    QStackedWidget*   m_innerStack    = nullptr;
    QWidget*          m_unitsPage     = nullptr;
    QScrollArea*      m_unitsScroll   = nullptr;
    QWidget*          m_unitsContainerInner = nullptr;
    QGridLayout*      m_unitsGrid     = nullptr;
    UnitSessionsView* m_sessionsView  = nullptr;

    QList<UnitCard*>  m_unitCards;
    QWidget*          m_addUnitTile   = nullptr;
    int               m_unitColumnCount = 4;
};
