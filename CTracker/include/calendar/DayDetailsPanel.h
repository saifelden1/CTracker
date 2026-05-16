#pragma once

#include <QWidget>
#include <QDate>

class QLabel;
class QLineEdit;
class QPushButton;
class QTextEdit;

struct CalendarDayData;

// DayDetailsPanel: shows details for a selected calendar day.
// Three sections: To Do list, Completed list (strikethrough), Notes textarea.
// Header: formatted date + close × button → emits closed().
class DayDetailsPanel : public QWidget {
    Q_OBJECT
public:
    explicit DayDetailsPanel(QWidget* parent = nullptr);

    void showDay(const CalendarDayData& data);  // populate all sections
    void clear();                               // empty state placeholder

signals:
    void todoAdded(const QDate& date, const QString& text);
    void todoToggled(const QDate& date, int index, bool completed);
    void notesChanged(const QDate& date, const QString& text);
    void closed();

private slots:
    void onAddTodo();
    void onNotesTextChanged();

private:
    void setupUi();

    QDate m_currentDate;   // context for emitted signals

    QLabel*      m_dateLabel     = nullptr;
    QPushButton* m_closeButton   = nullptr;

    // To Do section
    QWidget*     m_todoSection   = nullptr;
    QLineEdit*   m_todoInput     = nullptr;
    QPushButton* m_todoAddBtn    = nullptr;

    // Completed section
    QWidget*     m_completedSection = nullptr;

    // Notes section
    QTextEdit*   m_notesEdit     = nullptr;

    // Placeholder for empty state
    QLabel*      m_emptyLabel    = nullptr;
};