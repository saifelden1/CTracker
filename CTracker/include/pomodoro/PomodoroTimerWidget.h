#pragma once

#include <QWidget>
#include <QString>
#include <QDateTime>

class QTimer;
class CircularProgressBar;
class QPushButton;
class QLabel;

// PomodoroTimerWidget: embeds a CircularProgressBar (256 px) driven by a
// 1 Hz QTimer. Center label shows MM:SS via CircularProgressBar::setCustomText.
// On completion, auto-switches mode (Work→Break, Break→Work) and emits completed().
// Restores from DatabaseManager::getPomodoroState() on construction for
// cross-session/cross-navigation resume.
class PomodoroTimerWidget : public QWidget {
    Q_OBJECT
public:
    enum class Mode  { Work, Break };
    enum class State { Idle, Running, Paused };

    explicit PomodoroTimerWidget(QWidget* parent = nullptr);

    void setMode(Mode mode);
    void setWorkDurationMinutes(int minutes);
    void setBreakDurationMinutes(int minutes);

    Mode  mode()           const;
    State state()          const;
    int   remainingSeconds() const;

public slots:
    void start();
    void pause();
    void resume();
    void reset();

signals:
    void modeChanged(Mode mode);
    void stateChanged(State state);
    void tick(int remainingSeconds);
    void completed(Mode finishedMode);   // emitted when timer hits 0

private slots:
    void onTick();

private:
    void setupUi();
    void updateDisplay();
    void updateButtons();
    void persistState();
    void restoreState();

    QTimer*              m_tickTimer     = nullptr;
    CircularProgressBar* m_ring          = nullptr;
    QPushButton*         m_startBtn      = nullptr;
    QPushButton*         m_pauseBtn      = nullptr;
    QPushButton*         m_resumeBtn     = nullptr;
    QPushButton*         m_resetBtn      = nullptr;
    QLabel*              m_modeLabel     = nullptr;

    int   m_workDurationMinutes  = 25;
    int   m_breakDurationMinutes = 5;
    int   m_totalSeconds         = 25 * 60;
    int   m_remainingSeconds     = 25 * 60;
    Mode  m_mode                 = Mode::Work;
    State m_state                = State::Idle;
};