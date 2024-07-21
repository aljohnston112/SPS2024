#ifndef WAVELET_DIRECTIONS_H
#define WAVELET_DIRECTIONS_H

#include <map>
#include "csv_util.h"

namespace DirectionData {

    using StockData = std::map<CSV::Column, std::vector<double>>;

    using DirectionData = std::map<CSV::Column, std::vector<int>>;

    using AllDirectionData = std::map<std::string, DirectionData>;

    using NamedSeries = std::pair<
            CSV::Column,
            std::vector<double>
    >;

    using DirectionDataResults = std::pair<
            CSV::Column,
            std::vector<int>
    >;

    using AllDirectionDataResults = std::pair<
            std::string,
            std::map<CSV::Column, std::vector<int>>
    >;

}

DirectionData::AllDirectionData calculateAllDirectionData();

std::map<CSV::Column, std::vector<int>> calculateDirectionDataForOne(
        const std::string &destinationFilePath,
        DirectionData::StockData &stockData
);

void saveDirectionData(
        const std::string &&filePath,
        DirectionData::DirectionData &&directionData
);

void calculateDirectionDataForNamedSeries(
        DirectionData::NamedSeries &&namedSeries,
        std::promise<DirectionData::DirectionDataResults> &&promise
);

void calculateDirectionData(
        const std::string &&stockDataFilePath,
        std::promise<DirectionData::AllDirectionDataResults> &&promise
);

#endif //WAVELET_DIRECTIONS_H
