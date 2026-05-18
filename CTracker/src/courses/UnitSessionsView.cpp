#include "courses/UnitSessionsView.h"

#include "courses/SessionTaskRow.h"
#include "shared/CircularProgressBar.h"
#include "core/DatabaseManager.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>

// ============================================================
//  UnitSessionsView.cpp  (Task 7.5a)
//
//  The "Page 1" content of CourseDetailView's inner QStackedWidget.
//  When a UnitCard is clicked, the parent calls loadUnit(id) on us
//  and swaps the stack's current widget to this view.
//
//  We re-emit session-progress and session-rename signals so the
//  parent (CourseDetailView) does the DB write — that keeps the
//  hot-path code in exactly one place (matches the pattern
//  EntityDetailView already uses with UnitExpandableWidget).
// ============================================================

UnitSessionsView::UnitSessionsView(QWidget* parent)
    : QWidget(parent) {
    setupUi();
}

void UnitSessionsView::setupUi() {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(8);

    // ── Sub-header ───────────────────────────────────────────
    auto* header = new QWidget(this);
    header->setObjectName("unitSessionsHeader");
    auto* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(12);

    m_backBtn = new QPushButton(QStringLiteral("↩ Units"), header);   // ↩
    m_backBtn->setFlat(true);
    m_backBtn->setObjectName("unitSessionsBackBtn");

    m_titleLabel = new QLabel(tr("(no unit loaded)"), header);
    m_titleLabel->setObjectName("unitSessionsTitle");
    m_titleLabel->setStyleSheet(QStringLiteral(
        "QLabel#unitSessionsTitle {"
        "  color: #e4e6eb;"
        "  font-size: 18px;"
        "  font-weight: 600;"
        "}"
    ));

    m_ring = new CircularProgressBar(header);
    m_ring->setFixedSize(48, 48);
    m_ring->setLineWidth(6);
    m_ring->setProgress(0);

    m_addSessionBtn = new QPushButton(tr("+ Session"), header);
    m_renameUnitBtn = new QPushButton(tr("Rename"),    header);
    m_deleteUnitBtn = new QPushButton(tr("Delete"),    header);
    m_deleteUnitBtn->setObjectName("dangerButton");

    headerLayout->addWidget(m_backBtn);
    headerLayout->addWidget(m_titleLabel, 1);
    headerLayout->addWidget(m_ring);
    headerLayout->addWidget(m_addSessionBtn);
    headerLayout->addWidget(m_renameUnitBtn);
    headerLayout->addWidget(m_deleteUnitBtn);

    outer->addWidget(header);

    // Thin separator between header and rows
    auto* sep = new QFrame(this);
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet(QStringLiteral("color: #2d323d;"));
    outer->addWidget(sep);

    // ── Sessions scroll area ────────────────────────────────
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);

    m_rowsContainer = new QWidget(m_scrollArea);
    m_rowsLayout = new QVBoxLayout(m_rowsContainer);
    m_rowsLayout->setContentsMargins(0, 4, 0, 4);
    m_rowsLayout->setSpacing(4);
    m_rowsLayout->setAlignment(Qt::AlignTop);

    m_scrollArea->setWidget(m_rowsContainer);
    outer->addWidget(m_scrollArea, 1);

    // ── Wiring ──────────────────────────────────────────────
    connect(m_backBtn,        &QPushButton::clicked, this, &UnitSessionsView::backRequested);
    connect(m_addSessionBtn,  &QPushButton::clicked, this, &UnitSessionsView::onAddSessionClicked);
    connect(m_renameUnitBtn,  &QPushButton::clicked, this, &UnitSessionsView::onRenameUnitClicked);
    connect(m_deleteUnitBtn,  &QPushButton::clicked, this, &UnitSessionsView::onDeleteUnitClicked);
}

void UnitSessionsView::loadUnit(int unitId) {
    m_unitId = unitId;

    QString name;
    if (!fetchUnitName(&name)) {
        // Unit doesn't exist (anymore) — bail back to the grid.
        m_unitId   = -1;
        m_unitName.clear();
        m_titleLabel->setText(tr("(no unit loaded)"));
        clearRows();
        m_ring->setProgress(0);
        emit backRequested();
        return;
    }
    m_unitName = name;
    m_titleLabel->setText(m_unitName);
    rebuildRows();
    refreshRing();
}

void UnitSessionsView::refresh() {
    if (m_unitId < 0) {
        return;
    }
    QString name;
    if (!fetchUnitName(&name)) {
        // Externally removed (cascade or direct).
        const int removed = m_unitId;
        m_unitId = -1;
        m_unitName.clear();
        clearRows();
        m_ring->setProgress(0);
        m_titleLabel->setText(tr("(no unit loaded)"));
        emit unitRemoved(removed);
        emit backRequested();
        return;
    }
    if (name != m_unitName) {
        m_unitName = name;
        m_titleLabel->setText(name);
    }
    rebuildRows();
    refreshRing();
}

