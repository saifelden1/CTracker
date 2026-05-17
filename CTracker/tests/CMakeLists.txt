# Tests for CTracker — QtTest framework.
#
# Each test_*.cpp file becomes its own executable so a crash in one
# test doesn't take down the rest. add_test() registers it with CTest
# so `ctest` from the build dir runs everything.
#
# Paths reflect the Phase 0 feature-folder layout.

set(CTRACKER_TEST_SOURCES
    test_databasemanager_v1.cpp
    test_databasemanager_v2.cpp
    test_models.cpp
    test_widgets.cpp
)

# Headers / sources from the main app that tests need to link against.
# Kept here (not in a library yet) to avoid restructuring the main target.
set(CTRACKER_LIB_SOURCES
    ${CMAKE_SOURCE_DIR}/src/core/DatabaseManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/DataImporter.cpp
    ${CMAKE_SOURCE_DIR}/src/core/DataExporter.cpp
    ${CMAKE_SOURCE_DIR}/src/analytics/ActivityLogModel.cpp
    ${CMAKE_SOURCE_DIR}/src/analytics/HeatmapAggregator.cpp
    ${CMAKE_SOURCE_DIR}/src/analytics/ContributionHeatmap.cpp
    ${CMAKE_SOURCE_DIR}/src/todos/TodoModel.cpp
    ${CMAKE_SOURCE_DIR}/src/shared/CategoryModel.cpp
    ${CMAKE_SOURCE_DIR}/src/shared/CircularProgressBar.cpp
    ${CMAKE_SOURCE_DIR}/src/courses/UnitExpandableWidget.cpp
    ${CMAKE_SOURCE_DIR}/src/courses/SessionTaskRow.cpp
    ${CMAKE_SOURCE_DIR}/src/pomodoro/PomodoroTimerWidget.cpp
)
set(CTRACKER_LIB_HEADERS
    ${CMAKE_SOURCE_DIR}/include/core/DatabaseManager.h
    ${CMAKE_SOURCE_DIR}/include/core/DataStructures.h
    ${CMAKE_SOURCE_DIR}/include/core/DataImporter.h
    ${CMAKE_SOURCE_DIR}/include/core/DataExporter.h
    ${CMAKE_SOURCE_DIR}/include/analytics/ActivityLogModel.h
    ${CMAKE_SOURCE_DIR}/include/analytics/HeatmapAggregator.h
    ${CMAKE_SOURCE_DIR}/include/analytics/ContributionHeatmap.h
    ${CMAKE_SOURCE_DIR}/include/todos/TodoModel.h
    ${CMAKE_SOURCE_DIR}/include/shared/CategoryModel.h
    ${CMAKE_SOURCE_DIR}/include/shared/CircularProgressBar.h
    ${CMAKE_SOURCE_DIR}/include/courses/UnitExpandableWidget.h
    ${CMAKE_SOURCE_DIR}/include/courses/SessionTaskRow.h
    ${CMAKE_SOURCE_DIR}/include/pomodoro/PomodoroTimerWidget.h
)

foreach(test_src ${CTRACKER_TEST_SOURCES})
    get_filename_component(test_name ${test_src} NAME_WE)
    qt_add_executable(${test_name}
        ${test_src}
        ${CTRACKER_LIB_SOURCES}
        ${CTRACKER_LIB_HEADERS}
    )
    # Output test executables to the build root so they can find Qt DLLs
    # and platform plugins that are deployed alongside CTracker.exe.
    set_target_properties(${test_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    )
    target_include_directories(${test_name} PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(${test_name} PRIVATE
        Qt6::Core
        Qt6::Gui          # QColor — core/DataStructures.h needs it
        Qt6::Widgets      # Widget tests need this
        Qt6::Sql
        Qt6::Test
    )
    # Use offscreen platform so tests don't need a real display
    add_test(NAME ${test_name} COMMAND ${test_name})
    set_tests_properties(${test_name} PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )
endforeach()
