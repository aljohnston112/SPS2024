#include <algorithm>
#include <memory>
#include <sstream>

#include "csv_util.h"
#include "csvmonkey/CsvCell.h"
#include "csvmonkey/CsvReader.h"
#include "csvmonkey/MappedFileCursor.h"

namespace CSV {


    std::string ColumnToString(Column col) {
        switch (col) {
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

    std::map<Column, std::vector<double>> readStockCSV(
            const std::string &filename
    ) {

        csvmonkey::MappedFileCursor file{};
        file.open(filename.c_str());
        csvmonkey::CsvReader reader(file);
        reader.read_row();
        csvmonkey::CsvCell *dateCell =
                reader.row().by_value("<DATE>");
        csvmonkey::CsvCell *openCell =
                reader.row().by_value("<OPEN>");
        csvmonkey::CsvCell *highCell =
                reader.row().by_value("<HIGH>");
        csvmonkey::CsvCell *lowCell =
                reader.row().by_value("<LOW>");
        csvmonkey::CsvCell *closeCell =
                reader.row().by_value("<CLOSE>");
        csvmonkey::CsvCell *volumeCell =
                reader.row().by_value("<VOL>");

        auto monthVector = std::vector<double>{};
        auto dayVector = std::vector<double>{};
        auto openVector = std::vector<double>{};
        auto highVector = std::vector<double>{};
        auto lowVector = std::vector<double>{};
        auto closeVector = std::vector<double>{};
        auto volumeVector = std::vector<double>{};

        while (reader.read_row()) {
            const char* date = dateCell->ptr;

            int number;
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
                    openCell->as_double()
            );

            highVector.emplace_back(
                    highCell->as_double()
            );

            lowVector.emplace_back(
                    lowCell->as_double()
            );

            closeVector.emplace_back(
                    closeCell->as_double()
            );

            volumeVector.emplace_back(
                    volumeCell->as_double()
            );
        }

        std::map<Column, std::vector<double>> data{};
        data.emplace(std::pair{month, monthVector});
        data.emplace(std::pair{day, dayVector});
        data.emplace(std::pair{open, openVector});
        data.emplace(std::pair{high, highVector});
        data.emplace(std::pair{low, lowVector});
        data.emplace(std::pair{close, closeVector});
        data.emplace(std::pair{volume, volumeVector});
        return data;

//        std::ifstream file(filename);
//
//        auto monthVector = std::vector<double>{};
//        auto dayVector = std::vector<double>{};
//        auto openVector = std::vector<double>{};
//        auto highVector = std::vector<double>{};
//        auto lowVector = std::vector<double>{};
//        auto closeVector = std::vector<double>{};
//        auto volumeVector = std::vector<double>{};
//
//        if (file.is_open()) {
//
//            std::string line;
//
//            // Skip header
//            std::getline(
//                    file,
//                    line
//            );
//
//            while (
//                    std::getline(
//                            file,
//                            line
//                    )
//                    ) {
//                std::istringstream stringStreamOfLine(line);
//                std::string value;
//                int columnIndex = 0;
//                while (
//                        std::getline(
//                                stringStreamOfLine,
//                                value,
//                                ','
//                        )
//                        ) {
//                    if (columnIndex != 0 &&
//                        columnIndex != 1 &&
//                        columnIndex != 9
//                            ) {
//                        if (columnIndex == 2) {
////                            int year = std::stoi(
////                                    value.substr(
////                                            0,
////                                            4
////                                    )
////                            );
//                            int month = std::stoi(
//                                    value.substr(
//                                            4,
//                                            2
//                                    )
//                            );
//                            int day = std::stoi(
//                                    value.substr(
//                                            6,
//                                            2
//                                    )
//                            );
//                            monthVector.push_back(month);
//                            dayVector.push_back(day);
//                        } else if (columnIndex > 3 && columnIndex < 9) {
//
//                            double number;
//                            std::from_chars(
//                                    value.data(),
//                                    value.data() + value.size(),
//                                    number
//                            );
//                            switch (columnIndex) {
//                                case 4:
//                                    openVector.push_back(number);
//                                    break;
//                                case 5:
//                                    highVector.push_back(number);
//                                    break;
//                                case 6:
//                                    lowVector.push_back(number);
//                                    break;
//                                case 7:
//                                    closeVector.push_back(number);
//                                    break;
//                                case 8:
//                                    volumeVector.push_back(number);
//                                    break;
//                            }
//                        }
//                    }
//                    columnIndex++;
//                }
//            }
//        }
//        file.close();
//        std::map<Column, std::vector<double>> data{};
//        data.emplace(std::pair{month, monthVector});
//        data.emplace(std::pair{day, dayVector});
//        data.emplace(std::pair{open, openVector});
//        data.emplace(std::pair{high, highVector});
//        data.emplace(std::pair{low, lowVector});
//        data.emplace(std::pair{close, closeVector});
//        data.emplace(std::pair{volume, volumeVector});
//        return data;
    }

    std::string extract_symbol(const std::string &path) {
        std::string symbol;
        const size_t lastSlashPos = path.find_last_of('/');
        if (lastSlashPos != std::string::npos) {
            symbol = path.substr(lastSlashPos + 1);
            const size_t dotPos = symbol.find('.');
            if (dotPos != std::string::npos) {
                symbol = symbol.substr(0, dotPos);
            }
            return symbol;
        }
        return symbol;
    }

}
