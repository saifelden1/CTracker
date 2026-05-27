#pragma once

#include <QWidget>
#include <QString>
#include <QHash>
#include <QList>

#include "core/DataStructures.h"

class QLineEdit;
class QComboBox;
class QPushButton;
class QVBoxLayout;
class QScrollArea;
class TaskBoardCard;

// ============================================================
//  TasksBoardWidget — Phase 10 (Projects tasks board)
//
//  Four-column kanban widget that replaces the old slider-based
//  unit/session list inside ProjectDetailView. It is intentionally
//  a thin view over the existing schema:
//
//    - Reads tasks via DatabaseManager::fetchTasksForProject(projectId)
//    - Groups them by Status into Todo / In progress / Review / Done
//    - Renders each as a TaskBoardCard with the unit name attached
//    - Status changes go through DatabaseManager::setSessionTaskStatus
//      (which also keeps CurrentProgress in sync so the project's
//      overall-progress ring stays accurate)
//    - "+ Task" reuses DatabaseManager::addSessionTask(unit, name);
//      the schema default 'todo' status places it in the first column
//
//  The widget owns its own filter/search/sort state; clients only
//  call setProject() once.
// ============================================================
class TasksBoardWidget : public QWidget {
    Q_OBJECT
public:
    explicit TasksBoardWidget(QWidget* parent = nullptr);

    // Switch the board to a different project (-1 clears).
    // Also re-reads project priority so cards get the correct stripe.
    void setProject(int projectId);

public slots:
    // Called by ProjectDetailView on DatabaseManager::dataChanged.
    void refresh();

private slots:
    void onAddTaskClicked();
    void onSearchChanged(const QString& text);
    void onUnitFilterChanged(int index);
    void onSortChanged(int index);
    void onCardStatusChange(int taskId, const QString& newStatus);
    void onCardDeleteRequested(int taskId);

private:
    void setupUi();
    void rebuild();                       // full repaint from m_tasks
    void clearColumns();
    QList<SessionTaskData> currentlyVisible() const;

    // ── Toolbar ──
    QLineEdit*   m_searchInput  = nullptr;
    QComboBox*   m_unitFilter   = nullptr;
    QComboBox*   m_sortCombo    = nullptr;
    QPushButton* m_addTaskBtn   = nullptr;

    // ── Columns (Todo / In progress / Review / Done) ──
    struct Column {
        QString      status;
        QWidget*     header     = nullptr;
        class QLabel* countLabel = nullptr;
        QWidget*     body       = nullptr;
        QVBoxLayout* bodyLayout = nullptr;
    };
    QList<Column> m_columns;

    // ── State ──
    int                       m_projectId       = -1;
    QString                   m_projectPriority = "medium";
    QList<SessionTaskData>    m_tasks;
    QHash<int, TaskBoardCard*> m_cardsByTaskId;
    QString                   m_search;
    int                       m_unitFilterId    = -1;   // -1 = all
    int                       m_sortIndex       = 0;
};
