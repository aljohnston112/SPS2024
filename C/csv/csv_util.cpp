#include <sstream>

#include "csv_util.h"
#include "../csvmonkey/CsvCell.h"
#include "../csvmonkey/CsvReader.h"
#include "../csvmonkey/MappedFileCursor.h"

namespace CSV {
    std::string ColumnToString(const Column column) {
        switch (column) {
        case month:
            return "month";
        case day:
            return "day";
        case open:
            return "open";
        case high:
            return "high";
        case low:
            return "low";
        case close:
            return "close";
        case volume:
            return "volume";
        default:
            throw std::exception();
        }
    }

    std::vector<std::vector<double>> readStockCSV(
        const std::string& filename
    ) {
        csvmonkey::MappedFileCursor file{};
        file.open(filename.c_str());
        csvmonkey::CsvReader reader(file);
        reader.read_row();

        const auto& row = reader.row();
        const csvmonkey::CsvCell* dateCell =
            row.with_column_name("<DATE>");
        const csvmonkey::CsvCell* openCell =
            row.with_column_name("<OPEN>");
        const csvmonkey::CsvCell* highCell =
            row.with_column_name("<HIGH>");
        const csvmonkey::CsvCell* lowCell =
            row.with_column_name("<LOW>");
        const csvmonkey::CsvCell* closeCell =
            row.with_column_name("<CLOSE>");
        const csvmonkey::CsvCell* volumeCell =
            row.with_column_name("<VOL>");

        auto monthVector = std::vector<double>{};
        auto dayVector = std::vector<double>{};
        auto openVector = std::vector<double>{};
        auto highVector = std::vector<double>{};
        auto lowVector = std::vector<double>{};
        auto closeVector = std::vector<double>{};
        auto volumeVector = std::vector<double>{};

        while (reader.read_row()) {
            const char* date = dateCell->ptr;

            int number{};
            std::from_chars(
                date + 6,
                date + 8,
                number
            );
            dayVector.push_back(number);

            std::from_chars(
                date + 4,
                date + 6,
                number
            );
            monthVector.push_back(number);

            openVector.emplace_back(
                openCell->from_chars_as_type<double>()
            );

            highVector.emplace_back(
                highCell->from_chars_as_type<double>()
            );

            lowVector.emplace_back(
                lowCell->from_chars_as_type<double>()
            );

            closeVector.emplace_back(
                closeCell->from_chars_as_type<double>()
            );

            volumeVector.emplace_back(
                volumeCell->from_chars_as_type<double>()
            );
        }

        std::vector<std::vector<double>> data{};
        data.emplace_back(monthVector);
        data.emplace_back(dayVector);
        data.emplace_back(openVector);
        data.emplace_back(highVector);
        data.emplace_back(lowVector);
        data.emplace_back(closeVector);
        data.emplace_back(volumeVector);
        return data;
    }

    std::string extract_symbol(const std::string& path) {
        std::string symbol;
        if (
            const size_t lastSlashPosition = path.find_last_of('/');
            lastSlashPosition != std::string::npos
        ) {
            symbol = path.substr(lastSlashPosition + 1);
            if (
                const size_t dotPosition = symbol.find('.');
                dotPosition != std::string::npos
            ) {
                symbol = symbol.substr(0, dotPosition);
            }
            return symbol;
        }
        return symbol;
    }
}
