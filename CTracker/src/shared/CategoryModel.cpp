#include "shared/CategoryModel.h"
#include "core/DatabaseManager.h"
#include <QSqlQuery>

CategoryModel::CategoryModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(DatabaseManager::instance(), &DatabaseManager::dataChanged, this, &CategoryModel::refresh);
    refresh();
}

int CategoryModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_categories.size();
}

QVariant CategoryModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_categories.size()) return QVariant();
    const auto &cat = m_categories[index.row()];
    switch (role) {
        case IdRole: return cat.id;
        case Qt::DisplayRole:
        case NameRole: return cat.name;
        case ColorRole: return cat.color;
        case EntityCountRole: return cat.entityCount;
    }
    return QVariant();
}

QHash<int, QByteArray> CategoryModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[NameRole] = "name";
    roles[ColorRole] = "color";
    roles[EntityCountRole] = "entityCount";
    return roles;
}

void CategoryModel::refresh() {
    beginResetModel();
    m_categories.clear();
    // Dummy / basic implementation, full query would go here
    endResetModel();
}
