#include "todos/TodoView.h"

#include "todos/TodoRow.h"
#include "todos/TodoModel.h"
#include "core/DatabaseManager.h"
#include "core/DataStructures.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QTimer>

// Task 7.7: TodoView implementation.

TodoView::TodoView(QWidget* parent)
    : QWidget(parent) {
    setupUi();
    connect(DatabaseManager::instance(), &DatabaseManager::dataChanged,
            this, &TodoView::onDataChanged);
    refreshTodos();
}

void TodoView::setupUi() {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(24, 24, 24, 24);
    outer->setSpacing(24);

    // ── Header: title + 2 stat boxes ──
    auto* headerRow = new QHBoxLayout();
    headerRow->setSpacing(16);

    auto* titleLabel = new QLabel(tr("To-Do List"), this);
    titleLabel->setObjectName("todoViewTitle");

    auto* activeBox = new QWidget(this);
    activeBox->setObjectName("todoStatBox");
    auto* activeBoxLayout = new QVBoxLayout(activeBox);
    activeBoxLayout->setContentsMargins(12, 8, 12, 8);
    activeBoxLayout->setSpacing(4);

    m_activeCountLabel = new QLabel("0", activeBox);
    m_activeCountLabel->setObjectName("todoStatValue");
    auto* activeLabel = new QLabel(tr("Active"), activeBox);
    activeLabel->setObjectName("todoStatLabel");

    activeBoxLayout->addWidget(m_activeCountLabel);
    activeBoxLayout->addWidget(activeLabel);

    auto* completedBox = new QWidget(this);
    completedBox->setObjectName("todoStatBox");
    auto* completedBoxLayout = new QVBoxLayout(completedBox);
    completedBoxLayout->setContentsMargins(12, 8, 12, 8);
    completedBoxLayout->setSpacing(4);

    m_completedCountLabel = new QLabel("0", completedBox);
    m_completedCountLabel->setObjectName("todoStatValue");
    auto* completedLabel = new QLabel(tr("Completed"), completedBox);
    completedLabel->setObjectName("todoStatLabel");

    completedBoxLayout->addWidget(m_completedCountLabel);
    completedBoxLayout->addWidget(completedLabel);

    headerRow->addWidget(titleLabel, 1);
    headerRow->addWidget(activeBox);
    headerRow->addWidget(completedBox);

    outer->addLayout(headerRow);

    // ── Add-task input ──
    auto* addRow = new QHBoxLayout();
    addRow->setSpacing(8);

    m_addInput = new QLineEdit(this);
    m_addInput->setPlaceholderText(tr("Add a new task..."));
    m_addInput->setMinimumHeight(40);
    connect(m_addInput, &QLineEdit::returnPressed, this, &TodoView::onAddTodo);

    m_addButton = new QPushButton(QStringLiteral("+"), this);
    m_addButton->setObjectName("todoAddButton");
    m_addButton->setFixedSize(40, 40);
    connect(m_addButton, &QPushButton::clicked, this, &TodoView::onAddTodo);

    addRow->addWidget(m_addInput, 1);
    addRow->addWidget(m_addButton);

    outer->addLayout(addRow);

    // ── Active section ──
    auto* activeHeader = new QLabel(tr("Active Tasks"), this);
    activeHeader->setObjectName("todoSectionHeader");
    outer->addWidget(activeHeader);

    m_activeSection = new QWidget(this);
    m_activeLayout = new QVBoxLayout(m_activeSection);
    m_activeLayout->setContentsMargins(0, 0, 0, 0);
    m_activeLayout->setSpacing(4);
    m_activeLayout->addStretch();

    outer->addWidget(m_activeSection);

    // ── Completed section ──
    auto* completedHeader = new QLabel(tr("Completed"), this);
    completedHeader->setObjectName("todoSectionHeader");
    outer->addWidget(completedHeader);

    m_completedSection = new QWidget(this);
    m_completedLayout = new QVBoxLayout(m_completedSection);
    m_completedLayout->setContentsMargins(0, 0, 0, 0);
    m_completedLayout->setSpacing(4);
    m_completedLayout->addStretch();

    outer->addWidget(m_completedSection);

    outer->addStretch();
}

void TodoView::refreshTodos() {
    populateActiveTodos();
    populateCompletedTodos();
}

void TodoView::populateActiveTodos() {
    // Clear existing widgets
    QLayoutItem* item;
    while ((item = m_activeLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    auto* db = DatabaseManager::instance();
    const QList<TodoData> todos = db->fetchActiveTodos();

    m_activeCountLabel->setText(QString::number(todos.size()));

    for (const TodoData& todo : todos) {
        auto* row = new TodoRow(todo, m_activeSection);
        connect(row, &TodoRow::completedToggled,
                this, &TodoView::onTodoCompletedToggled);
        connect(row, &TodoRow::deleteRequested,
                this, &TodoView::onTodoDeleteRequested);
        m_activeLayout->insertWidget(m_activeLayout->count() - 1, row);
    }
}

void TodoView::populateCompletedTodos() {
    // Clear existing widgets
    QLayoutItem* item;
    while ((item = m_completedLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    auto* db = DatabaseManager::instance();
    const QList<TodoData> todos = db->fetchCompletedTodos();

    m_completedCountLabel->setText(QString::number(todos.size()));

    for (const TodoData& todo : todos) {
        auto* row = new TodoRow(todo, m_completedSection);
        connect(row, &TodoRow::completedToggled,
                this, &TodoView::onTodoCompletedToggled);
        connect(row, &TodoRow::deleteRequested,
                this, &TodoView::onTodoDeleteRequested);
        m_completedLayout->insertWidget(m_completedLayout->count() - 1, row);
    }
}

void TodoView::onAddTodo() {
    const QString title = m_addInput->text().trimmed();
    if (title.isEmpty()) return;

    auto* db = DatabaseManager::instance();
    db->addTodo(title, "medium");  // default priority
    m_addInput->clear();
    refreshTodos();
}

void TodoView::onTodoCompletedToggled(int todoId, bool completed) {
    Q_UNUSED(completed);
    auto* db = DatabaseManager::instance();
    db->toggleTodoCompleted(todoId);
    
    // Delay refresh to avoid deleting the sender widget immediately
    QTimer::singleShot(0, this, &TodoView::refreshTodos);
}

void TodoView::onTodoDeleteRequested(int todoId) {
    auto* db = DatabaseManager::instance();
    db->removeTodo(todoId);
    
    // Delay refresh to avoid deleting the sender widget immediately
    QTimer::singleShot(0, this, &TodoView::refreshTodos);
}

void TodoView::onDataChanged() {
    refreshTodos();
}
