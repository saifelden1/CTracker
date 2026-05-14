#include "EntityCard.h"

#include "CircularProgressBar.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QEnterEvent>
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
