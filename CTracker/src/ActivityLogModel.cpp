#include "ActivityLogModel.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QVariant>

#include "DatabaseManager.h"

ActivityLogModel::ActivityLogModel(QObject* parent)
    : QAbstractTableModel(parent)
{
    // Default window: last 365 days, all types.
    m_filterTo   = QDate::currentDate();
    m_filterFrom = m_filterTo.addDays(-365);
}

int ActivityLogModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return m_entries.size();
}

int ActivityLogModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return ColumnCount;
}

QVariant ActivityLogModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size())
        return {};

    const ActivityLogEntry& e = m_entries.at(index.row());

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case TimestampCol:
            return e.timestamp.toString(Qt::ISODate);
        case ItemNameCol:
            return itemName(e.itemId);
        case TypeCol:
            return e.type;
        case ProgressChangeCol:
            return QStringLiteral("%1 → %2 (Δ %3)")
                .arg(e.oldValue).arg(e.newValue).arg(e.progressDelta);
        }
    }

    if (role == Qt::TextAlignmentRole && index.column() == ProgressChangeCol)
        return Qt::AlignCenter;

    return {};
}

QVariant ActivityLogModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) return {};

    switch (section) {
    case TimestampCol:      return tr("Timestamp");
    case ItemNameCol:       return tr("Item");
    case TypeCol:           return tr("Type");
    case ProgressChangeCol: return tr("Progress");
    }
    return {};
}

void ActivityLogModel::setFilterDateRange(const QDate& from, const QDate& to)
{
    m_filterFrom = from;
    m_filterTo   = to;
    refresh();
}

void ActivityLogModel::setFilterItemType(const QString& type)
{
    m_filterType = type;
    refresh();
}

void ActivityLogModel::clearFilters()
{
    m_filterType = QString();
    m_filterFrom = QDate();
    m_filterTo   = QDate();
    refresh();
}

void ActivityLogModel::refresh()
{
    beginResetModel();
    m_nameCache.clear();

    QDate from = m_filterFrom.isValid() ? m_filterFrom : QDate(1970, 1, 1);
    QDate to   = m_filterTo.isValid()   ? m_filterTo   : QDate::currentDate().addYears(1);

    QList<ActivityLogEntry> raw = DatabaseManager::instance()->getActivityLog(from, to);

    m_entries.clear();
    m_entries.reserve(raw.size());
    for (const ActivityLogEntry& entry : raw) {
        if (passesTypeFilter(entry))
            m_entries.append(entry);
    }

    endResetModel();
}

bool ActivityLogModel::passesTypeFilter(const ActivityLogEntry& entry) const
{
    if (m_filterType.isEmpty()) return true;
    return entry.type.compare(m_filterType, Qt::CaseInsensitive) == 0;
}

QMap<QDate, int> ActivityLogModel::getDailyProgressTotals(const QDate& from, const QDate& to) const
{
    QMap<QDate, int> totals;
    QList<ActivityLogEntry> rows = DatabaseManager::instance()->getActivityLog(from, to);
    for (const ActivityLogEntry& e : rows) {
        if (!passesTypeFilter(e)) continue;
        totals[e.timestamp.date()] += e.progressDelta;
    }
    return totals;
}

QMap<QDate, int> ActivityLogModel::getDailyActivityCounts(const QDate& from, const QDate& to) const
{
    QMap<QDate, int> counts;
    QList<ActivityLogEntry> rows = DatabaseManager::instance()->getActivityLog(from, to);
    for (const ActivityLogEntry& e : rows) {
        if (!passesTypeFilter(e)) continue;
        counts[e.timestamp.date()] += 1;
    }
    return counts;
}

QString ActivityLogModel::itemName(int itemId) const
{
    auto it = m_nameCache.find(itemId);
    if (it != m_nameCache.end()) return it.value();

    // SessionsTasks.ItemName lives in that table — look it up directly.
    QSqlQuery q(QSqlDatabase::database());
    q.prepare(QStringLiteral("SELECT Name FROM SessionsTasks WHERE ID = :id"));
    q.bindValue(QStringLiteral(":id"), itemId);

    QString name;
    if (q.exec() && q.next())
        name = q.value(0).toString();

    m_nameCache.insert(itemId, name);
    return name;
}
