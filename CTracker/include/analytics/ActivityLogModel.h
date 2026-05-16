#pragma once

#include <QAbstractTableModel>
#include <QDate>
#include <QList>
#include <QMap>
#include <QString>

#include "core/DataStructures.h"

// ============================================================
//  ActivityLogModel.h
//
//  A QAbstractTableModel that exposes ActivityLog rows to any
//  Qt view (QTableView, QML, etc.). It also provides aggregation
//  helpers used by the heatmap (daily totals / counts).
//
//  The model owns an in-memory cache of ActivityLogEntry rows
//  that mirrors the current filter window. refresh() re-queries
//  the database and rebuilds that cache.
// ============================================================

class ActivityLogModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column {
        TimestampCol      = 0,
        ItemNameCol       = 1,
        TypeCol           = 2,
        ProgressChangeCol = 3,
        ColumnCount       = 4
    };

    explicit ActivityLogModel(QObject* parent = nullptr);

    // ---- QAbstractTableModel overrides ----
    int      rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int      columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    // ---- Filters ----
    void setFilterDateRange(const QDate& from, const QDate& to);
    void setFilterItemType(const QString& type);   // "Course", "Project", or empty for all
    void clearFilters();

    // Reload from DatabaseManager using the current filters.
    void refresh();

    // ---- Aggregations (used by the heatmap) ----
    QMap<QDate, int> getDailyProgressTotals(const QDate& from, const QDate& to) const;
    QMap<QDate, int> getDailyActivityCounts(const QDate& from, const QDate& to) const;

private:
    // True if the entry passes the current type filter.
    bool passesTypeFilter(const ActivityLogEntry& entry) const;

    // Resolve itemId → display name. Cached per refresh to avoid N queries per repaint.
    QString itemName(int itemId) const;

    QList<ActivityLogEntry> m_entries;
    QDate                   m_filterFrom;
    QDate                   m_filterTo;
    QString                 m_filterType;       // empty => no type filter
    mutable QMap<int, QString> m_nameCache;     // itemId → name
};
