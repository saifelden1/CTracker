#include "projects/ProjectDetailView.h"

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QDesktopServices>
#include <QUrl>

#include "core/DatabaseManager.h"
#include "core/DataStructures.h"

// ============================================================
//  ProjectDetailView — Task 7.6
//
//  Project-specific detail page. Extends the shared EntityDetailView
//  scaffolding with:
//    1. Two-column layout: left = task checklist (units/sessions
//       scroll area from base class), right = project info panel
//    2. Status / priority / deadline badges in the title bar
//    3. Project info sidebar: description, deadline, team members,
//       and clickable links (opened via QDesktopServices::openUrl)
//
//  The base class handles units, sessions, progress ring, and CRUD.
// ============================================================

ProjectDetailView::ProjectDetailView(QWidget* parent)
    : EntityDetailView(EntityCard::EntityType::Project, parent) {
    setupProjectChrome();
}

void ProjectDetailView::setupProjectChrome() {
    // ── Restructure content into two columns ──
    // The base class added m_scrollArea directly to m_outerLayout.
    // Remove it so we can place it inside a two-column layout instead.
    m_outerLayout->removeWidget(m_scrollArea);

    auto* twoColumnLayout = new QHBoxLayout();
    twoColumnLayout->setSpacing(16);

    // Left column: task checklist (the scroll area from base class)
    twoColumnLayout->addWidget(m_scrollArea, 2);

    // Right column: project info panel
    m_infoPanel = new QWidget(this);
    m_infoPanel->setObjectName("projectInfoPanel");
    m_infoPanel->setStyleSheet(
        QStringLiteral(
            "QWidget#projectInfoPanel {"
            "  background: #252932;"
            "  border-radius: 8px;"
            "  border: 1px solid #2d323d;"
            "}"));

    auto* infoLayout = new QVBoxLayout(m_infoPanel);
    infoLayout->setContentsMargins(16, 16, 16, 16);
    infoLayout->setSpacing(16);
    infoLayout->setAlignment(Qt::AlignTop);

    // ── Info panel header ──
    auto* infoHeader = new QLabel(tr("Project Info"), m_infoPanel);
    infoHeader->setObjectName("infoPanelHeader");
    infoHeader->setStyleSheet(
        QStringLiteral(
            "QLabel#infoPanelHeader {"
            "  font-size: 16px;"
            "  font-weight: 600;"
            "  color: #e4e6eb;"
            "}"));
    infoLayout->addWidget(infoHeader);

    // ── Description ──
    auto* descTitle = new QLabel(tr("Description"), m_infoPanel);
    descTitle->setStyleSheet(
        QStringLiteral("QLabel { color: #9ca3af; font-size: 12px; }"));
    infoLayout->addWidget(descTitle);

    m_descLabel = new QLabel(m_infoPanel);
    m_descLabel->setObjectName("projectDescLabel");
    m_descLabel->setWordWrap(true);
    m_descLabel->setStyleSheet(
        QStringLiteral(
            "QLabel#projectDescLabel {"
            "  color: #e4e6eb;"
            "  font-size: 14px;"
            "}"));
    m_descLabel->setText(tr("(no description)"));
    infoLayout->addWidget(m_descLabel);

    // ── Deadline ──
    auto* deadlineTitle = new QLabel(tr("Deadline"), m_infoPanel);
    deadlineTitle->setStyleSheet(
        QStringLiteral("QLabel { color: #9ca3af; font-size: 12px; }"));
    infoLayout->addWidget(deadlineTitle);

    m_deadlineLabel = new QLabel(m_infoPanel);
    m_deadlineLabel->setObjectName("projectDeadlineLabel");
    m_deadlineLabel->setStyleSheet(
        QStringLiteral(
            "QLabel#projectDeadlineLabel {"
            "  color: #e4e6eb;"
            "  font-size: 14px;"
            "}"));
    m_deadlineLabel->setText(tr("(no deadline)"));
    infoLayout->addWidget(m_deadlineLabel);

    // ── Team members ──
    auto* teamTitle = new QLabel(tr("Team Members"), m_infoPanel);
    teamTitle->setObjectName("teamTitle");
    teamTitle->setStyleSheet(
        QStringLiteral("QLabel { color: #9ca3af; font-size: 12px; }"));
    infoLayout->addWidget(teamTitle);

    m_teamContainer = new QWidget(m_infoPanel);
    m_teamLayout = new QVBoxLayout(m_teamContainer);
    m_teamLayout->setContentsMargins(0, 0, 0, 0);
    m_teamLayout->setSpacing(4);
    m_teamLayout->setAlignment(Qt::AlignTop);
    infoLayout->addWidget(m_teamContainer);

    // ── Links ──
    auto* linksTitle = new QLabel(tr("Links"), m_infoPanel);
    linksTitle->setObjectName("linksTitle");
    linksTitle->setStyleSheet(
        QStringLiteral("QLabel { color: #9ca3af; font-size: 12px; }"));
    infoLayout->addWidget(linksTitle);

    m_linksContainer = new QWidget(m_infoPanel);
    m_linksLayout = new QVBoxLayout(m_linksContainer);
    m_linksLayout->setContentsMargins(0, 0, 0, 0);
    m_linksLayout->setSpacing(4);
    m_linksLayout->setAlignment(Qt::AlignTop);
    infoLayout->addWidget(m_linksContainer);

    infoLayout->addStretch(1);

    twoColumnLayout->addWidget(m_infoPanel, 1);

    // Add the two-column layout to the outer layout (replacing the scroll area)
    m_outerLayout->addLayout(twoColumnLayout, 1);

    // ── Title bar badges ──
    // Insert after m_titleLabel (index 1 in m_titleLayout)
    // Layout: back | title | statusBadge | priorityBadge | deadlineBadge | stretch | ring | +Unit | +Task | Delete

    m_statusBadge = new QLabel(m_titleBar);
    m_statusBadge->setObjectName("projectStatusBadge");
    m_statusBadge->setFixedHeight(24);
    m_statusBadge->setAlignment(Qt::AlignCenter);
    m_titleLayout->insertWidget(2, m_statusBadge);

    m_priorityBadge = new QLabel(m_titleBar);
    m_priorityBadge->setObjectName("projectPriorityBadge");
    m_priorityBadge->setFixedHeight(24);
    m_priorityBadge->setAlignment(Qt::AlignCenter);
    m_titleLayout->insertWidget(3, m_priorityBadge);

    m_deadlineBadge = new QLabel(m_titleBar);
    m_deadlineBadge->setObjectName("projectDeadlineBadge");
    m_deadlineBadge->setFixedHeight(24);
    m_deadlineBadge->setAlignment(Qt::AlignCenter);
    m_titleLayout->insertWidget(4, m_deadlineBadge);
}

