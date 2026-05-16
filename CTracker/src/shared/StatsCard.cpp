#include "shared/StatsCard.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QEnterEvent>
#include <QStyle>

StatsCard::StatsCard(const QString& title, QWidget* parent)
    : QFrame(parent) {
    setObjectName("statsCard");
    setFrameShape(QFrame::StyledPanel);
    setupUi(title);
}

void StatsCard::setupUi(const QString& title) {
    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(16, 16, 16, 16);
    outerLayout->setSpacing(8);

    // Top row: stretch | icon (top-right)
    auto* topRow = new QHBoxLayout();
    topRow->setSpacing(0);

    auto* spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(24, 24);
    m_iconLabel->setAlignment(Qt::AlignRight | Qt::AlignTop);
    m_iconLabel->setVisible(false);  // hidden until setIcon() is called

    topRow->addWidget(spacer);
    topRow->addWidget(m_iconLabel);
    outerLayout->addLayout(topRow);

    // Title label — muted, small font
    m_titleLabel = new QLabel(title, this);
    m_titleLabel->setObjectName("statsCardTitle");
    outerLayout->addWidget(m_titleLabel);

    // Value label — large, bold
    m_valueLabel = new QLabel(QStringLiteral("—"), this);
    m_valueLabel->setObjectName("statsCardValue");
    outerLayout->addWidget(m_valueLabel);

    // Subtitle label — muted
    m_subtitleLabel = new QLabel(this);
    m_subtitleLabel->setObjectName("statsCardSubtitle");
    outerLayout->addWidget(m_subtitleLabel);

    // Badge label — optional accent pill, hidden by default
    m_badgeLabel = new QLabel(this);
    m_badgeLabel->setObjectName("statsCardBadge");
    m_badgeLabel->setAlignment(Qt::AlignLeft);
    m_badgeLabel->setVisible(false);
    outerLayout->addWidget(m_badgeLabel);

    outerLayout->addStretch();
}

void StatsCard::setValue(const QString& value) {
    m_valueLabel->setText(value);
}

void StatsCard::setSubtitle(const QString& subtitle) {
    m_subtitleLabel->setText(subtitle);
}

void StatsCard::setBadgeText(const QString& text) {
    if (text.isEmpty()) {
        m_badgeLabel->setVisible(false);
        return;
    }
    m_badgeLabel->setText(text);
    m_badgeLabel->setVisible(true);
}

void StatsCard::setIcon(const QIcon& icon) {
    if (icon.isNull()) {
        m_iconLabel->setVisible(false);
        return;
    }
    m_iconLabel->setPixmap(icon.pixmap(24, 24));
    m_iconLabel->setVisible(true);
}

void StatsCard::enterEvent(QEnterEvent* event) {
    setProperty("hover", true);
    style()->unpolish(this);
    style()->polish(this);
    QFrame::enterEvent(event);
}

void StatsCard::leaveEvent(QEvent* event) {
    setProperty("hover", false);
    style()->unpolish(this);
    style()->polish(this);
    QFrame::leaveEvent(event);
}