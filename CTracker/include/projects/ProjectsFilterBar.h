#pragma once

#include <QWidget>
#include <QString>
#include "core/DataStructures.h"

class QLineEdit;
class QPushButton;
class QComboBox;
class QTimer;

// ProjectsFilterBar: search input + filter toggle + "Add New" button.
// Collapsible filter panel: Priority dropdown + Status dropdown.
// Active-filter badges with × to remove + "Clear all" link.
// Emits filterChanged(ProjectFilter) (debounced 200 ms on search) and addNewRequested().
class ProjectsFilterBar : public QWidget {
    Q_OBJECT
public:
    explicit ProjectsFilterBar(QWidget* parent = nullptr);

    ProjectFilter currentFilter() const;

signals:
    void filterChanged(const ProjectFilter& filter);
    void addNewRequested();

private slots:
    void onSearchTextChanged();
    void onSearchDebounced();
    void onPriorityChanged(int index);
    void onStatusChanged(int index);
    void onToggleFilterPanel();
    void onAddNewClicked();
    void onClearAllFilters();

private:
    void setupUi();
    void emitFilter();
    void updateFilterBadges();

    QLineEdit*   m_searchInput      = nullptr;
    QPushButton* m_filterToggleBtn  = nullptr;
    QPushButton* m_addNewBtn        = nullptr;

    // Collapsible filter panel
    QWidget*     m_filterPanel      = nullptr;
    QComboBox*   m_priorityCombo    = nullptr;
    QComboBox*   m_statusCombo      = nullptr;

    // Filter badges row
    QWidget*     m_badgesRow        = nullptr;
    QPushButton* m_clearAllBtn      = nullptr;

    // Debounce timer for search
    QTimer*      m_debounceTimer    = nullptr;

    // Current filter state
    ProjectFilter m_currentFilter;
};
