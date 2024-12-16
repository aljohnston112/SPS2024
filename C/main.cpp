#include <algorithm>
#include <filesystem>
#include <future>
#include <fstream>
#include <ranges>

#include "config.h"
#include "csv/csv_util.h"
#include "csv/file_util.h"
#include "directions/directions.h"
#include "thread_pool.h"
#include "timer.h"
#include "tree.h"

struct PairHash {
    template <typename T1, typename T2>
    std::size_t operator()(const std::pair<T1, T2>& pair) const {
        auto hash1 = std::hash<T1>{}(pair.first);
        auto hash2 = std::hash<T2>{}(pair.second);
        auto h = hash1 ^ (hash2 << 1);
        // The below is used by Java's internal implementation
        h ^= (h >> 20) ^ (h >> 12);
        return h ^ (h >> 7) ^ (h >> 4);
    }
};

void countDirections(
    DirectionData::AllDirectionDataResults&& direction_data,
    std::promise<std::map<ulong, uint>>&& promise
) {
    std::map<ulong, uint> countResults{};
    const auto length = direction_data.second.begin()->size();
    for (size_t i = 0; i < length; ++i) {
        uint bits = 0;
        for (size_t j = 0; j < direction_data.second.size(); j++) {
            bits += direction_data.second[j][i] << j++;
        }
        countResults[bits]++;
    }
    promise.set_value(countResults);
}

std::map<ulong, uint> getCountResultsForAll(
    DirectionData::AllDirectionData directions
) {
    auto countPromises =
        thread_pool::ThreadPool::getCPUWorkInstance()
        ->createAndRunTasks<
            std::map<ulong, uint>,
            DirectionData::AllDirectionData,
            DirectionData::AllDirectionDataResults
        >(
            countDirections,
            directions
        );

    std::map<ulong, uint> countResultsForAll{};
    for (auto& count : countPromises) {
        for (const auto& [key, value] : count) {
            countResultsForAll[key] += value;
        }
    }

    auto resultFileName = sps_config::results_data_folder + "AllPatternCounts.txt";
    std::ofstream resultFile(resultFileName);
    size_t totalCount = 0;
    for (const auto& [key, value] : countResultsForAll) {
        resultFile << "Pattern: " << key << ", Count: " << value << "\n";
        totalCount += value;
    }
    resultFile << "Total Count: " << totalCount << "\n";
    resultFile.close();

    return countResultsForAll;
}

void getStockDataForSingle(
    std::string&& fileName,
    std::promise<DirectionData::AllStockDataResults>&& promise
) {
    auto symbol = CSV::extract_symbol(fileName);
    promise.set_value(
        {
            symbol,
            CSV::readStockCSV(fileName)
        }
    );
}

std::unordered_map<std::string, DirectionData::StockData> getAllStockData() {
    const std::string stockDataFolder = sps_config::intermediate_data_folder;
    auto filePaths = getAllFilesPaths(stockDataFolder);

    std::vector<DirectionData::AllStockDataResults> stockDataResults =
        thread_pool::ThreadPool::getDiskReadInstance()
        ->createAndRunTasks<
            DirectionData::AllStockDataResults,
            std::vector<std::string>,
            std::string
        >(
            getStockDataForSingle,
            filePaths
        );

    std::unordered_map<std::string, DirectionData::StockData> stockFiles{};
    for (auto& stockData : stockDataResults) {
        auto& [symbol, data] = stockData;
        stockFiles.emplace(
            symbol,
            data
        );
    }
    return stockFiles;
}

