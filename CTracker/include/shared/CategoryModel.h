#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QColor>
#include <QVector>
#include "core/DataStructures.h"

class CategoryModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        ColorRole,
        EntityCountRole
    };

    explicit CategoryModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    void refresh();

private:
    QVector<CategoryData> m_categories;
};
