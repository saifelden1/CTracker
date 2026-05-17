#include "pomodoro/PomodoroView.h"

#include "core/DatabaseManager.h"
#include "core/DataStructures.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QFrame>
#include <QDate>

// Task 7.8: PomodoroView — two-column layout.
// Left: mode toggle + timer + settings card.
// Right: today's progress + recent sessions.
// Settings disabled while timer is running.

PomodoroView::PomodoroView(QWidget* parent)
    : QWidget(parent) {
    setObjectName("pomodoroView");
    setupUi();
    populateCourseDropdown();
    syncDurationsFromPreferences();
    updateModeToggleButtons();

    // Wire timer signals
    connect(m_timerWidget, &PomodoroTimerWidget::completed,
            this, &PomodoroView::onTimerCompleted);
    connect(m_timerWidget, &PomodoroTimerWidget::stateChanged,
            this, &PomodoroView::onTimerStateChanged);
    connect(m_timerWidget, &PomodoroTimerWidget::modeChanged,
            this, [this]() { updateModeToggleButtons(); });

    // Wire settings combos
    connect(m_courseDropdown, &QComboBox::currentIndexChanged,
            this, &PomodoroView::onCourseSelected);
    connect(m_workDurationCombo, &QComboBox::currentIndexChanged,
            this, &PomodoroView::onWorkDurationChanged);
    connect(m_breakDurationCombo, &QComboBox::currentIndexChanged,
            this, &PomodoroView::onBreakDurationChanged);

    // Live refresh on DB changes
    connect(DatabaseManager::instance(), &DatabaseManager::dataChanged,
            this, &PomodoroView::onDataChanged);

    refreshData();
}

