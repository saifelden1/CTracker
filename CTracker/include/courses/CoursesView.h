#pragma once

#include <QWidget>
#include <QString>
#include <QList>

#include "core/DataStructures.h"
#include "courses/EntityCard.h"

class CoursesFilterBar;
class EmptyState;
class QGridLayout;
class QScrollArea;

// CoursesView: the main page for browsing courses/projects.
//
// Layout:
//   ┌──────────────────────────────────────────────────┐
//   │ CoursesFilterBar (search + filter + add-new)     │
//   ├──────────────────────────────────────────────────┤
//   │ QScrollArea                                       │
//   │   ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐           │
//   │   │ Card │ │ Card │ │ Card │ │ Card │  (grid)    │
//   │   └──────┘ └──────┘ └──────┘ └──────┘           │
//   │   ┌──────┐ ┌──────┐                              │
//   │   │ Card │ │ Card │                              │
//   │   └──────┘ └──────┘                              │
//   │                                                   │
//   │   OR EmptyState (when no courses / no results)    │
//   └──────────────────────────────────────────────────┘
//
// Responsive grid: 1–4 columns based on available width.
// Applies CourseFilter (search/category/status) in real time.
// Subscribes to DatabaseManager::dataChanged for live refresh.
class CoursesView : public QWidget {
    Q_OBJECT
public:
    explicit CoursesView(QWidget* parent = nullptr);

signals:
    // Emitted when a course card is clicked — MainWindow wires this to
    // CourseDetailView::loadCourse + page switch.
    void courseSelected(int courseId);

    // Emitted when a project card is clicked — MainWindow wires this to
    // ProjectDetailView::loadProject + page switch.
    void projectSelected(int projectId);

    // Emitted when "Add New" is clicked — MainWindow wires this to
    // EntityCreateDialog (Task 7.10).
    void addNewRequested();

private slots:
    void onFilterChanged(const CourseFilter& filter);
    void onAddNewRequested();
    void onCardClicked(int entityId, EntityCard::EntityType type);
    void onDataChanged();

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void setupUi();
    void refreshCards();
    void applyFilter();
    void rebuildGrid();
    void updateColumnCount();
    void showEmptyState(bool noEntitiesAtAll);

    // Returns true if the entity matches the current filter.
    bool matchesFilter(const EntityData& entity) const;

    // ── Child widgets ──
    CoursesFilterBar* m_filterBar    = nullptr;
    QScrollArea*      m_scrollArea   = nullptr;
    QWidget*          m_gridContainer = nullptr;
    QGridLayout*      m_gridLayout   = nullptr;
    EmptyState*       m_emptyState   = nullptr;

    // ── Data ──
    QList<EntityData>        m_allEntities;     // unfiltered, from DB
    QList<EntityData>        m_filteredEntities; // after filter applied
    QList<EntityCard*>       m_cards;            // currently visible cards
    QList<CategoryData>      m_categories;       // for filter bar + card pills

    CourseFilter m_currentFilter;
    int          m_columnCount = 4;              // responsive, updated on resize
};