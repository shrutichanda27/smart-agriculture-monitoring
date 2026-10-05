cmake_minimum_required(VERSION 3.10)
project(smart_agriculture_monitoring CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pthread")

# Find SQLite3 package
find_package(SQLite3 REQUIRED)

# Common library sources
set(COMMON_SOURCES
    src/sensor_processor.cpp
    src/serial_reader.cpp
    src/alert_engine.cpp
    src/database.cpp
    src/dashboard_generator.cpp
    src/http_server.cpp
    src/driver_monitor.cpp
)

# Core library for modularity and test linking
add_library(agri_core STATIC ${COMMON_SOURCES})
target_include_directories(agri_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(agri_core PUBLIC SQLite::SQLite3)

# Main executable
add_executable(smart_agriculture src/main.cpp)
target_link_libraries(smart_agriculture PRIVATE agri_core)

# Tests
enable_testing()

add_executable(parser_test tests/parser_test.cpp)
target_link_libraries(parser_test PRIVATE agri_core)
add_test(NAME parser_test COMMAND parser_test)

add_executable(alert_test tests/alert_test.cpp)
target_link_libraries(alert_test PRIVATE agri_core)
add_test(NAME alert_test COMMAND alert_test)

add_executable(database_test tests/database_test.cpp)
target_link_libraries(database_test PRIVATE agri_core)
add_test(NAME database_test COMMAND database_test)

add_executable(http_server_test tests/http_server_test.cpp)
target_link_libraries(http_server_test PRIVATE agri_core)
add_test(NAME http_server_test COMMAND http_server_test)
