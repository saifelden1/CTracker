#pragma once

#include <QWidget>
#include <QString>
#include <QList>

#include "courses/EntityCard.h"

class QLabel;
class QPushButton;
class QScrollArea;
class QVBoxLayout;
class QHBoxLayout;
class CircularProgressBar;
class UnitExpandableWidget;

// Shared detail-view base for CourseDetailView and ProjectDetailView.
// Provides the common scaffolding: title bar, scroll area with unit widgets,
// CRUD operations, and data-change refresh.
//
// Subclasses extend by:
//   - Adding widgets to the title bar (status toggle, badges) via m_titleLayout
//   - Restructuring the content area (two-column for projects) via m_outerLayout
//   - Overriding loadEntity() to fetch extra data (status, ProjectMetaData)
//   - Overriding onDataChanged() to refresh extra UI elements
class EntityDetailView : public QWidget {
    Q_OBJECT
public:
    explicit EntityDetailView(EntityCard::EntityType type, QWidget* parent = nullptr);

    virtual void loadEntity(int entityId);

    int currentEntityId() const { return m_entityId; }

signals:
    void entityRemoved(int entityId);
    void backRequested();

protected:
    // ── UI setup ──
    void setupUi();

    // ── Data management ──
    void clearUnits();
    void rebuildUnits();
    void refreshOverall();

    // ── Type and state ──
    EntityCard::EntityType m_type;
    int     m_entityId    = -1;
    QString m_entityName;

    // ── Title bar widgets (subclasses add extras to m_titleLayout) ──
    QWidget*      m_titleBar      = nullptr;
    QHBoxLayout*  m_titleLayout   = nullptr;
    QLabel*       m_titleLabel    = nullptr;
    CircularProgressBar* m_overallRing = nullptr;
    QPushButton*  m_addUnitBtn    = nullptr;
    QPushButton*  m_addSessionBtn = nullptr;
    QPushButton*  m_deleteBtn     = nullptr;
    QPushButton*  m_backBtn       = nullptr;

    // ── Content area ──
    QVBoxLayout*  m_outerLayout   = nullptr;
    QScrollArea*  m_scrollArea    = nullptr;
    QWidget*      m_unitsContainer = nullptr;
    QVBoxLayout*  m_unitsLayout   = nullptr;

    QList<UnitExpandableWidget*> m_unitWidgets;

protected slots:
    virtual void onDataChanged();

private slots:
    void onAddUnitClicked();
    void onAddSessionClicked();
    void onDeleteEntityClicked();
    void onSessionProgressChanged(int sessionId, int oldValue, int newValue);
    void onSessionRenamed(int sessionId, const QString& newName);
};
