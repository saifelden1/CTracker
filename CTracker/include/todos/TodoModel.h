#pragma once

#include <QSortFilterProxyModel>
#include <QAbstractListModel>
#include "core/DataStructures.h"

class TodoBaseModel : public QAbstractListModel {
    Q_OBJECT
public:
    explicit TodoBaseModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
public slots:
    void refresh();
private:
    QVector<TodoData> m_todos;
};

class TodoModel : public QObject {
    Q_OBJECT
public:
    explicit TodoModel(QObject *parent = nullptr);

    QSortFilterProxyModel* activeModel() const { return m_activeModel; }
    QSortFilterProxyModel* completedModel() const { return m_completedModel; }

signals:
    void activeCountChanged(int count);
    void completedCountChanged(int count);

private slots:
    void updateCounts();

private:
    TodoBaseModel *m_sourceModel;
    QSortFilterProxyModel *m_activeModel;
    QSortFilterProxyModel *m_completedModel;
};
