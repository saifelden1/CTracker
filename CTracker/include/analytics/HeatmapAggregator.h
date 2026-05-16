#pragma once

#include <QObject>
#include <QMap>
#include <QDate>

#include "analytics/ContributionHeatmap.h"

// Task 5.6: HeatmapAggregator — aggregates activity data from multiple sources
// (ActivityLog, Todos, PomodoroSessions) into ContributionHeatmap::DayData.
//
// Two modes:
// - RecentBuckets: intensity bucketed 0 / 1 / 2-3 / 4-6 / 7+ (for HomeDashboard)
// - NormalizedRange: intensity = floor((count / max) * 4) bounded [0,4] (for AnalyticsView)
class HeatmapAggregator : public QObject {
    Q_OBJECT
public:
    enum class Mode {
        RecentBuckets,    // Bucket-based: 0, 1, 2-3, 4-6, 7+
        NormalizedRange   // Normalized: floor((count / max) * 4)
    };

    explicit HeatmapAggregator(QObject* parent = nullptr);

    QMap<QDate, ContributionHeatmap::DayData> aggregate(const QDate& from,
                                                         const QDate& to,
                                                         Mode mode);

private:
    int computeIntensityBucketed(int count) const;
    int computeIntensityNormalized(int count, int maxCount) const;
};
