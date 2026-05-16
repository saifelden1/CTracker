#include "shared/HomeDashboard.h"

#include "shared/StatsCard.h"
#include "calendar/CalendarWidget.h"
#include "calendar/DayDetailsPanel.h"
#include "analytics/ContributionHeatmap.h"
#include "analytics/HeatmapAggregator.h"
#include "core/DatabaseManager.h"
#include "core/DataStructures.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDate>

// Task 7.2: HomeDashboard — final unified form with stats, calendar, day details, and heatmap.

HomeDashboard::HomeDashboard(QWidget* parent)
    : QWidget(parent) {
    setupUi();
    connect(DatabaseManager::instance(), &DatabaseManager::dataChanged,
            this, &HomeDashboard::onDataChanged);
    refreshStatsCards();
    refreshCalendar();
    refreshHeatmap();
}

void HomeDashboard::setupUi() {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(24, 24, 24, 24);
    outer->setSpacing(24);

    // ── Top row: 3 × StatsCard ──
    auto* statsRow = new QHBoxLayout();
    statsRow->setSpacing(16);

    m_activeCoursesCard = new StatsCard(tr("Active Courses"), this);
    m_projectsCard      = new StatsCard(tr("Projects"), this);
    m_completionCard    = new StatsCard(tr("Completion Rate"), this);

    statsRow->addWidget(m_activeCoursesCard);
    statsRow->addWidget(m_projectsCard);
    statsRow->addWidget(m_completionCard);

    outer->addLayout(statsRow);

    // ── Lower section: Calendar (left) + DayDetails/Heatmap stack (right) ──
    auto* lowerRow = new QHBoxLayout();
    lowerRow->setSpacing(16);

    // Left: Calendar
    m_calendar = new CalendarWidget(this);
    m_calendar->setMinimumSize(320, 280);
    connect(m_calendar, &CalendarWidget::dateClicked,
            this, &HomeDashboard::onCalendarDateClicked);

    // Right: stacked DayDetailsPanel (top) + ContributionHeatmap (bottom)
    auto* rightCol = new QVBoxLayout();
    rightCol->setSpacing(16);

    m_dayDetails = new DayDetailsPanel(this);
    m_dayDetails->setVisible(false);  // hidden until a day is clicked
    connect(m_dayDetails, &DayDetailsPanel::closed,
            this, &HomeDashboard::onDayDetailsClosed);
    connect(m_dayDetails, &DayDetailsPanel::todoAdded,
            this, &HomeDashboard::onTodoAdded);
    connect(m_dayDetails, &DayDetailsPanel::todoToggled,
            this, &HomeDashboard::onTodoToggled);
    connect(m_dayDetails, &DayDetailsPanel::notesChanged,
            this, &HomeDashboard::onNotesChanged);

    m_heatmap = new ContributionHeatmap(this);
    m_heatmap->setMinimumHeight(180);

    rightCol->addWidget(m_dayDetails);
    rightCol->addWidget(m_heatmap, 1);

    lowerRow->addWidget(m_calendar, 1);
    lowerRow->addLayout(rightCol, 2);

    outer->addLayout(lowerRow, 1);
}

// ── Stats computation ────────────────────────────────────────

int HomeDashboard::countActiveCourses() const {
    auto* db = DatabaseManager::instance();
    const QList<EntityData> courses = db->fetchAllCourses();
    int count = 0;
    for (const EntityData& c : courses) {
        if (c.status == "active") ++count;
    }
    return count;
}

int HomeDashboard::countPausedCourses() const {
    auto* db = DatabaseManager::instance();
    const QList<EntityData> courses = db->fetchAllCourses();
    int count = 0;
    for (const EntityData& c : courses) {
        if (c.status == "paused") ++count;
    }
    return count;
}

int HomeDashboard::countProjects() const {
    return DatabaseManager::instance()->fetchAllProjects().size();
}

int HomeDashboard::countProjectsDueSoon() const {
    auto* db = DatabaseManager::instance();
    const QList<EntityData> projects = db->fetchAllProjects();
    const QDate today = QDate::currentDate();
    int count = 0;
    for (const EntityData& p : projects) {
        const ProjectMetaData meta = db->getProjectMeta(p.id);
        if (meta.deadline.isValid()) {
            const int daysLeft = today.daysTo(meta.deadline);
            if (daysLeft >= 0 && daysLeft <= 7) {
                ++count;
            }
        }
    }
    return count;
}

