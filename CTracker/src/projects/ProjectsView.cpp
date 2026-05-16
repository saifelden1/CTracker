#include "projects/ProjectsView.h"

#include "projects/ProjectsFilterBar.h"
#include "projects/ProjectCard.h"
#include "shared/EmptyState.h"
#include "core/DatabaseManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QResizeEvent>
#include <QLabel>

// ============================================================
//  ProjectsView — Task 7.4
//
//  The main page for browsing projects. Embeds a
//  ProjectsFilterBar at the top and a responsive QGridLayout of
//  ProjectCard widgets below. An EmptyState widget is shown when
//  there are no projects at all or when the active filter yields
//  zero results.
//
//  Responsive breakpoints (based on available scroll-area width):
//    <  500 px → 1 column
//    <  700 px → 2 columns
//    < 1000 px → 3 columns
//    ≥ 1000 px → 4 columns
//
//  The view subscribes to DatabaseManager::dataChanged so any
//  mutation (add, rename, delete, status change, priority change)
//  triggers a full refresh of the card grid.
//
//  Note: Each project card requires fetching ProjectMetaData via
//  getProjectMeta(). The task notes suggest batching to one JOIN
//  later for performance optimization.
// ============================================================

ProjectsView::ProjectsView(QWidget* parent)
    : QWidget(parent) {
    setObjectName("projectsView");
    setupUi();

    // Initial data load
    refreshCards();

    // Subscribe to live DB updates
    connect(DatabaseManager::instance(), &DatabaseManager::dataChanged,
            this, &ProjectsView::onDataChanged);
}

void ProjectsView::setupUi() {
    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(24, 24, 24, 24);
    outerLayout->setSpacing(16);

    // ── Header label ──
    auto* headerLabel = new QLabel(QStringLiteral("Projects"), this);
    headerLabel->setObjectName("projectsViewHeader");
    QFont headerFont = headerLabel->font();
    headerFont.setWeight(QFont::Medium);
    headerFont.setPointSize(18);
    headerLabel->setFont(headerFont);
    outerLayout->addWidget(headerLabel);

    // ── Filter bar ──
    m_filterBar = new ProjectsFilterBar(this);
    outerLayout->addWidget(m_filterBar);

    connect(m_filterBar, &ProjectsFilterBar::filterChanged,
            this, &ProjectsView::onFilterChanged);
    connect(m_filterBar, &ProjectsFilterBar::addNewRequested,
            this, &ProjectsView::onAddNewRequested);

    // ── Scroll area containing the grid ──
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setObjectName("projectsScrollArea");
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_gridContainer = new QWidget(m_scrollArea);
    m_gridLayout = new QGridLayout(m_gridContainer);
    m_gridLayout->setContentsMargins(0, 0, 0, 0);
    m_gridLayout->setSpacing(24);
    m_gridLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    m_scrollArea->setWidget(m_gridContainer);
    outerLayout->addWidget(m_scrollArea, 1);

    // ── Empty state (stacked on top of scroll area, hidden by default) ──
    m_emptyState = new EmptyState(this);
    m_emptyState->setObjectName("projectsEmptyState");
    m_emptyState->setTitle(QStringLiteral("No projects yet"));
    m_emptyState->setDescription(
        QStringLiteral("Get started by adding your first project to begin tracking your work."));
    m_emptyState->setActionLabel(QStringLiteral("Add Your First Project"));
    m_emptyState->setVisible(false);

    // We overlay the empty state inside the scroll area's viewport region.
    // Using a separate layout approach: add empty state to outerLayout
    // at the same stretch factor as scroll area, and toggle visibility.
    outerLayout->addWidget(m_emptyState, 1);
    connect(m_emptyState, &EmptyState::actionRequested,
            this, &ProjectsView::onAddNewRequested);
}

// ── Public slot wiring ────────────────────────────────────────

void ProjectsView::onFilterChanged(const ProjectFilter& filter) {
    m_currentFilter = filter;
    applyFilter();
    rebuildGrid();
}

void ProjectsView::onAddNewRequested() {
    emit addNewRequested();
}

void ProjectsView::onCardClicked(int projectId) {
    emit projectSelected(projectId);
}

void ProjectsView::onDataChanged() {
    refreshCards();
}

// ── Resize handling ───────────────────────────────────────────

void ProjectsView::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    updateColumnCount();
}

// ── Data management ───────────────────────────────────────────

void ProjectsView::refreshCards() {
    auto* db = DatabaseManager::instance();

    // Fetch all projects (includes category/status via LEFT JOIN)
    m_allProjects = db->fetchAllProjects();

    // Fetch metadata for each project
    // NOTE: Task 7.4 suggests batching to one JOIN later for performance
    m_projectMeta.clear();
    for (const EntityData& project : m_allProjects) {
        m_projectMeta.insert(project.id, db->getProjectMeta(project.id));
    }

    // Apply current filter and rebuild
    applyFilter();
    rebuildGrid();
}

