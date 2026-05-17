#include "analytics/AnalyticsView.h"

#include "shared/StatsCard.h"
#include "analytics/ContributionHeatmap.h"
#include "analytics/HeatmapAggregator.h"
#include "analytics/AnalyticsSummary.h"
#include "core/DatabaseManager.h"
#include "core/DataStructures.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QScrollArea>
#include <QDate>
#include <QList>
#include <QPainter>
#include <algorithm>

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChart>

// Task 7.9: AnalyticsView — final v2 form.
// Top row: 4 × StatsCard.
// 5 chart widgets + course progress list + heatmap.

AnalyticsView::AnalyticsView(QWidget* parent)
    : QWidget(parent) {
    setObjectName("analyticsView");
    setupUi();

    connect(DatabaseManager::instance(), &DatabaseManager::dataChanged,
            this, &AnalyticsView::onDataChanged);

    refreshStatsCards();
    refreshCharts();
    refreshHeatmap();
}

void AnalyticsView::setupUi() {
    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto* container = new QWidget(scrollArea);
    auto* outer = new QVBoxLayout(container);
    outer->setContentsMargins(24, 24, 24, 24);
    outer->setSpacing(24);

    // ── Header ──────────────────────────────────────────────
    auto* headerLabel = new QLabel(tr("Analytics"), container);
    headerLabel->setObjectName("analyticsTitle");
    QFont titleFont = headerLabel->font();
    titleFont.setWeight(QFont::Medium);
    titleFont.setPointSize(18);
    headerLabel->setFont(titleFont);
    outer->addWidget(headerLabel);

    auto* subtitleLabel = new QLabel(tr("Track your progress and productivity insights"), container);
    subtitleLabel->setObjectName("analyticsSubtitle");
    outer->addWidget(subtitleLabel);

    // ── Stats cards row ──────────────────────────────────────
    auto* statsRow = new QHBoxLayout();
    statsRow->setSpacing(16);

    m_streakCard      = new StatsCard(tr("Day Streak"), container);
    m_hoursCard       = new StatsCard(tr("Total Hours"), container);
    m_avgSessionsCard = new StatsCard(tr("Avg Sessions/Day"), container);
    m_weekCompCard    = new StatsCard(tr("vs Last Week"), container);

    statsRow->addWidget(m_streakCard);
    statsRow->addWidget(m_hoursCard);
    statsRow->addWidget(m_avgSessionsCard);
    statsRow->addWidget(m_weekCompCard);

    outer->addLayout(statsRow);

    // ── Charts row 1: Progress over time + Study hours/week ──
    auto* chartsRow1 = new QHBoxLayout();
    chartsRow1->setSpacing(16);

    // Progress over time (line chart)
    {
        auto* frame = new QFrame(container);
        frame->setObjectName("analyticsChartFrame");
        auto* layout = new QVBoxLayout(frame);
        layout->setContentsMargins(16, 16, 16, 16);
        layout->setSpacing(8);

        auto* chartTitle = new QLabel(tr("Progress Over Time"), frame);
        chartTitle->setObjectName("analyticsChartTitle");
        layout->addWidget(chartTitle);

        auto* chart = new QChart();
        chart->setTheme(QChart::ChartThemeDark);
        chart->setMargins(QMargins(0, 0, 0, 0));
        chart->legend()->hide();

        auto* series = new QLineSeries(chart);
        series->setColor(QColor("#10b981"));
        series->setPointsVisible(true);

        auto* axisX = new QCategoryAxis();
        axisX->setLabelsColor(QColor("#9ca3af"));
        axisX->setGridLineColor(QColor("#2d323d"));
        chart->addAxis(axisX, Qt::AlignBottom);
        chart->addSeries(series);
        series->attachAxis(axisX);

        auto* axisY = new QValueAxis();
        axisY->setRange(0, 100);
        axisY->setLabelsColor(QColor("#9ca3af"));
        axisY->setGridLineColor(QColor("#2d323d"));
        axisY->setTitleText(tr("Completion %"));
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);

        m_progressChartView = new QChartView(chart, frame);
        m_progressChartView->setRenderHint(QPainter::Antialiasing);
        m_progressChartView->setRubberBand(QChartView::NoRubberBand);
        m_progressChartView->setMinimumHeight(250);
        layout->addWidget(m_progressChartView);

        chartsRow1->addWidget(frame, 1);
    }

    // Study hours/week (bar chart)
    {
        auto* frame = new QFrame(container);
        frame->setObjectName("analyticsChartFrame");
        auto* layout = new QVBoxLayout(frame);
        layout->setContentsMargins(16, 16, 16, 16);
        layout->setSpacing(8);

        auto* chartTitle = new QLabel(tr("Study Hours Per Week"), frame);
        chartTitle->setObjectName("analyticsChartTitle");
        layout->addWidget(chartTitle);

        auto* chart = new QChart();
        chart->setTheme(QChart::ChartThemeDark);
        chart->setMargins(QMargins(0, 0, 0, 0));
        chart->legend()->hide();

        auto* series = new QBarSeries(chart);
        auto* barSet = new QBarSet(tr("Hours"));
        barSet->setColor(QColor("#10b981"));
        series->append(barSet);
        chart->addSeries(series);

        auto* axisX = new QCategoryAxis();
        axisX->setLabelsColor(QColor("#9ca3af"));
        axisX->setGridLineColor(QColor("#2d323d"));
        chart->addAxis(axisX, Qt::AlignBottom);
        series->attachAxis(axisX);

        auto* axisY = new QValueAxis();
        axisY->setLabelsColor(QColor("#9ca3af"));
        axisY->setGridLineColor(QColor("#2d323d"));
        axisY->setTitleText(tr("Hours"));
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);

        m_hoursPerWeekView = new QChartView(chart, frame);
        m_hoursPerWeekView->setRenderHint(QPainter::Antialiasing);
        m_hoursPerWeekView->setRubberBand(QChartView::NoRubberBand);
        m_hoursPerWeekView->setMinimumHeight(250);
        layout->addWidget(m_hoursPerWeekView);

        chartsRow1->addWidget(frame, 1);
    }

    outer->addLayout(chartsRow1);

    // ── Charts row 2: Course progress + Time distribution ──
    auto* chartsRow2 = new QHBoxLayout();
    chartsRow2->setSpacing(16);

    // Course progress breakdown (custom horizontal-bar list)
    {
        auto* frame = new QFrame(container);
        frame->setObjectName("analyticsChartFrame");
        auto* layout = new QVBoxLayout(frame);
        layout->setContentsMargins(16, 16, 16, 16);
        layout->setSpacing(8);

        auto* chartTitle = new QLabel(tr("Course Progress"), frame);
        chartTitle->setObjectName("analyticsChartTitle");
        layout->addWidget(chartTitle);

        m_courseProgressList = new QWidget(frame);
        m_courseProgressList->setObjectName("courseProgressList");
        auto* listLayout = new QVBoxLayout(m_courseProgressList);
        listLayout->setContentsMargins(0, 0, 0, 0);
        listLayout->setSpacing(8);
        listLayout->addStretch();
        layout->addWidget(m_courseProgressList, 1);

        chartsRow2->addWidget(frame, 1);
    }

    // Time distribution (pie chart)
    {
        auto* frame = new QFrame(container);
        frame->setObjectName("analyticsChartFrame");
        auto* layout = new QVBoxLayout(frame);
        layout->setContentsMargins(16, 16, 16, 16);
        layout->setSpacing(8);

        auto* chartTitle = new QLabel(tr("Time Distribution"), frame);
        chartTitle->setObjectName("analyticsChartTitle");
        layout->addWidget(chartTitle);

        auto* chart = new QChart();
        chart->setTheme(QChart::ChartThemeDark);
        chart->setMargins(QMargins(0, 0, 0, 0));
        chart->legend()->setVisible(true);
        chart->legend()->setAlignment(Qt::AlignBottom);
        chart->legend()->setColor(QColor("#9ca3af"));
        chart->legend()->setLabelColor(QColor("#e4e6eb"));

        auto* series = new QPieSeries(chart);
        series->setHoleSize(0.45);
        chart->addSeries(series);

        m_timeDistView = new QChartView(chart, frame);
        m_timeDistView->setRenderHint(QPainter::Antialiasing);
        m_timeDistView->setRubberBand(QChartView::NoRubberBand);
        m_timeDistView->setMinimumHeight(250);
        layout->addWidget(m_timeDistView);

        chartsRow2->addWidget(frame, 1);
    }

    outer->addLayout(chartsRow2);

    // ── Charts row 3: Weekly activity pattern ──
    {
        auto* frame = new QFrame(container);
        frame->setObjectName("analyticsChartFrame");
        auto* layout = new QVBoxLayout(frame);
        layout->setContentsMargins(16, 16, 16, 16);
        layout->setSpacing(8);

        auto* chartTitle = new QLabel(tr("Weekly Activity Pattern"), frame);
        chartTitle->setObjectName("analyticsChartTitle");
        layout->addWidget(chartTitle);

        auto* chart = new QChart();
        chart->setTheme(QChart::ChartThemeDark);
        chart->setMargins(QMargins(0, 0, 0, 0));
        chart->legend()->hide();

        auto* series = new QBarSeries(chart);
        auto* barSet = new QBarSet(tr("Sessions"));
        barSet->setColor(QColor("#3b82f6"));
        series->append(barSet);
        chart->addSeries(series);

        auto* axisX = new QCategoryAxis();
        axisX->setLabelsColor(QColor("#9ca3af"));
        axisX->setGridLineColor(QColor("#2d323d"));
        chart->addAxis(axisX, Qt::AlignBottom);
        series->attachAxis(axisX);

        auto* axisY = new QValueAxis();
        axisY->setLabelsColor(QColor("#9ca3af"));
        axisY->setGridLineColor(QColor("#2d323d"));
        axisY->setTitleText(tr("Sessions"));
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);

        m_weeklyActivityView = new QChartView(chart, frame);
        m_weeklyActivityView->setRenderHint(QPainter::Antialiasing);
        m_weeklyActivityView->setRubberBand(QChartView::NoRubberBand);
        m_weeklyActivityView->setMinimumHeight(200);
        layout->addWidget(m_weeklyActivityView);

        outer->addWidget(frame);
    }

    // ── Heatmap section ──────────────────────────────────────
    auto* heatmapSection = new QVBoxLayout();
    heatmapSection->setSpacing(8);

    // Year navigation row
    auto* navRow = new QHBoxLayout();
    navRow->setSpacing(8);

    m_prevBtn = new QPushButton(QStringLiteral("\u25C0"), container);   // ◀
    m_nextBtn = new QPushButton(QStringLiteral("\u25B6"), container);   // ▶
    m_yearLabel = new QLabel(QString::number(m_year), container);
    m_yearLabel->setObjectName("yearLabel");
    m_yearLabel->setAlignment(Qt::AlignCenter);
    m_yearLabel->setMinimumWidth(80);

    m_prevBtn->setFlat(true);
    m_nextBtn->setFlat(true);

    navRow->addStretch(1);
    navRow->addWidget(m_prevBtn);
    navRow->addWidget(m_yearLabel);
    navRow->addWidget(m_nextBtn);
    navRow->addStretch(1);

    heatmapSection->addLayout(navRow);

    m_heatmap = new ContributionHeatmap(container);
    heatmapSection->addWidget(m_heatmap, 0, Qt::AlignHCenter);

    // Legend
    auto* legend = new QFrame(container);
    legend->setObjectName("heatmapLegend");
    auto* legendLayout = new QHBoxLayout(legend);
    legendLayout->setContentsMargins(0, 4, 0, 0);
    legendLayout->setSpacing(6);
    legendLayout->addStretch(1);
    legendLayout->addWidget(new QLabel(tr("Less"), legend));
    static const char* colors[5] = {
        "#161b22", "#0e4429", "#006d32", "#26a641", "#39d353"
    };
    for (const char* hex : colors) {
        auto* swatch = new QLabel(legend);
        swatch->setFixedSize(12, 12);
        swatch->setStyleSheet(QString("background: %1; border-radius: 2px;").arg(hex));
        legendLayout->addWidget(swatch);
    }
    legendLayout->addWidget(new QLabel(tr("More"), legend));
    legendLayout->addStretch(1);
    heatmapSection->addWidget(legend, 0, Qt::AlignHCenter);

    outer->addLayout(heatmapSection);

    outer->addStretch(1);

    scrollArea->setWidget(container);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(scrollArea);

    connect(m_prevBtn, &QPushButton::clicked, this, &AnalyticsView::onPreviousYear);
    connect(m_nextBtn, &QPushButton::clicked, this, &AnalyticsView::onNextYear);
}

