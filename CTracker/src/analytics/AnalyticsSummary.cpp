#include "analytics/AnalyticsSummary.h"
#include "core/DatabaseManager.h"

#include <QDate>
#include <QList>
#include <algorithm>

// Task 5.7: AnalyticsSummaryComputer — computes aggregate statistics
// from DatabaseManager queries. Used by AnalyticsView's StatsCards.

AnalyticsSummary AnalyticsSummaryComputer::compute() {
    AnalyticsSummary summary{};
    auto* db = DatabaseManager::instance();
    const QDate today = QDate::currentDate();

    // ── Day streak ──────────────────────────────────────────
    // Walk backwards from today counting consecutive days with activity.
    // A day has activity if totalMinutesOn > 0 or countCompletedTodosOn > 0.
    {
        int streak = 0;
        QDate d = today;
        while (d.isValid()) {
            const int mins = db->totalMinutesOn(d);
            const int todos = db->countCompletedTodosOn(d);
            if (mins > 0 || todos > 0) {
                ++streak;
                d = d.addDays(-1);
            } else {
                break;
            }
        }
        summary.currentStreakDays = streak;
    }

    // ── Longest streak ──────────────────────────────────────
    // Scan the last 365 days to find the longest consecutive streak.
    {
        int longest = 0;
        int current = 0;
        QDate d = today.addDays(-365);
        while (d <= today) {
            const int mins = db->totalMinutesOn(d);
            const int todos = db->countCompletedTodosOn(d);
            if (mins > 0 || todos > 0) {
                ++current;
                longest = std::max(longest, current);
            } else {
                current = 0;
            }
            d = d.addDays(1);
        }
        summary.longestStreakDays = longest;
    }

    // ── Month hours studied ─────────────────────────────────
    // Sum work-mode pomodoro minutes for the current month, convert to hours.
    {
        const QDate monthStart = QDate(today.year(), today.month(), 1);
        int totalMin = 0;
        QDate d = monthStart;
        while (d <= today) {
            totalMin += db->totalMinutesOn(d);
            d = d.addDays(1);
        }
        summary.monthHoursStudied = totalMin / 60;
    }

    // ── Avg sessions per day (last 7 days) ──────────────────
    {
        int totalSessions = 0;
        QDate d = today.addDays(-6);
        while (d <= today) {
            const QList<PomodoroSessionData> sessions = db->fetchSessionsOn(d);
            for (const PomodoroSessionData& s : sessions) {
                if (s.mode == "work") ++totalSessions;
            }
            d = d.addDays(1);
        }
        summary.avgSessionsPerDay7d = (totalSessions > 0)
            ? static_cast<double>(totalSessions) / 7.0 : 0.0;
    }

    // ── Week-over-week percentage ────────────────────────────
    // Compare this week's work minutes to last week's.
    {
        int thisWeekMin = 0;
        QDate d = today.addDays(-6);
        while (d <= today) {
            thisWeekMin += db->totalMinutesOn(d);
            d = d.addDays(1);
        }

        int lastWeekMin = 0;
        d = today.addDays(-13);
        while (d <= today.addDays(-7)) {
            lastWeekMin += db->totalMinutesOn(d);
            d = d.addDays(1);
        }

        if (lastWeekMin > 0) {
            summary.weekOverWeekPct = static_cast<double>(thisWeekMin - lastWeekMin)
                / static_cast<double>(lastWeekMin) * 100.0;
        } else if (thisWeekMin > 0) {
            summary.weekOverWeekPct = 100.0;  // infinite growth → cap at 100%
        } else {
            summary.weekOverWeekPct = 0.0;
        }
    }

    return summary;
}
