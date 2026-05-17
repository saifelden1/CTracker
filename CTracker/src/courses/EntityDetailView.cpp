#include "courses/EntityDetailView.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>

#include "shared/CircularProgressBar.h"
#include "courses/UnitExpandableWidget.h"
#include "core/DatabaseManager.h"

// ============================================================
//  EntityDetailView — shared base for CourseDetailView and
//  ProjectDetailView (Tasks 7.5 / 7.6).
//
//  Provides the common scaffolding: title bar with back button,
//  name label, overall-progress ring, add-unit / add-session /
//  delete buttons, and a scroll area of UnitExpandableWidget
//  items. Subclasses extend the title bar and content area.
// ============================================================

EntityDetailView::EntityDetailView(EntityCard::EntityType type, QWidget* parent)
    : QWidget(parent),
      m_type(type) {
    setupUi();
    connect(DatabaseManager::instance(), &DatabaseManager::dataChanged,
            this, &EntityDetailView::onDataChanged);
}

void EntityDetailView::setupUi() {
    m_outerLayout = new QVBoxLayout(this);
    m_outerLayout->setContentsMargins(16, 16, 16, 16);
    m_outerLayout->setSpacing(12);

    // ---- Title bar ----
    m_titleBar = new QWidget(this);
    m_titleLayout = new QHBoxLayout(m_titleBar);
    m_titleLayout->setContentsMargins(0, 0, 0, 0);
    m_titleLayout->setSpacing(12);

    m_backBtn = new QPushButton(QStringLiteral("\u2190 Back"), m_titleBar);   // ←
    m_backBtn->setFlat(true);

    m_titleLabel = new QLabel(tr("(no entity loaded)"), m_titleBar);
    m_titleLabel->setObjectName("detailTitle");

    m_overallRing = new CircularProgressBar(m_titleBar);
    m_overallRing->setFixedSize(64, 64);
    m_overallRing->setLineWidth(8);

    m_addUnitBtn    = new QPushButton(tr("+ Unit"), m_titleBar);
    m_addSessionBtn = new QPushButton(
        m_type == EntityCard::EntityType::Course ? tr("+ Session") : tr("+ Task"),
        m_titleBar);
    m_deleteBtn     = new QPushButton(tr("Delete"), m_titleBar);
    m_deleteBtn->setObjectName("dangerButton");

    m_titleLayout->addWidget(m_backBtn);
    m_titleLayout->addWidget(m_titleLabel, 1);
    m_titleLayout->addWidget(m_overallRing);
    m_titleLayout->addWidget(m_addUnitBtn);
    m_titleLayout->addWidget(m_addSessionBtn);
    m_titleLayout->addWidget(m_deleteBtn);

    m_outerLayout->addWidget(m_titleBar);

    // ---- Units scroll area ----
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);

    m_unitsContainer = new QWidget(m_scrollArea);
    m_unitsLayout = new QVBoxLayout(m_unitsContainer);
    m_unitsLayout->setContentsMargins(0, 0, 0, 0);
    m_unitsLayout->setSpacing(8);
    m_unitsLayout->setAlignment(Qt::AlignTop);

    m_scrollArea->setWidget(m_unitsContainer);
    m_outerLayout->addWidget(m_scrollArea, 1);

    connect(m_backBtn,       &QPushButton::clicked, this, &EntityDetailView::backRequested);
    connect(m_addUnitBtn,    &QPushButton::clicked, this, &EntityDetailView::onAddUnitClicked);
    connect(m_addSessionBtn, &QPushButton::clicked, this, &EntityDetailView::onAddSessionClicked);
    connect(m_deleteBtn,     &QPushButton::clicked, this, &EntityDetailView::onDeleteEntityClicked);
}

void EntityDetailView::loadEntity(int entityId) {
    m_entityId = entityId;

    auto* db = DatabaseManager::instance();
    const QList<EntityData> all = db->fetchAllEntities();
    m_entityName.clear();
    for (const EntityData& e : all) {
        if (e.id == entityId) {
            m_entityName = e.name;
            break;
        }
    }
    m_titleLabel->setText(m_entityName.isEmpty() ? tr("(unknown)") : m_entityName);

    rebuildUnits();
    refreshOverall();
}

void EntityDetailView::clearUnits() {
    for (UnitExpandableWidget* u : m_unitWidgets) {
        m_unitsLayout->removeWidget(u);
        u->deleteLater();
    }
    m_unitWidgets.clear();
}

void EntityDetailView::rebuildUnits() {
    clearUnits();
    if (m_entityId < 0) {
        return;
    }
    auto* db = DatabaseManager::instance();
    const QList<UnitData> units = db->getUnitsForParent(m_entityId);
    for (const UnitData& u : units) {
        auto* widget = new UnitExpandableWidget(u.id, u.name, m_unitsContainer);
        const QList<SessionTaskData> sessions = db->getSessionTasksForUnit(u.id);
        for (const SessionTaskData& s : sessions) {
            widget->addSessionTask(s.id, s.name, s.progress);
        }
        connect(widget, &UnitExpandableWidget::sessionTaskProgressChanged,
                this, &EntityDetailView::onSessionProgressChanged);
        connect(widget, &UnitExpandableWidget::sessionTaskRenamed,
                this, &EntityDetailView::onSessionRenamed);

        m_unitsLayout->addWidget(widget);
        m_unitWidgets.append(widget);
    }
}

