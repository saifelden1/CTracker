#include "pomodoro/PomodoroTimerWidget.h"

#include "shared/CircularProgressBar.h"
#include "core/DatabaseManager.h"
#include "core/DataStructures.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QPushButton>
#include <QLabel>
#include <QDateTime>

PomodoroTimerWidget::PomodoroTimerWidget(QWidget* parent)
    : QWidget(parent) {
    setObjectName("pomodoroTimerWidget");
    setupUi();
    restoreState();
    updateDisplay();
    updateButtons();
}

void PomodoroTimerWidget::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);
    mainLayout->setAlignment(Qt::AlignCenter);

    // Mode label (Work / Break)
    m_modeLabel = new QLabel(QStringLiteral("Work"), this);
    m_modeLabel->setObjectName("pomodoroModeLabel");
    m_modeLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_modeLabel);

    // Circular progress ring — 256 px
    m_ring = new CircularProgressBar(this);
    m_ring->setMinimumSize(256, 256);
    m_ring->setMaximumSize(256, 256);
    m_ring->setLineWidth(12);
    mainLayout->addWidget(m_ring, 0, Qt::AlignHCenter);

    // Control buttons row
    auto* btnRow = new QHBoxLayout();
    btnRow->setSpacing(8);
    btnRow->setAlignment(Qt::AlignCenter);

    m_startBtn  = new QPushButton(QStringLiteral("Start"),  this);
    m_pauseBtn  = new QPushButton(QStringLiteral("Pause"),  this);
    m_resumeBtn = new QPushButton(QStringLiteral("Resume"), this);
    m_resetBtn  = new QPushButton(QStringLiteral("Reset"),  this);

    m_startBtn->setObjectName("pomodoroStartBtn");
    m_pauseBtn->setObjectName("pomodoroPauseBtn");
    m_resumeBtn->setObjectName("pomodoroResumeBtn");
    m_resetBtn->setObjectName("pomodoroResetBtn");

    connect(m_startBtn,  &QPushButton::clicked, this, &PomodoroTimerWidget::start);
    connect(m_pauseBtn,  &QPushButton::clicked, this, &PomodoroTimerWidget::pause);
    connect(m_resumeBtn, &QPushButton::clicked, this, &PomodoroTimerWidget::resume);
    connect(m_resetBtn,  &QPushButton::clicked, this, &PomodoroTimerWidget::reset);

    btnRow->addWidget(m_startBtn);
    btnRow->addWidget(m_pauseBtn);
    btnRow->addWidget(m_resumeBtn);
    btnRow->addWidget(m_resetBtn);
    mainLayout->addLayout(btnRow);

    // 1-second tick timer
    m_tickTimer = new QTimer(this);
    m_tickTimer->setInterval(1000);
    connect(m_tickTimer, &QTimer::timeout, this, &PomodoroTimerWidget::onTick);
}

// ── Public API ──────────────────────────────────────────────

PomodoroTimerWidget::Mode PomodoroTimerWidget::mode() const {
    return m_mode;
}

PomodoroTimerWidget::State PomodoroTimerWidget::state() const {
    return m_state;
}

int PomodoroTimerWidget::remainingSeconds() const {
    return m_remainingSeconds;
}

void PomodoroTimerWidget::setMode(Mode mode) {
    if (mode == m_mode) return;
    m_mode = mode;
    m_totalSeconds = (mode == Mode::Work)
        ? m_workDurationMinutes * 60
        : m_breakDurationMinutes * 60;
    m_modeLabel->setText(mode == Mode::Work ? QStringLiteral("Work") : QStringLiteral("Break"));
    emit modeChanged(mode);
}

void PomodoroTimerWidget::setWorkDurationMinutes(int minutes) {
    if (minutes <= 0) return;
    m_workDurationMinutes = minutes;
    if (m_mode == Mode::Work) {
        m_totalSeconds = minutes * 60;
        // Only reset remaining time if timer is idle
        if (m_state == State::Idle) {
            m_remainingSeconds = m_totalSeconds;
            updateDisplay();
        }
    }
}

void PomodoroTimerWidget::setBreakDurationMinutes(int minutes) {
    if (minutes <= 0) return;
    m_breakDurationMinutes = minutes;
    if (m_mode == Mode::Break) {
        m_totalSeconds = minutes * 60;
        // Only reset remaining time if timer is idle
        if (m_state == State::Idle) {
            m_remainingSeconds = m_totalSeconds;
            updateDisplay();
        }
    }
}

// ── Slots ───────────────────────────────────────────────────

