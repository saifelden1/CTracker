#include "courses/CoursesFilterBar.h"

#include "core/DataStructures.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QTimer>
#include <QFrame>

CoursesFilterBar::CoursesFilterBar(QWidget* parent)
    : QWidget(parent) {
    setObjectName("coursesFilterBar");
    setupUi();
}

void CoursesFilterBar::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(4);

    // ── Top row: search | filter toggle + add new ──
    auto* topRow = new QHBoxLayout();
    topRow->setSpacing(8);

    m_searchInput = new QLineEdit(this);
    m_searchInput->setObjectName("coursesSearchInput");
    m_searchInput->setPlaceholderText(QStringLiteral("Search courses..."));
    m_searchInput->setClearButtonEnabled(true);

    m_filterToggleBtn = new QPushButton(QStringLiteral("Filter"), this);
    m_filterToggleBtn->setObjectName("coursesFilterToggle");
    m_filterToggleBtn->setCheckable(true);

    m_addNewBtn = new QPushButton(QStringLiteral("+ Add New"), this);
    m_addNewBtn->setObjectName("coursesAddNewBtn");

    topRow->addWidget(m_searchInput, 1);
    topRow->addWidget(m_filterToggleBtn);
    topRow->addWidget(m_addNewBtn);
    mainLayout->addLayout(topRow);

    // ── Collapsible filter panel ──
    m_filterPanel = new QWidget(this);
    m_filterPanel->setObjectName("coursesFilterPanel");
    m_filterPanel->setVisible(false);  // collapsed by default

    auto* filterLayout = new QHBoxLayout(m_filterPanel);
    filterLayout->setContentsMargins(8, 4, 8, 4);
    filterLayout->setSpacing(12);

    m_categoryCombo = new QComboBox(this);
    m_categoryCombo->setObjectName("coursesCategoryCombo");
    m_categoryCombo->addItem(QStringLiteral("All Categories"), -1);

    m_statusCombo = new QComboBox(this);
    m_statusCombo->setObjectName("coursesStatusCombo");
    m_statusCombo->addItem(QStringLiteral("All Statuses"), QStringLiteral("all"));
    m_statusCombo->addItem(QStringLiteral("Active"),       QStringLiteral("active"));
    m_statusCombo->addItem(QStringLiteral("Paused"),       QStringLiteral("paused"));

    filterLayout->addWidget(m_categoryCombo);
    filterLayout->addWidget(m_statusCombo);
    filterLayout->addStretch();
    mainLayout->addWidget(m_filterPanel);

    // ── Filter badges row ──
    m_badgesRow = new QWidget(this);
    m_badgesRow->setObjectName("coursesBadgesRow");
    auto* badgesLayout = new QHBoxLayout(m_badgesRow);
    badgesLayout->setContentsMargins(8, 2, 8, 2);
    badgesLayout->setSpacing(4);

    m_clearAllBtn = new QPushButton(QStringLiteral("Clear all"), this);
    m_clearAllBtn->setObjectName("coursesClearAllBtn");
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
            this, &CoursesFilterBar::onSearchTextChanged);
    connect(m_debounceTimer, &QTimer::timeout,
            this, &CoursesFilterBar::onSearchDebounced);
    connect(m_categoryCombo, &QComboBox::currentIndexChanged,
            this, &CoursesFilterBar::onCategoryChanged);
    connect(m_statusCombo, &QComboBox::currentIndexChanged,
            this, &CoursesFilterBar::onStatusChanged);
    connect(m_filterToggleBtn, &QPushButton::toggled,
            this, &CoursesFilterBar::onToggleFilterPanel);
    connect(m_addNewBtn, &QPushButton::clicked,
            this, &CoursesFilterBar::onAddNewClicked);
    connect(m_clearAllBtn, &QPushButton::clicked,
            this, &CoursesFilterBar::onClearAllFilters);
}

// ── Public API ──────────────────────────────────────────────

CourseFilter CoursesFilterBar::currentFilter() const {
    return m_currentFilter;
}

void CoursesFilterBar::setCategories(const QList<CategoryData>& cats) {
    m_categoryCombo->clear();
    m_categoryCombo->addItem(QStringLiteral("All Categories"), -1);
    for (const auto& cat : cats) {
        m_categoryCombo->addItem(cat.name, cat.id);
    }
}

// ── Private slots ───────────────────────────────────────────

void CoursesFilterBar::onSearchTextChanged() {
    m_debounceTimer->start();  // restart 200 ms debounce
}

void CoursesFilterBar::onSearchDebounced() {
    m_currentFilter.search = m_searchInput->text().trimmed();
    emitFilter();
}

void CoursesFilterBar::onCategoryChanged(int index) {
    QVariant data = m_categoryCombo->itemData(index);
    m_currentFilter.categoryId = data.toInt();
    updateFilterBadges();
    emitFilter();
}

void CoursesFilterBar::onStatusChanged(int index) {
    QVariant data = m_statusCombo->itemData(index);
    m_currentFilter.status = data.toString();
    updateFilterBadges();
    emitFilter();
}

void CoursesFilterBar::onToggleFilterPanel() {
    m_filterPanel->setVisible(m_filterToggleBtn->isChecked());
}

void CoursesFilterBar::onAddNewClicked() {
    emit addNewRequested();
}

void CoursesFilterBar::onClearAllFilters() {
    m_searchInput->clear();
    m_categoryCombo->setCurrentIndex(0);  // "All Categories"
    m_statusCombo->setCurrentIndex(0);    // "All Statuses"
    m_currentFilter.search.clear();
    m_currentFilter.categoryId = -1;
    m_currentFilter.status = "all";
    updateFilterBadges();
    emitFilter();
}

// ── Private helpers ─────────────────────────────────────────

void CoursesFilterBar::emitFilter() {
    emit filterChanged(m_currentFilter);
}

void CoursesFilterBar::updateFilterBadges() {
    bool hasActiveFilter = !m_currentFilter.search.isEmpty()
        || m_currentFilter.categoryId >= 0
        || m_currentFilter.status != "all";
    m_badgesRow->setVisible(hasActiveFilter);
}