void ProjectsView::applyFilter() {
    m_filteredProjects.clear();

    for (const EntityData& project : m_allProjects) {
        const ProjectMetaData& meta = m_projectMeta.value(project.id);
        if (matchesFilter(project, meta)) {
            m_filteredProjects.append(project);
        }
    }

    // Decide which empty state to show
    bool noProjectsAtAll = m_allProjects.isEmpty();
    bool noFilteredResults = m_filteredProjects.isEmpty() && !noProjectsAtAll;
    showEmptyState(noProjectsAtAll);

    if (noFilteredResults && !noProjectsAtAll) {
        // Search returned no results — update empty state text
        m_emptyState->setTitle(QStringLiteral("No results found"));
        if (!m_currentFilter.search.isEmpty()) {
            m_emptyState->setDescription(
                QStringLiteral("No projects match \"%1\". Try a different search term.")
                    .arg(m_currentFilter.search));
        } else {
            m_emptyState->setDescription(
                QStringLiteral("No projects match the current filters. Try adjusting them."));
        }
        m_emptyState->setActionLabel(QString());  // hide action button for "no results"
        m_emptyState->setVisible(true);
        m_scrollArea->setVisible(false);
    }
}

bool ProjectsView::matchesFilter(const EntityData& project, const ProjectMetaData& meta) const {
    // Search: case-insensitive substring match on name
    if (!m_currentFilter.search.isEmpty()) {
        if (!project.name.contains(m_currentFilter.search, Qt::CaseInsensitive)) {
            return false;
        }
    }

    // Priority filter: "all" means any priority
    if (m_currentFilter.priority != "all") {
        if (meta.priority != m_currentFilter.priority) {
            return false;
        }
    }

    // Status filter: "all" means any status
    if (m_currentFilter.status != "all") {
        if (project.status != m_currentFilter.status) {
            return false;
        }
    }

    return true;
}

// ── Grid layout ───────────────────────────────────────────────

void ProjectsView::rebuildGrid() {
    // Remove old cards from the grid and delete them
    for (ProjectCard* card : m_cards) {
        m_gridLayout->removeWidget(card);
        card->deleteLater();
    }
    m_cards.clear();

    // If no filtered results, the empty state is already shown — skip grid
    if (m_filteredProjects.isEmpty()) {
        return;
    }

    // Ensure scroll area is visible (empty state hides it)
    m_scrollArea->setVisible(true);
    m_emptyState->setVisible(false);

    // Create new ProjectCard widgets and place them in the grid
    int col = 0;
    int row = 0;

    for (const EntityData& project : m_filteredProjects) {
        const ProjectMetaData& meta = m_projectMeta.value(project.id);

        auto* card = new ProjectCard(project.id, m_gridContainer);
        
        // Set basic project info
        card->setName(project.name);
        card->setDescription(meta.description);
        card->setPriority(meta.priority);
        card->setDeadline(meta.deadline);
        card->setProgress(project.overallProgress);
        card->setTeamSize(meta.team.size());

        // Calculate task count (sessions/tasks for this project)
        // For now, we'll use a placeholder - this would need to be computed
        // from the Units/SessionsTasks hierarchy
        // TODO: Compute actual task counts from database
        card->setTaskCount(0, 0);

        // Wire click signal
        connect(card, &ProjectCard::clicked,
                this, &ProjectsView::onCardClicked);

        m_gridLayout->addWidget(card, row, col);
        m_cards.append(card);

        ++col;
        if (col >= m_columnCount) {
            col = 0;
            ++row;
        }
    }
}

void ProjectsView::updateColumnCount() {
    // Compute available width for the grid (scroll area viewport width)
    int availableWidth = m_scrollArea->viewport() ? m_scrollArea->viewport()->width() : width();

    // ProjectCard is similar to EntityCard: ~160 px wide + 24 px spacing between columns
    // Breakpoints chosen so cards don't feel cramped:
    //   1 col: <  500 px
    //   2 col: <  700 px
    //   3 col: < 1000 px
    //   4 col: ≥ 1000 px
    int newColumnCount = 4;
    if (availableWidth < 500) {
        newColumnCount = 1;
    } else if (availableWidth < 700) {
        newColumnCount = 2;
    } else if (availableWidth < 1000) {
        newColumnCount = 3;
    }

    if (newColumnCount != m_columnCount) {
        m_columnCount = newColumnCount;
        rebuildGrid();
    }
}

void ProjectsView::showEmptyState(bool noProjectsAtAll) {
    if (noProjectsAtAll) {
        // No projects in the DB at all — show the "add first" empty state
        m_emptyState->setTitle(QStringLiteral("No projects yet"));
        m_emptyState->setDescription(
            QStringLiteral("Get started by adding your first project to begin tracking your work."));
        m_emptyState->setActionLabel(QStringLiteral("Add Your First Project"));
        m_emptyState->setVisible(true);
        m_scrollArea->setVisible(false);
    } else if (m_filteredProjects.isEmpty()) {
        // Has projects but filter yields nothing — handled in applyFilter()
        // (the description is set there based on the active filter)
    } else {
        // Normal state: has filtered results, show the grid
        m_emptyState->setVisible(false);
        m_scrollArea->setVisible(true);
    }
}