void process() {
    auto directions = calculateAllDirectionData();
    auto allStockData = getAllStockData();
    auto countResultsForAll = getCountResultsForAll(directions);

    std::unordered_map<std::pair<ulong, ulong>, uint, PairHash> globalGainCounts;
    std::unordered_map<std::pair<ulong, ulong>, uint, PairHash> globalLossCounts;
    std::vector<std::pair<std::pair<ulong, ulong>, double>> globalWinLossRatios;
    std::unordered_map<std::pair<ulong, ulong>, double, PairHash> globalNetData;

    std::mutex mutex;
    std::condition_variable cv;
    size_t completedTasks = 0;

    for (const auto& sellInt : std::views::keys(countResultsForAll)) {
        for (const auto& buyInt : std::views::keys(countResultsForAll)) {
            if (sellInt != buyInt) {
                thread_pool::ThreadPool::getCPUWorkInstance()->addTask(
                    [
                        buyInt, sellInt,
                        &mutex, &cv, &completedTasks,
                        &allStockData, &directions,
                        &countResultsForAll,
                        &globalNetData, &globalGainCounts, &globalLossCounts
                    ] {
                        double globalPercent = 1.0;

                        for (auto& [symbol, directionData] : directions) {
                            auto& stockData = allStockData[symbol];

                            const auto globalLength = directionData.begin()->size();

                            const auto& highDataVector = stockData.at(CSV::high);
                            const auto& lowDataVector = stockData.at(CSV::low);
                            const auto& monthDirectionVector = directionData.at(CSV::month);
                            const auto& dayDirectionVector = directionData.at(CSV::day);
                            const auto& openDirectionVector = directionData.at(CSV::open);
                            const auto& highDirectionVector = directionData.at(CSV::high);
                            const auto& lowDirectionVector = directionData.at(CSV::low);
                            const auto& closeDirectionVector = directionData.at(CSV::close);
                            const auto& volumeDirectionVector = directionData.at(CSV::volume);

                            constexpr int daysPerYear = 252;
                            // const int daysPerMonth = 21;
                            constexpr size_t originalLength = daysPerYear;
                            size_t length = globalLength;
                            double chunkPercent = 1.0;
                            // for (size_t start = 0; start <= globalLength; start += length) {
                            for (size_t start = globalLength - length;
                                 start <= globalLength; start += length) {
                                const auto* highData = highDataVector.data();
                                const auto* lowData = lowDataVector.data();
                                const auto* monthDirectionData = monthDirectionVector.data();
                                const auto* dayDirectionData = dayDirectionVector.data();
                                const auto* openDirectionData = openDirectionVector.data();
                                const auto* highDirectionData = highDirectionVector.data();
                                const auto* lowDirectionData = lowDirectionVector.data();
                                const auto* closeDirectionData = closeDirectionVector.data();
                                const auto* volumeDirectionData = volumeDirectionVector.data();

                                if (globalLength <= start + length - 1) {
                                    length = globalLength - start;
                                }

                                lowData += start;
                                highData += start;
                                monthDirectionData += start;
                                dayDirectionData += start;
                                openDirectionData += start;
                                highDirectionData += start;
                                lowDirectionData += start;
                                closeDirectionData += start;
                                volumeDirectionData += start;

                                bool bought = false;
                                double buyPrice = 0;
                                for (size_t i = 0; i < length; ++i) {
                                    uint bits = 0;
                                    int j = 0;
                                    bits += monthDirectionData[i] << j++;
                                    bits += dayDirectionData[i] << j++;
                                    bits += openDirectionData[i] << j++;
                                    bits += highDirectionData[i] << j++;
                                    bits += lowDirectionData[i] << j++;
                                    bits += closeDirectionData[i] << j++;
                                    bits += volumeDirectionData[i] << j;

                                    if (
                                        const auto currentInt = bits;
                                        currentInt == buyInt && !bought
                                    ) {
                                        buyPrice = highData[i + 2];
                                        bought = true;
                                    } else if (currentInt == sellInt && bought) {
                                        if (
                                            const double sellPrice = lowData[i + 2];
                                            sellPrice != buyPrice && buyPrice != 0
                                        ) {
                                            chunkPercent = (sellPrice / buyPrice) * chunkPercent;
                                        }
                                        bought = false;
                                    }
                                }
                                length = originalLength;
                            }
                            {
                                const auto pair = std::pair(buyInt, sellInt);
                                if (chunkPercent > 1.0) {
                                    globalGainCounts[pair] += 1;
                                } else if (chunkPercent != 1.0) {
                                    globalLossCounts[pair] += 1;
                                }
                                globalPercent += chunkPercent;
                            }
                        }
                        {
                            std::lock_guard lock(mutex);
                            if (
                                const auto pair = std::pair(buyInt, sellInt);
                                !globalNetData.contains(pair)
                            ) {
                                globalNetData[pair] = globalPercent;
                            } else {
                                globalNetData[pair] += globalPercent;
                            }
                            completedTasks++;
                        }
                        if (completedTasks == countResultsForAll.size() * (countResultsForAll.size() - 1)) {
                            cv.notify_one();
                        }
                    }
                );
            }
        }
    }
    {
        std::unique_lock lock(mutex);
        cv.wait(
            lock,
            [&completedTasks, &countResultsForAll] {
                return completedTasks == countResultsForAll.size() * (countResultsForAll.size() - 1);
            }
        );
    }

    for (const auto& pattern : std::views::keys(globalNetData)) {
        globalNetData[pattern] /= static_cast<double>(directions.size());
    }

    for (const auto& [patternPair, gains] : globalGainCounts) {
        if (
            const uint losses = globalLossCounts[patternPair];
            losses > 0
        ) {
            double ratio = static_cast<double>(gains) / losses;
            globalWinLossRatios.emplace_back(patternPair, ratio);
        }
    }

    std::ranges::sort(
        globalWinLossRatios,
        [](const auto& lhs, const auto& rhs) {
            return lhs.second > rhs.second;
        }
    );


    std::filesystem::create_directory(sps_config::results_data_folder);
    std::string filePath = sps_config::results_data_folder + "results.txt";
    std::ofstream outFile(filePath);
    outFile << "Global Gain Counts:\n";
    for (const auto& [patternPair, gains] : globalGainCounts) {
        outFile << "Buy Pattern: " << patternPair.first
            << ", Sell Pattern: " << patternPair.second
            << ", Gains: " << gains << "\n";
    }

    outFile << "\nGlobal Loss Counts:\n";
    for (const auto& [patternPair, losses] : globalLossCounts) {
        outFile << "Buy Pattern: " << patternPair.first
            << ", Sell Pattern: " << patternPair.second
            << ", Losses: " << losses << "\n";
    }

    outFile << "\nGlobal Win/Loss Ratios:\n";
    for (const auto& [patternPair, ratio] : globalWinLossRatios) {
        outFile << "Buy Pattern: " << patternPair.first
            << ", Sell Pattern: " << patternPair.second
            << ", Win/Loss Ratio: " << ratio << "\n";
    }

    outFile << "\nGlobal Net Data:\n";
    for (const auto& [patternPair, net] : globalNetData) {
        outFile << "Buy Pattern: " << patternPair.first
            << ", Sell Pattern: " << patternPair.second
            << ", Net: " << net << "\n";
    }
}

