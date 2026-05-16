#pragma once

#include <QFrame>
#include <QString>

class QLabel;
class CircularProgressBar;
class QMouseEvent;
class QEnterEvent;

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

signals:
    void clicked(int entityId, EntityType type);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    void setupUi(int initialProgress);

    int        m_entityId = -1;
    EntityType m_type     = EntityType::Course;
    QString    m_name;

    CircularProgressBar* m_progressBar = nullptr;
    QLabel*              m_nameLabel   = nullptr;
    QLabel*              m_typeLabel   = nullptr;
};
