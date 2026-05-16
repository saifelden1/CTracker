#pragma once

#include <QFrame>
#include <QString>
#include <QColor>

class QLabel;
class CircularProgressBar;
class CategoryPill;
class QMouseEvent;
class QEnterEvent;
struct CategoryData;

// EntityCard: a card widget displaying a course or project with a
// CircularProgressBar, name, type badge, and (v2 additions):
//   - CategoryPill slot (top-left) — visible when categoryId >= 0
//   - "Paused" status badge (top-right) — visible when status == "paused"
class EntityCard : public QFrame {
    Q_OBJECT
public:
    enum class EntityType { Course, Project };
    Q_ENUM(EntityType)

    explicit EntityCard(int entityId,
                        const QString& name,
                        EntityType type,
                        int progress,
                        QWidget* parent = nullptr);

    int        entityId() const { return m_entityId; }
    EntityType type()     const { return m_type; }
    QString    name()     const { return m_name; }

    void setProgress(int percentage);
    void setName(const QString& name);

    // v2 additions (Task 6.5)
    void setCategory(const CategoryData& cat);  // shows CategoryPill (top-left)
    void clearCategory();                        // hides CategoryPill
    void setStatus(const QString& status);        // shows "Paused" badge when status=="paused"

signals:
    void clicked(int entityId, EntityType type);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void setupUi(int initialProgress);
    void positionOverlays();  // absolute-position CategoryPill and status badge

    int        m_entityId = -1;
    EntityType m_type     = EntityType::Course;
    QString    m_name;
    QString    m_status   = "active";

    CircularProgressBar* m_progressBar   = nullptr;
    QLabel*              m_nameLabel     = nullptr;
    QLabel*              m_typeLabel     = nullptr;

    // v2 additions
    CategoryPill*        m_categoryPill  = nullptr;  // top-left overlay
    QLabel*              m_statusBadge   = nullptr;  // top-right overlay ("Paused")
};
