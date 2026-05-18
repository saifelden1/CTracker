#include "courses/CoursesView.h"

#include "courses/CoursesFilterBar.h"
#include "courses/EntityCard.h"
#include "shared/EmptyState.h"
#include "core/DatabaseManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QResizeEvent>
#include <QLabel>

// ============================================================
//  CoursesView — Task 7.3
//
//  The main page for browsing courses and projects. Embeds a
//  CoursesFilterBar at the top and a responsive QGridLayout of
//  EntityCard widgets below. An EmptyState widget is shown when
//  there are no entities at all or when the active filter yields
//  zero results.
//
//  Responsive breakpoints (based on available scroll-area width):
//    <  500 px → 1 column
//    <  700 px → 2 columns
//    < 1000 px → 3 columns
//    ≥ 1000 px → 4 columns
//
//  The view subscribes to DatabaseManager::dataChanged so any
//  mutation (add, rename, delete, status change, category change)
//  triggers a full refresh of the card grid.
// ============================================================

CoursesView::CoursesView(QWidget* parent)
    : QWidget(parent) {
    setObjectName("coursesView");
    setupUi();

    // Initial data load
    refreshCards();

    // Subscribe to live DB updates
    connect(DatabaseManager::instance(), &DatabaseManager::dataChanged,
            this, &CoursesView::onDataChanged);
}

void CoursesView::setupUi() {
    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(24, 24, 24, 24);
    outerLayout->setSpacing(16);

    // ── Header label ──
    auto* headerLabel = new QLabel(QStringLiteral("Courses"), this);
    headerLabel->setObjectName("coursesViewHeader");
    QFont headerFont = headerLabel->font();
    headerFont.setWeight(QFont::Medium);
    headerFont.setPointSize(18);
    headerLabel->setFont(headerFont);
    outerLayout->addWidget(headerLabel);

    // ── Subtitle: entity count ──
    m_subtitleLabel = new QLabel(this);
    m_subtitleLabel->setObjectName("coursesViewSubtitle");
    m_subtitleLabel->setStyleSheet(
        QStringLiteral("QLabel#coursesViewSubtitle { color: #9ca3af; }"));
    outerLayout->addWidget(m_subtitleLabel);

    // ── Filter bar ──
    m_filterBar = new CoursesFilterBar(this);
    outerLayout->addWidget(m_filterBar);

    connect(m_filterBar, &CoursesFilterBar::filterChanged,
            this, &CoursesView::onFilterChanged);
    connect(m_filterBar, &CoursesFilterBar::addNewRequested,
            this, &CoursesView::onAddNewRequested);

    // ── Scroll area containing the grid ──
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setObjectName("coursesScrollArea");
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
    m_emptyState->setObjectName("coursesEmptyState");
    m_emptyState->setTitle(QStringLiteral("No courses or projects yet"));
    m_emptyState->setDescription(
        QStringLiteral("Get started by adding your first course or project to begin tracking your progress."));
    m_emptyState->setActionLabel(QStringLiteral("Add Your First Item"));
    m_emptyState->setVisible(false);

    // We overlay the empty state inside the scroll area's viewport region.
    // Using a separate layout approach: add empty state to outerLayout
    // at the same stretch factor as scroll area, and toggle visibility.
    outerLayout->addWidget(m_emptyState, 1);
    connect(m_emptyState, &EmptyState::actionRequested,
            this, &CoursesView::onAddNewRequested);
}

// ── Public slot wiring ────────────────────────────────────────

void CoursesView::onFilterChanged(const CourseFilter& filter) {
    m_currentFilter = filter;
    applyFilter();
    rebuildGrid();
}

void CoursesView::onAddNewRequested() {
    emit addNewRequested();
}

void CoursesView::onCardClicked(int entityId, EntityCard::EntityType type) {
    if (type == EntityCard::EntityType::Course) {
        emit courseSelected(entityId);
    } else {
        emit projectSelected(entityId);
    }
}

void CoursesView::onDataChanged() {
    refreshCards();
}

// ── Resize handling ───────────────────────────────────────────

void CoursesView::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    updateColumnCount();
}

// ── Data management ───────────────────────────────────────────

void CoursesView::refreshCards() {
    auto* db = DatabaseManager::instance();

    // Fetch only courses (projects are shown in ProjectsView)
    m_allEntities = db->fetchAllCourses();

    // Fetch categories for the filter bar and card pills
    m_categories = db->fetchAllCategories();
    m_filterBar->setCategories(m_categories);

    // Update subtitle with entity count
    if (m_allEntities.isEmpty()) {
        m_subtitleLabel->setText(
            QStringLiteral("Start tracking your courses"));
    } else {
        m_subtitleLabel->setText(
            QStringLiteral("%1 course%2")
                .arg(m_allEntities.size())
                .arg(m_allEntities.size() == 1 ? "" : "s"));
    }

    // Apply current filter and rebuild
    applyFilter();
    rebuildGrid();
}

