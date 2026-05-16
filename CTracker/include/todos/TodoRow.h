#pragma once

#include <QWidget>
#include <QString>

class QCheckBox;
class QLabel;
class QPushButton;

struct TodoData;

// TodoRow: a single row in the TodoView.
// Layout: checkbox | title | priority badge | trash button.
// Completed state = muted color + strikethrough; hover bg = surface-hover.
class TodoRow : public QWidget {
    Q_OBJECT
public:
    explicit TodoRow(const TodoData& data, QWidget* parent = nullptr);

    int todoId() const { return m_todoId; }

    void setCompleted(bool completed);
    void setPriority(const QString& priority);

signals:
    void completedToggled(int todoId, bool completed);
    void deleteRequested(int todoId);

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    void setupUi(const TodoData& data);
    void applyPriorityStyle(const QString& priority);
    void applyCompletedStyle(bool completed);

    int m_todoId = -1;

    QCheckBox*    m_checkbox     = nullptr;
    QLabel*       m_titleLabel   = nullptr;
    QLabel*       m_priorityBadge = nullptr;
    QPushButton*  m_deleteBtn    = nullptr;
};