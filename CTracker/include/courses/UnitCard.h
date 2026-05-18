#pragma once

#include <QFrame>
#include <QString>
#include <QDateTime>

class QLabel;
class QMouseEvent;
class QEnterEvent;
class QResizeEvent;
class CircularProgressBar;

// Task 7.5a — UnitCard
//
// A 160 × 180 px card representing one Unit inside a course. Lives in
// the new units grid on CourseDetailView. Clicking emits clicked(unitId)
// which the parent uses to drill into the per-unit sessions sub-view.
//
// Visual breakdown (top → bottom):
//   ┌────────────────────────────┐
//   │   <unit name, semibold>    │  ← 2-line wrap
//   │                            │
//   │        ◯ 64 px ring        │  ← CircularProgressBar (mean of sessions)
//   │                            │
//   │       "5 / 8 done"          │  ← session counts subtitle
//   │   "Last worked: 3d ago"    │  ← humanised last-activity timestamp
//   └────────────────────────────┘
//
// Hover affordance via setProperty("hover", true/false) + unpolish/polish,
// mirroring EntityCard and ProjectCard so the same QSS rules apply.
class UnitCard : public QFrame {
    Q_OBJECT
public:
    explicit UnitCard(int unitId,
                      const QString& name,
                      QWidget* parent = nullptr);

    int     unitId() const { return m_unitId; }
    QString name()   const { return m_name; }

    void setName(const QString& name);
    void setProgress(int percentage);                    // drives the ring
    void setSessionCounts(int total, int completed);     // "5 sessions" or "3 / 8 done"
    void setLastActivity(const QDateTime& when);         // invalid() → "Never started"

signals:
    void clicked(int unitId);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event)      override;
    void leaveEvent(QEvent* event)           override;

private:
    void setupUi();

    int     m_unitId = -1;
    QString m_name;

    QLabel*              m_nameLabel     = nullptr;
    CircularProgressBar* m_ring          = nullptr;
    QLabel*              m_countsLabel   = nullptr;
    QLabel*              m_activityLabel = nullptr;
};
