#include "ContributionHeatmap.h"

#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QToolTip>
#include <QLocale>
#include <algorithm>

ContributionHeatmap::ContributionHeatmap(QWidget* parent)
    : QWidget(parent) {
    setMouseTracking(true);
    setMinimumSize(sizeHint());
    calculateCells();
}

QSize ContributionHeatmap::sizeHint() const {
    const int w = LEFT_PAD + COLS * (CELL_SIZE + CELL_SPACING);
    const int h = TOP_PAD  + ROWS * (CELL_SIZE + CELL_SPACING);
    return QSize(w, h);
}

void ContributionHeatmap::setData(const QMap<QDate, DayData>& data) {
    m_data = data;
    calculateCells();
    update();
}

void ContributionHeatmap::setYear(int year) {
    if (year == m_currentYear) {
        return;
    }
    m_currentYear = year;
    calculateCells();
    update();
}

void ContributionHeatmap::clearData() {
    m_data.clear();
    calculateCells();
    update();
}

void ContributionHeatmap::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    calculateCells();
}

void ContributionHeatmap::calculateCells() {
    m_cells.clear();
    m_cells.reserve(COLS * ROWS);

    // Anchor the grid on the Sunday on/before Jan 1 of the current year.
    QDate anchor(m_currentYear, 1, 1);
    while (anchor.dayOfWeek() != 7) {           // Qt: Mon=1..Sun=7
        anchor = anchor.addDays(-1);
    }

    for (int col = 0; col < COLS; ++col) {
        for (int row = 0; row < ROWS; ++row) {
            const QDate date = anchor.addDays(col * 7 + row);
            Cell cell;
            cell.date = date;
            cell.rect = QRect(
                LEFT_PAD + col * (CELL_SIZE + CELL_SPACING),
                TOP_PAD  + row * (CELL_SIZE + CELL_SPACING),
                CELL_SIZE,
                CELL_SIZE);

            const auto it = m_data.constFind(date);
            cell.intensity = (it != m_data.constEnd()) ? it->intensityLevel : 0;
            m_cells.append(cell);
        }
    }
}

QColor ContributionHeatmap::getIntensityColor(int intensity) const {
    // 5-level GitHub-style gradient.
    static const QColor table[5] = {
        QColor("#161b22"),  // 0: no activity
        QColor("#0e4429"),  // 1
        QColor("#006d32"),  // 2
        QColor("#26a641"),  // 3
        QColor("#39d353"),  // 4
    };
    return table[std::clamp(intensity, 0, 4)];
}

void ContributionHeatmap::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    drawLabels(painter);

    for (const Cell& cell : m_cells) {
        if (cell.date.year() != m_currentYear &&
            cell.date.addDays(-7).year() != m_currentYear &&
            cell.date.addDays(7).year() != m_currentYear) {
            continue;
        }
        painter.fillRect(cell.rect, getIntensityColor(cell.intensity));
    }
}

void ContributionHeatmap::drawLabels(QPainter& painter) {
    painter.setPen(QColor("#7d8590"));
    QFont f = painter.font();
    f.setPointSize(8);
    painter.setFont(f);

    // Day-of-week labels on the left (Mon, Wed, Fri are conventional).
    static const struct { int row; const char* label; } dayLabels[] = {
        { 1, "Mon" }, { 3, "Wed" }, { 5, "Fri" }
    };
    for (const auto& d : dayLabels) {
        const int y = TOP_PAD + d.row * (CELL_SIZE + CELL_SPACING) + CELL_SIZE;
        painter.drawText(2, y, QString::fromLatin1(d.label));
    }

    // Month labels along the top: drawn at the first column whose Sunday lands in a new month.
    int lastMonth = -1;
    for (int col = 0; col < COLS; ++col) {
        if (col * ROWS >= m_cells.size()) {
            break;
        }
        const QDate firstOfCol = m_cells[col * ROWS].date;
        if (firstOfCol.year() != m_currentYear) {
            continue;
        }
        if (firstOfCol.month() != lastMonth) {
            lastMonth = firstOfCol.month();
            const int x = LEFT_PAD + col * (CELL_SIZE + CELL_SPACING);
            painter.drawText(x, TOP_PAD - 4,
                QLocale().monthName(lastMonth, QLocale::ShortFormat));
        }
    }
}

const ContributionHeatmap::Cell* ContributionHeatmap::getCellAtPosition(const QPoint& pos) const {
    for (const Cell& cell : m_cells) {
        if (cell.rect.contains(pos)) {
            return &cell;
        }
    }
    return nullptr;
}

void ContributionHeatmap::mouseMoveEvent(QMouseEvent* event) {
    const Cell* cell = getCellAtPosition(event->pos());
    if (!cell || cell->date.year() != m_currentYear) {
        QToolTip::hideText();
        return;
    }
    const auto it = m_data.constFind(cell->date);
    const int progress = (it != m_data.constEnd()) ? it->totalProgress : 0;
    const int count    = (it != m_data.constEnd()) ? it->activityCount : 0;

    const QString text = QString("%1\n%2 progress · %3 activities")
        .arg(cell->date.toString("MMM d, yyyy"))
        .arg(progress)
        .arg(count);
    QToolTip::showText(event->globalPosition().toPoint(), text, this);
    emit dayHovered(cell->date, progress);
}

void ContributionHeatmap::leaveEvent(QEvent* /*event*/) {
    QToolTip::hideText();
}
