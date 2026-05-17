#include "calendar/CalendarWidget.h"

#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QDate>
#include <QFont>
#include <QFontMetrics>
#include <QLocale>

// Canonical palette tokens (matching CLAUDE.md §5).
// NOTE: QColor must be constructed from numeric RGB at file-scope.
// QColor("#hex") touches QtGui internals that are not yet initialized
// during pre-main dynamic init and corrupts the heap on Windows/MinGW.
static const QColor kBg         (0x1a, 0x1d, 0x24);
static const QColor kSurface    (0x25, 0x29, 0x32);
static const QColor kSurfaceHover(0x2d, 0x32, 0x3d);
static const QColor kPrimary    (0x10, 0xb9, 0x81);
static const QColor kText       (0xe4, 0xe6, 0xeb);
static const QColor kMuted      (0x9c, 0xa3, 0xaf);
static const QColor kSubtle     (0x6b, 0x72, 0x80);
static const QColor kBorder     (0x2d, 0x32, 0x3d);

CalendarWidget::CalendarWidget(QWidget* parent)
    : QWidget(parent) {
    QDate today = QDate::currentDate();
    m_displayYear  = today.year();
    m_displayMonth = today.month();
    m_selectedDate = today;

    setObjectName("calendarWidget");
    setMinimumSize(300, 250);
    setFocusPolicy(Qt::StrongFocus);
}

QDate CalendarWidget::selectedDate() const {
    return m_selectedDate;
}

void CalendarWidget::setSelectedDate(const QDate& date) {
    if (date.isValid() && date != m_selectedDate) {
        m_selectedDate = date;
        m_displayYear  = date.year();
        m_displayMonth = date.month();
        update();
    }
}

void CalendarWidget::setIndicatorDates(const QSet<QDate>& dates) {
    m_indicatorDates = dates;
    update();
}

// ── Navigation ──────────────────────────────────────────────

void CalendarWidget::navigateMonth(int offset) {
    QDate d(m_displayYear, m_displayMonth, 1);
    d = d.addMonths(offset);
    m_displayYear  = d.year();
    m_displayMonth = d.month();
    emit monthChanged(m_displayYear, m_displayMonth);
    update();
}

// ── Geometry helpers ────────────────────────────────────────

QRect CalendarWidget::headerArrowRect(bool left) const {
    int x = left ? 0 : width() - ARROW_W;
    return QRect(x, 0, ARROW_W, HEADER_H);
}

QRect CalendarWidget::dayCellRect(int row, int col) const {
    int x = col * CELL_W;
    int y = HEADER_H + LABELS_H + row * CELL_H;
    return QRect(x, y, CELL_W, CELL_H);
}

QDate CalendarWidget::cellDate(int row, int col) const {
    // First day of the displayed month
    QDate firstOfMonth(m_displayYear, m_displayMonth, 1);
    // Day-of-week for the 1st (Mon=1..Sun=7 in Qt, but we display Sun-first)
    int firstDow = firstOfMonth.dayOfWeek();           // Mon=1..Sun=7
    int sundayCol = (firstDow % 7);                    // shift so Sunday = col 0
    // The date at (row, col):
    int dayNumber = (row * COLS + col) - sundayCol + 1;
    return QDate(m_displayYear, m_displayMonth, dayNumber);
}

int CalendarWidget::cellFromPoint(const QPoint& pos) const {
    int col = pos.x() / CELL_W;
    int row = (pos.y() - HEADER_H - LABELS_H) / CELL_H;
    if (row < 0 || row >= ROWS || col < 0 || col >= COLS) return -1;
    return row * COLS + col;
}

// ── Painting ────────────────────────────────────────────────

