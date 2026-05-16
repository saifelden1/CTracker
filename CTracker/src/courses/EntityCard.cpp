#include "courses/EntityCard.h"

#include "shared/CircularProgressBar.h"
#include "shared/CategoryPill.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QEnterEvent>
#include <QResizeEvent>
#include <QStyle>

EntityCard::EntityCard(int entityId,
                        const QString& name,
                        EntityType type,
                        int progress,
                        QWidget* parent)
    : QFrame(parent),
      m_entityId(entityId),
      m_type(type),
      m_name(name) {
    setObjectName("entityCard");
    setFrameShape(QFrame::StyledPanel);
    setFixedSize(160, 180);
    setCursor(Qt::PointingHandCursor);
    setupUi(progress);
}

void EntityCard::setupUi(int initialProgress) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 16, 12, 12);
    layout->setSpacing(8);

    m_progressBar = new CircularProgressBar(this);
    m_progressBar->setMinimumSize(96, 96);
    m_progressBar->setProgress(initialProgress);

    m_nameLabel = new QLabel(m_name, this);
    m_nameLabel->setAlignment(Qt::AlignCenter);
    m_nameLabel->setWordWrap(true);

    m_typeLabel = new QLabel(
        m_type == EntityType::Course ? QStringLiteral("Course")
                                     : QStringLiteral("Project"),
        this);
    m_typeLabel->setObjectName("entityTypeBadge");
    m_typeLabel->setAlignment(Qt::AlignCenter);

    layout->addWidget(m_progressBar, 0, Qt::AlignHCenter);
    layout->addWidget(m_nameLabel);
    layout->addWidget(m_typeLabel);

    // ── v2 additions: overlay widgets ──

    // CategoryPill (top-left) — starts hidden
    m_categoryPill = new CategoryPill(this);
    m_categoryPill->setVisible(false);

    // Status badge (top-right) — starts hidden
    m_statusBadge = new QLabel(QStringLiteral("Paused"), this);
    m_statusBadge->setObjectName("entityStatusBadge");
    m_statusBadge->setAlignment(Qt::AlignCenter);
    m_statusBadge->setFixedSize(50, 20);
    m_statusBadge->setVisible(false);
    // Muted styling: gray text on semi-transparent gray bg
    m_statusBadge->setStyleSheet(
        QStringLiteral(
            "QLabel#entityStatusBadge {"
            "  background-color: rgba(156, 163, 175, 38);"  // 15% alpha ≈ 38
            "  color: #9ca3af;"
            "  border-radius: 4px;"
            "  font-size: 11px;"
            "}"
        ));
}

void EntityCard::setProgress(int percentage) {
    if (m_progressBar) {
        m_progressBar->setProgress(percentage);
    }
}

void EntityCard::setName(const QString& name) {
    m_name = name;
    if (m_nameLabel) {
        m_nameLabel->setText(name);
    }
}

// ── v2 additions ────────────────────────────────────────────

void EntityCard::setCategory(const CategoryData& cat) {
    m_categoryPill->setCategory(cat);
    m_categoryPill->setVisible(true);
    positionOverlays();
}

void EntityCard::clearCategory() {
    m_categoryPill->clearCategory();
    positionOverlays();
}

void EntityCard::setStatus(const QString& status) {
    m_status = status;
    if (status == "paused") {
        m_statusBadge->setVisible(true);
    } else {
        m_statusBadge->setVisible(false);
    }
    positionOverlays();
}

void EntityCard::positionOverlays() {
    // Position CategoryPill at top-left (4 px margin)
    if (m_categoryPill->isVisible()) {
        m_categoryPill->move(4, 4);
        m_categoryPill->raise();
    }

    // Position status badge at top-right (4 px margin)
    if (m_statusBadge->isVisible()) {
        int x = width() - m_statusBadge->width() - 4;
        m_statusBadge->move(x, 4);
        m_statusBadge->raise();
    }
}

// ── Event handlers ──────────────────────────────────────────

void EntityCard::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked(m_entityId, m_type);
    }
    QFrame::mousePressEvent(event);
}

void EntityCard::enterEvent(QEnterEvent* event) {
    setProperty("hover", true);
    style()->unpolish(this);
    style()->polish(this);
    QFrame::enterEvent(event);
}

void EntityCard::leaveEvent(QEvent* event) {
    setProperty("hover", false);
    style()->unpolish(this);
    style()->polish(this);
    QFrame::leaveEvent(event);
}

void EntityCard::resizeEvent(QResizeEvent* event) {
    QFrame::resizeEvent(event);
    positionOverlays();
}