void EntityDetailView::refreshOverall() {
    if (m_entityId < 0) {
        m_overallRing->setProgress(0);
        return;
    }
    auto* db = DatabaseManager::instance();
    const QList<UnitData> units = db->getUnitsForParent(m_entityId);
    int sum = 0;
    int count = 0;
    for (const UnitData& u : units) {
        const QList<SessionTaskData> sessions = db->getSessionTasksForUnit(u.id);
        for (const SessionTaskData& s : sessions) {
            sum += s.progress;
            ++count;
        }
    }
    m_overallRing->setProgress(count == 0 ? 0 : sum / count);
}

void EntityDetailView::onAddUnitClicked() {
    if (m_entityId < 0) {
        return;
    }
    bool ok = false;
    const QString name = QInputDialog::getText(
        this, tr("New Unit"), tr("Unit name:"), QLineEdit::Normal, QString(), &ok);
    if (!ok) {
        return;
    }
    const QString trimmed = name.trimmed();
    if (trimmed.isEmpty()) {
        return;
    }
    
    // Add unit with error checking
    auto* db = DatabaseManager::instance();
    if (!db) {
        QMessageBox::critical(this, tr("Error"), tr("Database not available"));
        return;
    }
    
    const int unitId = db->addUnit(m_entityId, trimmed);
    if (unitId < 0) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to add unit"));
        return;
    }
    
    // dataChanged() signal triggers onDataChanged → rebuild.
}

void EntityDetailView::onAddSessionClicked() {
    if (m_unitWidgets.isEmpty()) {
        QMessageBox::information(this, tr("No units"),
            tr("Add a unit first before adding %1.")
                .arg(m_type == EntityCard::EntityType::Course ? tr("sessions") : tr("tasks")));
        return;
    }

    QStringList unitNames;
    unitNames.reserve(m_unitWidgets.size());
    for (UnitExpandableWidget* u : m_unitWidgets) {
        unitNames << u->name();
    }

    bool ok = false;
    const QString chosen = QInputDialog::getItem(
        this, tr("Choose unit"),
        m_type == EntityCard::EntityType::Course ? tr("Add session to:") : tr("Add task to:"),
        unitNames, 0, false, &ok);
    if (!ok || chosen.isEmpty()) {
        return;
    }

    int unitId = -1;
    for (UnitExpandableWidget* u : m_unitWidgets) {
        if (u->name() == chosen) {
            unitId = u->unitId();
            break;
        }
    }
    if (unitId < 0) {
        return;
    }

    const QString name = QInputDialog::getText(
        this,
        m_type == EntityCard::EntityType::Course ? tr("New Session") : tr("New Task"),
        tr("Name:"),
        QLineEdit::Normal, QString(), &ok);
    if (!ok) {
        return;
    }
    const QString trimmed = name.trimmed();
    if (trimmed.isEmpty()) {
        return;
    }
    DatabaseManager::instance()->addSessionTask(unitId, trimmed, 0);
}

void EntityDetailView::onDeleteEntityClicked() {
    if (m_entityId < 0) {
        return;
    }
    const QString kind = (m_type == EntityCard::EntityType::Course) ? tr("course") : tr("project");
    const auto choice = QMessageBox::warning(
        this, tr("Delete %1").arg(kind),
        tr("Delete \"%1\"?\nAll units and %2 will be removed.\nThis cannot be undone.")
            .arg(m_entityName,
                 m_type == EntityCard::EntityType::Course ? tr("sessions") : tr("tasks")),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    if (choice != QMessageBox::Yes) {
        return;
    }

    auto* db = DatabaseManager::instance();
    const int removed = m_entityId;
    const bool ok = (m_type == EntityCard::EntityType::Course)
        ? db->removeCourse(removed)
        : db->removeProject(removed);

    if (!ok) {
        QMessageBox::critical(this, tr("Delete failed"),
            tr("Could not delete this %1.").arg(kind));
        return;
    }
    m_entityId = -1;
    m_entityName.clear();
    m_titleLabel->setText(tr("(no entity loaded)"));
    clearUnits();
    m_overallRing->setProgress(0);
    emit entityRemoved(removed);
    emit backRequested();
}

void EntityDetailView::onSessionProgressChanged(int sessionId, int oldValue, int newValue) {
    Q_UNUSED(oldValue);
    DatabaseManager::instance()->updateSessionTaskProgress(sessionId, newValue);
    // DatabaseManager handles activity logging + dataChanged signal.
}

void EntityDetailView::onSessionRenamed(int sessionId, const QString& newName) {
    DatabaseManager::instance()->renameSessionTask(sessionId, newName);
}

void EntityDetailView::onDataChanged() {
    if (m_entityId < 0) {
        return;
    }
    // Confirm the entity still exists; if it was deleted elsewhere, clear.
    auto* db = DatabaseManager::instance();
    const QList<EntityData> all = db->fetchAllEntities();
    bool stillExists = false;
    for (const EntityData& e : all) {
        if (e.id == m_entityId) {
            stillExists = true;
            m_entityName = e.name;
            m_titleLabel->setText(m_entityName);
            break;
        }
    }
    if (!stillExists) {
        m_entityId = -1;
        clearUnits();
        m_overallRing->setProgress(0);
        m_titleLabel->setText(tr("(no entity loaded)"));
        return;
    }
    rebuildUnits();
    refreshOverall();
}
