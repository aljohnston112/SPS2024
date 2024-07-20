#ifndef MAIN_CSV_UTIL_H
#define MAIN_CSV_UTIL_H

#include <string>
#include <vector>
#include <map>
#include "csvmonkey/MappedFileCursor.h"
#include "csvmonkey/CsvReader.h"
#include "csvmonkey/CsvCell.h"

namespace CSV {

    enum Column {
        month,
        day,
        open,
        high,
        low,
        close,
        volume
    };

    std::string ColumnToString(Column col);

    std::map<Column, std::vector<double>> readStockCSV(
            const std::string &filename
    );

    std::string extract_symbol(const std::string &path);

    template<class T>
    std::map<Column, std::vector<T>> readCSV(
            const std::string &filename
    ) {

        csvmonkey::MappedFileCursor file{};
        file.open(filename.c_str());
        csvmonkey::CsvReader reader(file);
        reader.read_row();
        csvmonkey::CsvCell *monthCell =
                reader.row().by_value("month");
        csvmonkey::CsvCell *dayCell =
                reader.row().by_value("day");
        csvmonkey::CsvCell *openCell =
                reader.row().by_value("open");
        csvmonkey::CsvCell *highCell =
                reader.row().by_value("high");
        csvmonkey::CsvCell *lowCell =
                reader.row().by_value("low");
        csvmonkey::CsvCell *closeCell =
                reader.row().by_value("close");
        csvmonkey::CsvCell *volumeCell =
                reader.row().by_value("volume");

        auto monthVector = std::vector<T>{};
        auto dayVector = std::vector<T>{};
        auto openVector = std::vector<T>{};
        auto highVector = std::vector<T>{};
        auto lowVector = std::vector<T>{};
        auto closeVector = std::vector<T>{};
        auto volumeVector = std::vector<T>{};

        T(csvmonkey::CsvCell::*t)() const;
        if constexpr (std::is_same<T, int>::value) {
            t = &csvmonkey::CsvCell::as_int;
        } else if constexpr (std::is_same<T, double>::value) {
            t = &csvmonkey::CsvCell::as_double;
        }
        while (reader.read_row()) {
            monthVector.push_back(
                    (monthCell->*t)()
            );
            dayVector.push_back(
                    (dayCell->*t)()
            );

            openVector.emplace_back(
                    (openCell->*t)()
            );

            highVector.emplace_back(
                    (highCell->*t)()
            );

            lowVector.emplace_back(
                    (lowCell->*t)()
            );

            closeVector.emplace_back(
                    (closeCell->*t)()
            );

            volumeVector.emplace_back(
                    (volumeCell->*t)()
            );
        }

        std::map<Column, std::vector<T>> data{};
        data.emplace(std::pair{month, monthVector});
        data.emplace(std::pair{day, dayVector});
        data.emplace(std::pair{open, openVector});
        data.emplace(std::pair{high, highVector});
        data.emplace(std::pair{low, lowVector});
        data.emplace(std::pair{close, closeVector});
        data.emplace(std::pair{volume, volumeVector});
        return data;
    }

}

#endif //MAIN_CSV_UTIL_H
