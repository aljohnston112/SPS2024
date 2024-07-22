#include <filesystem>
#include <future>
#include <ranges>

#include "../config.h"
#include "../csvmonkey/CsvWriter.h"
#include "directions.h"
#include "../csv/file_util.h"
#include "../csv/csv_util.h"
#include "../thread_pool.h"

DirectionData::AllDirectionData calculateAllDirectionData() {
    auto stockDataFilePaths = getAllFilesPaths(
        sps_config::intermediate_data_folder
    );

    DirectionData::AllDirectionData results{};
    for (
        auto allDirectionDataResults =
            thread_pool::ThreadPool::getDiskReadInstance()
            ->createAndRunTasks<
                DirectionData::AllDirectionDataResults,
                std::vector<std::string>,
                std::string
            >(
                getDirectionData,
                stockDataFilePaths
            );
        auto& result : allDirectionDataResults
    ) {
        results.emplace(result);
    }
    return results;
}

void saveDirectionData(
    const std::string& filePath,
    DirectionData::DirectionData&& directionData
) {
    writeCsv(
        filePath,
        directionData
    );
}

DirectionData::DirectionData calculateDirectionDataForOne(
    const std::string& destinationFilePath,
    DirectionData::StockData& stockData
) {
    // Add column labels
    std::vector<DirectionData::NamedSeries> namedSeries{};
    const auto it = namedSeries.begin();
    for (size_t i = 0; i < stockData.size(); i++) {
        namedSeries.emplace(
            it + static_cast<int>(i),
            CSV::Column{static_cast<int>(i)},
            stockData.at(i)
        );
    }

    // Convert to directions
    DirectionData::DirectionDataResultsVector directionDataResults =
        thread_pool::ThreadPool::getCPUWorkInstance()
        ->createAndRunTasks<
            DirectionData::DirectionDataResults,
            DirectionData::AllNamedSeries,
            DirectionData::NamedSeries
        >(
            calculateDirectionDataForNamedSeries,
            namedSeries
        );

    // Remove column labels
    DirectionData::DirectionData directionData{};
    for (size_t i = 0; i < directionDataResults.size(); i++) {
        directionData.emplace_back(directionDataResults[CSV::Column{static_cast<int>(i)}].second);
    }

    // Write the directionData to disk
    std::filesystem::create_directory(sps_config::final_data_folder);
    std::filesystem::create_directory(sps_config::direction_data_folder);
    thread_pool::ThreadPool::getDiskWriteInstance()
        ->addTask(
            [
                destinationFilePath,
                directionData = std::move(directionData)
            ] mutable {
                saveDirectionData(
                    destinationFilePath,
                    std::move(directionData)
                );
            }
        );

    return directionData;
}

void getDirectionData(
    const std::string&& stockDataFilePath,
    std::promise<DirectionData::AllDirectionDataResults>&& promise
) {
    std::string symbol = CSV::extract_symbol(stockDataFilePath);
    const std::string directionFilePath =
        sps_config::direction_data_folder + symbol + ".txt";

    DirectionData::DirectionData directionData{};
    if (std::filesystem::exists(directionFilePath)) {
        directionData = CSV::readCSV<int>(directionFilePath);
    }
    else {
        std::vector<std::vector<double>> data =
            CSV::readStockCSV(stockDataFilePath);
        directionData = std::move(
            calculateDirectionDataForOne(
                directionFilePath,
                data
            )
        );
    }

    promise.set_value(
        std::pair(
            symbol,
            directionData
        )
    );
}


void calculateDirectionDataForNamedSeries(
    DirectionData::NamedSeries&& namedSeries,
    std::promise<DirectionData::DirectionDataResults>&& promise
) {
    auto currentVector = std::vector<int>{};
    for (
        auto const windows =
            namedSeries.second | std::views::slide(2);
        const auto& window : windows
    ) {
        int result;
        if (window[1] - window[0] > 0) {
            result = 1;
        }
        else {
            result = 0;
        }
        currentVector.push_back(result);
    }
    promise.set_value(
        std::pair(
            namedSeries.first,
            currentVector
        )
    );
}
