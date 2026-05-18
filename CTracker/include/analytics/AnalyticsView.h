#pragma once

#include <QWidget>
#include <QDate>
#include <QList>

class StatsCard;
class ContributionHeatmap;
class QLabel;
class QPushButton;
class QChartView;

// Task 7.9: AnalyticsView — final v2 form.
// Top row: 4 × StatsCard (Day Streak, Total Hours, Avg Sessions/Day, Week Comparison).
// 3 chart widgets via QChartView:
//   - Study hours/week (QBarSeries, 8 weeks)
//   - Time distribution (QPieSeries over Pomodoro minutes per course)
//   - Weekly activity pattern (QBarSeries, Mon-Sun)
// Dark-themed QChart palette, custom tooltips, NoRubberBand, antialiased.
// ContributionHeatmap below charts in NormalizedRange mode, configurable year nav.
class AnalyticsView : public QWidget {
    Q_OBJECT
public:
    explicit AnalyticsView(QWidget* parent = nullptr);

    void loadYear(int year);
    int  currentYear() const { return m_year; }

public slots:
    void onDataChanged();

private slots:
    void onPreviousYear();
    void onNextYear();

private:
    void setupUi();
    void refreshStatsCards();
    void refreshCharts();
    void refreshHeatmap();

    // ── Stats cards ──
    StatsCard* m_streakCard      = nullptr;
    StatsCard* m_hoursCard       = nullptr;
    StatsCard* m_avgSessionsCard = nullptr;
    StatsCard* m_weekCompCard    = nullptr;

    // ── Charts ──
    QChartView* m_hoursPerWeekView     = nullptr;
    QChartView* m_timeDistView         = nullptr;
    QChartView* m_weeklyActivityView   = nullptr;

    // ── Heatmap ──
    ContributionHeatmap* m_heatmap     = nullptr;
    QPushButton*         m_prevBtn     = nullptr;
    QPushButton*         m_nextBtn     = nullptr;
    QLabel*              m_yearLabel   = nullptr;

    int m_year = QDate::currentDate().year();
};
