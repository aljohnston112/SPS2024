#ifndef WAVELET_DIRECTIONS_H
#define WAVELET_DIRECTIONS_H

#include <future>
#include <map>

#include "../csv/csv_util.h"

namespace DirectionData {
    using RawStockData = std::vector<std::vector<double>>;
    using StockNameToRawDataPair = std::pair<std::string, RawStockData>;
    using StockNameToRawDataMap = std::map<std::string, RawStockData>;

    using NamedRawDataPair = std::pair<CSV::Column, std::vector<double>>;
    using NamedRawDataTrainingSet = std::vector<NamedRawDataPair>;

    using DirectionData = std::vector<std::vector<int>>;
    using StockNameToDirectionDataPair = std::pair<const std::string, DirectionData>;
    using StockNameToDirectionDataMap = std::map<std::string, DirectionData>;

    using NamedDirectionDataPair = std::pair<const CSV::Column, std::vector<int>>;
    using NamedDirectionDataTrainingSet = std::vector<NamedDirectionDataPair>;
} // namespace DirectionData

DirectionData::StockNameToDirectionDataMap readOrCalculateAllDirectionData();

DirectionData::DirectionData calculateDirectionDataForOne(
    const std::string& destinationFilePath,
    DirectionData::RawStockData& stockData
);

void saveDirectionData(
    const std::string&& filePath,
    DirectionData::DirectionData&& directionData
);

void calculateDirectionDataForNamedSeries(
    DirectionData::NamedRawDataPair& namedSeries,
    std::promise<DirectionData::NamedDirectionDataPair>&& promise
);

void getDirectionData(
    const std::string& stockDataFilePath,
    std::promise<DirectionData::StockNameToDirectionDataPair>&& promise
);

#endif // WAVELET_DIRECTIONS_H
