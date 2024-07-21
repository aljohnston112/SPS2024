cmake_minimum_required(VERSION 3.28)
project(SPS2024)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -msse4.2")
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message("Build type is Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("Build type is Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
else()
    message("Unknown build type: ${CMAKE_BUILD_TYPE}")
endif()

add_executable(
        SPS2024
        C/main.cpp
        C/csv_util.cpp
        C/csv_util.h
        C/data_sanitizer.cpp
        C/data_sanitizer.h
        C/file_util.cpp
        C/file_util.h
        C/directions.cpp
        C/directions.h
        C/thread_pool.h
        C/thread_pool.cpp
        C/csvmonkey/csvmonkey.hpp
        C/csvmonkey/MappedFileCursor.h
        C/csvmonkey/CsvCell.h
        C/csvmonkey/StringSpanner.h
        C/csvmonkey/CsvCursor.h
        C/csvmonkey/CsvReader.h
        C/csvmonkey/CsvWriter.h
        C/csvmonkey/CSVCell.cpp
        C/csvmonkey/CsvCursor.cpp
        C/csvmonkey/CsvWriter.cpp
        C/csvmonkey/MappedFileCursor.cpp
        C/csvmonkey/StringSpanner.cpp
        C/config.h
        C/timer.h
)
