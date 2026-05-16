cmake_minimum_required(VERSION 3.20)
project(CTracker VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt 6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql Test)

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
    src/shared/CircularProgressBar.cpp
    src/shared/SideNavigationBar.cpp
    src/shared/HomeDashboard.cpp
    src/shared/MainWindow.cpp

    # courses/ — course / entity detail surface
    src/courses/SessionTaskRow.cpp
    src/courses/UnitExpandableWidget.cpp
    src/courses/EntityCard.cpp
    src/courses/EntityDetailView.cpp

    # analytics/
    src/analytics/ActivityLogModel.cpp
    src/analytics/ContributionHeatmap.cpp
    src/analytics/AnalyticsView.cpp

    # settings/
    src/settings/SettingsView.cpp
)

# Header files (AUTOMOC processes these)
set(HEADERS
    # core/
    include/core/DataStructures.h
    include/core/DatabaseManager.h
    include/core/DataImporter.h
    include/core/DataExporter.h

    # shared/
    include/shared/CircularProgressBar.h
    include/shared/SideNavigationBar.h
    include/shared/HomeDashboard.h
    include/shared/MainWindow.h

    # courses/
    include/courses/SessionTaskRow.h
    include/courses/UnitExpandableWidget.h
    include/courses/EntityCard.h
    include/courses/EntityDetailView.h
    include/courses/CourseDetailView.h   # alias header

    # projects/
    include/projects/ProjectDetailView.h # alias header

    # analytics/
    include/analytics/ActivityLogModel.h
    include/analytics/ContributionHeatmap.h
    include/analytics/AnalyticsView.h

    # settings/
    include/settings/SettingsView.h
)

# Create executable
qt_add_executable(CTracker
    MANUAL_FINALIZATION
    ${SOURCES}
    ${HEADERS}
    resources/resources.qrc
)

# Include directories — `include/` is the single include root.
target_include_directories(CTracker PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Link Qt libraries
target_link_libraries(CTracker PRIVATE
    Qt6::Core
    Qt6::Gui          # QColor and friends — pulled in by core/DataStructures.h
    Qt6::Widgets
    Qt6::Sql
)

add_subdirectory(tests)