DirectionData::DirectionData processFirst() {
    const std::string stockDataFilePath = sps_config::intermediate_data_folder + "a.us.txt";
    DirectionData::StockData data = CSV::readStockCSV(stockDataFilePath);
    const std::string symbol = CSV::extract_symbol(stockDataFilePath);
    const std::string directionFilePath = sps_config::direction_data_folder + symbol + ".txt";
    auto directions =
        calculateDirectionDataForOne(
            directionFilePath,
            data
        );
    return directions;
}

void readDataFromFile(
    std::unordered_map<std::pair<ulong, ulong>, uint, PairHash>& globalGainCounts,
    std::unordered_map<std::pair<ulong, ulong>, uint, PairHash>& globalLossCounts,
    std::vector<std::pair<std::pair<ulong, ulong>, double>>& globalWinLossRatios,
    std::unordered_map<std::pair<ulong, ulong>, double, PairHash>& globalNetData,
    const std::string& filename
) {
    std::ifstream inFile(filename);

    if (!inFile) {
        std::cerr << "Error opening file for reading: " << filename << std::endl;
        return;
    }

    std::string line;
    bool isCount = false;
    bool isGain = false;
    bool isNet = false;
    while (std::getline(inFile, line)) {
        if (!line.empty()) {
            if (line.find("Global Gain Counts:") != std::string::npos) {
                isCount = true;
                isGain = true;
                isNet = false;
            } else if (line.find("Global Loss Counts:") != std::string::npos) {
                isCount = true;
                isGain = false;
                isNet = false;
            } else if (line.find("Global Win/Loss Ratios:") != std::string::npos) {
                isCount = false;
                isNet = false;
            } else if (line.find("Global Net Data:") != std::string::npos) {
                isCount = false;
                isNet = true;
            } else {
                std::istringstream iss(line);
                std::string token;
                std::pair<ulong, ulong> patternPair;
                double value = 0.0;
                uint count = 0;

                while (std::getline(iss, token, ':')) {
                    std::istringstream line_stream(token);
                    if (count == 1) {
                        line_stream >> patternPair.first;
                    } else if (count == 2) {
                        line_stream >> patternPair.second;
                    } else if (count == 3) {
                        line_stream >> value;
                    }
                    ++count;
                }

                if (isCount) {
                    if (isGain) {
                        globalGainCounts.emplace(patternPair, value);
                    } else {
                        globalLossCounts.emplace(patternPair, value);
                    }
                } else if (isNet) {
                    globalNetData.emplace(patternPair, value);
                } else {
                    globalWinLossRatios.emplace_back(patternPair, value);
                }
            }
        }
    }

    inFile.close();
}

