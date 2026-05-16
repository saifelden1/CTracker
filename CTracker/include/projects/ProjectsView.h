#pragma once

#include <QWidget>
#include <QString>
#include <QList>
#include <QMap>

#include "core/DataStructures.h"

class ProjectsFilterBar;
class ProjectCard;
class EmptyState;
class QGridLayout;
class QScrollArea;

// ProjectsView: the main page for browsing projects.
//
// Layout:
//   ┌──────────────────────────────────────────────────┐
//   │ ProjectsFilterBar (search + filter + add-new)    │
//   ├──────────────────────────────────────────────────┤
//   │ QScrollArea                                       │
//   │   ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐           │
//   │   │ Card │ │ Card │ │ Card │ │ Card │  (grid)    │
//   │   └──────┘ └──────┘ └──────┘ └──────┘           │
//   │   ┌──────┐ ┌──────┐                              │
//   │   │ Card │ │ Card │                              │
//   │   └──────┘ └──────┘                              │
//   │                                                   │
//   │   OR EmptyState (when no projects / no results)  │
//   └──────────────────────────────────────────────────┘
//
// Responsive grid: 1–4 columns based on available width.
// Applies ProjectFilter (search/priority/status) in real time.
// Subscribes to DatabaseManager::dataChanged for live refresh.
class ProjectsView : public QWidget {
    Q_OBJECT
public:
    explicit ProjectsView(QWidget* parent = nullptr);

signals:
    // Emitted when a project card is clicked — MainWindow wires this to
    // ProjectDetailView::loadProject + page switch.
    void projectSelected(int projectId);

    // Emitted when "Add New" is clicked — MainWindow wires this to
    // EntityCreateDialog in project-only mode (Task 7.10).
    void addNewRequested();

private slots:
    void onFilterChanged(const ProjectFilter& filter);
    void onAddNewRequested();
    void onCardClicked(int projectId);
    void onDataChanged();

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void setupUi();
    void refreshCards();
    void applyFilter();
    void rebuildGrid();
    void updateColumnCount();
    void showEmptyState(bool noProjectsAtAll);

    // Returns true if the project matches the current filter.
    bool matchesFilter(const EntityData& project, const ProjectMetaData& meta) const;

    // ── Child widgets ──
    ProjectsFilterBar* m_filterBar    = nullptr;
    QScrollArea*       m_scrollArea   = nullptr;
    QWidget*           m_gridContainer = nullptr;
    QGridLayout*       m_gridLayout   = nullptr;
    EmptyState*        m_emptyState   = nullptr;

    // ── Data ──
    QList<EntityData>           m_allProjects;      // unfiltered, from DB
    QMap<int, ProjectMetaData>  m_projectMeta;      // projectId → metadata
    QList<EntityData>           m_filteredProjects; // after filter applied
    QList<ProjectCard*>         m_cards;            // currently visible cards

    ProjectFilter m_currentFilter;
    int           m_columnCount = 4;                // responsive, updated on resize
};
