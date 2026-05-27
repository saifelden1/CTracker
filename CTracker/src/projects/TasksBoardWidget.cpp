#include "projects/TasksBoardWidget.h"

#include "projects/TaskBoardCard.h"
#include "projects/TaskDetailDialog.h"
#include "core/DatabaseManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QInputDialog>
#include <QMessageBox>
#include <QSet>
#include <QSignalBlocker>

// ============================================================
//  TasksBoardWidget — Phase 10
//
//  The widget mirrors design/projects-page-mockup.html. It is
//  intentionally read-mostly: it pulls fresh rows from the DB on
//  every refresh() rather than keeping a local mutable model.
//  That keeps state-sync trivial — DatabaseManager::dataChanged is
//  the single source of truth.
// ============================================================

namespace {

// Column headers + dot colors (matches the HTML mock).
struct ColumnSpec { const char* status; const char* label; const char* dot; };
const ColumnSpec kColumns[] = {
    { "todo",        "Todo",        "#6b7280" },
    { "in_progress", "In progress", "#3b82f6" },
    { "review",      "Review",      "#f59e0b" },
    { "done",        "Done",        "#10b981" },
};
constexpr int kNumColumns = sizeof(kColumns) / sizeof(kColumns[0]);

} // namespace

TasksBoardWidget::TasksBoardWidget(QWidget* parent)
    : QWidget(parent) {
    setObjectName("tasksBoardWidget");
    setupUi();

    connect(DatabaseManager::instance(), &DatabaseManager::dataChanged,
            this, &TasksBoardWidget::refresh);
}

void TasksBoardWidget::setupUi() {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(12);

    // ── Toolbar: search + unit filter + sort + add ──
    auto* toolbar = new QHBoxLayout();
    toolbar->setSpacing(8);

    m_searchInput = new QLineEdit(this);
    m_searchInput->setPlaceholderText(tr("Search tasks…"));
    m_searchInput->setClearButtonEnabled(true);
    m_searchInput->setStyleSheet(
        "QLineEdit {"
        "  background: #1f2229; border: 1px solid #2d323d;"
        "  border-radius: 6px; padding: 6px 10px; color: #e4e6eb;"
        "}");

    m_unitFilter = new QComboBox(this);
    m_unitFilter->setStyleSheet(
        "QComboBox {"
        "  background: #1f2229; border: 1px solid #2d323d;"
        "  border-radius: 6px; padding: 6px 10px; color: #e4e6eb;"
        "}");

    m_sortCombo = new QComboBox(this);
    m_sortCombo->addItem(tr("Sort: Due date"));
    m_sortCombo->addItem(tr("Sort: Name"));
    m_sortCombo->addItem(tr("Sort: Newest first"));
    m_sortCombo->setStyleSheet(m_unitFilter->styleSheet());

    m_addTaskBtn = new QPushButton(tr("+ Task"), this);
    m_addTaskBtn->setObjectName("boardAddTaskBtn");
    m_addTaskBtn->setCursor(Qt::PointingHandCursor);
    m_addTaskBtn->setStyleSheet(
        "QPushButton#boardAddTaskBtn {"
        "  background: #10b981; color: #ffffff;"
        "  border: 1px solid #10b981; border-radius: 6px;"
        "  padding: 6px 14px; font-weight: 500;"
        "}"
        "QPushButton#boardAddTaskBtn:hover { background: #0ea271; }");

    toolbar->addWidget(m_searchInput, 1);
    toolbar->addWidget(m_unitFilter);
    toolbar->addWidget(m_sortCombo);
    toolbar->addWidget(m_addTaskBtn);
    outer->addLayout(toolbar);

    // ── 4-column board ──
    auto* board = new QWidget(this);
    auto* boardLayout = new QHBoxLayout(board);
    boardLayout->setContentsMargins(0, 0, 0, 0);
    boardLayout->setSpacing(12);

    for (int i = 0; i < kNumColumns; ++i) {
        const ColumnSpec& spec = kColumns[i];

        auto* column = new QFrame(board);
        column->setObjectName("kanbanColumn");
        column->setStyleSheet(
            "QFrame#kanbanColumn {"
            "  background: #1f2229; border: 1px solid #2d323d;"
            "  border-radius: 10px;"
            "}");
        auto* colLayout = new QVBoxLayout(column);
        colLayout->setContentsMargins(0, 0, 0, 0);
        colLayout->setSpacing(0);

        // Header
        auto* header = new QWidget(column);
        auto* headerLayout = new QHBoxLayout(header);
        headerLayout->setContentsMargins(10, 10, 10, 10);
        headerLayout->setSpacing(8);

        auto* dot = new QLabel(header);
        dot->setFixedSize(8, 8);
        dot->setStyleSheet(QStringLiteral(
            "QLabel { background: %1; border-radius: 4px; }").arg(spec.dot));

        auto* title = new QLabel(tr(spec.label), header);
        title->setStyleSheet("QLabel { color: #e4e6eb; font-weight: 600; font-size: 13px; }");

        auto* countLabel = new QLabel("0", header);
        countLabel->setStyleSheet(
            "QLabel {"
            "  background: #252932; color: #9ca3af;"
            "  padding: 1px 8px; border-radius: 10px;"
            "  font-size: 11px; font-weight: 500;"
            "}");

        headerLayout->addWidget(dot);
        headerLayout->addWidget(title);
        headerLayout->addStretch();
        headerLayout->addWidget(countLabel);
        colLayout->addWidget(header);

        // Body: scrollable list of cards
        auto* scroll = new QScrollArea(column);
        scroll->setObjectName("kanbanScroll");
        scroll->setWidgetResizable(true);
        scroll->setFrameShape(QFrame::NoFrame);
        scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scroll->setStyleSheet(
            "QScrollArea#kanbanScroll { background: transparent; border: none; }"
            "QScrollArea#kanbanScroll > QWidget > QWidget { background: transparent; }");

        auto* body = new QWidget(scroll);
        auto* bodyLayout = new QVBoxLayout(body);
        bodyLayout->setContentsMargins(10, 0, 10, 10);
        bodyLayout->setSpacing(8);
        bodyLayout->addStretch();   // keeps cards flush to the top

        scroll->setWidget(body);
        colLayout->addWidget(scroll, 1);

        boardLayout->addWidget(column, 1);

        Column c;
        c.status     = QString::fromLatin1(spec.status);
        c.header     = header;
        c.countLabel = countLabel;
        c.body       = body;
        c.bodyLayout = bodyLayout;
        m_columns.append(c);
    }

    outer->addWidget(board, 1);

    // ── Signals ──
    connect(m_searchInput, &QLineEdit::textChanged,
            this, &TasksBoardWidget::onSearchChanged);
    connect(m_unitFilter, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &TasksBoardWidget::onUnitFilterChanged);
    connect(m_sortCombo, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &TasksBoardWidget::onSortChanged);
    connect(m_addTaskBtn, &QPushButton::clicked,
            this, &TasksBoardWidget::onAddTaskClicked);
}

