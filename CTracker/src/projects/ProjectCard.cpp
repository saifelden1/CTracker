#include "projects/ProjectCard.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QMouseEvent>
#include <QEnterEvent>
#include <QDate>
#include <QStyle>

// Priority color mapping.
// Use numeric-RGB ctor — string-hex form is unsafe at file-scope (pre-main).
static const QColor kPriorityHigh   (0xef, 0x44, 0x44);
static const QColor kPriorityMedium (0xf5, 0x9e, 0x0b);
static const QColor kPriorityLow    (0x6b, 0x72, 0x80);

static QColor priorityColor(const QString& priority) {
    if (priority == "high")   return kPriorityHigh;
    if (priority == "medium") return kPriorityMedium;
    return kPriorityLow;
}

ProjectCard::ProjectCard(int projectId, QWidget* parent)
    : QFrame(parent),
      m_projectId(projectId) {
    setObjectName("projectCard");
    setFrameShape(QFrame::StyledPanel);
    setCursor(Qt::PointingHandCursor);
    setupUi();
}

void ProjectCard::setupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(8);

    // Name label — semibold
    m_nameLabel = new QLabel(this);
    m_nameLabel->setObjectName("projectCardName");
    layout->addWidget(m_nameLabel);

    // Description label — 2-line truncate
    m_descLabel = new QLabel(this);
    m_descLabel->setObjectName("projectCardDesc");
    m_descLabel->setWordWrap(true);
    m_descLabel->setMaximumHeight(40);  // ~2 lines at 14px font
    layout->addWidget(m_descLabel);

    // Badges row: priority + deadline
    auto* badgeRow = new QHBoxLayout();
    badgeRow->setSpacing(8);

    m_priorityBadge = new QLabel(this);
    m_priorityBadge->setObjectName("projectPriorityBadge");
    m_priorityBadge->setAlignment(Qt::AlignCenter);
    m_priorityBadge->setFixedSize(60, 20);

    m_deadlineBadge = new QLabel(this);
    m_deadlineBadge->setObjectName("projectDeadlineBadge");
    m_deadlineBadge->setAlignment(Qt::AlignCenter);
    m_deadlineBadge->setFixedSize(80, 20);
    m_deadlineBadge->setVisible(false);  // hidden until setDeadline()

    badgeRow->addWidget(m_priorityBadge);
    badgeRow->addWidget(m_deadlineBadge);
    badgeRow->addStretch();
    layout->addLayout(badgeRow);

    // Progress bar — horizontal
    m_progressBar = new QProgressBar(this);
    m_progressBar->setObjectName("projectProgressBar");
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(false);
    m_progressBar->setFixedHeight(8);
    layout->addWidget(m_progressBar);

    // Bottom row: task count + team size
    auto* bottomRow = new QHBoxLayout();
    bottomRow->setSpacing(12);

    m_taskCountLabel = new QLabel(this);
    m_taskCountLabel->setObjectName("projectTaskCount");

    m_teamSizeLabel = new QLabel(this);
    m_teamSizeLabel->setObjectName("projectTeamSize");

    bottomRow->addWidget(m_taskCountLabel);
    bottomRow->addStretch();
    bottomRow->addWidget(m_teamSizeLabel);
    layout->addLayout(bottomRow);
}

void ProjectCard::setName(const QString& name) {
    m_nameLabel->setText(name);
}

void ProjectCard::setDescription(const QString& text) {
    m_descLabel->setText(text);
}

void ProjectCard::setPriority(const QString& priority) {
    m_priorityBadge->setText(priority);
    applyPriorityStyle(priority);
}

void ProjectCard::setDeadline(const QDate& deadline) {
    if (!deadline.isValid()) {
        m_deadlineBadge->setVisible(false);
        return;
    }

    int daysLeft = QDate::currentDate().daysTo(deadline);
    QString text;
    if (daysLeft <= 0) {
        text = QStringLiteral("Overdue");
    } else {
        text = QStringLiteral("%1d left").arg(daysLeft);
    }
    m_deadlineBadge->setText(text);
    m_deadlineBadge->setVisible(true);
    applyDeadlineStyle(deadline);
}

void ProjectCard::setProgress(int pct) {
    m_progressBar->setValue(std::clamp(pct, 0, 100));
}

void ProjectCard::setTaskCount(int done, int total) {
    m_taskCountLabel->setText(QStringLiteral("%1/%2 tasks").arg(done).arg(total));
}

void ProjectCard::setTeamSize(int n) {
    if (n <= 0) {
        m_teamSizeLabel->setVisible(false);
        return;
    }
    m_teamSizeLabel->setText(QStringLiteral("👥 %1").arg(n));
    m_teamSizeLabel->setVisible(true);
}

void ProjectCard::applyPriorityStyle(const QString& priority) {
    QColor color = priorityColor(priority);
    const int alpha15 = static_cast<int>(255 * 0.15);
    const QString rgbaBg = QStringLiteral("rgba(%1, %2, %3, %4)")
                               .arg(color.red())
                               .arg(color.green())
                               .arg(color.blue())
                               .arg(alpha15);

    m_priorityBadge->setStyleSheet(
        QStringLiteral(
            "QLabel#projectPriorityBadge {"
            "  background-color: %1;"
            "  color: %2;"
            "  border-radius: 4px;"
            "  font-size: 11px;"
            "}"
        ).arg(rgbaBg, color.name()));
}

void ProjectCard::applyDeadlineStyle(const QDate& deadline) {
    int daysLeft = QDate::currentDate().daysTo(deadline);
    QColor color;
    if (daysLeft <= 3) {
        color = QColor("#ef4444");  // red
    } else if (daysLeft <= 7) {
        color = QColor("#f59e0b");  // amber
    } else {
        color = QColor("#6b7280");  // gray
    }

    const int alpha15 = static_cast<int>(255 * 0.15);
    const QString rgbaBg = QStringLiteral("rgba(%1, %2, %3, %4)")
                               .arg(color.red())
                               .arg(color.green())
                               .arg(color.blue())
                               .arg(alpha15);

    m_deadlineBadge->setStyleSheet(
        QStringLiteral(
            "QLabel#projectDeadlineBadge {"
            "  background-color: %1;"
            "  color: %2;"
            "  border-radius: 4px;"
            "  font-size: 11px;"
            "}"
        ).arg(rgbaBg, color.name()));
}

void ProjectCard::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked(m_projectId);
    }
    QFrame::mousePressEvent(event);
}

void ProjectCard::enterEvent(QEnterEvent* event) {
    setProperty("hover", true);
    style()->unpolish(this);
    style()->polish(this);
    QFrame::enterEvent(event);
}

void ProjectCard::leaveEvent(QEvent* event) {
    setProperty("hover", false);
    style()->unpolish(this);
    style()->polish(this);
    QFrame::leaveEvent(event);
}