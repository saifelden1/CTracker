#pragma once

#include "core/DataStructures.h"

// Task 5.7: AnalyticsSummaryComputer — computes aggregate statistics
// from DatabaseManager queries for display in AnalyticsView's StatsCards.
class AnalyticsSummaryComputer {
public:
    static AnalyticsSummary compute();
};
