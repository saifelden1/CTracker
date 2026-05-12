# Tests for CTracker — QtTest framework.
#
# Each test_*.cpp file becomes its own executable so a crash in one
# test doesn't take down the rest. add_test() registers it with CTest
# so `ctest` from the build dir runs everything.

set(CTRACKER_TEST_SOURCES
    # test_databasemanager.cpp  # placeholder — Phase 9 will populate
)

# Headers / sources from the main app that tests need to link against.
# Kept here (not in a library yet) to avoid restructuring the main target.
set(CTRACKER_LIB_SOURCES
    ${CMAKE_SOURCE_DIR}/src/DatabaseManager.cpp
    ${CMAKE_SOURCE_DIR}/src/ActivityLogModel.cpp
    ${CMAKE_SOURCE_DIR}/src/DataImporter.cpp
    ${CMAKE_SOURCE_DIR}/src/DataExporter.cpp
)
set(CTRACKER_LIB_HEADERS
    ${CMAKE_SOURCE_DIR}/include/DatabaseManager.h
    ${CMAKE_SOURCE_DIR}/include/DataStructures.h
    ${CMAKE_SOURCE_DIR}/include/ActivityLogModel.h
    ${CMAKE_SOURCE_DIR}/include/DataImporter.h
    ${CMAKE_SOURCE_DIR}/include/DataExporter.h
)

foreach(test_src ${CTRACKER_TEST_SOURCES})
    get_filename_component(test_name ${test_src} NAME_WE)
    qt_add_executable(${test_name}
        ${test_src}
        ${CTRACKER_LIB_SOURCES}
        ${CTRACKER_LIB_HEADERS}
    )
    target_include_directories(${test_name} PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(${test_name} PRIVATE
        Qt6::Core
        Qt6::Sql
        Qt6::Test
    )
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
