#pragma once

#include <QWidget>
#include <QDate>
#include <QMap>
#include <QList>
#include <QRect>

class QPainter;

class ContributionHeatmap : public QWidget {
    Q_OBJECT
public:
    struct DayData {
        QDate date;
        int   totalProgress  = 0;
        int   activityCount  = 0;
        int   intensityLevel = 0; // 0..4
    };

    explicit ContributionHeatmap(QWidget* parent = nullptr);

    void setData(const QMap<QDate, DayData>& data);
    void setYear(int year);
    int  year() const { return m_currentYear; }
    void clearData();

    QSize sizeHint() const override;

signals:
    void dayHovered(const QDate& date, int totalProgress);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    struct Cell {
        QRect rect;
        QDate date;
        int   intensity = 0;
    };

    void calculateCells();
    void drawLabels(QPainter& painter);
    QColor getIntensityColor(int intensity) const;
    const Cell* getCellAtPosition(const QPoint& pos) const;

    QMap<QDate, DayData> m_data;
    QList<Cell>          m_cells;
    int                  m_currentYear = QDate::currentDate().year();

    // Grid configuration (matches the React prototype spec).
    static constexpr int COLS         = 53;
    static constexpr int ROWS         = 7;
    static constexpr int CELL_SIZE    = 12;
    static constexpr int CELL_SPACING = 3;
    static constexpr int LEFT_PAD     = 28;   // room for day-of-week labels
    static constexpr int TOP_PAD      = 18;   // room for month labels
};
