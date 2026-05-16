#pragma once

#include <QFrame>
#include <QString>

class QLabel;
class QIcon;

// StatsCard: a QFrame displaying an icon (top-right), title, large value,
// subtitle, and optional badge. Used by HomeDashboard (3 cards) and
// AnalyticsView (4 cards).
//
// Layout: lg (8 px) border-radius, surface (#252932) background,
// hover shadow via property-based QSS.
class StatsCard : public QFrame {
    Q_OBJECT
public:
    explicit StatsCard(const QString& title, QWidget* parent = nullptr);

    void setValue(const QString& value);        // large number / text
    void setSubtitle(const QString& subtitle);  // e.g. "Active courses average"
    void setBadgeText(const QString& text);     // optional, e.g. "2 paused"
    void setIcon(const QIcon& icon);            // top-right icon

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    void setupUi(const QString& title);

    QLabel* m_titleLabel    = nullptr;
    QLabel* m_valueLabel    = nullptr;
    QLabel* m_subtitleLabel = nullptr;
    QLabel* m_badgeLabel    = nullptr;
    QLabel* m_iconLabel     = nullptr;
};