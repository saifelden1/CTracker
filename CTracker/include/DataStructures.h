#pragma once

#include <QString>
#include <QDateTime>
#include <QDate>
#include <QList>

// ============================================================
//  DataStructures.h
//  Plain data containers (no logic). These are used to pass
//  data between DatabaseManager and the UI widgets.
// usess sql lite to map the data types we made here to there relative places 
//every data type is linked ot ite parent by an id 
//so the entrydata ->unit daya-> session data
// ============================================================

// Represents a Course or a Project stored in CoursesProjects table
struct EntityData {
    int     id;
    QString name;
    QString type;           // "Course" or "Project"
    QDateTime createdAt;
    int     overallProgress = 0; // Calculated from children, not stored
};

// Represents a Unit belonging to a Course or Project
struct UnitData {
    int     id;
    int     parentId;       // ID of the parent Course/Project
    QString name;
};

// Represents a single Session (inside a Course) or Task (inside a Project)
struct SessionTaskData {
    int     id;
    int     unitId;         // ID of the parent Unit
    QString name;
    int     progress;       // 0 to 100
};

// Represents one row in the ActivityLog table
struct ActivityLogEntry {
    int       id;
    int       itemId;       // FK → SessionsTasks.ID
    QDateTime timestamp;
    int       oldValue;
    int       newValue;
    int       progressDelta;
    QString   type;         // "Course" or "Project"
};

// Represents one day in the contribution heatmap
struct HeatmapDataPoint {
    QDate date;
    int   totalProgress  = 0;   // Sum of progress deltas for the day
    int   activityCount  = 0;   // Number of activity log entries for the day
    int   intensityLevel = 0;   // 0..4 → maps to the 5-step color gradient
};
