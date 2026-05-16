#pragma once

#include <QWidget>
#include <QList>

#include "courses/EntityCard.h"

class QScrollArea;
class QGridLayout;
class QLabel;

class HomeDashboard : public QWidget {
    Q_OBJECT
public:
    explicit HomeDashboard(QWidget* parent = nullptr);

    void refreshCards();

signals:
    void courseSelected(int courseId);
    void projectSelected(int projectId);

public slots:
    void onDataChanged();

private:
    void setupUi();
    void loadEntities();
    void createCard(int id, const QString& name, EntityCard::EntityType type);
    void clearCards();

    // Computes overall progress as mean(progress) across every session/task
    // belonging to every unit of the given entity. Returns 0 when the entity
    // has no sessions.
    int  computeOverallProgress(int entityId) const;

    QScrollArea* m_scrollArea    = nullptr;
    QWidget*     m_cardsContainer = nullptr;
    QGridLayout* m_cardsLayout   = nullptr;
    QLabel*      m_emptyLabel    = nullptr;

    QList<EntityCard*> m_cards;
};
