#include <cmath>
#include <ranges>
#include <future>
#include <filesystem>

#include "directions.h"
#include "ThreadPool.h"
#include "csvmonkey/CsvWriter.h"
#include "file_util.h"

DirectionData::AllDirectionData calculateAllDirectionData() {

    const std::string stockDataFolder = "data/intermediate/";
    auto stockDataFilePaths = getAllFilesPaths(stockDataFolder);

    std::vector<std::future<DirectionData::AllDirectionDataResults>> futures;

    auto promise = std::promise<DirectionData::AllDirectionDataResults>();
    futures.emplace_back(promise.get_future());
    DirectionData::AllDirectionData results = thread_pool::ThreadPool::getDiskReadInstance()
            ->createAndRunTasks<
                    DirectionData::AllDirectionDataResults,
                    std::vector<std::string>,
                    std::string,
                    DirectionData::AllDirectionData
            >(
                    calculateDirectionData,
                    stockDataFilePaths
            );

    for (auto &result: results) {
        results.emplace(result);
    }
    return results;
}

void saveDirectionData(
        const std::string &filePath,
        DirectionData::DirectionData &&directionData
) {
    writeCsv(
            filePath,
            directionData
    );
}

DirectionData::DirectionData calculateDirectionDataForOne(
        const std::string &fileName,
        std::map<CSV::Column, std::vector<double>> &stockData
) {

    // Add worker tasks to calculate direction stockData
    DirectionData::DirectionData directionData = thread_pool::ThreadPool::getCPUWorkInstance()
            ->createAndRunTasks<
                    DirectionData::DirectionDataResults,
                    std::map<CSV::Column, std::vector<double>>,
                    std::pair<const CSV::Column, std::vector<double>>,
                    DirectionData::DirectionData
            >(
                    calculateDirectionDataForNamedSeries,
                    stockData
            );

    // Extract the direction stockData
    for (auto &result: directionData) {
        directionData.emplace(result);
    }

    // Write the directionData to disk
    thread_pool::ThreadPool::getDiskWriteInstance()
            ->addTask(
                    std::move(
                            [
                                    fileName,
                                    directionData = std::move(directionData)
                            ] mutable {
                                saveDirectionData(
                                        fileName,
                                        std::move(directionData)
                                );
                            }
                    )
            );

    return directionData;

}

void calculateDirectionData(
        const std::string &&stockDataFilePath,
        std::promise<DirectionData::AllDirectionDataResults> &&promise
) {
    std::string directionFolder = "data/final/direction/";
    std::string symbol = CSV::extract_symbol(stockDataFilePath);
    std::string directionFilePath = directionFolder + symbol + ".txt";

    DirectionData::DirectionData directionData{};
    if (std::filesystem::exists(directionFilePath)) {
        directionData = CSV::readCSV<int>(directionFilePath);
    } else {
        std::map<CSV::Column, std::vector<double>> data =
                CSV::readStockCSV(stockDataFilePath);
        directionData = calculateDirectionDataForOne(
                directionFilePath,
                data
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
        DirectionData::NamedSeries &&namedSeries,
        std::promise<DirectionData::DirectionDataResults> &&promise
) {
    auto currentVector = std::vector<int>{};
    auto const windows = namedSeries.second | std::views::slide(2);
    for (const auto &window: windows) {
        int result;
        if (window[1] - window[0] > 0) {
            result = 1;
        } else {
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