void TasksBoardWidget::setProject(int projectId) {
    m_projectId = projectId;

    if (m_projectId >= 0) {
        m_projectPriority = DatabaseManager::instance()
                                ->getProjectMeta(m_projectId).priority;
        if (m_projectPriority.isEmpty()) m_projectPriority = "medium";
    } else {
        m_projectPriority = "medium";
    }

    refresh();
}

void TasksBoardWidget::refresh() {
    if (m_projectId < 0) {
        m_tasks.clear();
        rebuild();
        return;
    }

    auto* db = DatabaseManager::instance();
    // Re-fetch project priority — user might have changed it from another view.
    m_projectPriority = db->getProjectMeta(m_projectId).priority;
    if (m_projectPriority.isEmpty()) m_projectPriority = "medium";

    m_tasks = db->fetchTasksForProject(m_projectId);

    // Refresh unit filter combo (preserve current selection where possible).
    QSet<int> unitIds;
    QList<QPair<int, QString>> units;   // unique, order by first appearance
    for (const SessionTaskData& t : m_tasks) {
        if (!unitIds.contains(t.unitId)) {
            unitIds.insert(t.unitId);
            units.append({t.unitId, t.unitName});
        }
    }
    {
        QSignalBlocker block(m_unitFilter);
        m_unitFilter->clear();
        m_unitFilter->addItem(tr("Unit: All"), -1);
        for (const auto& u : units) {
            m_unitFilter->addItem(u.second, u.first);
        }
        // Restore selection
        int idx = m_unitFilter->findData(m_unitFilterId);
        m_unitFilter->setCurrentIndex(idx < 0 ? 0 : idx);
        m_unitFilterId = m_unitFilter->currentData().toInt();
    }

    rebuild();
}

QList<SessionTaskData> TasksBoardWidget::currentlyVisible() const {
    QList<SessionTaskData> out;
    out.reserve(m_tasks.size());
    for (const SessionTaskData& t : m_tasks) {
        if (m_unitFilterId >= 0 && t.unitId != m_unitFilterId) continue;
        if (!m_search.isEmpty()
            && !t.name.contains(m_search, Qt::CaseInsensitive)) continue;
        out.append(t);
    }

    auto byName = [](const SessionTaskData& a, const SessionTaskData& b) {
        return a.name.localeAwareCompare(b.name) < 0;
    };
    auto byDue = [](const SessionTaskData& a, const SessionTaskData& b) {
        // Invalid dates sort to the end.
        const bool aValid = a.dueDate.isValid();
        const bool bValid = b.dueDate.isValid();
        if (aValid != bValid) return aValid;
        if (aValid && bValid) return a.dueDate < b.dueDate;
        return a.id < b.id;
    };
    auto byNewest = [](const SessionTaskData& a, const SessionTaskData& b) {
        return a.id > b.id;
    };

    switch (m_sortIndex) {
        case 1: std::sort(out.begin(), out.end(), byName);   break;
        case 2: std::sort(out.begin(), out.end(), byNewest); break;
        case 0:
        default: std::sort(out.begin(), out.end(), byDue);   break;
    }
    return out;
}