bool UnitSessionsView::fetchUnitName(QString* outName) const {
    if (!outName || m_unitId < 0) {
        return false;
    }
    auto* db = DatabaseManager::instance();
    // Walk all parents to find the unit. There's no direct
    // getUnitById() — getUnitsForParent is what's available, so we
    // need the parent id first. Cheaper: just scan all entities.
    // This is fine; CTracker has a handful of courses per user.
    const QList<EntityData> all = db->fetchAllEntities();
    for (const EntityData& e : all) {
        const QList<UnitData> units = db->getUnitsForParent(e.id);
        for (const UnitData& u : units) {
            if (u.id == m_unitId) {
                *outName = u.name;
                return true;
            }
        }
    }
    return false;
}

void UnitSessionsView::clearRows() {
    for (auto it = m_rows.begin(); it != m_rows.end(); ++it) {
        SessionTaskRow* row = it.value();
        if (row) {
            disconnect(row, nullptr, this, nullptr);
            m_rowsLayout->removeWidget(row);
            delete row;
        }
    }
    m_rows.clear();
}

void UnitSessionsView::rebuildRows() {
    clearRows();
    if (m_unitId < 0) {
        return;
    }
    auto* db = DatabaseManager::instance();
    const QList<SessionTaskData> sessions = db->getSessionTasksForUnit(m_unitId);
    for (const SessionTaskData& s : sessions) {
        auto* row = new SessionTaskRow(s.id, s.name, s.progress, m_rowsContainer);
        m_rowsLayout->addWidget(row);
        m_rows.insert(s.id, row);

        connect(row, &SessionTaskRow::progressChanged,
                this, &UnitSessionsView::onChildProgressChanged);
        connect(row, &SessionTaskRow::nameChanged,
                this, &UnitSessionsView::onChildNameChanged);
    }
}

void UnitSessionsView::refreshRing() {
    if (m_unitId < 0 || m_rows.isEmpty()) {
        m_ring->setProgress(0);
        return;
    }
    int sum = 0;
    for (auto* r : m_rows) {
        sum += r->progress();
    }
    m_ring->setProgress(sum / m_rows.size());
}

void UnitSessionsView::onAddSessionClicked() {
    if (m_unitId < 0) {
        return;
    }
    bool ok = false;
    const QString name = QInputDialog::getText(
        this, tr("New Session"), tr("Session name:"),
        QLineEdit::Normal, QString(), &ok);
    if (!ok) {
        return;
    }
    const QString trimmed = name.trimmed();
    if (trimmed.isEmpty()) {
        return;
    }
    auto* db = DatabaseManager::instance();
    const int newId = db->addSessionTask(m_unitId, trimmed, 0);
    if (newId < 0) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to add session"));
        return;
    }
    // dataChanged → parent rebuilds units grid; we rebuild our rows.
    emit sessionAdded(m_unitId);
}

void UnitSessionsView::onRenameUnitClicked() {
    if (m_unitId < 0) {
        return;
    }
    bool ok = false;
    const QString name = QInputDialog::getText(
        this, tr("Rename Unit"), tr("Unit name:"),
        QLineEdit::Normal, m_unitName, &ok);
    if (!ok) {
        return;
    }
    const QString trimmed = name.trimmed();
    if (trimmed.isEmpty() || trimmed == m_unitName) {
        return;
    }
    auto* db = DatabaseManager::instance();
    if (!db->renameUnit(m_unitId, trimmed)) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to rename unit"));
        return;
    }
    m_unitName = trimmed;
    m_titleLabel->setText(trimmed);
    emit unitRenamed(m_unitId, trimmed);
}

void UnitSessionsView::onDeleteUnitClicked() {
    if (m_unitId < 0) {
        return;
    }
    const auto choice = QMessageBox::warning(
        this, tr("Delete unit"),
        tr("Delete unit \"%1\"?\nAll sessions inside will be removed.\n"
           "This cannot be undone.").arg(m_unitName),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    if (choice != QMessageBox::Yes) {
        return;
    }
    auto* db = DatabaseManager::instance();
    const int removed = m_unitId;
    if (!db->removeUnit(removed)) {
        QMessageBox::critical(this, tr("Delete failed"),
                              tr("Could not delete this unit."));
        return;
    }
    m_unitId = -1;
    m_unitName.clear();
    m_titleLabel->setText(tr("(no unit loaded)"));
    clearRows();
    m_ring->setProgress(0);
    emit unitRemoved(removed);
    emit backRequested();
}

void UnitSessionsView::onChildProgressChanged(int oldValue, int newValue) {
    auto* row = qobject_cast<SessionTaskRow*>(sender());
    if (!row) {
        return;
    }
    refreshRing();
    emit sessionProgressChanged(row->sessionId(), oldValue, newValue);
}

void UnitSessionsView::onChildNameChanged(const QString& newName) {
    auto* row = qobject_cast<SessionTaskRow*>(sender());
    if (!row) {
        return;
    }
    emit sessionRenamed(row->sessionId(), newName);
}
