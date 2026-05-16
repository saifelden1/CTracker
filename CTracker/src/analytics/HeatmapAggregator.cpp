#include "analytics/HeatmapAggregator.h"

#include "core/DatabaseManager.h"
#include "core/DataStructures.h"

#include <algorithm>

// Task 5.6: HeatmapAggregator implementation.
// Aggregates ActivityLog + completed Todos + completed Pomodoro sessions.

HeatmapAggregator::HeatmapAggregator(QObject* parent)
    : QObject(parent) {
}

QMap<QDate, ContributionHeatmap::DayData> HeatmapAggregator::aggregate(
    const QDate& from,
    const QDate& to,
    Mode mode) {

    QMap<QDate, ContributionHeatmap::DayData> result;
    auto* db = DatabaseManager::instance();

    // Initialize all dates in range with zero counts
    for (QDate d = from; d <= to; d = d.addDays(1)) {
        ContributionHeatmap::DayData dayData;
        dayData.date = d;
        dayData.totalProgress = 0;
        dayData.activityCount = 0;
        dayData.intensityLevel = 0;
        result.insert(d, dayData);
    }

    // ── Aggregate ActivityLog entries ──
    const QList<ActivityLogEntry> logEntries = db->getActivityLog(from, to);
    for (const ActivityLogEntry& entry : logEntries) {
        const QDate date = entry.timestamp.date();
        if (result.contains(date)) {
            result[date].totalProgress += entry.progressDelta;
            result[date].activityCount += 1;
        }
    }

    // ── Aggregate completed Todos ──
    // Count todos completed on each day
    for (QDate d = from; d <= to; d = d.addDays(1)) {
        const int completedCount = db->countCompletedTodosOn(d);
        if (result.contains(d)) {
            result[d].activityCount += completedCount;
        }
    }

    // ── Aggregate completed Pomodoro sessions ──
    for (QDate d = from; d <= to; d = d.addDays(1)) {
        const QList<PomodoroSessionData> sessions = db->fetchSessionsOn(d);
        if (result.contains(d)) {
            result[d].activityCount += sessions.size();
        }
    }

    // ── Compute intensity levels based on mode ──
    if (mode == Mode::RecentBuckets) {
        // Bucketed intensity: 0, 1, 2-3, 4-6, 7+
        for (auto it = result.begin(); it != result.end(); ++it) {
            it->intensityLevel = computeIntensityBucketed(it->activityCount);
        }
    } else {
        // Normalized intensity: floor((count / max) * 4) bounded [0,4]
        int maxCount = 0;
        for (const auto& dayData : result) {
            maxCount = std::max(maxCount, dayData.activityCount);
        }
        for (auto it = result.begin(); it != result.end(); ++it) {
            it->intensityLevel = computeIntensityNormalized(it->activityCount, maxCount);
        }
    }

    return result;
}

int HeatmapAggregator::computeIntensityBucketed(int count) const {
    if (count == 0) return 0;
    if (count == 1) return 1;
    if (count <= 3) return 2;
    if (count <= 6) return 3;
    return 4;  // 7+
}

int HeatmapAggregator::computeIntensityNormalized(int count, int maxCount) const {
    if (count == 0 || maxCount == 0) return 0;
    const double ratio = static_cast<double>(count) / maxCount;
    const int intensity = static_cast<int>(ratio * 4.0);
    return std::clamp(intensity, 0, 4);
}
