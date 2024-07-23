#include <gtest/gtest.h>

#include "../C/tree/ProbabilityTree.h"
#include "../C/config.h"
#include "../C/csv/csv_util.h"
#include "../C/directions/directions.h"

TEST(Node, TestAdd) {
    // const std::string stockDataFilePath = sps_config::intermediate_data_folder + "a.us.txt";
    // DirectionData::StockData data = CSV::readStockCSV(stockDataFilePath);
    // const std::string symbol = CSV::extract_symbol(stockDataFilePath);
    // const std::string directionFilePath = sps_config::direction_data_folder + symbol + ".txt";
    // auto directions =
    //     calculateDirectionDataForOne(
    //         directionFilePath,
    //         data
    //     );
    auto node = ProbabilityTree{Hash<int>()};
    // node.add(std::move(directions[CSV::low]));
    node.add({1, 4, 1, 5});
    auto r = node.predict({1, 4});
    printf("");
}