int HomeDashboard::computeCompletionRate() const {
    auto* db = DatabaseManager::instance();
    const QList<EntityData> courses = db->fetchAllCourses();
    int sum = 0;
    int count = 0;
    for (const EntityData& c : courses) {
        if (c.status != "active") continue;
        const QList<UnitData> units = db->getUnitsForParent(c.id);
        for (const UnitData& u : units) {
            const QList<SessionTaskData> sessions = db->getSessionTasksForUnit(u.id);
            for (const SessionTaskData& s : sessions) {
                sum += s.progress;
                ++count;
            }
        }
    }
    return (count > 0) ? (sum / count) : 0;
}

void HomeDashboard::refreshStatsCards() {
    const int activeCourses = countActiveCourses();
    const int pausedCourses = countPausedCourses();
    const int projects      = countProjects();
    const int dueSoon       = countProjectsDueSoon();
    const int completion    = computeCompletionRate();

    m_activeCoursesCard->setValue(QString::number(activeCourses));
    if (pausedCourses > 0) {
        m_activeCoursesCard->setBadgeText(tr("%1 paused").arg(pausedCourses));
    } else {
        m_activeCoursesCard->setBadgeText({});
    }

    m_projectsCard->setValue(QString::number(projects));
    if (dueSoon > 0) {
        m_projectsCard->setSubtitle(tr("%1 due soon").arg(dueSoon));
    } else {
        m_projectsCard->setSubtitle({});
    }

    m_completionCard->setValue(QString("%1%").arg(completion));
    m_completionCard->setSubtitle(tr("Active courses average"));
}

void HomeDashboard::refreshCalendar() {
    auto* db = DatabaseManager::instance();
    const QDate today = QDate::currentDate();
    const QDate from  = today.addDays(-30);
    const QDate to    = today.addDays(30);
    const QSet<QDate> dates = db->datesWithContent(from, to);
    m_calendar->setIndicatorDates(dates);
}

void HomeDashboard::refreshHeatmap() {
    // Task 7.2: Heatmap in RecentBuckets mode for last 12 weeks (84 days).
    const QDate today = QDate::currentDate();
    const QDate from  = today.addDays(-84);
    const QDate to    = today;

    HeatmapAggregator aggregator;
    const QMap<QDate, ContributionHeatmap::DayData> data =
        aggregator.aggregate(from, to, HeatmapAggregator::Mode::RecentBuckets);

    m_heatmap->setData(data);
}

// ── Slots ────────────────────────────────────────────────────

void HomeDashboard::onDataChanged() {
    refreshStatsCards();
    refreshCalendar();
    refreshHeatmap();
}

void HomeDashboard::onCalendarDateClicked(const QDate& date) {
    auto* db = DatabaseManager::instance();
    const CalendarDayData dayData = db->getDay(date);
    m_dayDetails->showDay(dayData);
    m_dayDetails->setVisible(true);
}

void HomeDashboard::onDayDetailsClosed() {
    m_dayDetails->setVisible(false);
}

void HomeDashboard::onTodoAdded(const QDate& date, const QString& text) {
    auto* db = DatabaseManager::instance();
    CalendarDayData dayData = db->getDay(date);
    dayData.todo.append(text);
    db->upsertDay(dayData);
    refreshCalendar();
}

void HomeDashboard::onTodoToggled(const QDate& date, int index, bool completed) {
    auto* db = DatabaseManager::instance();
    CalendarDayData dayData = db->getDay(date);

    if (completed) {
        // Move from todo → completed
        if (index >= 0 && index < dayData.todo.size()) {
            const QString item = dayData.todo.takeAt(index);
            dayData.completed.append(item);
        }
    } else {
        // Move from completed → todo
        if (index >= 0 && index < dayData.completed.size()) {
            const QString item = dayData.completed.takeAt(index);
            dayData.todo.append(item);
        }
    }

    db->upsertDay(dayData);
    m_dayDetails->showDay(dayData);  // refresh display
    refreshCalendar();
}

void HomeDashboard::onNotesChanged(const QDate& date, const QString& text) {
    auto* db = DatabaseManager::instance();
    CalendarDayData dayData = db->getDay(date);
    dayData.notes = text;
    db->upsertDay(dayData);
    refreshCalendar();
}