void PomodoroView::setupUi() {
    auto* outer = new QHBoxLayout(this);
    outer->setContentsMargins(24, 24, 24, 24);
    outer->setSpacing(24);

    // ── Left column ──────────────────────────────────────────
    auto* leftCol = new QVBoxLayout();
    leftCol->setSpacing(16);

    // Mode toggle row
    auto* modeRow = new QHBoxLayout();
    modeRow->setSpacing(8);

    m_workModeBtn = new QPushButton(tr("Work Session"), this);
    m_workModeBtn->setObjectName("pomodoroWorkModeBtn");
    m_workModeBtn->setCheckable(true);
    m_workModeBtn->setCursor(Qt::PointingHandCursor);
    connect(m_workModeBtn, &QPushButton::clicked, this, &PomodoroView::onModeToggleWork);

    m_breakModeBtn = new QPushButton(tr("Break"), this);
    m_breakModeBtn->setObjectName("pomodoroBreakModeBtn");
    m_breakModeBtn->setCheckable(true);
    m_breakModeBtn->setCursor(Qt::PointingHandCursor);
    connect(m_breakModeBtn, &QPushButton::clicked, this, &PomodoroView::onModeToggleBreak);

    modeRow->addWidget(m_workModeBtn, 1);
    modeRow->addWidget(m_breakModeBtn, 1);
    leftCol->addLayout(modeRow);

    // Timer widget
    m_timerWidget = new PomodoroTimerWidget(this);
    leftCol->addWidget(m_timerWidget, 0, Qt::AlignHCenter);

    // Settings card
    m_settingsCard = new QFrame(this);
    m_settingsCard->setObjectName("pomodoroSettingsCard");
    auto* settingsLayout = new QVBoxLayout(m_settingsCard);
    settingsLayout->setContentsMargins(16, 16, 16, 16);
    settingsLayout->setSpacing(12);

    auto* settingsTitle = new QLabel(tr("Settings"), m_settingsCard);
    settingsTitle->setObjectName("pomodoroSettingsTitle");
    settingsLayout->addWidget(settingsTitle);

    // Course dropdown
    auto* courseRow = new QHBoxLayout();
    courseRow->setSpacing(8);
    auto* courseLabel = new QLabel(tr("Course:"), m_settingsCard);
    m_courseDropdown = new QComboBox(m_settingsCard);
    m_courseDropdown->setObjectName("pomodoroCourseDropdown");
    m_courseDropdown->setMinimumWidth(200);
    courseRow->addWidget(courseLabel);
    courseRow->addWidget(m_courseDropdown, 1);
    settingsLayout->addLayout(courseRow);

    // Work duration dropdown
    auto* workRow = new QHBoxLayout();
    workRow->setSpacing(8);
    auto* workLabel = new QLabel(tr("Work (min):"), m_settingsCard);
    m_workDurationCombo = new QComboBox(m_settingsCard);
    m_workDurationCombo->setObjectName("pomodoroWorkDuration");
    for (int min : {15, 20, 25, 30, 45, 50}) {
        m_workDurationCombo->addItem(QString::number(min), min);
    }
    workRow->addWidget(workLabel);
    workRow->addWidget(m_workDurationCombo, 1);
    settingsLayout->addLayout(workRow);

    // Break duration dropdown
    auto* breakRow = new QHBoxLayout();
    breakRow->setSpacing(8);
    auto* breakLabel = new QLabel(tr("Break (min):"), m_settingsCard);
    m_breakDurationCombo = new QComboBox(m_settingsCard);
    m_breakDurationCombo->setObjectName("pomodoroBreakDuration");
    for (int min : {5, 10, 15}) {
        m_breakDurationCombo->addItem(QString::number(min), min);
    }
    breakRow->addWidget(breakLabel);
    breakRow->addWidget(m_breakDurationCombo, 1);
    settingsLayout->addLayout(breakRow);

    leftCol->addWidget(m_settingsCard);
    leftCol->addStretch(1);

    // ── Right column ─────────────────────────────────────────
    auto* rightCol = new QVBoxLayout();
    rightCol->setSpacing(16);

    // Today's Progress card
    m_todayProgressCard = new QFrame(this);
    m_todayProgressCard->setObjectName("pomodoroTodayCard");
    auto* todayLayout = new QVBoxLayout(m_todayProgressCard);
    todayLayout->setContentsMargins(16, 16, 16, 16);
    todayLayout->setSpacing(8);

    auto* todayTitle = new QLabel(tr("Today's Progress"), m_todayProgressCard);
    todayTitle->setObjectName("pomodoroTodayTitle");
    todayLayout->addWidget(todayTitle);

    auto* todayStatsRow = new QHBoxLayout();
    todayStatsRow->setSpacing(16);

    auto* sessionsBox = new QWidget(m_todayProgressCard);
    sessionsBox->setObjectName("pomodoroStatBox");
    auto* sessionsLayout = new QVBoxLayout(sessionsBox);
    sessionsLayout->setContentsMargins(12, 8, 12, 8);
    sessionsLayout->setSpacing(4);
    m_todaySessionsLabel = new QLabel("0", sessionsBox);
    m_todaySessionsLabel->setObjectName("pomodoroStatValue");
    auto* sessionsSubLabel = new QLabel(tr("Sessions"), sessionsBox);
    sessionsSubLabel->setObjectName("pomodoroStatLabel");
    sessionsLayout->addWidget(m_todaySessionsLabel);
    sessionsLayout->addWidget(sessionsSubLabel);
    todayStatsRow->addWidget(sessionsBox);

    auto* minutesBox = new QWidget(m_todayProgressCard);
    minutesBox->setObjectName("pomodoroStatBox");
    auto* minutesLayout = new QVBoxLayout(minutesBox);
    minutesLayout->setContentsMargins(12, 8, 12, 8);
    minutesLayout->setSpacing(4);
    m_todayMinutesLabel = new QLabel("0", minutesBox);
    m_todayMinutesLabel->setObjectName("pomodoroStatValue");
    auto* minutesSubLabel = new QLabel(tr("Minutes"), minutesBox);
    minutesSubLabel->setObjectName("pomodoroStatLabel");
    minutesLayout->addWidget(m_todayMinutesLabel);
    minutesLayout->addWidget(minutesSubLabel);
    todayStatsRow->addWidget(minutesBox);

    todayLayout->addLayout(todayStatsRow);
    rightCol->addWidget(m_todayProgressCard);

    // Recent Sessions card
    m_recentSessionsCard = new QFrame(this);
    m_recentSessionsCard->setObjectName("pomodoroRecentCard");
    auto* recentLayout = new QVBoxLayout(m_recentSessionsCard);
    recentLayout->setContentsMargins(16, 16, 16, 16);
    recentLayout->setSpacing(8);

    auto* recentTitle = new QLabel(tr("Recent Sessions"), m_recentSessionsCard);
    recentTitle->setObjectName("pomodoroRecentTitle");
    recentLayout->addWidget(recentTitle);

    m_recentSessionsContent = new QLabel(m_recentSessionsCard);
    m_recentSessionsContent->setObjectName("pomodoroRecentContent");
    m_recentSessionsContent->setWordWrap(true);
    m_recentSessionsContent->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    recentLayout->addWidget(m_recentSessionsContent, 1);

    rightCol->addWidget(m_recentSessionsCard, 1);

    // ── Assemble columns ────────────────────────────────────
    auto* leftWidget = new QWidget(this);
    leftWidget->setLayout(leftCol);

    auto* rightWidget = new QWidget(this);
    rightWidget->setLayout(rightCol);

    outer->addWidget(leftWidget, 2);
    outer->addWidget(rightWidget, 1);
}