// ── Data refresh ──────────────────────────────────────────────

void AnalyticsView::refreshStatsCards() {
    const AnalyticsSummary summary = AnalyticsSummaryComputer::compute();

    m_streakCard->setValue(QString::number(summary.currentStreakDays));
    m_streakCard->setSubtitle(tr("Longest: %1 days").arg(summary.longestStreakDays));

    m_hoursCard->setValue(QString("%1h").arg(summary.monthHoursStudied));
    m_hoursCard->setSubtitle(tr("This month"));

    m_avgSessionsCard->setValue(
        QString::number(summary.avgSessionsPerDay7d, 'f', 1));
    m_avgSessionsCard->setSubtitle(tr("Last 7 days"));

    // Week comparison: show +/− percentage
    const double pct = summary.weekOverWeekPct;
    if (pct > 0) {
        m_weekCompCard->setValue(QString("+%1%").arg(QString::number(pct, 'f', 1)));
    } else if (pct < 0) {
        m_weekCompCard->setValue(QString("%1%").arg(QString::number(pct, 'f', 1)));
    } else {
        m_weekCompCard->setValue(QStringLiteral("0%"));
    }
    m_weekCompCard->setSubtitle(tr("Productivity change"));
}

void AnalyticsView::refreshCharts() {
    auto* db = DatabaseManager::instance();
    const QDate today = QDate::currentDate();

    // ── Progress over time (8 weeks line chart) ──
    {
        auto* chart = m_progressChartView->chart();
        auto* series = dynamic_cast<QLineSeries*>(chart->series().first());
        auto* axisX = dynamic_cast<QCategoryAxis*>(chart->axes(Qt::Horizontal).first());
        series->clear();
        const QStringList xLabels = axisX->categoriesLabels();
        for (const QString& lbl : xLabels)
            axisX->remove(lbl);

        for (int w = 7; w >= 0; --w) {
            const QDate weekStart = today.addDays(-7 * w);
            const QDate weekEnd = weekStart.addDays(6);
            const QString label = QStringLiteral("W%1").arg(8 - w);

            // Compute average completion % of active courses for this week
            const QList<EntityData> courses = db->fetchAllCourses();
            int totalProgress = 0;
            int count = 0;
            for (const EntityData& c : courses) {
                if (c.status != "active") continue;
                const QList<UnitData> units = db->getUnitsForParent(c.id);
                for (const UnitData& u : units) {
                    const QList<SessionTaskData> sessions = db->getSessionTasksForUnit(u.id);
                    for (const SessionTaskData& s : sessions) {
                        totalProgress += s.progress;
                        ++count;
                    }
                }
            }
            const int avg = (count > 0) ? totalProgress / count : 0;
            series->append(8 - w, avg);
            axisX->append(label, 8 - w);
        }
    }

    // ── Study hours/week (8 weeks bar chart) ──
    {
        auto* chart = m_hoursPerWeekView->chart();
        auto* series = dynamic_cast<QBarSeries*>(chart->series().first());
        auto* barSet = series->barSets().first();
        auto* axisX = dynamic_cast<QCategoryAxis*>(chart->axes(Qt::Horizontal).first());
        while (barSet->count() > 0)
            barSet->remove(0);
        const QStringList xLabels = axisX->categoriesLabels();
        for (const QString& lbl : xLabels)
            axisX->remove(lbl);

        for (int w = 7; w >= 0; --w) {
            const QDate weekStart = today.addDays(-7 * w);
            const QDate weekEnd = weekStart.addDays(6);
            const QString label = QStringLiteral("W%1").arg(8 - w);

            int weekMinutes = 0;
            QDate d = weekStart;
            while (d <= weekEnd) {
                weekMinutes += db->totalMinutesOn(d);
                d = d.addDays(1);
            }
            const double hours = weekMinutes / 60.0;
            barSet->append(hours);
            axisX->append(label, 8 - w);
        }
    }

    // ── Time distribution (pie chart) ──
    {
        auto* chart = m_timeDistView->chart();
        auto* series = dynamic_cast<QPieSeries*>(chart->series().first());
        series->clear();

        // Get pomodoro minutes per course for the last 30 days
        const QList<EntityData> courses = db->fetchAllCourses();
        const QList<PomodoroSessionData> recentSessions = db->fetchRecentSessions(100);

        QMap<int, int> courseMinutes;  // courseId → total minutes
        int freeMinutes = 0;

        for (const PomodoroSessionData& s : recentSessions) {
            if (s.mode != "work") continue;
            if (s.courseId >= 0) {
                courseMinutes[s.courseId] += s.durationMinutes;
            } else {
                freeMinutes += s.durationMinutes;
            }
        }

        for (const EntityData& c : courses) {
            if (courseMinutes.contains(c.id) && courseMinutes[c.id] > 0) {
                auto* slice = series->append(c.name, courseMinutes[c.id]);
                if (c.categoryColor.isValid()) {
                    slice->setColor(c.categoryColor);
                } else {
                    slice->setColor(QColor("#10b981"));
                }
                slice->setLabelVisible(true);
            }
        }
        if (freeMinutes > 0) {
            auto* slice = series->append(tr("Free"), freeMinutes);
            slice->setColor(QColor("#6b7280"));
            slice->setLabelVisible(true);
        }
    }

    // ── Weekly activity pattern (Mon-Sun bar chart) ──
    {
        auto* chart = m_weeklyActivityView->chart();
        auto* series = dynamic_cast<QBarSeries*>(chart->series().first());
        auto* barSet = series->barSets().first();
        auto* axisX = dynamic_cast<QCategoryAxis*>(chart->axes(Qt::Horizontal).first());
        while (barSet->count() > 0)
            barSet->remove(0);
        const QStringList xLabels = axisX->categoriesLabels();
        for (const QString& lbl : xLabels)
            axisX->remove(lbl);

        const QStringList dayNames = {
            tr("Mon"), tr("Tue"), tr("Wed"), tr("Thu"), tr("Fri"), tr("Sat"), tr("Sun")
        };

        // Average sessions per day-of-week over the last 4 weeks
        QMap<int, int> daySessionCounts;  // Qt::Monday=1 .. Qt::Sunday=7
        QMap<int, int> dayWeekCount;

        for (int w = 3; w >= 0; --w) {
            QDate d = today.addDays(-7 * w);
            for (int i = 0; i < 7; ++i) {
                const QList<PomodoroSessionData> sessions = db->fetchSessionsOn(d);
                int workCount = 0;
                for (const PomodoroSessionData& s : sessions) {
                    if (s.mode == "work") ++workCount;
                }
                daySessionCounts[d.dayOfWeek()] += workCount;
                dayWeekCount[d.dayOfWeek()]++;
                d = d.addDays(1);
            }
        }

        for (int day = 1; day <= 7; ++day) {
            double avg = 0.0;
            if (dayWeekCount.contains(day) && dayWeekCount[day] > 0) {
                avg = static_cast<double>(daySessionCounts[day]) / dayWeekCount[day];
            }
            barSet->append(avg);
            axisX->append(dayNames[day - 1], day);
        }
    }

    // ── Course progress list (custom) ──
    refreshCourseProgressList();
}

