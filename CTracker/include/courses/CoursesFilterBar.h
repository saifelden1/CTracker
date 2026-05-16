#pragma once

#include <QWidget>
#include <QString>
#include <QList>
#include "core/DataStructures.h"

class QLineEdit;
class QPushButton;
class QComboBox;

struct CategoryData;

// CoursesFilterBar: search input + filter toggle + "Add New" button.
// Collapsible filter panel: Category dropdown + Status dropdown.
// Active-filter badges with × to remove + "Clear all" link.
// Emits filterChanged(CourseFilter) (debounced 200 ms on search) and addNewRequested().
class CoursesFilterBar : public QWidget {
    Q_OBJECT
public:
    explicit CoursesFilterBar(QWidget* parent = nullptr);

    CourseFilter currentFilter() const;
    void setCategories(const QList<CategoryData>& cats);

signals:
    void filterChanged(const CourseFilter& filter);
    void addNewRequested();

private slots:
    void onSearchTextChanged();
    void onSearchDebounced();
    void onCategoryChanged(int index);
    void onStatusChanged(int index);
    void onToggleFilterPanel();
    void onAddNewClicked();
    void onClearAllFilters();

private:
    void setupUi();
    void emitFilter();
    void updateFilterBadges();

    QLineEdit*   m_searchInput     = nullptr;
    QPushButton* m_filterToggleBtn  = nullptr;
    QPushButton* m_addNewBtn        = nullptr;

    // Collapsible filter panel
    QWidget*     m_filterPanel      = nullptr;
    QComboBox*   m_categoryCombo    = nullptr;
    QComboBox*   m_statusCombo      = nullptr;

    // Filter badges row
    QWidget*     m_badgesRow        = nullptr;
    QPushButton* m_clearAllBtn      = nullptr;

    // Debounce timer for search
    QTimer*      m_debounceTimer    = nullptr;

    // Current filter state
    CourseFilter m_currentFilter;
};