void CoursesView::applyFilter() {
    m_filteredEntities.clear();

    for (const EntityData& entity : m_allEntities) {
        if (matchesFilter(entity)) {
            m_filteredEntities.append(entity);
        }
    }

    // Decide which empty state to show
    bool noEntitiesAtAll = m_allEntities.isEmpty();
    bool noFilteredResults = m_filteredEntities.isEmpty() && !noEntitiesAtAll;
    showEmptyState(noEntitiesAtAll);

    if (noFilteredResults && !noEntitiesAtAll) {
        // Search returned no results — update empty state text
        m_emptyState->setTitle(QStringLiteral("No results found"));
        if (!m_currentFilter.search.isEmpty()) {
            m_emptyState->setDescription(
                QStringLiteral("No courses match \"%1\". Try a different search term.")
                    .arg(m_currentFilter.search));
        } else {
            m_emptyState->setDescription(
                QStringLiteral("No courses match the current filters. Try adjusting them."));
        }
        m_emptyState->setActionLabel(QString());  // hide action button for "no results"
        m_emptyState->setVisible(true);
        m_scrollArea->setVisible(false);
        m_subtitleLabel->setVisible(false);
    }
}

bool CoursesView::matchesFilter(const EntityData& entity) const {
    // Search: case-insensitive substring match on name
    if (!m_currentFilter.search.isEmpty()) {
        if (!entity.name.contains(m_currentFilter.search, Qt::CaseInsensitive)) {
            return false;
        }
    }

    // Category filter: categoryId == -1 means "all"
    if (m_currentFilter.categoryId >= 0) {
        if (entity.categoryId != m_currentFilter.categoryId) {
            return false;
        }
    }

    // Status filter: "all" means any status
    if (m_currentFilter.status != "all") {
        if (entity.status != m_currentFilter.status) {
            return false;
        }
    }

    return true;
}

// ── Grid layout ───────────────────────────────────────────────

void CoursesView::rebuildGrid() {
    // Remove old cards from the grid and delete them
    for (EntityCard* card : m_cards) {
        m_gridLayout->removeWidget(card);
        card->deleteLater();
    }
    m_cards.clear();

    // If no filtered results, the empty state is already shown — skip grid
    if (m_filteredEntities.isEmpty()) {
        return;
    }

    // Ensure scroll area is visible (empty state hides it)
    m_scrollArea->setVisible(true);
    m_emptyState->setVisible(false);

    // Build a lookup for categories (to set CategoryPill on each card)
    // We use a map from categoryId → CategoryData for fast lookup.
    QMap<int, CategoryData> catMap;
    for (const CategoryData& cat : m_categories) {
        catMap.insert(cat.id, cat);
    }

    // Create new EntityCard widgets and place them in the grid
    int col = 0;
    int row = 0;

    for (const EntityData& entity : m_filteredEntities) {
        // Map the string type from EntityData to EntityCard::EntityType
        EntityCard::EntityType cardType =
            (entity.type == "Project")
                ? EntityCard::EntityType::Project
                : EntityCard::EntityType::Course;

        auto* card = new EntityCard(
            entity.id,
            entity.name,
            cardType,
            entity.overallProgress,
            m_gridContainer);

        // Set category pill if entity has a category
        if (entity.categoryId >= 0 && catMap.contains(entity.categoryId)) {
            card->setCategory(catMap.value(entity.categoryId));
        }

        // Set status badge (shows "Paused" overlay when paused)
        card->setStatus(entity.status);

        // Wire click signal
        connect(card, &EntityCard::clicked,
                this, &CoursesView::onCardClicked);

        m_gridLayout->addWidget(card, row, col);
        m_cards.append(card);

        ++col;
        if (col >= m_columnCount) {
            col = 0;
            ++row;
        }
    }
}

void CoursesView::updateColumnCount() {
    // Compute available width for the grid (scroll area viewport width)
    int availableWidth = m_scrollArea->viewport() ? m_scrollArea->viewport()->width() : width();

    // EntityCard is 160 px wide + 24 px spacing between columns
    // We dynamically calculate how many cards can fit in a row.
    int cardTotalWidth = 160 + 24;
    int newColumnCount = qMax(1, (availableWidth + 24) / cardTotalWidth);

    if (newColumnCount != m_columnCount) {
        m_columnCount = newColumnCount;
        rebuildGrid();
    }
}

void CoursesView::showEmptyState(bool noEntitiesAtAll) {
    if (noEntitiesAtAll) {
        // No courses in the DB at all — show the "add first" empty state
        m_emptyState->setTitle(QStringLiteral("No courses yet"));
        m_emptyState->setDescription(
            QStringLiteral("Get started by adding your first course to begin tracking your progress."));
        m_emptyState->setActionLabel(QStringLiteral("Add Your First Course"));
        m_emptyState->setVisible(true);
        m_scrollArea->setVisible(false);
        m_subtitleLabel->setVisible(false);
    } else if (m_filteredEntities.isEmpty()) {
        // Has entities but filter yields nothing — handled in applyFilter()
        // (the description is set there based on the active filter)
    } else {
        // Normal state: has filtered results, show the grid
        m_emptyState->setVisible(false);
        m_scrollArea->setVisible(true);
        m_subtitleLabel->setVisible(true);
    }
}
