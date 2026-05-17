#include "courses/CourseDetailView.h"

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "core/DatabaseManager.h"

// ============================================================
//  CourseDetailView — Task 7.5
//
//  Course-specific detail page. Extends the shared EntityDetailView
//  scaffolding with:
//    1. A Pause/Resume toggle button that calls setCourseStatus()
//    2. A status badge ("Active" / "Paused" / "Completed") next to
//       the course name
//    3. A session count subtitle below the title bar
//
//  The base class handles units, sessions, progress ring, and CRUD.
// ============================================================

CourseDetailView::CourseDetailView(QWidget* parent)
    : EntityDetailView(EntityCard::EntityType::Course, parent) {
    setupCourseChrome();
}

void CourseDetailView::setupCourseChrome() {
    // ── Status badge (inserted after title label) ──
    m_statusBadge = new QLabel(m_titleBar);
    m_statusBadge->setObjectName("courseStatusBadge");
    m_statusBadge->setFixedHeight(24);
    m_statusBadge->setAlignment(Qt::AlignCenter);
    m_statusBadge->setStyleSheet(
        QStringLiteral(
            "QLabel#courseStatusBadge {"
            "  padding: 2px 8px;"
            "  border-radius: 4px;"
            "  font-size: 12px;"
            "  font-weight: 500;"
            "  color: #e4e6eb;"
            "  background: #10b981;"   // default = active (green)
            "}"));

    // Insert after m_titleLabel (index 1 in m_titleLayout)
    m_titleLayout->insertWidget(2, m_statusBadge);

    // ── Pause/Resume toggle button ──
    m_statusToggleBtn = new QPushButton(tr("Pause Course"), m_titleBar);
    m_statusToggleBtn->setObjectName("statusToggleBtn");
    m_statusToggleBtn->setStyleSheet(
        QStringLiteral(
            "QPushButton#statusToggleBtn {"
            "  padding: 6px 16px;"
            "  border-radius: 6px;"
            "  border: 1px solid #2d323d;"
            "  background: #252932;"
            "  color: #e4e6eb;"
            "  font-weight: 500;"
            "}"
            "QPushButton#statusToggleBtn:hover {"
            "  background: #2d323d;"
            "}"));

    // Insert before m_overallRing (which is at index after badge + title)
    // We want: back | title | badge | stretch | toggle | ring | +Unit | +Session | Delete
    m_titleLayout->insertWidget(3, m_statusToggleBtn);

    connect(m_statusToggleBtn, &QPushButton::clicked,
            this, &CourseDetailView::onStatusToggleClicked);

    // ── Session count subtitle ──
    m_sessionCountLabel = new QLabel(m_titleBar);
    m_sessionCountLabel->setObjectName("sessionCountLabel");
    m_sessionCountLabel->setStyleSheet(
        QStringLiteral("QLabel#sessionCountLabel { color: #9ca3af; font-size: 13px; }"));

    // Add below the title bar into the outer layout (after m_titleBar widget)
    m_outerLayout->insertWidget(1, m_sessionCountLabel);
}

void CourseDetailView::loadEntity(int entityId) {
    // Base class handles name + units + overall progress
    EntityDetailView::loadEntity(entityId);

    if (m_entityId < 0) {
        m_courseStatus = "active";
        refreshStatusUI();
        return;
    }

    // Fetch course status from DB
    auto* db = DatabaseManager::instance();
    m_courseStatus = db->getCourseStatus(m_entityId);
    refreshStatusUI();
}

void CourseDetailView::onDataChanged() {
    // Base class rebuilds units + overall progress
    EntityDetailView::onDataChanged();

    if (m_entityId < 0) {
        m_courseStatus = "active";
        refreshStatusUI();
        return;
    }

    // Refresh status (may have changed externally)
    auto* db = DatabaseManager::instance();
    m_courseStatus = db->getCourseStatus(m_entityId);
    refreshStatusUI();
}

void CourseDetailView::onStatusToggleClicked() {
    if (m_entityId < 0) {
        return;
    }

    // Toggle between "active" and "paused"
    QString newStatus;
    if (m_courseStatus == "active") {
        newStatus = "paused";
    } else if (m_courseStatus == "paused") {
        newStatus = "active";
    } else {
        // "completed" → allow re-activating
        newStatus = "active";
    }

    auto* db = DatabaseManager::instance();
    if (db->setCourseStatus(m_entityId, newStatus)) {
        m_courseStatus = newStatus;
        refreshStatusUI();
        // dataChanged() signal will trigger further refresh
    }
}

void CourseDetailView::refreshStatusUI() {
    // ── Status badge ──
    QString badgeText;
    QString badgeColor;
    if (m_courseStatus == "active") {
        badgeText  = tr("Active");
        badgeColor = QStringLiteral("#10b981");   // primary green
    } else if (m_courseStatus == "paused") {
        badgeText  = tr("Paused");
        badgeColor = QStringLiteral("#f59e0b");   // warning amber
    } else {  // completed
        badgeText  = tr("Completed");
        badgeColor = QStringLiteral("#3b82f6");   // info blue
    }

    m_statusBadge->setText(badgeText);
    m_statusBadge->setStyleSheet(
        QStringLiteral(
            "QLabel#courseStatusBadge {"
            "  padding: 2px 8px;"
            "  border-radius: 4px;"
            "  font-size: 12px;"
            "  font-weight: 500;"
            "  color: #e4e6eb;"
            "  background: %1;"
            "}").arg(badgeColor));

    // ── Toggle button text ──
    if (m_courseStatus == "active") {
        m_statusToggleBtn->setText(tr("Pause Course"));
    } else if (m_courseStatus == "paused") {
        m_statusToggleBtn->setText(tr("Resume Course"));
    } else {
        m_statusToggleBtn->setText(tr("Re-activate Course"));
    }

    // ── Session count subtitle ──
    if (m_entityId < 0) {
        m_sessionCountLabel->setText(QString());
        return;
    }

    auto* db = DatabaseManager::instance();
    const QList<UnitData> units = db->getUnitsForParent(m_entityId);
    int totalSessions = 0;
    for (const UnitData& u : units) {
        totalSessions += db->getSessionTasksForUnit(u.id).size();
    }
    m_sessionCountLabel->setText(
        tr("%1 sessions across %2 units")
            .arg(totalSessions)
            .arg(units.size()));
}
