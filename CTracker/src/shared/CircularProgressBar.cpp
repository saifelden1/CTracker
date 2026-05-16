#include "shared/CircularProgressBar.h"

#include <QPainter>
#include <QPaintEvent>
#include <QFont>
#include <algorithm>

CircularProgressBar::CircularProgressBar(QWidget* parent)
    : QWidget(parent) {
    setAttribute(Qt::WA_OpaquePaintEvent, false);
}

QSize CircularProgressBar::sizeHint() const {
    return QSize(100, 100);
}

QSize CircularProgressBar::minimumSizeHint() const {
    return QSize(40, 40);
}

void CircularProgressBar::setProgress(int value) {
    const int clamped = std::clamp(value, 0, 100);
    if (clamped == m_progress) {
        return;
    }
    m_progress = clamped;
    emit progressChanged(m_progress);
    update();
}

void CircularProgressBar::setLineWidth(int width) {
    if (width <= 0 || width == m_lineWidth) {
        return;
    }
    m_lineWidth = width;
    update();
}

void CircularProgressBar::setBackgroundColor(const QColor& color) {
    if (color == m_backgroundColor) {
        return;
    }
    m_backgroundColor = color;
    update();
}

void CircularProgressBar::setProgressColor(const QColor& color) {
    if (color == m_progressColor) {
        return;
    }
    m_progressColor = color;
    update();
}

void CircularProgressBar::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const int side = std::min(width(), height());
    const qreal pad = m_lineWidth / 2.0 + 1.0;
    const QRectF rect(
        (width()  - side) / 2.0 + pad,
        (height() - side) / 2.0 + pad,
        side - 2 * pad,
        side - 2 * pad);

    drawBackground(painter, rect);
    drawProgress(painter, rect);
    drawText(painter, rect);
}

void CircularProgressBar::drawBackground(QPainter& painter, const QRectF& rect) {
    QPen pen(m_backgroundColor);
    pen.setWidth(m_lineWidth);
    pen.setCapStyle(Qt::FlatCap);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    // Full 360° ring. Qt uses 16ths of a degree.
    painter.drawArc(rect, 0, 360 * 16);
}

void CircularProgressBar::drawProgress(QPainter& painter, const QRectF& rect) {
    if (m_progress <= 0) {
        return;
    }
    QPen pen(m_progressColor);
    pen.setWidth(m_lineWidth);
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    // Start at 12 o'clock (90°), sweep clockwise (negative span).
    const int startAngle = 90 * 16;
    const int spanAngle  = -static_cast<int>(m_progress * 3.6 * 16);
    painter.drawArc(rect, startAngle, spanAngle);
}

void CircularProgressBar::drawText(QPainter& painter, const QRectF& rect) {
    QFont f = painter.font();
    f.setBold(true);
    f.setPointSizeF(std::max(8.0, rect.width() / 5.0));
    painter.setFont(f);
    painter.setPen(QColor("#e4e6eb"));
    painter.drawText(rect, Qt::AlignCenter, QString("%1%").arg(m_progress));
}
