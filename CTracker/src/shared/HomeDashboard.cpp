#include "shared/HomeDashboard.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QLabel>

#include "core/DatabaseManager.h"

namespace {
constexpr int kColumns = 3;
}

HomeDashboard::HomeDashboard(QWidget* parent)
    : QWidget(parent) {
    setupUi();
    connect(DatabaseManager::instance(), &DatabaseManager::dataChanged,
            this, &HomeDashboard::onDataChanged);
    loadEntities();
}

void HomeDashboard::setupUi() {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(16, 16, 16, 16);
    outer->setSpacing(12);

    auto* title = new QLabel(tr("Dashboard"), this);
    title->setObjectName("dashboardTitle");
    outer->addWidget(title);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);

    m_cardsContainer = new QWidget(m_scrollArea);
    m_cardsLayout = new QGridLayout(m_cardsContainer);
    m_cardsLayout->setContentsMargins(0, 0, 0, 0);
    m_cardsLayout->setHorizontalSpacing(16);
    m_cardsLayout->setVerticalSpacing(16);
    m_cardsLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    m_emptyLabel = new QLabel(
        tr("No courses or projects yet. Use Settings → Import Data to load one."),
        m_cardsContainer);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setVisible(false);

    m_scrollArea->setWidget(m_cardsContainer);
    outer->addWidget(m_scrollArea, 1);
}

void HomeDashboard::clearCards() {
    for (EntityCard* card : m_cards) {
        m_cardsLayout->removeWidget(card);
        card->deleteLater();
    }
    m_cards.clear();
    m_cardsLayout->removeWidget(m_emptyLabel);
}

void HomeDashboard::createCard(int id, const QString& name, EntityCard::EntityType type) {
    const int progress = computeOverallProgress(id);
    auto* card = new EntityCard(id, name, type, progress, m_cardsContainer);
    connect(card, &EntityCard::clicked, this,
            [this](int entityId, EntityCard::EntityType t) {
                if (t == EntityCard::EntityType::Course) {
                    emit courseSelected(entityId);
                } else {
                    emit projectSelected(entityId);
                }
            });
    const int index = m_cards.size();
    m_cardsLayout->addWidget(card, index / kColumns, index % kColumns);
    m_cards.append(card);
}

void HomeDashboard::loadEntities() {
    clearCards();

    auto* db = DatabaseManager::instance();
    const QList<EntityData> entities = db->fetchAllEntities();

    if (entities.isEmpty()) {
        m_cardsLayout->addWidget(m_emptyLabel, 0, 0, 1, kColumns);
        m_emptyLabel->setVisible(true);
        return;
    }

    m_emptyLabel->setVisible(false);
    for (const EntityData& e : entities) {
        const auto type = (e.type == QStringLiteral("Project"))
            ? EntityCard::EntityType::Project
            : EntityCard::EntityType::Course;
        createCard(e.id, e.name, type);
    }
}

void HomeDashboard::refreshCards() {
    loadEntities();
}

void HomeDashboard::onDataChanged() {
    refreshCards();
}

int HomeDashboard::computeOverallProgress(int entityId) const {
    auto* db = DatabaseManager::instance();
    const QList<UnitData> units = db->getUnitsForParent(entityId);
    if (units.isEmpty()) {
        return 0;
    }
    int sum = 0;
    int count = 0;
    for (const UnitData& u : units) {
        const QList<SessionTaskData> sessions = db->getSessionTasksForUnit(u.id);
        for (const SessionTaskData& s : sessions) {
            sum += s.progress;
            ++count;
        }
    }
    if (count == 0) {
        return 0;
    }
    return sum / count;
}