template <class T>
struct Hash final : IHash<T> {
    size_t operator()(const T i) override {
        return i;
    }

    T to_element(const size_t i) override {
        return i;
    }

    size_t number_of_elements() override {
        return 2;
    }
};

int main() {
    timer::logFunctionTime(
        [] {
            auto directions = calculateAllDirectionData();
            auto allStockData = getAllStockData();
            auto tree = ProbabilityTree<Hash<int>, int>{};
            for (auto& directionData : std::ranges::views::values(directions)) {
                tree.add(directionData[CSV::low]);
            }

            uint globalGainCounts = 0;
            uint globalLossCounts = 0;
            double globalWinLossRatios;
            double globalNetData = 0;

            std::condition_variable cv;
            std::atomic<size_t> completedTasks = 0;

            thread_pool::ThreadPool::getCPUWorkInstance()->addTask(
                [
                    &tree,
                    &cv, &completedTasks,
                    &allStockData, &directions,
                    &globalNetData, &globalGainCounts, &globalLossCounts
                ] {
                    double globalPercent = 1.0;

                    for (auto& [symbol, directionData] : directions) {
                        auto& stockData = allStockData[symbol];

                        const auto globalLength = directionData.begin()->size();

                        const auto& highDataVector = stockData.at(CSV::high);
                        const auto& lowDataVector = stockData.at(CSV::low);
                        const auto& lowDirectionVector = directionData.at(CSV::low);

                        constexpr int daysPerYear = 252;
                        // const int daysPerMonth = 21;
                        constexpr size_t originalLength = daysPerYear;
                        size_t length = globalLength;
                        double chunkPercent = 1.0;
                        // for (size_t start = 0; start <= globalLength; start += length) {
                        for (size_t start = globalLength - length; start < globalLength; start += length) {
                            const auto* highData = highDataVector.data();
                            const auto* lowData = lowDataVector.data();
                            const auto* lowDirectionData = lowDirectionVector.data();
                            if (globalLength <= start + length - 1) {
                                length = globalLength - start;
                            }

                            lowData += start;
                            highData += start;

                            bool bought = false;
                            double buyPrice = 0;
                            for (size_t i = 0; i < length - 1; ++i) {
                                const int currentDirection = lowDirectionData[i];
                                if (
                                    const auto prediction = tree.predict(
                                        {
                                            lowDirectionData,
                                            lowDirectionData + i
                                        }
                                    );
                                    prediction.has_value() &&
                                    currentDirection == 0 && prediction.value() == 1 && !bought
                                ) {
                                    buyPrice = highData[i + 2];
                                    bought = true;
                                } else if (
                                    prediction.has_value() &&
                                    currentDirection == 1 && prediction.value() == 0 && bought
                                ) {
                                    if (
                                        const double sellPrice = lowData[i + 2];
                                        sellPrice != buyPrice && buyPrice != 0
                                    ) {
                                        chunkPercent = (sellPrice / buyPrice) * chunkPercent;
                                    }
                                    bought = false;
                                }
                            }
                            length = originalLength;
                        }
                        {
                            if (chunkPercent >= 1.0) {
                                globalGainCounts += 1;
                                std::cout << "    " << chunkPercent << std::endl;
                            } else {
                                globalLossCounts += 1;
                                std::cout << chunkPercent << std::endl;
                            }
                            globalPercent += chunkPercent;
                        }
                    }
                    {
                        if (
                            globalNetData == 0
                        ) {
                            globalNetData = globalPercent;
                        } else {
                            globalNetData += globalPercent;
                        }
                        completedTasks.fetch_add(1);
                    }
                    cv.notify_one();
                }
            );

            {
                std::mutex mutex;
                std::unique_lock lock(mutex);
                cv.wait(
                    lock,
                    [&completedTasks] {
                        return 1 == completedTasks;
                    }
                );
            }

            globalNetData /= static_cast<double>(directions.size());

            double ratio = static_cast<double>(globalGainCounts) / globalLossCounts;
            globalWinLossRatios = ratio;

            std::filesystem::create_directory(sps_config::results_data_folder);
            std::string filePath = sps_config::results_data_folder + "tree_results.txt";
            std::ofstream outFile(filePath);
            outFile << "Global Gain Counts: " << globalGainCounts << "\n";
            outFile << "Global Loss Counts: " << globalLossCounts << "\n";
            outFile << "Global Win/Loss Ratios: " << globalWinLossRatios << "\n";
            outFile << "Global Net Data: " << globalNetData << "\n";
        },
        ""
    );
}

