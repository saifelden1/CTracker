#pragma once

#include "courses/EntityDetailView.h"

#include <QDate>
#include <QStringList>

#include "core/DataStructures.h"

class QLabel;
class QPushButton;
class QVBoxLayout;
class QScrollArea;
class TasksBoardWidget;

// ============================================================
//  ProjectDetailView — Task 7.6
//
//  Extends EntityDetailView with project-specific UI:
//    - Two-column layout: left = task checklist (units/sessions),
//      right = project info panel (deadline, team, links)
//    - Status / priority / deadline badges in the title bar
//    - Links open in the default browser via QDesktopServices
//
//  The base class provides the scroll area of UnitExpandableWidget
//  items, add-unit / add-session / delete buttons, and the overall
//  progress ring. This class restructures the content into two
//  columns and adds the project info sidebar.
// ============================================================
class ProjectDetailView : public EntityDetailView {
    Q_OBJECT
public:
    explicit ProjectDetailView(QWidget* parent = nullptr);

    void loadEntity(int entityId) override;

    // Convenience alias used by MainWindow wiring (mirrors design.md).
    void loadProject(int projectId) { loadEntity(projectId); }

protected slots:
    void onDataChanged() override;

protected:
    // Catches double-clicks on the Description / Team / Links sections
    // so the user can edit them in place without us subclassing every
    // QLabel/QWidget that lives inside the info panel.
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void setupProjectChrome();   // inserts project-specific widgets + two-column layout
    void refreshProjectInfo();   // refreshes sidebar badges and info panel
    void refreshDeadlineBadge();

    // Inline-edit handlers — each opens a small modal, writes through
    // DatabaseManager::upsertProjectMeta, and lets dataChanged() drive
    // the refresh so the panel stays in sync with the DB.
    void editDescription();
    void editTeam();
    void editLinks();

    // ── Project state ──
    QString        m_projectStatus   = "active";
    ProjectMetaData m_projectMeta;

    // ── Title bar badges ──
    QLabel* m_statusBadge      = nullptr;
    QLabel* m_priorityBadge    = nullptr;
    QLabel* m_deadlineBadge    = nullptr;

    // ── Left-column tasks board (Phase 10) ──
    TasksBoardWidget* m_tasksBoard  = nullptr;

    // ── Right-column info panel ──
    QWidget*      m_infoPanel       = nullptr;
    QLabel*       m_descLabel       = nullptr;
    QLabel*       m_deadlineLabel   = nullptr;
    QWidget*      m_teamContainer   = nullptr;
    QVBoxLayout*  m_teamLayout      = nullptr;
    QWidget*      m_linksContainer  = nullptr;
    QVBoxLayout*  m_linksLayout     = nullptr;
};
