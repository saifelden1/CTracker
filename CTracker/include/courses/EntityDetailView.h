#pragma once

#include <QWidget>
#include <QString>
#include <QList>

#include "courses/EntityCard.h"

class QLabel;
class QPushButton;
class QScrollArea;
class QVBoxLayout;
class CircularProgressBar;
class UnitExpandableWidget;

// Shared detail-view base. CourseDetailView and ProjectDetailView differ only
// in the entity type they target (which affects rename/remove dispatch and
// the title label text).
class EntityDetailView : public QWidget {
    Q_OBJECT
public:
    explicit EntityDetailView(EntityCard::EntityType type, QWidget* parent = nullptr);

    void loadEntity(int entityId);

    int  currentEntityId() const { return m_entityId; }

signals:
    void entityRemoved(int entityId);
    void backRequested();

private slots:
    void onAddUnitClicked();
    void onAddSessionClicked();
    void onDeleteEntityClicked();
    void onSessionProgressChanged(int sessionId, int oldValue, int newValue);
    void onSessionRenamed(int sessionId, const QString& newName);
    void onDataChanged();

private:
    void setupUi();
    void clearUnits();
    void rebuildUnits();
    void refreshOverall();

    EntityCard::EntityType m_type;
    int                    m_entityId = -1;
    QString                m_entityName;

    QLabel*              m_titleLabel    = nullptr;
    CircularProgressBar* m_overallRing   = nullptr;
    QPushButton*         m_addUnitBtn    = nullptr;
    QPushButton*         m_addSessionBtn = nullptr;
    QPushButton*         m_deleteBtn     = nullptr;
    QPushButton*         m_backBtn       = nullptr;

    QScrollArea* m_scrollArea     = nullptr;
    QWidget*     m_unitsContainer = nullptr;
    QVBoxLayout* m_unitsLayout    = nullptr;

    QList<UnitExpandableWidget*> m_unitWidgets;
};