void PomodoroView::populateCourseDropdown() {
    m_courseDropdown->clear();
    m_courseDropdown->addItem(tr("No course (free session)"), -1);

    auto* db = DatabaseManager::instance();
    const QList<EntityData> courses = db->fetchAllCourses();
    for (const EntityData& course : courses) {
        m_courseDropdown->addItem(course.name, course.id);
    }

    // Restore saved courseId from PomodoroTimerState
    PomodoroTimerState state = db->getPomodoroState();
    if (state.courseId >= 0) {
        for (int i = 0; i < m_courseDropdown->count(); ++i) {
            if (m_courseDropdown->itemData(i).toInt() == state.courseId) {
                m_courseDropdown->setCurrentIndex(i);
                break;
            }
        }
    }
    m_selectedCourseId = m_courseDropdown->currentData().toInt();
}

void PomodoroView::syncDurationsFromPreferences() {
    auto* db = DatabaseManager::instance();
    const PreferencesData prefs = db->getPreferences();

    // Set work duration combo to match preferences
    for (int i = 0; i < m_workDurationCombo->count(); ++i) {
        if (m_workDurationCombo->itemData(i).toInt() == prefs.workMinutes) {
            m_workDurationCombo->setCurrentIndex(i);
            break;
        }
    }

    // Set break duration combo to match preferences
    for (int i = 0; i < m_breakDurationCombo->count(); ++i) {
        if (m_breakDurationCombo->itemData(i).toInt() == prefs.breakMinutes) {
            m_breakDurationCombo->setCurrentIndex(i);
            break;
        }
    }

    m_timerWidget->setWorkDurationMinutes(prefs.workMinutes);
    m_timerWidget->setBreakDurationMinutes(prefs.breakMinutes);
}

void PomodoroView::updateSettingsEnabledState() {
    const bool running = (m_timerWidget->state() == PomodoroTimerWidget::State::Running);
    m_courseDropdown->setEnabled(!running);
    m_workDurationCombo->setEnabled(!running);
    m_breakDurationCombo->setEnabled(!running);
    m_workModeBtn->setEnabled(!running);
    m_breakModeBtn->setEnabled(!running);
}

void PomodoroView::updateModeToggleButtons() {
    const bool isWork = (m_timerWidget->mode() == PomodoroTimerWidget::Mode::Work);
    m_workModeBtn->setChecked(isWork);
    m_breakModeBtn->setChecked(!isWork);
}

void PomodoroView::refreshData() {
    refreshTodayProgress();
    refreshRecentSessions();
}