void AnalyticsView::refreshCourseProgressList() {
    // Clear existing items
    auto* listLayout = m_courseProgressList->layout();
    QLayoutItem* item;
    while ((item = listLayout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    auto* db = DatabaseManager::instance();
    const QList<EntityData> courses = db->fetchAllCourses();

    for (const EntityData& c : courses) {
        if (c.status != "active") continue;

        // Compute overall progress
        const QList<UnitData> units = db->getUnitsForParent(c.id);
        int totalProgress = 0;
        int count = 0;
        for (const UnitData& u : units) {
            const QList<SessionTaskData> sessions = db->getSessionTasksForUnit(u.id);
            for (const SessionTaskData& s : sessions) {
                totalProgress += s.progress;
                ++count;
            }
        }
        const int avg = (count > 0) ? totalProgress / count : 0;

        auto* row = new QWidget(m_courseProgressList);
        row->setObjectName("courseProgressRow");
        auto* rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(4, 4, 4, 4);
        rowLayout->setSpacing(8);

        auto* nameLabel = new QLabel(c.name, row);
        nameLabel->setObjectName("courseProgressName");
        nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        rowLayout->addWidget(nameLabel, 1);

        // Progress bar (simple QFrame-based)
        auto* barBg = new QFrame(row);
        barBg->setObjectName("courseProgressBarBg");
        barBg->setFixedHeight(8);
        barBg->setMinimumWidth(120);
        barBg->setStyleSheet("QFrame#courseProgressBarBg { background: #2d323d; border-radius: 4px; }");

        auto* barFill = new QFrame(barBg);
        barFill->setObjectName("courseProgressBarFill");
        barFill->setFixedHeight(8);
        const QString fillColor = c.categoryColor.isValid()
            ? c.categoryColor.name() : "#10b981";
        barFill->setStyleSheet(
            QString("QFrame#courseProgressBarFill { background: %1; border-radius: 4px; }").arg(fillColor));
        barFill->setGeometry(0, 0, static_cast<int>(120.0 * avg / 100.0), 8);

        rowLayout->addWidget(barBg);

        auto* pctLabel = new QLabel(QString("%1%").arg(avg), row);
        pctLabel->setObjectName("courseProgressPct");
        pctLabel->setMinimumWidth(40);
        pctLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        rowLayout->addWidget(pctLabel);

        // Add to the list layout
        listLayout->addWidget(row);
    }
}

void AnalyticsView::refreshHeatmap() {
    const QDate from(m_year, 1, 1);
    const QDate to(m_year, 12, 31);

    HeatmapAggregator aggregator;
    const QMap<QDate, ContributionHeatmap::DayData> data =
        aggregator.aggregate(from, to, HeatmapAggregator::Mode::NormalizedRange);

    m_heatmap->setData(data);
    m_heatmap->setYear(m_year);
    m_yearLabel->setText(QString::number(m_year));
}

// ── Slots ────────────────────────────────────────────────────

void AnalyticsView::onPreviousYear() {
    loadYear(m_year - 1);
}

void AnalyticsView::onNextYear() {
    loadYear(m_year + 1);
}

void AnalyticsView::loadYear(int year) {
    m_year = year;
    refreshHeatmap();
}

void AnalyticsView::onDataChanged() {
    refreshStatsCards();
    refreshCharts();
    refreshHeatmap();
}
