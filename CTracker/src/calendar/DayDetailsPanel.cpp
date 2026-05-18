#include "calendar/DayDetailsPanel.h"

#include "core/DataStructures.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QScrollArea>
#include <QCheckBox>

// Task 6.9: DayDetailsPanel — shows details for a selected calendar day.
// Three sections: To Do list, Completed list, Notes textarea.

DayDetailsPanel::DayDetailsPanel(QWidget* parent)
    : QWidget(parent) {
    setupUi();
}

void DayDetailsPanel::setupUi() {
    setObjectName("dayDetailsPanel");

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(16, 16, 16, 16);
    outer->setSpacing(16);

    // ── Header: date + close button ──
    auto* header = new QHBoxLayout();
    header->setSpacing(8);

    m_dateLabel = new QLabel(this);
    m_dateLabel->setObjectName("dayDetailDate");

    m_closeButton = new QPushButton(QStringLiteral("×"), this);
    m_closeButton->setObjectName("dayDetailCloseBtn");
    m_closeButton->setFixedSize(24, 24);
    connect(m_closeButton, &QPushButton::clicked, this, &DayDetailsPanel::closed);

    header->addWidget(m_dateLabel, 1);
    header->addWidget(m_closeButton);

    outer->addLayout(header);

    // ── To Do section ──
    auto* todoHeader = new QLabel(tr("To Do"), this);
    todoHeader->setObjectName("dayDetailSectionHeader");
    outer->addWidget(todoHeader);

    auto* todoInputRow = new QHBoxLayout();
    todoInputRow->setSpacing(8);

    m_todoInput = new QLineEdit(this);
    m_todoInput->setPlaceholderText(tr("Add a task..."));
    connect(m_todoInput, &QLineEdit::returnPressed, this, &DayDetailsPanel::onAddTodo);

    m_todoAddBtn = new QPushButton(QStringLiteral("+"), this);
    m_todoAddBtn->setFixedSize(32, 32);
    connect(m_todoAddBtn, &QPushButton::clicked, this, &DayDetailsPanel::onAddTodo);

    todoInputRow->addWidget(m_todoInput, 1);
    todoInputRow->addWidget(m_todoAddBtn);

    outer->addLayout(todoInputRow);

    m_todoSection = new QWidget(this);
    m_todoSection->setObjectName("dayDetailTodoSection");
    auto* todoLayout = new QVBoxLayout(m_todoSection);
    todoLayout->setContentsMargins(0, 0, 0, 0);
    todoLayout->setSpacing(4);

    outer->addWidget(m_todoSection);

    // ── Completed section ──
    auto* completedHeader = new QLabel(tr("Completed"), this);
    completedHeader->setObjectName("dayDetailSectionHeader");
    outer->addWidget(completedHeader);

    m_completedSection = new QWidget(this);
    m_completedSection->setObjectName("dayDetailCompletedSection");
    auto* completedLayout = new QVBoxLayout(m_completedSection);
    completedLayout->setContentsMargins(0, 0, 0, 0);
    completedLayout->setSpacing(4);

    outer->addWidget(m_completedSection);

    // ── Notes section ──
    auto* notesHeader = new QLabel(tr("Notes"), this);
    notesHeader->setObjectName("dayDetailSectionHeader");
    outer->addWidget(notesHeader);

    m_notesEdit = new QTextEdit(this);
    m_notesEdit->setPlaceholderText(tr("Add notes for this day..."));
    m_notesEdit->setMaximumHeight(120);
    connect(m_notesEdit, &QTextEdit::textChanged, this, &DayDetailsPanel::onNotesTextChanged);

    outer->addWidget(m_notesEdit);

    // ── Empty state placeholder ──
    m_emptyLabel = new QLabel(tr("Select a day to view details"), this);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setObjectName("dayDetailEmptyState");
    m_emptyLabel->setVisible(false);

    outer->addStretch();
}

void DayDetailsPanel::showDay(const CalendarDayData& data) {
    m_currentDate = data.date;
    m_dateLabel->setText(data.date.toString("dddd, MMMM d, yyyy"));
    m_emptyLabel->setVisible(false);

    // Clear existing todo/completed widgets
    QLayoutItem* item;
    while ((item = m_todoSection->layout()->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    while ((item = m_completedSection->layout()->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    // Populate To Do list
    for (int i = 0; i < data.todo.size(); ++i) {
        auto* row = new QWidget(m_todoSection);
        auto* rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        rowLayout->setSpacing(8);

        auto* checkbox = new QCheckBox(row);
        checkbox->setChecked(false);
        connect(checkbox, &QCheckBox::toggled, this, [this, i](bool checked) {
            emit todoToggled(m_currentDate, i, checked);
        });

        auto* label = new QLabel(data.todo[i], row);

        rowLayout->addWidget(checkbox);
        rowLayout->addWidget(label, 1);

        m_todoSection->layout()->addWidget(row);
    }

    // Populate Completed list
    for (int i = 0; i < data.completed.size(); ++i) {
        auto* row = new QWidget(m_completedSection);
        auto* rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        rowLayout->setSpacing(8);

        auto* checkbox = new QCheckBox(row);
        checkbox->setChecked(true);
        connect(checkbox, &QCheckBox::toggled, this, [this, i](bool checked) {
            if (!checked) {
                emit todoToggled(m_currentDate, i, false);
            }
        });

        auto* label = new QLabel(data.completed[i], row);
        label->setStyleSheet("QLabel { text-decoration: line-through; color: #9ca3af; }");

        rowLayout->addWidget(checkbox);
        rowLayout->addWidget(label, 1);

        m_completedSection->layout()->addWidget(row);
    }

    // Set notes
    {
        QSignalBlocker blocker(m_notesEdit);
        m_notesEdit->setPlainText(data.notes);
    }
}

void DayDetailsPanel::clear() {
    m_currentDate = QDate();
    m_dateLabel->clear();
    m_todoInput->clear();
    m_notesEdit->clear();

    // Clear lists
    QLayoutItem* item;
    while ((item = m_todoSection->layout()->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    while ((item = m_completedSection->layout()->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    m_emptyLabel->setVisible(true);
}

void DayDetailsPanel::onAddTodo() {
    const QString text = m_todoInput->text().trimmed();
    if (text.isEmpty()) return;

    emit todoAdded(m_currentDate, text);
    m_todoInput->clear();
}

void DayDetailsPanel::onNotesTextChanged() {
    if (!m_currentDate.isValid()) return;
    emit notesChanged(m_currentDate, m_notesEdit->toPlainText());
}
