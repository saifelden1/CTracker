#include "shared/EntityCreateDialog.h"

#include "core/DatabaseManager.h"
#include "core/DataStructures.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QDateEdit>
#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>

// Task 7.10: EntityCreateDialog — modal dialog for creating Course or Project.

EntityCreateDialog::EntityCreateDialog(Mode mode, QWidget* parent)
    : QDialog(parent),
      m_mode(mode) {
    setObjectName("entityCreateDialog");
    setWindowTitle(mode == Mode::ProjectOnly ? tr("Create Project") : tr("Create Course / Project"));
    setMinimumWidth(400);

    setupUi();
}

void EntityCreateDialog::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    // ── Type selector (only in CourseOrProject mode) ──
    if (m_mode == Mode::CourseOrProject) {
        auto* typeRow = new QHBoxLayout();
        typeRow->setSpacing(8);
        auto* typeLabel = new QLabel(tr("Type:"), this);
        m_typeCombo = new QComboBox(this);
        m_typeCombo->addItem(tr("Course"), "course");
        m_typeCombo->addItem(tr("Project"), "project");
        m_typeCombo->setCurrentIndex(0);
        m_entityType = "course";
        connect(m_typeCombo, &QComboBox::currentIndexChanged,
                this, &EntityCreateDialog::onTypeChanged);
        typeRow->addWidget(typeLabel);
        typeRow->addWidget(m_typeCombo, 1);
        mainLayout->addLayout(typeRow);
    } else {
        m_entityType = "project";
    }

    // ── Name field ──
    auto* nameRow = new QHBoxLayout();
    nameRow->setSpacing(8);
    auto* nameLabel = new QLabel(tr("Name:"), this);
    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setObjectName("entityNameEdit");
    m_nameEdit->setPlaceholderText(tr("Enter name..."));
    connect(m_nameEdit, &QLineEdit::textChanged,
            this, &EntityCreateDialog::onNameChanged);
    nameRow->addWidget(nameLabel);
    nameRow->addWidget(m_nameEdit, 1);
    mainLayout->addLayout(nameRow);

    // ── Project-specific fields ──
    m_descLabel = new QLabel(tr("Description:"), this);
    m_descEdit = new QTextEdit(this);
    m_descEdit->setObjectName("entityDescEdit");
    m_descEdit->setMaximumHeight(80);
    m_descEdit->setPlaceholderText(tr("Optional description..."));
    mainLayout->addWidget(m_descLabel);
    mainLayout->addWidget(m_descEdit);

    m_priorityLabel = new QLabel(tr("Priority:"), this);
    m_priorityCombo = new QComboBox(this);
    m_priorityCombo->setObjectName("entityPriorityCombo");
    m_priorityCombo->addItem(tr("Low"), "low");
    m_priorityCombo->addItem(tr("Medium"), "medium");
    m_priorityCombo->addItem(tr("High"), "high");
    m_priorityCombo->setCurrentIndex(1);  // default medium
    auto* priorityRow = new QHBoxLayout();
    priorityRow->setSpacing(8);
    priorityRow->addWidget(m_priorityLabel);
    priorityRow->addWidget(m_priorityCombo, 1);
    mainLayout->addLayout(priorityRow);

    m_deadlineLabel = new QLabel(tr("Deadline:"), this);
    m_deadlineEdit = new QDateEdit(this);
    m_deadlineEdit->setObjectName("entityDeadlineEdit");
    m_deadlineEdit->setCalendarPopup(true);
    m_deadlineEdit->setDate(QDate::currentDate().addDays(30));
    m_deadlineEdit->setMinimumDate(QDate::currentDate());
    m_deadlineEdit->setDisplayFormat("yyyy-MM-dd");
    auto* deadlineRow = new QHBoxLayout();
    deadlineRow->setSpacing(8);
    deadlineRow->addWidget(m_deadlineLabel);
    deadlineRow->addWidget(m_deadlineEdit, 1);
    mainLayout->addLayout(deadlineRow);

    // ── Buttons ──
    auto* buttonRow = new QHBoxLayout();
    buttonRow->setSpacing(8);
    buttonRow->addStretch(1);

    m_cancelBtn = new QPushButton(tr("Cancel"), this);
    m_cancelBtn->setObjectName("entityCreateCancelBtn");
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    buttonRow->addWidget(m_cancelBtn);

    m_okBtn = new QPushButton(tr("Create"), this);
    m_okBtn->setObjectName("entityCreateOkBtn");
    m_okBtn->setEnabled(false);  // disabled until name is non-empty
    m_okBtn->setCursor(Qt::PointingHandCursor);
    connect(m_okBtn, &QPushButton::clicked, this, &EntityCreateDialog::onAccept);
    buttonRow->addWidget(m_okBtn);

    mainLayout->addLayout(buttonRow);

    // Initial visibility
    updateProjectFieldsVisibility();
}

void EntityCreateDialog::onNameChanged(const QString& text) {
    const bool valid = !text.trimmed().isEmpty();
    m_okBtn->setEnabled(valid);
}

void EntityCreateDialog::onTypeChanged(int index) {
    m_entityType = m_typeCombo->itemData(index).toString();
    updateProjectFieldsVisibility();
}

void EntityCreateDialog::updateProjectFieldsVisibility() {
    const bool isProject = (m_entityType == "project");
    m_descLabel->setVisible(isProject);
    m_descEdit->setVisible(isProject);
    m_priorityLabel->setVisible(isProject);
    m_priorityCombo->setVisible(isProject);
    m_deadlineLabel->setVisible(isProject);
    m_deadlineEdit->setVisible(isProject);
}

void EntityCreateDialog::onAccept() {
    auto* db = DatabaseManager::instance();

    if (m_entityType == "course") {
        const int id = db->addCourse(entityName());
        if (id < 0) {
            // DB error — reject
            reject();
            return;
        }
        m_createdId = id;
    } else {
        const int id = db->addProject(entityName());
        if (id < 0) {
            reject();
            return;
        }
        m_createdId = id;
        // Upsert project metadata
        ProjectMetaData meta;
        meta.projectId   = id;
        meta.description = description();
        meta.priority    = priority();
        meta.deadline    = deadline();
        db->upsertProjectMeta(meta);
    }

    accept();
}

QString EntityCreateDialog::entityType() const      { return m_entityType; }
QString EntityCreateDialog::entityName() const       { return m_nameEdit->text().trimmed(); }
int     EntityCreateDialog::createdEntityId() const  { return m_createdId; }
QString EntityCreateDialog::description() const      { return m_descEdit->toPlainText().trimmed(); }
QString EntityCreateDialog::priority() const         { return m_priorityCombo->currentData().toString(); }
QDate   EntityCreateDialog::deadline() const         { return m_deadlineEdit->date(); }