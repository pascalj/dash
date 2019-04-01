cmake_minimum_required(VERSION 3.1)
project(PatternTests)

find_package(GTest REQUIRED)
find_package(Threads REQUIRED)
find_package(DASH-MPI REQUIRED)

set(DASH_INCLUDE_DIR ${DASH_PATH}/include)
set(DASH_LIBRARY_DIR ${DASH_PATH}/lib)
set(CMAKE_BUILD_TYPE Debug)

include_directories(${GTEST_INCLUDE_DIRS} "../include")

add_executable(tests local_pattern_test.cxx)

target_link_libraries(tests ${GTEST_BOTH_LIBRARIES} Threads::Threads ${DASH_LIBRARIES})
