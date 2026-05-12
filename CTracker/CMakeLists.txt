cmake_minimum_required(VERSION 3.20)
project(CTracker VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt 6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql Test)

enable_testing()

# Source files
set(SOURCES
    src/main.cpp
    src/DatabaseManager.cpp
    src/ActivityLogModel.cpp
    src/DataImporter.cpp
    src/DataExporter.cpp
)

# Header files (moc will process these automatically via AUTOMOC)
set(HEADERS
    include/DataStructures.h
    include/DatabaseManager.h
    include/ActivityLogModel.h
    include/DataImporter.h
    include/DataExporter.h
)

# Create executable
qt_add_executable(CTracker
    MANUAL_FINALIZATION
    ${SOURCES}
    ${HEADERS}
    resources/resources.qrc
)

# Include directories
target_include_directories(CTracker PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Link Qt libraries
target_link_libraries(CTracker PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
)

add_subdirectory(tests)