void PomodoroView::refreshTodayProgress() {
    auto* db = DatabaseManager::instance();
    const QDate today = QDate::currentDate();

    const QList<PomodoroSessionData> sessions = db->fetchSessionsOn(today);
    const int workSessions = std::count_if(sessions.begin(), sessions.end(),
        [](const PomodoroSessionData& s) { return s.mode == "work"; });
    const int totalMinutes = db->totalMinutesOn(today);

    m_todaySessionsLabel->setText(QString::number(workSessions));
    m_todayMinutesLabel->setText(QString::number(totalMinutes));
}

void PomodoroView::refreshRecentSessions() {
    auto* db = DatabaseManager::instance();
    const QList<PomodoroSessionData> sessions = db->fetchRecentSessions(5);

    if (sessions.isEmpty()) {
        m_recentSessionsContent->setText(tr("No sessions yet. Start your first pomodoro!"));
        return;
    }

    QString html;
    for (const PomodoroSessionData& s : sessions) {
        const QString courseName = s.courseName.isEmpty()
            ? tr("Free session") : s.courseName;
        const QString timeStr = s.completedAt.toString("hh:mm");
        const QString modeStr = (s.mode == "work") ? tr("Work") : tr("Break");
        const QString color = (s.mode == "work") ? "#10b981" : "#3b82f6";

        html += QStringLiteral(
            "<div style='padding:6px 0; border-bottom:1px solid #2d323d;'>"
            "  <span style='color:%1; font-weight:600;'>%2</span>"
            "  — %3 min"
            "  <span style='color:#9ca3af;'> · %4 · %5</span>"
            "</div>"
        ).arg(color, modeStr, QString::number(s.durationMinutes), timeStr, courseName);
    }
    m_recentSessionsContent->setText(html);
}

// ── Slots ────────────────────────────────────────────────────

void PomodoroView::onModeToggleWork() {
    if (m_timerWidget->state() == PomodoroTimerWidget::State::Running) return;
    m_timerWidget->setMode(PomodoroTimerWidget::Mode::Work);
    m_timerWidget->reset();
    updateModeToggleButtons();
}

void PomodoroView::onModeToggleBreak() {
    if (m_timerWidget->state() == PomodoroTimerWidget::State::Running) return;
    m_timerWidget->setMode(PomodoroTimerWidget::Mode::Break);
    m_timerWidget->reset();
    updateModeToggleButtons();
}

void PomodoroView::onTimerCompleted(PomodoroTimerWidget::Mode finishedMode) {
    if (finishedMode == PomodoroTimerWidget::Mode::Work) {
        // Persist the completed work session to DB
        auto* db = DatabaseManager::instance();
        const PreferencesData prefs = db->getPreferences();
        db->insertPomodoroSession(m_selectedCourseId, prefs.workMinutes, "work");
    }
    refreshData();
}

void PomodoroView::onTimerStateChanged(PomodoroTimerWidget::State state) {
    updateSettingsEnabledState();
}

void PomodoroView::onCourseSelected(int index) {
    m_selectedCourseId = m_courseDropdown->itemData(index).toInt();
}

void PomodoroView::onWorkDurationChanged(int index) {
    const int minutes = m_workDurationCombo->itemData(index).toInt();
    m_timerWidget->setWorkDurationMinutes(minutes);

    // Persist to settings
    auto* db = DatabaseManager::instance();
    PreferencesData prefs = db->getPreferences();
    prefs.workMinutes = minutes;
    db->setPreferences(prefs);
}

void PomodoroView::onBreakDurationChanged(int index) {
    const int minutes = m_breakDurationCombo->itemData(index).toInt();
    m_timerWidget->setBreakDurationMinutes(minutes);

    // Persist to settings
    auto* db = DatabaseManager::instance();
    PreferencesData prefs = db->getPreferences();
    prefs.breakMinutes = minutes;
    db->setPreferences(prefs);
}

void PomodoroView::onDataChanged() {
    populateCourseDropdown();
    refreshData();
}