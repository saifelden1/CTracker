#include "courses/SessionTaskRow.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSlider>
#include <QStackedWidget>
#include <QMouseEvent>
#include <QKeyEvent>
#include <algorithm>

SessionTaskRow::SessionTaskRow(int sessionId,
                               const QString& name,
                               int progress,
                               QWidget* parent)
    : QWidget(parent),
      m_sessionId(sessionId),
      m_name(name),
      m_oldProgress(progress) {
    setupUi();
    setProgress(progress);
}

void SessionTaskRow::setupUi() {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setSpacing(12);

    m_nameStack = new QStackedWidget(this);

    m_nameLabel = new QLabel(m_name, m_nameStack);
    m_nameLabel->setMinimumWidth(180);
    m_nameLabel->installEventFilter(this);
    m_nameLabel->setCursor(Qt::IBeamCursor);

    m_nameEdit = new QLineEdit(m_nameStack);
    m_nameEdit->installEventFilter(this);

    m_nameStack->addWidget(m_nameLabel);
    m_nameStack->addWidget(m_nameEdit);
    m_nameStack->setCurrentWidget(m_nameLabel);

    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setRange(0, 100);
    m_slider->setSingleStep(1);
    m_slider->setPageStep(5);

    m_percentLabel = new QLabel(this);
    m_percentLabel->setMinimumWidth(40);
    m_percentLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    layout->addWidget(m_nameStack, 2);
    layout->addWidget(m_slider, 3);
    layout->addWidget(m_percentLabel, 0);

    connect(m_slider, &QSlider::sliderPressed,
            this, &SessionTaskRow::onSliderPressed);
    connect(m_slider, &QSlider::sliderReleased,
            this, &SessionTaskRow::onSliderReleased);
    connect(m_slider, &QSlider::valueChanged,
            this, &SessionTaskRow::onSliderValueChanged);

    connect(m_nameEdit, &QLineEdit::editingFinished,
            this, &SessionTaskRow::onNameEditingFinished);
}

int SessionTaskRow::progress() const {
    return m_slider ? m_slider->value() : 0;
}

void SessionTaskRow::setProgress(int value) {
    if (!m_slider) {
        return;
    }
    const int clamped = std::clamp(value, 0, 100);
    QSignalBlocker blocker(m_slider);
    m_slider->setValue(clamped);
    m_oldProgress = clamped;
    m_percentLabel->setText(QString("%1%").arg(clamped));
}

void SessionTaskRow::setName(const QString& newName) {
    m_name = newName;
    m_nameLabel->setText(newName);
}

bool SessionTaskRow::eventFilter(QObject* watched, QEvent* event) {
    if (watched == m_nameLabel && event->type() == QEvent::MouseButtonDblClick) {
        enterEditMode();
        return true;
    }
    if (watched == m_nameEdit && event->type() == QEvent::KeyPress) {
        auto* key = static_cast<QKeyEvent*>(event);
        if (key->key() == Qt::Key_Escape) {
            exitEditMode(false);
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void SessionTaskRow::enterEditMode() {
    m_nameEdit->setText(m_name);
    m_nameStack->setCurrentWidget(m_nameEdit);
    m_nameEdit->setFocus(Qt::MouseFocusReason);
    m_nameEdit->selectAll();
}

void SessionTaskRow::exitEditMode(bool commit) {
    if (commit) {
        const QString candidate = m_nameEdit->text().trimmed();
        if (!candidate.isEmpty() && candidate != m_name) {
            m_name = candidate;
            m_nameLabel->setText(m_name);
            emit nameChanged(m_name);
        }
    }
    m_nameStack->setCurrentWidget(m_nameLabel);
}

void SessionTaskRow::onNameEditingFinished() {
    // editingFinished fires on focus loss too — treat as commit.
    if (m_nameStack->currentWidget() == m_nameEdit) {
        exitEditMode(true);
    }
}

void SessionTaskRow::onSliderPressed() {
    m_sliderPressed = true;
    m_oldProgress = m_slider->value();
}

void SessionTaskRow::onSliderReleased() {
    m_sliderPressed = false;
    const int newValue = m_slider->value();
    if (newValue != m_oldProgress) {
        const int oldValue = m_oldProgress;
        m_oldProgress = newValue;
        emit progressChanged(oldValue, newValue);
    }
}

void SessionTaskRow::onSliderValueChanged(int value) {
    m_percentLabel->setText(QString("%1%").arg(value));
}
