cmake_minimum_required(VERSION 3.1)
project(PatternTests)

find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

include_directories(${GTEST_INCLUDE_DIRS} "../include")

add_executable(tests local_pattern_test.cxx)

target_link_libraries(tests ${GTEST_BOTH_LIBRARIES} Threads::Threads)
