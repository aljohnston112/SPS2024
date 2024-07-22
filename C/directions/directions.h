#ifndef WAVELET_DIRECTIONS_H
#define WAVELET_DIRECTIONS_H

#include <map>

#include "../csv/csv_util.h"

namespace DirectionData {
    using StockData = std::vector<std::vector<double>>;

    using NamedSeries = std::pair<CSV::Column, std::vector<double>>;

    using AllNamedSeries = std::vector<NamedSeries>;

    using AllStockData = std::map<std::string, StockData>;

    using AllStockDataResults = std::pair<std::string, StockData>;

    using DirectionDataResults = std::pair<const CSV::Column, std::vector<int>>;

    using DirectionDataResultsVector = std::vector<DirectionDataResults>;

    using DirectionData = std::vector<std::vector<int>>;

    using AllDirectionDataResults = std::pair<const std::string, DirectionData>;

    using AllDirectionData = std::map<std::string, DirectionData>;
} // namespace DirectionData

DirectionData::AllDirectionData calculateAllDirectionData();

DirectionData::DirectionData calculateDirectionDataForOne(
    const std::string& destinationFilePath,
    DirectionData::StockData& stockData
);

void saveDirectionData(
    const std::string&& filePath,
    DirectionData::DirectionData&& directionData
);

void calculateDirectionDataForNamedSeries(
    DirectionData::NamedSeries&& namedSeries,
    std::promise<DirectionData::DirectionDataResults>&& promise
);

void getDirectionData(
    const std::string&& stockDataFilePath,
    std::promise<DirectionData::AllDirectionDataResults>&& promise
);

#endif // WAVELET_DIRECTIONS_H