// int main() {
//     move_stock_data_to_single_folder();
//     // processFirst();
//     timer::logFunctionTime(
//         [] {
//             process();
//             //                std::unordered_map<std::pair<ulong, ulong>, uint, PairHash> globalGainCountsA;
//             //                std::unordered_map<std::pair<ulong, ulong>, uint, PairHash> globalLossCountsA;
//             //                std::vector<std::pair<std::pair<ulong, ulong>, double>> globalWinLossRatiosA;
//             //                std::unordered_map<std::pair<ulong, ulong>, double, PairHash> globalNetDataA;
//             //                readDataFromFile(
//             //                        globalGainCountsA,
//             //                        globalLossCountsA,
//             //                        globalWinLossRatiosA,
//             //                        globalNetDataA,
//             //                        "data/final/results/aggregated_monthly_results.txt"
//             //                );
//             //
//             //                std::unordered_map<std::pair<ulong, ulong>, uint, PairHash> globalGainCounts;
//             //                std::unordered_map<std::pair<ulong, ulong>, uint, PairHash> globalLossCounts;
//             //                std::vector<std::pair<std::pair<ulong, ulong>, double>> globalWinLossRatios;
//             //                std::unordered_map<std::pair<ulong, ulong>, double, PairHash> globalNetData;
//             //                readDataFromFile(
//             //                        globalGainCounts,
//             //                        globalLossCounts,
//             //                        globalWinLossRatios,
//             //                        globalNetData,
//             //                        "data/final/results/results.txt"
//             //                );
//             //
//             //                size_t total = 0;
//             //                for (auto &pair: globalGainCountsA) {
//             //                    total += pair.second;
//             //                }
//             //                for (auto &pair: globalLossCountsA) {
//             //                    total += pair.second;
//             //                }
//             //                std::cout << "Total counts: " << total << std::endl;
//             //
//             //                size_t chanceCountUp = 0;
//             //                size_t chanceCountDown = 0;
//             //                for (auto &pair: globalNetDataA) {
//             //                    auto &patternPair = pair.first;
//             //                    auto net = pair.second;
//             //                    if (net > 100.0 && globalNetData[patternPair] > 1.0
//             //                        // net > 1.0 && globalNetData[patternPair] > 1.0 && (patternPair.first == 26 || patternPair.second == 125)
//             //                            ) {
//             //                        chanceCountUp += globalGainCountsA[patternPair];
//             //                        chanceCountDown += globalLossCountsA[patternPair];
//             //                        std::cout << "Buy Pattern: " << patternPair.first << ", Sell Pattern: " << patternPair.second
//             //                                  << ", NetA: " << net << ", Net: " << globalNetData[patternPair] << std::endl;
//             //                    }
//             //                }
//             //                std::cout << "Chance up counts: " << chanceCountUp << std::endl;
//             //                std::cout << "Chance down counts: " << chanceCountDown << std::endl;
//             //                printf("");
//
//             std::unordered_map<std::pair<ulong, ulong>, uint, PairHash> globalGainCounts;
//             std::unordered_map<std::pair<ulong, ulong>, uint, PairHash> globalLossCounts;
//             std::vector<std::pair<std::pair<ulong, ulong>, double>> globalWinLossRatios;
//             std::unordered_map<std::pair<ulong, ulong>, double, PairHash> globalNetData;
//             readDataFromFile(
//                 globalGainCounts,
//                 globalLossCounts,
//                 globalWinLossRatios,
//                 globalNetData,
//                 "data/final/results/results.txt"
//             );
//             for (auto& [patternPair, net] : globalNetData) {
//                 if (net > 1.0
//                     // net > 1.0 && globalNetData[patternPair] > 1.0 && (patternPair.first == 26 || patternPair.second == 125)
//                 ) {
//                     std::cout << "Buy Pattern: " << patternPair.first << ", Sell Pattern: " << patternPair.second
//                         << ", NetA: " << net << ", Net: " << globalNetData[patternPair] << std::endl;
//                 }
//             }
//         },
//         ""
//     );
//     return 0;
// }

