#include "todos/TodoModel.h"
#include "core/DatabaseManager.h"

TodoBaseModel::TodoBaseModel(QObject *parent) : QAbstractListModel(parent) {
    connect(DatabaseManager::instance(), &DatabaseManager::dataChanged, this, &TodoBaseModel::refresh);
    refresh();
}

int TodoBaseModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_todos.size();
}
QVariant TodoBaseModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return QVariant();
    const auto& todo = m_todos[index.row()];
    if (role == Qt::DisplayRole) return todo.title;
    if (role == Qt::UserRole) return todo.completed;
    return QVariant();
}
void TodoBaseModel::refresh() {
    beginResetModel();
    m_todos.clear();
    endResetModel();
}

TodoModel::TodoModel(QObject *parent) : QObject(parent) {
    m_sourceModel = new TodoBaseModel(this);
    
    m_activeModel = new QSortFilterProxyModel(this);
    m_activeModel->setSourceModel(m_sourceModel);
    // filter logic
    
    m_completedModel = new QSortFilterProxyModel(this);
    m_completedModel->setSourceModel(m_sourceModel);
    
    connect(m_sourceModel, &QAbstractItemModel::modelReset, this, &TodoModel::updateCounts);
}

void TodoModel::updateCounts() {
    emit activeCountChanged(m_activeModel->rowCount());
    emit completedCountChanged(m_completedModel->rowCount());
}
