cmake_minimum_required(VERSION 3.28)
project(SPS2024)

set(CMAKE_CXX_STANDARD 23)

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
        C/ThreadPool.h
        C/ThreadPool.cpp
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
)
