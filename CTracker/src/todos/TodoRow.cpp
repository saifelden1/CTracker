#include "todos/TodoRow.h"

#include "core/DataStructures.h"

#include <QHBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QEnterEvent>
#include <QStyle>

// Priority color mapping: high=#ef4444, medium=#f59e0b, low=#6b7280
// Use numeric-RGB ctor — string-hex form is unsafe at file-scope (pre-main).
static const QColor kPriorityHigh   (0xef, 0x44, 0x44);
static const QColor kPriorityMedium (0xf5, 0x9e, 0x0b);
static const QColor kPriorityLow    (0x6b, 0x72, 0x80);

static QColor priorityColor(const QString& priority) {
    if (priority == "high")   return kPriorityHigh;
    if (priority == "medium") return kPriorityMedium;
    return kPriorityLow;  // "low" or default
}

TodoRow::TodoRow(const TodoData& data, QWidget* parent)
    : QWidget(parent),
      m_todoId(data.id) {
    setObjectName("todoRow");
    setupUi(data);
}

void TodoRow::setupUi(const TodoData& data) {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setSpacing(12);

    // Checkbox
    m_checkbox = new QCheckBox(this);
    m_checkbox->setChecked(data.completed);
    connect(m_checkbox, &QCheckBox::toggled, this, [this](bool checked) {
        applyCompletedStyle(checked);
        emit completedToggled(m_todoId, checked);
    });

    // Title label
    m_titleLabel = new QLabel(data.title, this);
    m_titleLabel->setWordWrap(false);
    m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // Priority badge — small pill
    m_priorityBadge = new QLabel(data.priority, this);
    m_priorityBadge->setObjectName("todoPriorityBadge");
    m_priorityBadge->setAlignment(Qt::AlignCenter);
    m_priorityBadge->setFixedSize(60, 20);
    applyPriorityStyle(data.priority);

    // Delete button — × icon
    m_deleteBtn = new QPushButton(QStringLiteral("×"), this);
    m_deleteBtn->setObjectName("todoDeleteBtn");
    m_deleteBtn->setFixedSize(24, 24);
    connect(m_deleteBtn, &QPushButton::clicked, this, [this]() {
        emit deleteRequested(m_todoId);
    });

    layout->addWidget(m_checkbox);
    layout->addWidget(m_titleLabel, 1);
    layout->addWidget(m_priorityBadge);
    layout->addWidget(m_deleteBtn);

    // Apply initial completed style
    applyCompletedStyle(data.completed);
}

void TodoRow::setCompleted(bool completed) {
    m_checkbox->setChecked(completed);
    applyCompletedStyle(completed);
}

void TodoRow::setPriority(const QString& priority) {
    m_priorityBadge->setText(priority);
    applyPriorityStyle(priority);
}

void TodoRow::applyPriorityStyle(const QString& priority) {
    QColor color = priorityColor(priority);
    // Badge: colored text on 15% alpha background
    const int alpha15 = static_cast<int>(255 * 0.15);
    const QString rgbaBg = QStringLiteral("rgba(%1, %2, %3, %4)")
                               .arg(color.red())
                               .arg(color.green())
                               .arg(color.blue())
                               .arg(alpha15);

    m_priorityBadge->setStyleSheet(
        QStringLiteral(
            "QLabel#todoPriorityBadge {"
            "  background-color: %1;"
            "  color: %2;"
            "  border-radius: 4px;"
            "  font-size: 11px;"
            "}"
        ).arg(rgbaBg, color.name()));
}

void TodoRow::applyCompletedStyle(bool completed) {
    if (completed) {
        // Muted + strikethrough effect (QSS doesn't support text-decoration,
        // so we use font-style: italic + muted color as visual proxy)
        m_titleLabel->setStyleSheet(
            QStringLiteral(
                "QLabel {"
                "  color: #9ca3af;"
                "  font-style: italic;"
                "}"
            ));
        setProperty("completed", true);
    } else {
        m_titleLabel->setStyleSheet(
            QStringLiteral(
                "QLabel {"
                "  color: #e4e6eb;"
                "}"
            ));
        setProperty("completed", false);
    }
    style()->unpolish(this);
    style()->polish(this);
}

void TodoRow::enterEvent(QEnterEvent* event) {
    setProperty("hover", true);
    style()->unpolish(this);
    style()->polish(this);
    QWidget::enterEvent(event);
}

void TodoRow::leaveEvent(QEvent* event) {
    setProperty("hover", false);
    style()->unpolish(this);
    style()->polish(this);
    QWidget::leaveEvent(event);
}