void ProjectDetailView::loadEntity(int entityId) {
    // Base class handles name + units + overall progress
    EntityDetailView::loadEntity(entityId);

    if (m_entityId < 0) {
        m_projectStatus = "active";
        m_projectMeta = ProjectMetaData();  // defaults
        refreshProjectInfo();
        return;
    }

    // Fetch project status and metadata from DB
    auto* db = DatabaseManager::instance();

    // Find the entity to get its status
    const QList<EntityData> all = db->fetchAllProjects();
    for (const EntityData& e : all) {
        if (e.id == m_entityId) {
            m_projectStatus = e.status;
            break;
        }
    }

    m_projectMeta = db->getProjectMeta(m_entityId);
    refreshProjectInfo();
}

void ProjectDetailView::onDataChanged() {
    // Base class rebuilds units + overall progress
    EntityDetailView::onDataChanged();

    if (m_entityId < 0) {
        m_projectStatus = "active";
        m_projectMeta = ProjectMetaData();
        refreshProjectInfo();
        return;
    }

    // Refresh project status and metadata
    auto* db = DatabaseManager::instance();

    const QList<EntityData> all = db->fetchAllProjects();
    for (const EntityData& e : all) {
        if (e.id == m_entityId) {
            m_projectStatus = e.status;
            break;
        }
    }

    m_projectMeta = db->getProjectMeta(m_entityId);
    refreshProjectInfo();
}

