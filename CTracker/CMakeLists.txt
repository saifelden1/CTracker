cmake_minimum_required(VERSION 3.20)
project(CTracker VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt 6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql Charts Svg Test)

enable_testing()

# -----------------------------------------------------------------------------
# Source tree is feature-grouped (Phase 0 reorg).
# Each top-level feature owns a folder under both include/ and src/.
# `include/` stays the single include root, so files reference each other
# as e.g. "core/DatabaseManager.h" or "courses/EntityCard.h".
# -----------------------------------------------------------------------------

# Source files (grouped by feature)
set(SOURCES
    src/main.cpp

    # core/ — data layer
    src/core/DatabaseManager.cpp
    src/core/DataImporter.cpp
    src/core/DataExporter.cpp

    # shared/ — cross-feature widgets and chrome
    src/shared/CategoryModel.cpp
    src/shared/CategoryPill.cpp
    src/shared/CircularProgressBar.cpp
    src/shared/EmptyState.cpp
    src/shared/StatsCard.cpp
    src/shared/SideNavigationBar.cpp
    src/shared/HomeDashboard.cpp
    src/shared/MainWindow.cpp
    src/shared/EntityCreateDialog.cpp

    # calendar/
    src/calendar/CalendarWidget.cpp
    src/calendar/DayDetailsPanel.cpp

    # courses/ — course / entity detail surface
    src/courses/SessionTaskRow.cpp
    src/courses/UnitExpandableWidget.cpp     # still used by ProjectDetailView
    src/courses/EntityCard.cpp
    src/courses/CoursesFilterBar.cpp
    src/courses/CoursesView.cpp
    src/courses/EntityDetailView.cpp
    src/courses/CourseDetailView.cpp
    src/courses/UnitCard.cpp                 # Task 7.5a — units-as-cards
    src/courses/UnitSessionsView.cpp         # Task 7.5a — per-unit sub-view

    # projects/
    src/projects/ProjectCard.cpp
    src/projects/ProjectsFilterBar.cpp
    src/projects/ProjectsView.cpp
    src/projects/ProjectDetailView.cpp
    src/projects/TaskBoardCard.cpp           # Phase 10 — kanban card
    src/projects/TasksBoardWidget.cpp        # Phase 10 — kanban board
    src/projects/TaskDetailDialog.cpp        # Phase 10 — task detail editor

    # pomodoro/
    src/pomodoro/PomodoroTimerWidget.cpp
    src/pomodoro/PomodoroView.cpp

    # analytics/
    src/analytics/ActivityLogModel.cpp
    src/analytics/AnalyticsSummary.cpp
    src/analytics/HeatmapAggregator.cpp
    src/analytics/ContributionHeatmap.cpp
    src/analytics/AnalyticsView.cpp

    # settings/
    src/settings/SettingsView.cpp

    # todos/
    src/todos/TodoModel.cpp
    src/todos/TodoRow.cpp
    src/todos/TodoView.cpp
)

# Header files (AUTOMOC processes these)
set(HEADERS
    # core/
    include/core/DataStructures.h
    include/core/DatabaseManager.h
    include/core/DataImporter.h
    include/core/DataExporter.h

    # shared/
    include/shared/CategoryModel.h
    include/shared/CategoryPill.h
    include/shared/CircularProgressBar.h
    include/shared/EmptyState.h
    include/shared/StatsCard.h
    include/shared/SideNavigationBar.h
    include/shared/HomeDashboard.h
    include/shared/MainWindow.h
    include/shared/EntityCreateDialog.h

    # calendar/
    include/calendar/CalendarWidget.h
    include/calendar/DayDetailsPanel.h

    # courses/
    include/courses/SessionTaskRow.h
    include/courses/UnitExpandableWidget.h
    include/courses/EntityCard.h
    include/courses/CoursesFilterBar.h
    include/courses/CoursesView.h
    include/courses/EntityDetailView.h
    include/courses/CourseDetailView.h
    include/courses/UnitCard.h               # Task 7.5a
    include/courses/UnitSessionsView.h       # Task 7.5a

    # pomodoro/
    include/pomodoro/PomodoroTimerWidget.h
    include/pomodoro/PomodoroView.h

    # projects/
    include/projects/ProjectCard.h
    include/projects/ProjectsFilterBar.h
    include/projects/ProjectsView.h
    include/projects/ProjectDetailView.h # alias header
    include/projects/TaskBoardCard.h         # Phase 10 — kanban card
    include/projects/TasksBoardWidget.h      # Phase 10 — kanban board
    include/projects/TaskDetailDialog.h      # Phase 10 — task detail editor

    # analytics/
    include/analytics/ActivityLogModel.h
    include/analytics/AnalyticsSummary.h
    include/analytics/HeatmapAggregator.h
    include/analytics/ContributionHeatmap.h
    include/analytics/AnalyticsView.h

    # settings/
    include/settings/SettingsView.h

    # todos/
    include/todos/TodoModel.h
    include/todos/TodoRow.h
    include/todos/TodoView.h
)

# Create executable
qt_add_executable(CTracker
    # WIN32 removed temporarily to see console output
    MANUAL_FINALIZATION
    ${SOURCES}
    ${HEADERS}
    resources/resources.qrc
    resources/app_icon.rc
)

# Include directories — `include/` is the single include root.
target_include_directories(CTracker PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Link Qt libraries
target_link_libraries(CTracker PRIVATE
    Qt6::Core
    Qt6::Gui          # QColor and friends — pulled in by core/DataStructures.h
    Qt6::Widgets
    Qt6::Sql
    Qt6::Charts
    Qt6::Svg          # SVG icon rendering (Task 8.4)
)

# Make this a proper Windows GUI app — no console window at launch.
# Without this, the PE subsystem stays "CONSOLE" and launching the exe
# opens an extra empty terminal alongside the window.
#
# With MinGW we cannot use WIN32_EXECUTABLE because Qt's QtEntryPoint
# pulls in the MSVC-only `__imp___argc`/`__imp___argv` symbols which
# don't exist in MinGW. Instead set the linker subsystem directly so
# our `int main(int, char*[])` entrypoint stays the one used.
if (WIN32 AND MINGW)
    target_link_options(CTracker PRIVATE -Wl,-subsystem,windows)
else()
    set_target_properties(CTracker PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# Finalize the executable (required for WIN32 apps)
qt_finalize_target(CTracker)

add_subdirectory(tests)