void CalendarWidget::paintEvent(QPaintEvent* /*event*/) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    // Background
    p.fillRect(rect(), kBg);

    QFont baseFont = font();
    baseFont.setPointSize(10);

    // ── Header: ◀ Month Year ▶ ──
    p.setFont(baseFont);
    QString monthName = QLocale().monthName(m_displayMonth);
    QString headerText = QStringLiteral("%1 %2").arg(monthName).arg(m_displayYear);
    QFontMetrics fm(baseFont);
    int headerTextX = (width() - fm.horizontalAdvance(headerText)) / 2;
    p.setPen(kText);
    p.drawText(headerTextX, (HEADER_H + fm.ascent()) / 2, headerText);

    // Arrows
    p.setPen(kMuted);
    p.drawText(headerArrowRect(true), Qt::AlignCenter, QStringLiteral("◀"));
    p.drawText(headerArrowRect(false), Qt::AlignCenter, QStringLiteral("▶"));

    // ── Day labels: S M T W T F S ──
    static const char* kDayLabels[] = { "S", "M", "T", "W", "T", "F", "S" };
    p.setPen(kSubtle);
    p.setFont(baseFont);
    for (int c = 0; c < COLS; ++c) {
        QRect cell = dayCellRect(-1, c);  // row -1 = label row (above grid)
        // Manually position: labels row sits between header and grid
        QRect labelRect(c * CELL_W, HEADER_H, CELL_W, LABELS_H);
        p.drawText(labelRect, Qt::AlignCenter, kDayLabels[c]);
    }

    // ── Day cells ──
    QDate today = QDate::currentDate();
    QDate firstOfMonth(m_displayYear, m_displayMonth, 1);
    int firstDow = firstOfMonth.dayOfWeek();
    int sundayOffset = (firstDow % 7);

    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
            int dayNumber = (row * COLS + col) - sundayOffset + 1;
            QDate cellDate(m_displayYear, m_displayMonth, dayNumber);

            QRect cell = dayCellRect(row, col);

            // Skip cells that belong to adjacent months
            if (dayNumber < 1 || dayNumber > firstOfMonth.daysInMonth()) {
                p.setPen(kSubtle);
                p.drawText(cell, Qt::AlignCenter, QString::number(
                    dayNumber < 1
                        ? QDate(m_displayYear, m_displayMonth, 1).addDays(dayNumber - 1).day()
                        : QDate(m_displayYear, m_displayMonth, 1).addDays(dayNumber - 1).day()));
                continue;
            }

            // Selected day background
            if (cellDate == m_selectedDate) {
                p.setPen(Qt::NoPen);
                p.setBrush(kSurfaceHover);
                p.drawRoundedRect(cell.adjusted(2, 2, -2, -2), 4, 4);
            }

            // Today highlight ring
            if (cellDate == today) {
                p.setPen(QPen(kPrimary, 2));
                p.setBrush(Qt::NoBrush);
                p.drawRoundedRect(cell.adjusted(2, 2, -2, -2), 4, 4);
            }

            // Day number
            p.setPen(cellDate == m_selectedDate ? kPrimary : kText);
            p.setFont(baseFont);
            p.drawText(cell, Qt::AlignCenter, QString::number(dayNumber));

            // Indicator dot
            if (m_indicatorDates.contains(cellDate)) {
                int dotX = cell.center().x();
                int dotY = cell.bottom() - 6;
                p.setPen(Qt::NoPen);
                p.setBrush(kPrimary);
                p.drawEllipse(dotX - DOT_SIZE / 2, dotY - DOT_SIZE / 2, DOT_SIZE, DOT_SIZE);
            }
        }
    }
}

// ── Mouse interaction ───────────────────────────────────────

void CalendarWidget::mousePressEvent(QMouseEvent* event) {
    const QPoint pos = event->pos();

    // Check navigation arrows
    if (headerArrowRect(true).contains(pos)) {
        navigateMonth(-1);
        return;
    }
    if (headerArrowRect(false).contains(pos)) {
        navigateMonth(+1);
        return;
    }

    // Check day cells
    int idx = cellFromPoint(pos);
    if (idx < 0) return;

    int row = idx / COLS;
    int col = idx % COLS;

    QDate firstOfMonth(m_displayYear, m_displayMonth, 1);
    int firstDow = firstOfMonth.dayOfWeek();
    int sundayOffset = (firstDow % 7);
    int dayNumber = idx - sundayOffset + 1;

    QDate clicked(m_displayYear, m_displayMonth, dayNumber);
    if (!clicked.isValid()) return;
    if (clicked.month() != m_displayMonth) return;  // ignore out-of-month clicks

    m_selectedDate = clicked;
    emit dateClicked(clicked);
    update();
}

// ── Keyboard navigation ─────────────────────────────────────

void CalendarWidget::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
    case Qt::Key_Left:
        m_selectedDate = m_selectedDate.addDays(-1);
        if (m_selectedDate.month() != m_displayMonth) navigateMonth(-1);
        else update();
        emit dateClicked(m_selectedDate);
        break;
    case Qt::Key_Right:
        m_selectedDate = m_selectedDate.addDays(1);
        if (m_selectedDate.month() != m_displayMonth) navigateMonth(+1);
        else update();
        emit dateClicked(m_selectedDate);
        break;
    case Qt::Key_Up:
        m_selectedDate = m_selectedDate.addDays(-7);
        if (m_selectedDate.month() != m_displayMonth) navigateMonth(-1);
        else update();
        emit dateClicked(m_selectedDate);
        break;
    case Qt::Key_Down:
        m_selectedDate = m_selectedDate.addDays(7);
        if (m_selectedDate.month() != m_displayMonth) navigateMonth(+1);
        else update();
        emit dateClicked(m_selectedDate);
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}