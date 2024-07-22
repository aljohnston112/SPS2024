#include <cmath>
#include <ranges>
#include <future>
#include <filesystem>

#include "directions.h"

#include "config.h"
#include "thread_pool.h"
#include "csvmonkey/CsvWriter.h"
#include "file_util.h"

DirectionData::AllDirectionData calculateAllDirectionData() {
        auto stockDataFilePaths = getAllFilesPaths(
                sps_config::intermediate_data_folder
        );

        std::vector<std::future<DirectionData::AllDirectionDataResults> > futures;

        auto promise = std::promise<DirectionData::AllDirectionDataResults>();
        futures.emplace_back(promise.get_future());
        DirectionData::AllDirectionData results{};
        auto r = thread_pool::ThreadPool::getDiskReadInstance()
                        ->createAndRunTasks<
                                DirectionData::AllDirectionDataResults,
                                std::vector<std::string>,
                                std::string
                        >(
                                calculateDirectionData,
                                stockDataFilePaths
                        );

        for (auto &result: r) {
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
        const std::string &destinationFilePath,
        DirectionData::StockData &stockData
) {
        // Add worker tasks to calculate direction stockData
        std::vector<DirectionData::NamedSeries> namedSeries{};
        const auto it = namedSeries.begin();
        for (int i = 0; i < stockData.size(); i++) {
                namedSeries.emplace(
                        it + i,
                        std::pair{CSV::Column{i}, stockData.at(i)}
                );
        }
        DirectionData::DirectionData directionData = thread_pool::ThreadPool::getCPUWorkInstance()
                        ->createAndRunTasks<
                                DirectionData::DirectionDataResults,
                                DirectionData::AllNamedSeries,
                                DirectionData::NamedSeries
                        >(
                                calculateDirectionDataForNamedSeries,
                                namedSeries
                        );

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

void calculateDirectionData(
        const std::string &&stockDataFilePath,
        std::promise<DirectionData::AllDirectionDataResults> &&promise
) {
        std::string symbol = CSV::extract_symbol(stockDataFilePath);
        const std::string directionFilePath = sps_config::direction_data_folder + symbol + ".txt";

        DirectionData::DirectionData directionData{};
        if (std::filesystem::exists(directionFilePath)) {
                directionData = CSV::readCSV<int>(directionFilePath);
        } else {
                std::vector<std::vector<double> > data =
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
                currentVector
        );
}
