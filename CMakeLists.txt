cmake_minimum_required(VERSION 3.28)
project(SPS2024)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -msse4.2")
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message("Build type is Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O3")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("Build type is Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
else()
    message("Unknown build type: ${CMAKE_BUILD_TYPE}")
endif()

add_executable(
        SPS2024
        C/main.cpp
        C/csv/csv_util.cpp
        C/csv/csv_util.h
        C/csv/data_sanitizer.cpp
        C/csv/data_sanitizer.h
        C/csv/file_util.cpp
        C/csv/file_util.h
        C/directions/directions.cpp
        C/directions/directions.h
        C/thread_pool.h
        C/thread_pool.cpp
        C/csvmonkey/MappedFileCursor.h
        C/csvmonkey/CsvCell.h
        C/csvmonkey/StringSpanner.h
        C/csvmonkey/CsvCursor.h
        C/csvmonkey/CsvReader.h
        C/csvmonkey/CsvWriter.h
        C/csvmonkey/CsvCell.cpp
        C/csvmonkey/CsvCursor.cpp
        C/csvmonkey/CsvWriter.cpp
        C/csvmonkey/MappedFileCursor.cpp
        C/csvmonkey/StringSpanner.cpp
        C/config.h
        C/timer.h
        C/tree/Node.cpp
        C/tree/Node.h
)