void PomodoroTimerWidget::start() {
    if (m_state == State::Running) return;
    m_state = State::Running;
    m_tickTimer->start();
    updateButtons();
    persistState();
    emit stateChanged(m_state);
}

void PomodoroTimerWidget::pause() {
    if (m_state != State::Running) return;
    m_state = State::Paused;
    m_tickTimer->stop();
    updateButtons();
    persistState();
    emit stateChanged(m_state);
}

void PomodoroTimerWidget::resume() {
    if (m_state != State::Paused) return;
    m_state = State::Running;
    m_tickTimer->start();
    updateButtons();
    persistState();
    emit stateChanged(m_state);
}

void PomodoroTimerWidget::reset() {
    m_state = State::Idle;
    m_tickTimer->stop();
    m_remainingSeconds = m_totalSeconds;
    updateDisplay();
    updateButtons();
    persistState();
    emit stateChanged(m_state);
}

void PomodoroTimerWidget::onTick() {
    if (m_remainingSeconds <= 0) {
        // Timer completed
        m_tickTimer->stop();
        emit completed(m_mode);

        // Auto-switch mode: Work → Break, Break → Work
        Mode nextMode = (m_mode == Mode::Work) ? Mode::Break : Mode::Work;
        setMode(nextMode);
        m_state = State::Idle;
        m_remainingSeconds = m_totalSeconds;
        updateDisplay();
        updateButtons();
        persistState();
        emit stateChanged(m_state);
        return;
    }

    m_remainingSeconds--;
    updateDisplay();
    emit tick(m_remainingSeconds);
}

// ── Private helpers ─────────────────────────────────────────

void PomodoroTimerWidget::updateDisplay() {
    // Compute ring progress percentage
    int pct = 0;
    if (m_totalSeconds > 0) {
        pct = static_cast<int>(100.0 * (m_totalSeconds - m_remainingSeconds) / m_totalSeconds);
    }
    m_ring->setProgress(pct);

    // Format MM:SS for center text
    int mins = m_remainingSeconds / 60;
    int secs = m_remainingSeconds % 60;
    QString timeText = QStringLiteral("%1:%2")
        .arg(mins, 2, 10, QLatin1Char('0'))
        .arg(secs, 2, 10, QLatin1Char('0'));
    m_ring->setCustomText(timeText);
}

void PomodoroTimerWidget::updateButtons() {
    // Visibility logic:
    // Idle:     Start + Reset visible; Pause/Resume hidden
    // Running:  Pause + Reset visible; Start/Resume hidden
    // Paused:   Resume + Reset visible; Start/Pause hidden
    m_startBtn->setVisible(m_state == State::Idle);
    m_pauseBtn->setVisible(m_state == State::Running);
    m_resumeBtn->setVisible(m_state == State::Paused);
    m_resetBtn->setVisible(true);  // always available
}

void PomodoroTimerWidget::persistState() {
    PomodoroTimerState state;
    state.mode             = (m_mode == Mode::Work)
        ? PomodoroTimerState::Work : PomodoroTimerState::Break;
    state.state            = (m_state == State::Idle)
        ? PomodoroTimerState::Idle
        : (m_state == State::Running)
            ? PomodoroTimerState::Running
            : PomodoroTimerState::Paused;
    state.courseId         = -1;  // set by PomodoroView when a course is selected
    state.totalSeconds     = m_totalSeconds;
    state.remainingSeconds = m_remainingSeconds;
    state.startedAt        = (m_state == State::Running) ? QDateTime::currentDateTime() : QDateTime();

    DatabaseManager::instance()->savePomodoroState(state);
}

void PomodoroTimerWidget::restoreState() {
    PomodoroTimerState state = DatabaseManager::instance()->getPomodoroState();

    m_mode = (state.mode == PomodoroTimerState::Work) ? Mode::Work : Mode::Break;
    m_totalSeconds = state.totalSeconds;
    m_remainingSeconds = state.remainingSeconds;

    // Only restore to Paused if there's meaningful remaining time
    if (state.state == PomodoroTimerState::Paused && m_remainingSeconds > 0) {
        m_state = State::Paused;
    } else if (state.state == PomodoroTimerState::Running && m_remainingSeconds > 0) {
        // If it was running, we pause it so the user can decide to resume
        // (time has passed since the app was closed)
        m_state = State::Paused;
    } else {
        m_state = State::Idle;
        m_remainingSeconds = m_totalSeconds;
    }

    m_modeLabel->setText(m_mode == Mode::Work ? QStringLiteral("Work") : QStringLiteral("Break"));
}