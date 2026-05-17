#ifndef STOCK_DATA_H
#define STOCK_DATA_H
#include <string>
#include <unordered_map>

#include "config.h"
#include "thread_pool.h"
#include "csv/file_util.h"
#include "directions/directions.h"

namespace stock_data {

    inline void getStockDataForSingleStock(
        const std::string& fileName,
        std::promise<DirectionData::StockNameToRawDataPair>&& promise
    ) {
        auto symbol = CSV::extract_symbol(fileName);
        promise.set_value(
            {
                symbol,
                CSV::readStockCSV(fileName)
            }
        );
    }

    inline std::unordered_map<std::string, DirectionData::RawStockData> getAllStockData() {
        const std::string stockDataFolder = sps_config::intermediate_data_folder;
        auto filePaths = getAllFilesPaths(stockDataFolder);

        std::vector<DirectionData::StockNameToRawDataPair> stockDataResults =
            thread_pool::ThreadPool::getDiskReadInstance()
            ->createAndRunTasks<
                DirectionData::StockNameToRawDataPair,
                std::vector<std::string>,
                std::string
            >(
                getStockDataForSingleStock,
                filePaths
            );

        std::unordered_map<std::string, DirectionData::RawStockData> stockFiles{};
        for (auto& stockData : stockDataResults) {
            auto& [symbol, data] = stockData;
            stockFiles.emplace(
                symbol,
                data
            );
        }
        return stockFiles;
    }

}


#endif //STOCK_DATA_H