void ProjectDetailView::refreshProjectInfo() {
    // ── Status badge ──
    QString statusText;
    QString statusColor;
    if (m_projectStatus == "active") {
        statusText  = tr("Active");
        statusColor = QStringLiteral("#10b981");
    } else if (m_projectStatus == "paused") {
        statusText  = tr("Paused");
        statusColor = QStringLiteral("#f59e0b");
    } else {  // completed
        statusText  = tr("Completed");
        statusColor = QStringLiteral("#3b82f6");
    }

    m_statusBadge->setText(statusText);
    m_statusBadge->setStyleSheet(
        QStringLiteral(
            "QLabel#projectStatusBadge {"
            "  padding: 2px 8px;"
            "  border-radius: 4px;"
            "  font-size: 12px;"
            "  font-weight: 500;"
            "  color: #e4e6eb;"
            "  background: %1;"
            "}").arg(statusColor));

    // ── Priority badge ──
    QString priorityText = m_projectMeta.priority;
    QString priorityColor;
    if (priorityText == "high") {
        priorityColor = QStringLiteral("#ef4444");   // error red
    } else if (priorityText == "medium") {
        priorityColor = QStringLiteral("#f59e0b");   // warning amber
    } else {  // low
        priorityColor = QStringLiteral("#6b7280");   // subtle gray
    }

    m_priorityBadge->setText(tr("%1 priority").arg(priorityText));
    m_priorityBadge->setStyleSheet(
        QStringLiteral(
            "QLabel#projectPriorityBadge {"
            "  padding: 2px 8px;"
            "  border-radius: 4px;"
            "  font-size: 12px;"
            "  font-weight: 500;"
            "  color: #e4e6eb;"
            "  background: %1;"
            "}").arg(priorityColor));

    // ── Deadline badge ──
    refreshDeadlineBadge();

    // ── Description ──
    if (m_projectMeta.description.isEmpty()) {
        m_descLabel->setText(tr("(no description)"));
    } else {
        m_descLabel->setText(m_projectMeta.description);
    }

    // ── Deadline in info panel ──
    if (!m_projectMeta.deadline.isValid()) {
        m_deadlineLabel->setText(tr("(no deadline)"));
    } else {
        m_deadlineLabel->setText(
            m_projectMeta.deadline.toString(QStringLiteral("MMMM d, yyyy")));
    }

    // ── Team members ──
    // Clear old team labels. deleteLater() is unsafe in a findChild loop:
    // the child remains discoverable until the event loop runs.
    QLayoutItem* item = nullptr;
    while ((item = m_teamLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    if (m_projectMeta.team.isEmpty()) {
        auto* noTeam = new QLabel(tr("(no team members)"), m_teamContainer);
        noTeam->setStyleSheet(QStringLiteral("QLabel { color: #9ca3af; font-size: 13px; }"));
        m_teamLayout->addWidget(noTeam);
    } else {
        for (const QString& member : m_projectMeta.team) {
            auto* memberBadge = new QLabel(member, m_teamContainer);
            memberBadge->setStyleSheet(
                QStringLiteral(
                    "QLabel {"
                    "  padding: 4px 10px;"
                    "  border-radius: 4px;"
                    "  background: #2d323d;"
                    "  color: #e4e6eb;"
                    "  font-size: 13px;"
                    "}"));
            m_teamLayout->addWidget(memberBadge);
        }
    }

    // ── Links ──
    // Clear old link labels.
    while ((item = m_linksLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    if (m_projectMeta.links.isEmpty()) {
        auto* noLinks = new QLabel(tr("(no links)"), m_linksContainer);
        noLinks->setStyleSheet(QStringLiteral("QLabel { color: #9ca3af; font-size: 13px; }"));
        m_linksLayout->addWidget(noLinks);
    } else {
        for (const ProjectMetaData::Link& link : m_projectMeta.links) {
            auto* linkLabel = new QLabel(m_linksContainer);
            linkLabel->setStyleSheet(
                QStringLiteral(
                    "QLabel {"
                    "  color: #10b981;"
                    "  font-size: 13px;"
                    "  cursor: pointer;"
                    "}"));
            linkLabel->setTextFormat(Qt::RichText);
            linkLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
            linkLabel->setText(
                QStringLiteral("<a href=\"%1\" style=\"color: #10b981; text-decoration: none;\">%2</a>")
                    .arg(link.url, link.label));
            // Open in default browser when clicked
            connect(linkLabel, &QLabel::linkActivated,
                    [](const QString& url) {
                        QDesktopServices::openUrl(QUrl(url));
                    });
            m_linksLayout->addWidget(linkLabel);
        }
    }
}

void ProjectDetailView::refreshDeadlineBadge() {
    if (!m_projectMeta.deadline.isValid()) {
        m_deadlineBadge->setText(tr("No deadline"));
        m_deadlineBadge->setStyleSheet(
            QStringLiteral(
                "QLabel#projectDeadlineBadge {"
                "  padding: 2px 8px;"
                "  border-radius: 4px;"
                "  font-size: 12px;"
                "  font-weight: 500;"
                "  color: #9ca3af;"
                "  background: #2d323d;"
                "}"));
        return;
    }

    const QDate today = QDate::currentDate();
    const int daysLeft = today.daysTo(m_projectMeta.deadline);

    QString badgeText;
    QString badgeColor;

    if (daysLeft < 0) {
        badgeText  = tr("Overdue");
        badgeColor = QStringLiteral("#ef4444");   // error red
    } else if (daysLeft <= 3) {
        badgeText  = tr("%1d left").arg(daysLeft);
        badgeColor = QStringLiteral("#ef4444");   // error red
    } else if (daysLeft <= 7) {
        badgeText  = tr("%1d left").arg(daysLeft);
        badgeColor = QStringLiteral("#f59e0b");   // warning amber
    } else {
        badgeText  = tr("%1d left").arg(daysLeft);
        badgeColor = QStringLiteral("#6b7280");   // subtle gray
    }

    m_deadlineBadge->setText(badgeText);
    m_deadlineBadge->setStyleSheet(
        QStringLiteral(
            "QLabel#projectDeadlineBadge {"
            "  padding: 2px 8px;"
            "  border-radius: 4px;"
            "  font-size: 12px;"
            "  font-weight: 500;"
            "  color: #e4e6eb;"
            "  background: %1;"
            "}").arg(badgeColor));
}
