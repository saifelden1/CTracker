#include "projects/ProjectDetailView.h"

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QDesktopServices>
#include <QUrl>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QFormLayout>
#include <QInputDialog>
#include <QEvent>
#include <QMouseEvent>

#include "core/DatabaseManager.h"
#include "core/DataStructures.h"
#include "projects/TasksBoardWidget.h"

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
    // Phase 10: the left column is the kanban TasksBoardWidget, not the
    // base class's slider-based scroll area. We hide the base scroll
    // area (and the "+ Task" button — the board has its own) but keep
    // the "+ Unit" button visible since the board needs at least one
    // unit to add tasks against.
    m_outerLayout->removeWidget(m_scrollArea);
    m_scrollArea->setVisible(false);
    if (m_addSessionBtn) m_addSessionBtn->setVisible(false);

    auto* twoColumnLayout = new QHBoxLayout();
    twoColumnLayout->setSpacing(16);

    // Left column: kanban tasks board (Phase 10 replacement)
    m_tasksBoard = new TasksBoardWidget(this);
    twoColumnLayout->addWidget(m_tasksBoard, 2);

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

    // Helper styling for the small "(double-click to edit)" hint shown
    // next to every editable section title.
    const QString kSectionTitleQss = QStringLiteral(
        "QLabel { color: #9ca3af; font-size: 12px; }");
    const QString kEditHintHtml = QStringLiteral(
        "<span style=\"color:#9ca3af;\">%1</span>"
        "<span style=\"color:#6b7280;font-size:11px;\">  %2</span>");

    // ── Description ──
    auto* descTitle = new QLabel(m_infoPanel);
    descTitle->setStyleSheet(kSectionTitleQss);
    descTitle->setTextFormat(Qt::RichText);
    descTitle->setText(kEditHintHtml.arg(tr("Description"),
                                         tr("(double-click to edit)")));
    infoLayout->addWidget(descTitle);

    m_descLabel = new QLabel(m_infoPanel);
    m_descLabel->setObjectName("projectDescLabel");
    m_descLabel->setWordWrap(true);
    m_descLabel->setCursor(Qt::IBeamCursor);
    m_descLabel->setToolTip(tr("Double-click to edit the description"));
    m_descLabel->setStyleSheet(
        QStringLiteral(
            "QLabel#projectDescLabel {"
            "  color: #e4e6eb;"
            "  font-size: 14px;"
            "  padding: 4px;"
            "  border-radius: 4px;"
            "}"
            "QLabel#projectDescLabel:hover {"
            "  background: #2d323d;"
            "}"));
    m_descLabel->setText(tr("(no description)"));
    m_descLabel->installEventFilter(this);
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
    auto* teamTitle = new QLabel(m_infoPanel);
    teamTitle->setObjectName("teamTitle");
    teamTitle->setStyleSheet(kSectionTitleQss);
    teamTitle->setTextFormat(Qt::RichText);
    teamTitle->setText(kEditHintHtml.arg(tr("Team Members"),
                                         tr("(double-click to edit)")));
    infoLayout->addWidget(teamTitle);

    m_teamContainer = new QWidget(m_infoPanel);
    m_teamContainer->setObjectName("projectTeamContainer");
    m_teamContainer->setCursor(Qt::PointingHandCursor);
    m_teamContainer->setToolTip(tr("Double-click to edit team members"));
    m_teamContainer->setStyleSheet(
        QStringLiteral(
            "QWidget#projectTeamContainer { padding: 4px; border-radius: 4px; }"
            "QWidget#projectTeamContainer:hover { background: #2d323d; }"));
    m_teamLayout = new QVBoxLayout(m_teamContainer);
    m_teamLayout->setContentsMargins(0, 0, 0, 0);
    m_teamLayout->setSpacing(4);
    m_teamLayout->setAlignment(Qt::AlignTop);
    m_teamContainer->installEventFilter(this);
    infoLayout->addWidget(m_teamContainer);

    // ── Links ──
    auto* linksTitle = new QLabel(m_infoPanel);
    linksTitle->setObjectName("linksTitle");
    linksTitle->setStyleSheet(kSectionTitleQss);
    linksTitle->setTextFormat(Qt::RichText);
    linksTitle->setText(kEditHintHtml.arg(tr("Links"),
                                          tr("(double-click to edit)")));
    infoLayout->addWidget(linksTitle);

    m_linksContainer = new QWidget(m_infoPanel);
    m_linksContainer->setObjectName("projectLinksContainer");
    m_linksContainer->setCursor(Qt::PointingHandCursor);
    m_linksContainer->setToolTip(tr("Double-click to edit links"));
    m_linksContainer->setStyleSheet(
        QStringLiteral(
            "QWidget#projectLinksContainer { padding: 4px; border-radius: 4px; }"
            "QWidget#projectLinksContainer:hover { background: #2d323d; }"));
    m_linksLayout = new QVBoxLayout(m_linksContainer);
    m_linksLayout->setContentsMargins(0, 0, 0, 0);
    m_linksLayout->setSpacing(4);
    m_linksLayout->setAlignment(Qt::AlignTop);
    m_linksContainer->installEventFilter(this);
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

    // Phase 10: feed the kanban board. It reads the project's priority
    // internally so the card stripes match.
    if (m_tasksBoard) m_tasksBoard->setProject(m_entityId);

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
        auto* noTeam = new QLabel(tr("(no team members — double-click to add)"), m_teamContainer);
        noTeam->setStyleSheet(QStringLiteral("QLabel { color: #9ca3af; font-size: 13px; }"));
        // Let the double-click bubble up to the container's event filter.
        noTeam->setAttribute(Qt::WA_TransparentForMouseEvents);
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
            // Pass mouse events through so the container catches double-click.
            memberBadge->setAttribute(Qt::WA_TransparentForMouseEvents);
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
        auto* noLinks = new QLabel(tr("(no links — double-click to add)"), m_linksContainer);
        noLinks->setStyleSheet(QStringLiteral("QLabel { color: #9ca3af; font-size: 13px; }"));
        noLinks->setAttribute(Qt::WA_TransparentForMouseEvents);
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
            // LinksAccessibleByMouse keeps the single-click anchor behaviour
            // while letting double-click events propagate to the eventFilter
            // installed on the container (we also install it on the label
            // itself as a belt-and-braces guard).
            linkLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
            linkLabel->setText(
                QStringLiteral("<a href=\"%1\" style=\"color: #10b981; text-decoration: none;\">%2</a>")
                    .arg(link.url, link.label));
            // Open in default browser when clicked
            connect(linkLabel, &QLabel::linkActivated,
                    [](const QString& url) {
                        QDesktopServices::openUrl(QUrl(url));
                    });
            linkLabel->installEventFilter(this);
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

