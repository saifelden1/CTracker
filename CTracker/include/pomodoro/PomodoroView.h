#pragma once

#include <QWidget>
#include "pomodoro/PomodoroTimerWidget.h"

class QComboBox;
class QLabel;
class QPushButton;
class QFrame;

// Task 7.8: PomodoroView — two-column layout.
// Left: mode toggle (Work / Break) + PomodoroTimerWidget + Settings card
//   (course dropdown, work duration {15/20/25/30/45/50}, break duration {5/10/15}).
// Right: Today's Progress card (sessions + minutes from totalMinutesOn(today))
//   + Recent Sessions card (last 5).
// On completed(Work) → insertPomodoroSession + auto-switch + reset
//   (auto-switch handled inside PomodoroTimerWidget; view persists the session row).
// Settings widgets disabled while timer is running.
// Restores PomodoroTimerState from DB on construction (handled by PomodoroTimerWidget).
class PomodoroView : public QWidget {
    Q_OBJECT
public:
    explicit PomodoroView(QWidget* parent = nullptr);

public slots:
    void refreshData();

private slots:
    void onModeToggleWork();
    void onModeToggleBreak();
    void onTimerCompleted(PomodoroTimerWidget::Mode finishedMode);
    void onTimerStateChanged(PomodoroTimerWidget::State state);
    void onCourseSelected(int index);
    void onWorkDurationChanged(int index);
    void onBreakDurationChanged(int index);
    void onDataChanged();

private:
    void setupUi();
    void populateCourseDropdown();
    void syncDurationsFromPreferences();
    void updateSettingsEnabledState();
    void updateModeToggleButtons();
    void refreshTodayProgress();
    void refreshRecentSessions();

    // Left column
    QPushButton*         m_workModeBtn       = nullptr;
    QPushButton*         m_breakModeBtn      = nullptr;
    PomodoroTimerWidget* m_timerWidget       = nullptr;
    QFrame*              m_settingsCard      = nullptr;
    QComboBox*           m_courseDropdown    = nullptr;
    QComboBox*           m_workDurationCombo = nullptr;
    QComboBox*           m_breakDurationCombo = nullptr;

    // Right column
    QFrame*   m_todayProgressCard     = nullptr;
    QLabel*   m_todaySessionsLabel    = nullptr;
    QLabel*   m_todayMinutesLabel     = nullptr;
    QFrame*   m_recentSessionsCard    = nullptr;
    QLabel*   m_recentSessionsContent = nullptr;

    int m_selectedCourseId = -1;
};