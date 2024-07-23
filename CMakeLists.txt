cmake_minimum_required(VERSION 3.28)
project(SPS2024)

set(CMAKE_CXX_STANDARD 26)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -msse4.2")
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    message("Build type is Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O3")
elseif (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("Build type is Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
else ()
    message("Unknown build type: ${CMAKE_BUILD_TYPE}")
endif ()

set(
        SOURCE_FILES
        C/main.cpp
        C/csv/csv_util.cpp
        C/csv/data_sanitizer.cpp
        C/csv/file_util.cpp
        C/directions/directions.cpp
        C/thread_pool.cpp
        C/csvmonkey/CsvCell.cpp
        C/csvmonkey/CsvCursor.cpp
        C/csvmonkey/CsvWriter.cpp
        C/csvmonkey/MappedFileCursor.cpp
)

set(
        HEADER_FILES
        C/csv/csv_util.h
        C/csv/data_sanitizer.h
        C/csv/file_util.h
        C/directions/directions.h
        C/thread_pool.h
        C/csvmonkey/MappedFileCursor.h
        C/csvmonkey/CsvCell.h
        C/csvmonkey/StringSpanner.h
        C/csvmonkey/CsvCursor.h
        C/csvmonkey/CsvReader.h
        C/csvmonkey/CsvWriter.h
        C/config.h
        C/timer.h
        C/tree/ProbabilityTree.h
)


add_executable(
        SPS2024
        ${SOURCE_FILES}
        ${HEADER_FILES}
        C/main2.cpp
)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/e397860.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
        SPS2024Test
        test/NodeTest.cpp
        ${SOURCE_FILES}
        ${HEADER_FILES}
)

target_link_libraries(
        SPS2024Test
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(SPS2024Test)