#include "projects/ProjectsFilterBar.h"

#include "core/DataStructures.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QTimer>
#include <QFrame>

ProjectsFilterBar::ProjectsFilterBar(QWidget* parent)
    : QWidget(parent) {
    setObjectName("projectsFilterBar");
    setupUi();
}

void ProjectsFilterBar::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(4);

    // ── Top row: search | filter toggle + add new ──
    auto* topRow = new QHBoxLayout();
    topRow->setSpacing(8);

    m_searchInput = new QLineEdit(this);
    m_searchInput->setObjectName("projectsSearchInput");
    m_searchInput->setPlaceholderText(QStringLiteral("Search projects..."));
    m_searchInput->setClearButtonEnabled(true);

    m_filterToggleBtn = new QPushButton(QStringLiteral("Filter"), this);
    m_filterToggleBtn->setObjectName("projectsFilterToggle");
    m_filterToggleBtn->setCheckable(true);

    m_addNewBtn = new QPushButton(QStringLiteral("+ Add New"), this);
    m_addNewBtn->setObjectName("projectsAddNewBtn");

    topRow->addWidget(m_searchInput, 1);
    topRow->addWidget(m_filterToggleBtn);
    topRow->addWidget(m_addNewBtn);
    mainLayout->addLayout(topRow);

    // ── Collapsible filter panel ──
    m_filterPanel = new QWidget(this);
    m_filterPanel->setObjectName("projectsFilterPanel");
    m_filterPanel->setVisible(false);  // collapsed by default

    auto* filterLayout = new QHBoxLayout(m_filterPanel);
    filterLayout->setContentsMargins(8, 4, 8, 4);
    filterLayout->setSpacing(12);

    m_priorityCombo = new QComboBox(this);
    m_priorityCombo->setObjectName("projectsPriorityCombo");
    m_priorityCombo->addItem(QStringLiteral("All Priorities"), QStringLiteral("all"));
    m_priorityCombo->addItem(QStringLiteral("High"),           QStringLiteral("high"));
    m_priorityCombo->addItem(QStringLiteral("Medium"),         QStringLiteral("medium"));
    m_priorityCombo->addItem(QStringLiteral("Low"),            QStringLiteral("low"));

    m_statusCombo = new QComboBox(this);
    m_statusCombo->setObjectName("projectsStatusCombo");
    m_statusCombo->addItem(QStringLiteral("All Statuses"), QStringLiteral("all"));
    m_statusCombo->addItem(QStringLiteral("Active"),       QStringLiteral("active"));
    m_statusCombo->addItem(QStringLiteral("Paused"),       QStringLiteral("paused"));
    m_statusCombo->addItem(QStringLiteral("Completed"),    QStringLiteral("completed"));

    filterLayout->addWidget(m_priorityCombo);
    filterLayout->addWidget(m_statusCombo);
    filterLayout->addStretch();
    mainLayout->addWidget(m_filterPanel);

    // ── Filter badges row ──
    m_badgesRow = new QWidget(this);
    m_badgesRow->setObjectName("projectsBadgesRow");
    auto* badgesLayout = new QHBoxLayout(m_badgesRow);
    badgesLayout->setContentsMargins(8, 2, 8, 2);
    badgesLayout->setSpacing(4);

    m_clearAllBtn = new QPushButton(QStringLiteral("Clear all"), this);
    m_clearAllBtn->setObjectName("projectsClearAllBtn");
    badgesLayout->addWidget(m_clearAllBtn);
    badgesLayout->addStretch();
    m_badgesRow->setVisible(false);  // hidden when no active filters
    mainLayout->addWidget(m_badgesRow);

    // ── Debounce timer ──
    m_debounceTimer = new QTimer(this);
    m_debounceTimer->setSingleShot(true);
    m_debounceTimer->setInterval(200);

    // ── Signal wiring ──
    connect(m_searchInput, &QLineEdit::textChanged,
            this, &ProjectsFilterBar::onSearchTextChanged);
    connect(m_debounceTimer, &QTimer::timeout,
            this, &ProjectsFilterBar::onSearchDebounced);
    connect(m_priorityCombo, &QComboBox::currentIndexChanged,
            this, &ProjectsFilterBar::onPriorityChanged);
    connect(m_statusCombo, &QComboBox::currentIndexChanged,
            this, &ProjectsFilterBar::onStatusChanged);
    connect(m_filterToggleBtn, &QPushButton::toggled,
            this, &ProjectsFilterBar::onToggleFilterPanel);
    connect(m_addNewBtn, &QPushButton::clicked,
            this, &ProjectsFilterBar::onAddNewClicked);
    connect(m_clearAllBtn, &QPushButton::clicked,
            this, &ProjectsFilterBar::onClearAllFilters);
}

// ── Public API ──────────────────────────────────────────────

ProjectFilter ProjectsFilterBar::currentFilter() const {
    return m_currentFilter;
}

// ── Private slots ───────────────────────────────────────────

void ProjectsFilterBar::onSearchTextChanged() {
    m_debounceTimer->start();  // restart 200 ms debounce
}

void ProjectsFilterBar::onSearchDebounced() {
    m_currentFilter.search = m_searchInput->text().trimmed();
    emitFilter();
}

void ProjectsFilterBar::onPriorityChanged(int index) {
    QVariant data = m_priorityCombo->itemData(index);
    m_currentFilter.priority = data.toString();
    updateFilterBadges();
    emitFilter();
}

void ProjectsFilterBar::onStatusChanged(int index) {
    QVariant data = m_statusCombo->itemData(index);
    m_currentFilter.status = data.toString();
    updateFilterBadges();
    emitFilter();
}

void ProjectsFilterBar::onToggleFilterPanel() {
    m_filterPanel->setVisible(m_filterToggleBtn->isChecked());
}

void ProjectsFilterBar::onAddNewClicked() {
    emit addNewRequested();
}

void ProjectsFilterBar::onClearAllFilters() {
    m_searchInput->clear();
    m_priorityCombo->setCurrentIndex(0);  // "All Priorities"
    m_statusCombo->setCurrentIndex(0);    // "All Statuses"
    m_currentFilter.search.clear();
    m_currentFilter.priority = "all";
    m_currentFilter.status = "all";
    updateFilterBadges();
    emitFilter();
}

// ── Private helpers ─────────────────────────────────────────

void ProjectsFilterBar::emitFilter() {
    emit filterChanged(m_currentFilter);
}

void ProjectsFilterBar::updateFilterBadges() {
    bool hasActiveFilter = !m_currentFilter.search.isEmpty()
        || m_currentFilter.priority != "all"
        || m_currentFilter.status != "all";
    m_badgesRow->setVisible(hasActiveFilter);
}
