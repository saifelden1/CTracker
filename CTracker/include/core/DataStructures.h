#pragma once

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QDate>
#include <QList>
#include <QColor>

// ============================================================
//  DataStructures.h
//
//  PLAIN data containers (POD-style). NO logic, NO ownership,
//  NO QObject inheritance. These are the project's shared
//  vocabulary — DatabaseManager fills them in, models hold them,
//  views render them.
//
//  RULE: every field has a sensible default so that
//  `EntityData e;` (or any other struct) yields a valid-but-empty
//  instance that the rest of the code can safely treat as
//  "the no-record case" without segfaulting.
//
//  Hierarchy of stored records:
//    EntityData  (Course or Project)
//      └── UnitData
//            └── SessionTaskData
//                  └── ActivityLogEntry (one per slider move)
//
//  v2 additions (Phase 3) live below the v1 section so the file
//  reads chronologically and a reader unfamiliar with the project
//  can ramp up linearly.
// ============================================================


// ─────────────────────────────────────────────────────────────
//  v1 — original data shapes (Phase 3.1)
// ─────────────────────────────────────────────────────────────

// Represents a Course or a Project stored in CoursesProjects.
//
// v2 extension: the four trailing fields (categoryId..categoryColor +
// status) come from a LEFT JOIN onto the Categories table performed
// inside DatabaseManager::fetchAll*. They are populated on read; the
// UI never has to look the category up itself.
struct EntityData {
    int       id              = -1;
    QString   name;
    QString   type;                       // "Course" | "Project"
    QDateTime createdAt;
    int       overallProgress = 0;        // computed, not stored

    // v2 fields (Task 3.2)
    int       categoryId      = -1;       // -1 = uncategorised
    QString   status          = "active"; // "active" | "paused" | "completed"
    QString   categoryName;               // empty when categoryId == -1
    QColor    categoryColor;              // invalid() when categoryId == -1
};

// A Unit is a sub-grouping inside a Course or Project (e.g. a unit
// of a course, or a phase of a project).
struct UnitData {
    int     id       = -1;
    int     parentId = -1;                // FK → CoursesProjects.ID
    QString name;
};

// A Session (course flavour) or Task (project flavour). The terminal
// leaf of the hierarchy — this is what the user actually moves the
// progress slider on.
//
// Phase 10 additions (`status`, `dueDate`, `unitName`) are populated for
// the new Projects kanban board. Course code paths leave them at their
// defaults and keep driving the row from `progress`. `unitName` is
// resolved by a JOIN in `fetchTasksForProject()` so the board can
// render the unit-tag chip without a second query.
struct SessionTaskData {
    int     id       = -1;
    int     unitId   = -1;                // FK → Units.ID
    QString name;
    int     progress = 0;                 // 0..100

    // Phase 10 — Projects tasks board
    QString status   = "todo";            // "todo" | "in_progress" | "review" | "done"
    QDate   dueDate;                      // invalid() when unset
    QString unitName;                     // resolved on read for project queries
    QString description;                  // free-form, multi-line allowed
};

// One row of ActivityLog — written every time a slider value changes.
// Powers the contribution heatmap and the recent-activity feed.
struct ActivityLogEntry {
    int       id            = -1;
    int       itemId        = -1;         // FK → SessionsTasks.ID
    QDateTime timestamp;
    int       oldValue      = 0;
    int       newValue      = 0;
    int       progressDelta = 0;
    QString   type;                       // "Course" | "Project"
};

// One day's worth of activity, as consumed by the heatmap widget.
struct HeatmapDataPoint {
    QDate date;
    int   totalProgress  = 0;             // sum of progress deltas that day
    int   activityCount  = 0;             // number of activity log rows
    int   intensityLevel = 0;             // 0..4 → 5-step colour ramp
};


// ─────────────────────────────────────────────────────────────
//  v2 — new data shapes (Phase 3.3)
//
//  These mirror the new tables added in Phase 2.9 plus a few
//  cross-cutting types (filters, persisted timer state) that don't
//  correspond 1:1 to a table.
// ─────────────────────────────────────────────────────────────

// One row of the Categories table.
// `entityCount` is *derived* by a LEFT JOIN onto CoursesProjects in
// fetchAllCategories(); it is not stored.
struct CategoryData {
    int     id          = -1;
    QString name;
    QColor  color;                        // invalid() when default-constructed
    int     entityCount = 0;
};