// ============================================================
//  Inline-edit support
//
//  The Project Info sidebar (description, team, links) is editable
//  via double-click. Instead of giving each widget its own subclass,
//  we install an event filter on the three target widgets and route
//  Qt::MouseButtonDblClick events to the right editor.
// ============================================================

bool ProjectDetailView::eventFilter(QObject* watched, QEvent* event) {
    if (event->type() == QEvent::MouseButtonDblClick && m_entityId >= 0) {
        if (watched == m_descLabel) {
            editDescription();
            return true;
        }
        if (watched == m_teamContainer) {
            editTeam();
            return true;
        }
        if (watched == m_linksContainer) {
            editLinks();
            return true;
        }
        // Link labels are children of m_linksContainer; their parent is
        // the links container, so we treat any double-click bubbling
        // through one of them as a request to edit the link list.
        if (auto* w = qobject_cast<QWidget*>(watched);
            w && w->parentWidget() == m_linksContainer) {
            editLinks();
            return true;
        }
    }
    return EntityDetailView::eventFilter(watched, event);
}

void ProjectDetailView::editDescription() {
    bool ok = false;
    const QString updated = QInputDialog::getMultiLineText(
        this, tr("Edit description"),
        tr("Project description:"),
        m_projectMeta.description, &ok);
    if (!ok || updated == m_projectMeta.description) return;

    ProjectMetaData meta = m_projectMeta;
    meta.description = updated;
    DatabaseManager::instance()->upsertProjectMeta(meta);
    // dataChanged() → onDataChanged() → refreshProjectInfo() re-paints.
}

