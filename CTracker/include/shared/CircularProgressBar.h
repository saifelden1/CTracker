#pragma once

#include <QWidget>
#include <QColor>

class QPaintEvent;
class QPainter;

class CircularProgressBar : public QWidget {
    Q_OBJECT
    Q_PROPERTY(int progress READ progress WRITE setProgress NOTIFY progressChanged)
    Q_PROPERTY(int lineWidth READ lineWidth WRITE setLineWidth)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)
    Q_PROPERTY(QColor progressColor READ progressColor WRITE setProgressColor)

public:
    explicit CircularProgressBar(QWidget* parent = nullptr);

    int progress() const { return m_progress; }
    int lineWidth() const { return m_lineWidth; }
    QColor backgroundColor() const { return m_backgroundColor; }
    QColor progressColor() const { return m_progressColor; }

    void setProgress(int value);
    void setLineWidth(int width);
    void setBackgroundColor(const QColor& color);
    void setProgressColor(const QColor& color);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void progressChanged(int value);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void drawBackground(QPainter& painter, const QRectF& rect);
    void drawProgress(QPainter& painter, const QRectF& rect);
    void drawText(QPainter& painter, const QRectF& rect);

    int    m_progress       = 0;     // 0..100
    int    m_lineWidth      = 10;
    QColor m_backgroundColor{ "#2d323d" };
    QColor m_progressColor  { "#10b981" };
};