void TasksBoardWidget::clearColumns() {
    for (Column& col : m_columns) {
        // Drop every widget except the trailing stretch.
        // takeAt removes-from-layout but leaves the child widget owned;
        // deleteLater frees it after the event loop ticks.
        QLayoutItem* item;
        while ((item = col.bodyLayout->takeAt(0)) != nullptr) {
            if (item->widget()) item->widget()->deleteLater();
            delete item;
        }
        col.bodyLayout->addStretch();
        col.countLabel->setText("0");
    }
    m_cardsByTaskId.clear();
}

void TasksBoardWidget::rebuild() {
    clearColumns();

    const QList<SessionTaskData> visible = currentlyVisible();

    // Pre-count per-column so the badge updates even if a column is empty.
    QHash<QString, int> counts;
    for (const SessionTaskData& t : visible) counts[t.status]++;

    for (Column& col : m_columns) {
        col.countLabel->setText(QString::number(counts.value(col.status, 0)));
    }

    for (const SessionTaskData& t : visible) {
        // Find the matching column. Unknown statuses fall back to Todo
        // so a broken DB row never silently disappears.
        Column* target = nullptr;
        for (Column& col : m_columns) {
            if (col.status == t.status) { target = &col; break; }
        }
        if (!target) target = &m_columns[0];

        auto* card = new TaskBoardCard(t.id, target->body);
        card->setTitle   (t.name);
        card->setUnitName(t.unitName);
        card->setStatus  (t.status);
        card->setDueDate (t.dueDate);
        card->setPriority(m_projectPriority);
        card->setDescription(t.description);

        connect(card, &TaskBoardCard::statusChangeRequested,
                this, &TasksBoardWidget::onCardStatusChange);
        connect(card, &TaskBoardCard::deleteRequested,
                this, &TasksBoardWidget::onCardDeleteRequested);
        connect(card, &TaskBoardCard::clicked,
                this, [this](int taskId) {
                    TaskDetailDialog dlg(taskId, this);
                    dlg.exec();
                });

        // Polish: tooltip shows the description so users see it without
        // opening the dialog. Falls back to "No description".
        const QString tip = t.description.isEmpty()
                            ? tr("No description — click to add one.")
                            : t.description;
        card->setToolTip(tip);

        // Insert before the trailing stretch.
        const int insertAt = target->bodyLayout->count() - 1;
        target->bodyLayout->insertWidget(insertAt, card);
        m_cardsByTaskId.insert(t.id, card);
    }
}

void TasksBoardWidget::onAddTaskClicked() {
    if (m_projectId < 0) return;

    auto* db = DatabaseManager::instance();
    const QList<UnitData> units = db->getUnitsForParent(m_projectId);
    if (units.isEmpty()) {
        QMessageBox::information(
            this, tr("No units"),
            tr("Add a unit to this project first — every task belongs to a unit."));
        return;
    }

    // Pick the unit. Default to the one currently selected in the filter
    // (if any) so the user's intent carries through.
    QStringList unitNames;
    int defaultIdx = 0;
    for (int i = 0; i < units.size(); ++i) {
        unitNames << units[i].name;
        if (units[i].id == m_unitFilterId) defaultIdx = i;
    }
    bool ok = false;
    const QString chosenUnit = QInputDialog::getItem(
        this, tr("Add task"), tr("Unit:"), unitNames, defaultIdx, false, &ok);
    if (!ok) return;

    int chosenUnitId = -1;
    for (const UnitData& u : units) {
        if (u.name == chosenUnit) { chosenUnitId = u.id; break; }
    }
    if (chosenUnitId < 0) return;

    const QString name = QInputDialog::getText(
        this, tr("Add task"), tr("Task title:"),
        QLineEdit::Normal, QString(), &ok).trimmed();
    if (!ok || name.isEmpty()) return;

    // Default status='todo' is applied by the schema default.
    db->addSessionTask(chosenUnitId, name, 0);
    // refresh() will be triggered by DatabaseManager::dataChanged.
}

void TasksBoardWidget::onSearchChanged(const QString& text) {
    m_search = text.trimmed();
    rebuild();
}

void TasksBoardWidget::onUnitFilterChanged(int /*index*/) {
    m_unitFilterId = m_unitFilter->currentData().toInt();
    rebuild();
}

void TasksBoardWidget::onSortChanged(int index) {
    m_sortIndex = index;
    rebuild();
}

void TasksBoardWidget::onCardStatusChange(int taskId, const QString& newStatus) {
    DatabaseManager::instance()->setSessionTaskStatus(taskId, newStatus);
}

void TasksBoardWidget::onCardDeleteRequested(int taskId) {
    const auto reply = QMessageBox::question(
        this, tr("Delete task"),
        tr("Delete this task? This cannot be undone."),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (reply != QMessageBox::Yes) return;
    DatabaseManager::instance()->removeSessionTask(taskId);
}