// The 1-to-1 sidecar of a Project row in CoursesProjects.
// Nested `Link` keeps the {label, url} pair together without bleeding
// a name into the file's top-level namespace.
struct ProjectMetaData {
    struct Link {
        QString label;
        QString url;
    };

    int             projectId = -1;       // PK and FK to CoursesProjects.ID
    QString         description;
    QString         priority   = "medium";// "high" | "medium" | "low"
    QDate           deadline;             // invalid() when unset
    QStringList     team;
    QList<Link>     links;
};

// One row of the Todos table.
struct TodoData {
    int       id        = -1;
    QString   title;
    bool      completed = false;
    QString   priority  = "medium";       // "high" | "medium" | "low"
    QDateTime createdAt;
    QDateTime completedAt;                // invalid() when completed == false
};

// One row of PomodoroSessions. `courseName` is *resolved on read* by
// joining CoursesProjects so the UI doesn't need a second query.
// `courseId == -1` represents a session not linked to any course
// (CourseID was NULL or was nulled when the course was deleted).
struct PomodoroSessionData {
    int       id              = -1;
    int       courseId        = -1;
    QString   courseName;
    int       durationMinutes = 0;
    QDateTime completedAt;
    QString   mode            = "work";   // "work" | "break"
};

// One row of CalendarDayDetails. The three list fields come from
// JSON arrays stored in the corresponding TEXT columns.
struct CalendarDayData {
    QDate       date;
    QStringList todo;
    QStringList completed;
    QString     notes;

    // Convenience used by the day-cell indicator dot: any of the
    // three sections having content makes the dot appear.
    bool hasContent() const {
        return !todo.isEmpty() || !completed.isEmpty() || !notes.isEmpty();
    }
};

// Aggregate computed by analytics — not stored, recomputed on demand.
// Built by `analytics/AnalyticsSummary` free function (Task 5.7).
struct AnalyticsSummary {
    int    currentStreakDays    = 0;
    int    longestStreakDays    = 0;
    int    monthHoursStudied    = 0;
    double avgSessionsPerDay7d  = 0.0;
    double weekOverWeekPct      = 0.0;
};


// ─────────────────────────────────────────────────────────────
//  Filter / state structs (Phase 3.4) — NEW design gap-fills
//
//  These do NOT correspond to a table. They exist because models
//  and views need to share them. Promoting them out of any single
//  widget means the same struct can travel from a filter bar to a
//  view to a model adapter without forcing an awkward back-reference
//  on the widget header.
// ─────────────────────────────────────────────────────────────

// Emitted by CoursesFilterBar (Task 6.13), consumed by CoursesView.
// `categoryId == -1` means "show all categories"; `status == "all"`
// likewise means "any status".
struct CourseFilter {
    QString search;                       // case-insensitive substring of Name
    int     categoryId = -1;
    QString status     = "all";           // "all" | "active" | "paused" | "completed"
};

// Same idea for ProjectsView. Priority uses the same vocabulary as
// ProjectMetaData::priority.
struct ProjectFilter {
    QString search;
    QString priority = "all";             // "all" | "high" | "medium" | "low"
    QString status   = "all";             // "all" | "active" | "paused" | "completed"
};

// Persisted pomodoro timer state. Lives in the Settings table under
// reserved keys with the `pomodoro.state.` prefix (see Task 4.9).
//
// WHY a struct and not just a few global ints? Because we need to
// restore the *full* timer on app restart — the React reference
// design forgets state on reload; we want better. Storing
// `startedAt` lets us compute remainingSeconds on resume even after
// a long wall-clock gap.
struct PomodoroTimerState {
    enum Mode  { Work, Break };
    enum State { Idle, Running, Paused };

    Mode      mode             = Work;
    State     state            = Idle;
    int       courseId         = -1;      // -1 = "free" / uncategorised
    int       totalSeconds     = 25 * 60; // mirrors Settings.pomodoro.workMinutes
    int       remainingSeconds = 25 * 60;
    QDateTime startedAt;                  // invalid() when state == Idle
};

// Typed wrappers over Settings k/v rows (Task 4.8).
// SettingsView passes these around so its widgets never see raw strings.
struct ProfileData {
    QString name;
    QString email;
    QString goals;
};

struct PreferencesData {
    int  workMinutes   = 25;
    int  breakMinutes  = 5;
    bool notifications = true;
    bool sound         = true;
    int  autoPauseDays = 30;              // 0 = never auto-pause
};
