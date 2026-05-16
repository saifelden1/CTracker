#pragma once

#include <QWidget>
#include <QString>

class QLabel;
class QLineEdit;
class QPushButton;
class QVBoxLayout;
class TodoModel;

// Task 7.7: TodoView — dedicated to-do list view.
// Header: title + 2 stat boxes (active count, completed count).
// Add-task input with Enter-to-submit and + button.
// Two sections (Active / Completed) of TodoRow widgets.
// Within each section: priority high → medium → low.
class TodoView : public QWidget {
    Q_OBJECT
public:
    explicit TodoView(QWidget* parent = nullptr);

public slots:
    void refreshTodos();

private slots:
    void onAddTodo();
    void onTodoCompletedToggled(int todoId, bool completed);
    void onTodoDeleteRequested(int todoId);
    void onDataChanged();

private:
    void setupUi();
    void populateActiveTodos();
    void populateCompletedTodos();

    QLabel*      m_activeCountLabel     = nullptr;
    QLabel*      m_completedCountLabel  = nullptr;
    QLineEdit*   m_addInput             = nullptr;
    QPushButton* m_addButton            = nullptr;

    QWidget*     m_activeSection        = nullptr;
    QVBoxLayout* m_activeLayout         = nullptr;

    QWidget*     m_completedSection     = nullptr;
    QVBoxLayout* m_completedLayout      = nullptr;

    TodoModel*   m_model                = nullptr;
};
