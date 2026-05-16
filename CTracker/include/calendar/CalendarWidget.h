#pragma once

#include <QWidget>
#include <QDate>
#include <QSet>

// Custom calendar widget — NOT QCalendarWidget (we need indicator dots
// and full visual control). Custom-painted month grid with navigation
// arrows, day labels, current-day highlight, and indicator dots.
class CalendarWidget : public QWidget {
    Q_OBJECT
public:
    explicit CalendarWidget(QWidget* parent = nullptr);

    void setSelectedDate(const QDate& date);
    QDate selectedDate() const;

    void setIndicatorDates(const QSet<QDate>& dates);  // dots under days with content

signals:
    void dateClicked(const QDate& date);
    void monthChanged(int year, int month);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    void navigateMonth(int offset);  // -1 = prev, +1 = next

    // Grid geometry helpers
    QRect headerArrowRect(bool left) const;
    QRect dayCellRect(int row, int col) const;
    QDate cellDate(int row, int col) const;
    int  cellFromPoint(const QPoint& pos) const;

    // Displayed month/year
    int  m_displayYear  = 0;
    int  m_displayMonth = 0;   // 1–12

    QDate m_selectedDate;
    QSet<QDate> m_indicatorDates;

    // Grid constants
    static constexpr int COLS       = 7;
    static constexpr int ROWS       = 6;    // 6 week rows
    static constexpr int HEADER_H   = 36;   // ◀ Month Year ▶ row height
    static constexpr int LABELS_H   = 24;   // S M T W T F S row height
    static constexpr int CELL_W     = 40;
    static constexpr int CELL_H     = 32;
    static constexpr int DOT_SIZE   = 4;    // indicator dot diameter
    static constexpr int ARROW_W    = 24;   // clickable arrow width
};