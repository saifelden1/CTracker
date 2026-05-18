#include "courses/UnitCard.h"

#include "shared/CircularProgressBar.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QEnterEvent>
#include <QStyle>

// ============================================================
//  UnitCard.cpp  (Task 7.5a)
//
//  Mirrors EntityCard's QFrame + setProperty("hover") pattern so the
//  same dark-theme QSS rules apply without per-card stylesheets.
//  Two helper labels (counts + last-activity) make the card useful
//  at a glance — the user can scan a course and immediately see
//  which units have momentum and which are stale.
// ============================================================

namespace {

// Translate a timestamp into a short, human-readable "last worked"
// string. Same vocabulary the dashboard uses in casual copy.
QString humaniseSince(const QDateTime& when) {
    if (!when.isValid()) {
        return QObject::tr("Never started");
    }
    const qint64 days = when.daysTo(QDateTime::currentDateTime());
    if (days <= 0)   return QObject::tr("Last worked: today");
    if (days == 1)   return QObject::tr("Last worked: yesterday");
    if (days < 7)    return QObject::tr("Last worked: %1 days ago").arg(days);
    if (days < 30)   return QObject::tr("Last worked: %1 weeks ago").arg(days / 7);
    if (days < 365)  return QObject::tr("Last worked: %1 months ago").arg(days / 30);
    return QObject::tr("Last worked: over a year ago");
}

} // namespace

UnitCard::UnitCard(int unitId, const QString& name, QWidget* parent)
    : QFrame(parent),
      m_unitId(unitId),
      m_name(name) {
    setObjectName("unitCard");
    setFrameShape(QFrame::StyledPanel);
    setFixedSize(180, 200);
    setCursor(Qt::PointingHandCursor);
    setupUi();

    // Default inline styling — survives if no theme QSS is loaded yet.
    // The :hover / [hover="true"] selectors below let a future theme
    // override without changing the .cpp.
    setStyleSheet(QStringLiteral(
        "QFrame#unitCard {"
        "  background: #252932;"
        "  border: 1px solid #2d323d;"
        "  border-radius: 8px;"
        "}"
        "QFrame#unitCard[hover=\"true\"] {"
        "  border-color: #10b981;"
        "}"
        "QLabel#unitCardName {"
        "  color: #e4e6eb;"
        "  font-size: 14px;"
        "  font-weight: 600;"
        "}"
        "QLabel#unitCardCounts {"
        "  color: #9ca3af;"
        "  font-size: 12px;"
        "}"
        "QLabel#unitCardActivity {"
        "  color: #6b7280;"
        "  font-size: 11px;"
        "  font-style: italic;"
        "}"
    ));
}

void UnitCard::setupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(6);

    m_nameLabel = new QLabel(m_name, this);
    m_nameLabel->setObjectName("unitCardName");
    m_nameLabel->setAlignment(Qt::AlignHCenter);
    m_nameLabel->setWordWrap(true);
    m_nameLabel->setFixedHeight(36);   // ~2 lines @ 14 px

    m_ring = new CircularProgressBar(this);
    m_ring->setFixedSize(64, 64);
    m_ring->setLineWidth(7);
    m_ring->setProgress(0);

    m_countsLabel = new QLabel(tr("No sessions"), this);
    m_countsLabel->setObjectName("unitCardCounts");
    m_countsLabel->setAlignment(Qt::AlignHCenter);

    m_activityLabel = new QLabel(humaniseSince({}), this);
    m_activityLabel->setObjectName("unitCardActivity");
    m_activityLabel->setAlignment(Qt::AlignHCenter);
    m_activityLabel->setWordWrap(true);

    layout->addWidget(m_nameLabel);
    layout->addWidget(m_ring, 0, Qt::AlignHCenter);
    layout->addWidget(m_countsLabel);
    layout->addWidget(m_activityLabel);
    layout->addStretch(0);
}

void UnitCard::setName(const QString& name) {
    m_name = name;
    if (m_nameLabel) {
        m_nameLabel->setText(name);
    }
}

void UnitCard::setProgress(int percentage) {
    if (m_ring) {
        m_ring->setProgress(percentage);
    }
}

void UnitCard::setSessionCounts(int total, int completed) {
    if (!m_countsLabel) {
        return;
    }
    if (total <= 0) {
        m_countsLabel->setText(tr("No sessions"));
    } else if (completed <= 0) {
        m_countsLabel->setText(tr("%n session(s)", "", total));
    } else {
        m_countsLabel->setText(tr("%1 / %2 done").arg(completed).arg(total));
    }
}

void UnitCard::setLastActivity(const QDateTime& when) {
    if (m_activityLabel) {
        m_activityLabel->setText(humaniseSince(when));
    }
}

void UnitCard::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked(m_unitId);
    }
    QFrame::mousePressEvent(event);
}

void UnitCard::enterEvent(QEnterEvent* event) {
    setProperty("hover", true);
    style()->unpolish(this);
    style()->polish(this);
    QFrame::enterEvent(event);
}

void UnitCard::leaveEvent(QEvent* event) {
    setProperty("hover", false);
    style()->unpolish(this);
    style()->polish(this);
    QFrame::leaveEvent(event);
}
