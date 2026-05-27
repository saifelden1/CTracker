#include "projects/TaskDetailDialog.h"

#include "core/DatabaseManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QSqlQuery>

// ============================================================
//  TaskDetailDialog — implementation
// ============================================================

TaskDetailDialog::TaskDetailDialog(int taskId, QWidget* parent)
    : QDialog(parent),
      m_taskId(taskId) {
    setObjectName("taskDetailDialog");
    setWindowTitle(tr("Task details"));
    setModal(true);
    setMinimumWidth(420);

    // Stylesheet matches the rest of the dark-industrial theme.
    setStyleSheet(R"(
        QDialog#taskDetailDialog {
            background: #1f2229;
            color: #e4e6eb;
        }
        QLabel { color: #e4e6eb; }
        QLineEdit, QPlainTextEdit, QComboBox, QDateEdit {
            background: #252932;
            border: 1px solid #2d323d;
            border-radius: 6px;
            padding: 6px 8px;
            color: #e4e6eb;
        }
        QLineEdit:focus, QPlainTextEdit:focus,
        QComboBox:focus, QDateEdit:focus {
            border-color: #10b981;
        }
        QPushButton {
            background: #252932;
            border: 1px solid #2d323d;
            border-radius: 6px;
            padding: 6px 14px;
            color: #e4e6eb;
        }
        QPushButton:hover { background: #2d323d; }
        QPushButton[default="true"] {
            background: #10b981; border-color: #10b981; color: #ffffff;
        }
        QPushButton[default="true"]:hover { background: #0ea271; }
    )");

    buildUi();
    loadFromDb();
}

void TaskDetailDialog::buildUi() {
    auto* outer = new QVBoxLayout(this);
    outer->setSpacing(12);

    auto* form = new QFormLayout();
    form->setSpacing(8);
    form->setLabelAlignment(Qt::AlignLeft);

    // Title
    m_titleEdit = new QLineEdit(this);
    m_titleEdit->setPlaceholderText(tr("Task title"));
    form->addRow(tr("Title"), m_titleEdit);

    // Unit (read-only — unit reassignment is not a v1 feature)
    m_unitLabel = new QLabel(this);
    m_unitLabel->setStyleSheet("QLabel { color: #9ca3af; }");
    form->addRow(tr("Unit"), m_unitLabel);

    // Status
    m_statusCombo = new QComboBox(this);
    m_statusCombo->addItem(tr("Todo"),        "todo");
    m_statusCombo->addItem(tr("In progress"), "in_progress");
    m_statusCombo->addItem(tr("Review"),      "review");
    m_statusCombo->addItem(tr("Done"),        "done");
    form->addRow(tr("Status"), m_statusCombo);

    // Due date: checkbox toggles between "no date" and a QDateEdit
    auto* dueRow = new QHBoxLayout();
    dueRow->setSpacing(8);
    m_dueEnabled = new QCheckBox(tr("Set due date"), this);
    m_dueEnabled->setStyleSheet("QCheckBox { color: #e4e6eb; }");
    m_dueDate = new QDateEdit(this);
    m_dueDate->setCalendarPopup(true);
    m_dueDate->setDate(QDate::currentDate());
    m_dueDate->setEnabled(false);
    connect(m_dueEnabled, &QCheckBox::toggled, m_dueDate, &QDateEdit::setEnabled);
    dueRow->addWidget(m_dueEnabled);
    dueRow->addWidget(m_dueDate, 1);
    auto* dueHost = new QWidget(this);
    dueHost->setLayout(dueRow);
    form->addRow(tr("Due date"), dueHost);

    outer->addLayout(form);

    // Description — separate from form so it gets its own label row + stretch
    auto* descLabel = new QLabel(tr("Description"), this);
    outer->addWidget(descLabel);
    m_descEdit = new QPlainTextEdit(this);
    m_descEdit->setPlaceholderText(tr("Notes, links, acceptance criteria…"));
    m_descEdit->setMinimumHeight(140);
    outer->addWidget(m_descEdit, 1);

    // Buttons
    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    QPushButton* saveBtn = buttons->button(QDialogButtonBox::Save);
    if (saveBtn) saveBtn->setProperty("default", true);
    connect(buttons, &QDialogButtonBox::accepted, this, &TaskDetailDialog::onAccept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    outer->addWidget(buttons);
}

void TaskDetailDialog::loadFromDb() {
    auto* db = DatabaseManager::instance();
    m_original = db->getSessionTask(m_taskId);
    if (m_original.id < 0) {
        QMessageBox::warning(this, tr("Task not found"),
                             tr("This task no longer exists."));
        QDialog::reject();
        return;
    }

    m_titleEdit->setText(m_original.name);

    // Resolve unit name — getSessionTask doesn't JOIN, so query the
    // single Units row directly via the existing database accessor.
    QString unitName;
    {
        QSqlQuery q(db->database());
        q.prepare("SELECT Name FROM Units WHERE ID = :uid");
        q.bindValue(":uid", m_original.unitId);
        if (q.exec() && q.next()) {
            unitName = q.value(0).toString();
        }
    }
    m_unitLabel->setText(unitName.isEmpty() ? tr("(unknown)") : unitName);

    int statusIdx = m_statusCombo->findData(m_original.status);
    m_statusCombo->setCurrentIndex(statusIdx < 0 ? 0 : statusIdx);

    if (m_original.dueDate.isValid()) {
        m_dueEnabled->setChecked(true);
        m_dueDate->setEnabled(true);
        m_dueDate->setDate(m_original.dueDate);
    } else {
        m_dueEnabled->setChecked(false);
        m_dueDate->setEnabled(false);
    }

    m_descEdit->setPlainText(m_original.description);
}

void TaskDetailDialog::onAccept() {
    const QString newTitle  = m_titleEdit->text().trimmed();
    if (newTitle.isEmpty()) {
        QMessageBox::warning(this, tr("Title required"),
                             tr("Task title cannot be empty."));
        m_titleEdit->setFocus();
        return;
    }
    const QString newStatus = m_statusCombo->currentData().toString();
    const QDate   newDue    = m_dueEnabled->isChecked() ? m_dueDate->date() : QDate();
    const QString newDesc   = m_descEdit->toPlainText();

    auto* db = DatabaseManager::instance();

    // Delta writes — touch DB only where values actually changed so we
    // don't spam dataChanged() / ActivityLog with no-ops.
    if (newTitle != m_original.name) {
        db->renameSessionTask(m_taskId, newTitle);
    }
    if (newStatus != m_original.status) {
        db->setSessionTaskStatus(m_taskId, newStatus);
    }
    const QDate oldDue = m_original.dueDate;
    const bool dueChanged =
        (newDue.isValid() != oldDue.isValid()) ||
        (newDue.isValid() && oldDue.isValid() && newDue != oldDue);
    if (dueChanged) {
        db->setSessionTaskDueDate(m_taskId, newDue);
    }
    if (newDesc != m_original.description) {
        db->setSessionTaskDescription(m_taskId, newDesc);
    }

    QDialog::accept();
}

void TaskDetailDialog::onClearDueDate() {
    m_dueEnabled->setChecked(false);
    m_dueDate->setEnabled(false);
}
