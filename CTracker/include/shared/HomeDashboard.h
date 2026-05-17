#pragma once

#include <QWidget>
#include <QDate>

class StatsCard;
class CalendarWidget;
class DayDetailsPanel;
class ContributionHeatmap;
class QLabel;
class QTimer;

// Task 7.2: HomeDashboard — final unified form.
// Top row: 3 × StatsCard (Active Courses, Projects, Completion Rate).
// Lower section: CalendarWidget (left) + stacked DayDetailsPanel & ContributionHeatmap (right).
// Heatmap in RecentBuckets mode for last 12 weeks (84 days).
// Calendar dateClicked → DatabaseManager::getDay → DayDetailsPanel::showDay.
// Subscribes to dataChanged for live refresh.
class HomeDashboard : public QWidget {
    Q_OBJECT
public:
    explicit HomeDashboard(QWidget* parent = nullptr);

signals:
    void courseSelected(int courseId);
    void projectSelected(int projectId);

public slots:
    void onDataChanged();

private slots:
    void onCalendarDateClicked(const QDate& date);
    void onDayDetailsClosed();
    void onTodoAdded(const QDate& date, const QString& text);
    void onTodoToggled(const QDate& date, int index, bool completed);
    void onNotesChanged(const QDate& date, const QString& text);

private:
    void setupUi();
    void refreshStatsCards();
    void refreshCalendar();
    void refreshHeatmap();

    // Stats computation helpers
    int  countActiveCourses() const;
    int  countPausedCourses() const;
    int  countProjects() const;
    int  countProjectsDueSoon() const;  // within 7 days
    int  computeCompletionRate() const; // average progress of active courses

    QTimer* m_refreshTimer = nullptr;  // Debounce timer for dataChanged
    
    // Top row: 3 stats cards
    StatsCard* m_activeCoursesCard = nullptr;
    StatsCard* m_projectsCard      = nullptr;
    StatsCard* m_completionCard    = nullptr;

    // Lower section
    CalendarWidget*      m_calendar    = nullptr;
    DayDetailsPanel*     m_dayDetails  = nullptr;
    ContributionHeatmap* m_heatmap     = nullptr;
};