void ProjectDetailView::editTeam() {
    // Single textarea, one member per line — simplest UX, easiest to
    // round-trip. The existing QStringList stays the wire format.
    const QString current = m_projectMeta.team.join('\n');
    bool ok = false;
    const QString updated = QInputDialog::getMultiLineText(
        this, tr("Edit team members"),
        tr("One name per line:"),
        current, &ok);
    if (!ok) return;

    QStringList list;
    const QStringList raw = updated.split('\n', Qt::SkipEmptyParts);
    list.reserve(raw.size());
    for (const QString& line : raw) {
        const QString trimmed = line.trimmed();
        if (!trimmed.isEmpty()) list << trimmed;
    }
    if (list == m_projectMeta.team) return;

    ProjectMetaData meta = m_projectMeta;
    meta.team = list;
    DatabaseManager::instance()->upsertProjectMeta(meta);
}

void ProjectDetailView::editLinks() {
    // Two-column editor: each row is { Label, URL }. We use a small
    // ad-hoc QDialog because QInputDialog can't host two paired
    // QPlainTextEdits in a maintainable way.
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Edit links"));
    dlg.setModal(true);
    dlg.setMinimumWidth(480);
    dlg.setStyleSheet(QStringLiteral(
        "QDialog { background: #1f2229; color: #e4e6eb; }"
        "QLabel  { color: #e4e6eb; }"
        "QPlainTextEdit {"
        "  background: #252932; border: 1px solid #2d323d;"
        "  border-radius: 6px; padding: 6px 8px; color: #e4e6eb;"
        "}"
        "QPlainTextEdit:focus { border-color: #10b981; }"
        "QPushButton {"
        "  background: #252932; border: 1px solid #2d323d;"
        "  border-radius: 6px; padding: 6px 14px; color: #e4e6eb;"
        "}"
        "QPushButton:hover { background: #2d323d; }"
        "QPushButton[default=\"true\"] {"
        "  background: #10b981; border-color: #10b981; color: #ffffff;"
        "}"));

    auto* outer = new QVBoxLayout(&dlg);
    outer->setSpacing(8);

    auto* explain = new QLabel(
        tr("One link per line. Format: <code>Label | https://url</code>.\n"
           "Lines without a <code>|</code> separator use the URL as the label."),
        &dlg);
    explain->setTextFormat(Qt::RichText);
    explain->setWordWrap(true);
    explain->setStyleSheet(QStringLiteral("QLabel { color: #9ca3af; font-size: 12px; }"));
    outer->addWidget(explain);

    auto* edit = new QPlainTextEdit(&dlg);
    edit->setMinimumHeight(180);
    QStringList serialized;
    for (const ProjectMetaData::Link& l : m_projectMeta.links) {
        if (l.label.isEmpty() || l.label == l.url) {
            serialized << l.url;
        } else {
            serialized << QStringLiteral("%1 | %2").arg(l.label, l.url);
        }
    }
    edit->setPlainText(serialized.join('\n'));
    outer->addWidget(edit, 1);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dlg);
    if (auto* saveBtn = buttons->button(QDialogButtonBox::Save)) {
        saveBtn->setProperty("default", true);
    }
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    outer->addWidget(buttons);

    if (dlg.exec() != QDialog::Accepted) return;

    QList<ProjectMetaData::Link> parsed;
    const QStringList lines = edit->toPlainText().split('\n', Qt::SkipEmptyParts);
    for (const QString& raw : lines) {
        const QString line = raw.trimmed();
        if (line.isEmpty()) continue;

        ProjectMetaData::Link link;
        const int sep = line.indexOf('|');
        if (sep >= 0) {
            link.label = line.left(sep).trimmed();
            link.url   = line.mid(sep + 1).trimmed();
        } else {
            link.url   = line;
            link.label = line;
        }
        if (link.url.isEmpty()) continue;
        // Same-shape comparison short-circuit below uses .label/.url, so
        // give un-labelled rows a label that matches the URL.
        if (link.label.isEmpty()) link.label = link.url;
        parsed << link;
    }

    // Cheap structural compare — avoids a redundant upsert + refresh.
    auto sameLinks = [&]() {
        if (parsed.size() != m_projectMeta.links.size()) return false;
        for (int i = 0; i < parsed.size(); ++i) {
            if (parsed[i].label != m_projectMeta.links[i].label) return false;
            if (parsed[i].url   != m_projectMeta.links[i].url)   return false;
        }
        return true;
    };
    if (sameLinks()) return;

    ProjectMetaData meta = m_projectMeta;
    meta.links = parsed;
    DatabaseManager::instance()->upsertProjectMeta(meta);
}
