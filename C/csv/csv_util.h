#ifndef MAIN_CSV_UTIL_H
#define MAIN_CSV_UTIL_H

#include <string>
#include <vector>

#include "../csvmonkey/CsvCell.h"
#include "../csvmonkey/CsvReader.h"
#include "../csvmonkey/MappedFileCursor.h"

namespace CSV {
        enum Column: int {
                month = 0,
                day = 1,
                open = 2,
                high = 3,
                low = 4,
                close = 5,
                volume = 6
        };

        std::string ColumnToString(Column column);

        std::vector<std::vector<double>> readStockCSV(
                const std::string& filename
        );

        std::string extract_symbol(const std::string& path);

        template <class T>
        std::vector<std::vector<T>> readCSV(
                const std::string& filename
        ) {
                csvmonkey::MappedFileCursor file{};
                file.open(filename.c_str());
                csvmonkey::CsvReader reader(file);
                reader.read_row();

                const auto& row = reader.row();
                const csvmonkey::CsvCell* monthCell =
                        row.with_column_name("month");
                const csvmonkey::CsvCell* dayCell =
                        row.with_column_name("day");
                const csvmonkey::CsvCell* openCell =
                        row.with_column_name("open");
                const csvmonkey::CsvCell* highCell =
                        row.with_column_name("high");
                const csvmonkey::CsvCell* lowCell =
                        row.with_column_name("low");
                const csvmonkey::CsvCell* closeCell =
                        row.with_column_name("close");
                const csvmonkey::CsvCell* volumeCell =
                        row.with_column_name("volume");

                auto monthVector = std::vector<T>{};
                auto dayVector = std::vector<T>{};
                auto openVector = std::vector<T>{};
                auto highVector = std::vector<T>{};
                auto lowVector = std::vector<T>{};
                auto closeVector = std::vector<T>{};
                auto volumeVector = std::vector<T>{};

                T (csvmonkey::CsvCell::*t)() const;
                if constexpr (std::is_same_v<T, int>) {
                        t = &csvmonkey::CsvCell::from_chars_as_type<int>;
                }
                else if constexpr (std::is_same_v<T, double>) {
                        t = &csvmonkey::CsvCell::from_chars_as_type<double>;
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

                std::vector<std::vector<T>> data{};
                data.emplace_back(monthVector);
                data.emplace_back(dayVector);
                data.emplace_back(openVector);
                data.emplace_back(highVector);
                data.emplace_back(lowVector);
                data.emplace_back(closeVector);
                data.emplace_back(volumeVector);
                return data;
        }
}

#endif //MAIN_CSV_UTIL_H
