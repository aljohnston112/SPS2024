#include <filesystem>
#include <future>
#include <fstream>

#include "directions.h"
#include "file_util.h"
#include "Timer.h"
#include "ThreadPool.h"

struct PairHash {
    template<typename T1, typename T2>
    std::size_t operator()(const std::pair<T1, T2> &pair) const {
        auto hash1 = std::hash<T1>{}(pair.first);
        auto hash2 = std::hash<T2>{}(pair.second);
        auto h = hash1 ^ (hash2 << 1);
        // The below is used by Java's internal implementation
        h ^= (h >> 20) ^ (h >> 12);
        return h ^ (h >> 7) ^ (h >> 4);
    }
};

void process(size_t num = 0) {
    const std::string folder = "data/intermediate/";
    auto filePaths = getAllFilesPaths(folder);

    auto directions = calculateAllDirectionData();

    std::vector<std::future<std::map<ulong, uint>>> countFutures;
    std::map<ulong, uint> countResultsForAll{};
    for (const auto &directionPair: directions) {
        auto &data = directionPair.second;
        auto length = data.begin()->second.size();

        auto promise = std::make_shared<
                std::promise<std::map<ulong, uint>>
        >();
        countFutures.emplace_back(promise->get_future());


        thread_pool::ThreadPool::getCPUWorkInstance()->addTask(
                std::move(
                        [length, &data, p = std::move(promise)] {
                            std::map<ulong, uint> countResults{};
                            for (int i = 0; i < length; ++i) {
                                uint bits = 0;
                                int j = 0;
                                for (const auto &pair: data) {
                                    bits += pair.second[i] << j++;
                                }
                                countResults[bits]++;
                            }
                            p->set_value(countResults);
                        }
                )
        );
    }

    for (auto &count: countFutures) {
        for (auto &pair: count.get()) {
            countResultsForAll[pair.first] += pair.second;
        }
    }

    auto resultFileName = "data/final/results/AllPatternCounts.txt";
    std::ofstream resultFile(resultFileName);
    size_t totalCount = 0;
    for (const auto &entry: countResultsForAll) {
        resultFile << "Pattern: " << entry.first << ", Count: " << entry.second << "\n";
        totalCount += entry.second;
    }
    resultFile << "Total Count: " << totalCount << "\n";
    resultFile.close();

    std::vector<
            std::future<
                    std::pair<
                            std::string,
                            std::map<CSV::Column, std::vector<double>>
                    >
            >
    > stockFutures{};

    std::unordered_map<std::string, std::map<CSV::Column, std::vector<double>>> stockFiles{};
    for (const auto &directionPair: directions) {
        auto promise = std::make_shared<
                std::promise<
                        std::pair<
                                std::string,
                                std::map<CSV::Column, std::vector<double>>
                        >
                >
        >();
        stockFutures.emplace_back(promise->get_future());

        thread_pool::ThreadPool::getDiskReadInstance()->addTask(
                std::move(
                        [&directionPair, &folder, p = std::move(promise)] {
                            auto symbol = directionPair.first;
                            p->set_value(
                                    {
                                            symbol,
                                            CSV::readStockCSV(folder + symbol + ".us.txt")
                                    }
                            );
                        }
                )
        );
    }

    for (auto &f: stockFutures) {
        auto pair = f.get();
        stockFiles.emplace(
                pair.first,
                pair.second
        );
    }

    std::unordered_map<std::pair<ulong, ulong>, uint, PairHash> globalGainCounts;
    std::unordered_map<std::pair<ulong, ulong>, uint, PairHash> globalLossCounts;
    std::vector<std::pair<std::pair<ulong, ulong>, double>> globalWinLossRatios;
    std::unordered_map<std::pair<ulong, ulong>, double, PairHash> globalNetData;

    for (auto &sellBitPattern: countResultsForAll) {
        auto sellInt = sellBitPattern.first;
        for (auto &buyBitPattern: countResultsForAll) {
            auto buyInt = buyBitPattern.first;
            if (sellInt != buyInt) {

                std::mutex mutex;
                std::condition_variable cv;
                int completedTasks = 0;
                double globalPercent = 1.0;

                for (auto &directionPair: directions) {
                    thread_pool::ThreadPool::getCPUWorkInstance()->addTask(
                            std::move(
                                    [
                                            &directionPair, &stockFiles,
                                            &buyInt, &sellInt, &globalPercent,
                                            &mutex, &completedTasks, &cv, &directions,
                                            &globalGainCounts, &globalLossCounts
                                    ] {

                                        auto symbol = directionPair.first;
                                        auto &directionData = directionPair.second;
                                        auto globalLength = directionData.begin()->second.size();
                                        auto &stockData = stockFiles[symbol];

                                        const auto &highDataVector = stockData[CSV::high];
                                        const auto &lowDataVector = stockData[CSV::low];
                                        const auto monthDirectionVector = directionData[CSV::Column::month];
                                        const auto dayDirectionVector = directionData[CSV::Column::day];
                                        const auto openDirectionVector = directionData[CSV::Column::open];
                                        const auto highDirectionVector = directionData[CSV::Column::high];
                                        const auto lowDirectionVector = directionData[CSV::Column::low];
                                        const auto closeDirectionVector = directionData[CSV::Column::close];
                                        const auto volumeDirectionVector = directionData[CSV::Column::volume];

                                        const int daysPerYear = 252;
                                        const int daysPerMonth = 21;
                                        size_t originalLength = daysPerYear;
                                        size_t length = globalLength + 1;
                                        double chunkPercent = 1.0;
//                                for (size_t start = 0; start <= globalLength; start += length) {
                                        for (size_t start = globalLength - length - 1;
                                             start <= globalLength; start += length) {

                                            const auto *highData = highDataVector.data();
                                            const auto *lowData = lowDataVector.data();
                                            const auto *monthDirectionData = monthDirectionVector.data();
                                            const auto *dayDirectionData = dayDirectionVector.data();
                                            const auto *openDirectionData = openDirectionVector.data();
                                            const auto *highDirectionData = highDirectionVector.data();
                                            const auto *lowDirectionData = lowDirectionVector.data();
                                            const auto *closeDirectionData = closeDirectionVector.data();
                                            const auto *volumeDirectionData = volumeDirectionVector.data();

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
                                            double buyPrice;
                                            for (int i = 0; i < length; ++i) {
                                                uint bits = 0;
                                                int j = 0;
                                                bits += monthDirectionData[i] << j++;
                                                bits += dayDirectionData[i] << j++;
                                                bits += openDirectionData[i] << j++;
                                                bits += highDirectionData[i] << j++;
                                                bits += lowDirectionData[i] << j++;
                                                bits += closeDirectionData[i] << j++;
                                                bits += volumeDirectionData[i] << j;
                                                auto currentInt = bits;

                                                if (currentInt == buyInt && !bought) {
                                                    buyPrice = highData[i + 2];
                                                    bought = true;
                                                } else if (currentInt == sellInt && bought) {
                                                    double sellPrice = lowData[i + 2];
                                                    if (sellPrice != buyPrice && buyPrice != 0) {
                                                        chunkPercent = (sellPrice / buyPrice) * chunkPercent;
                                                    }
                                                    bought = false;
                                                }
                                            }
                                            length = originalLength;
                                        }
                                        {
                                            std::lock_guard<std::mutex> lock(mutex);
                                            auto pair = std::pair(buyInt, sellInt);
                                            if (chunkPercent > 1.0) {
                                                globalGainCounts[pair] += 1;
                                            } else if (chunkPercent != 1.0) {
                                                globalLossCounts[pair] += 1;
                                            }
                                            globalPercent += chunkPercent;
                                            completedTasks++;
                                            if (completedTasks == directions.size()) {
                                                cv.notify_one();
                                            }
                                        }
                                    }
                            )
                    );
                }
                {
                    std::unique_lock<std::mutex> lock(mutex);
                    cv.wait(
                            lock,
                            [&completedTasks, &directions] {
                                return completedTasks == directions.size();
                            }
                    );
                }

                auto pair = std::pair(buyInt, sellInt);
                if (globalNetData.find(pair) == globalNetData.end()) {
                    globalNetData[pair] = globalPercent;
                } else {
                    globalNetData[pair] += globalPercent;
                }

            }
        }
    }

    for (const auto &pair: globalNetData) {
        auto &pattern = pair.first;
        globalNetData[pattern] /= directions.size();
    }

    for (const auto &pattern: globalGainCounts) {
        std::pair<unsigned long, unsigned long> patternPair = pattern.first;
        uint gains = pattern.second;
        uid_t losses = globalLossCounts[patternPair];

        if (losses > 0) {
            double ratio = static_cast<double>(gains) / losses;
            globalWinLossRatios.emplace_back(patternPair, ratio);
        }
    }

    std::sort(globalWinLossRatios.begin(), globalWinLossRatios.end(),
              [](const auto &lhs, const auto &rhs) {
                  return lhs.second > rhs.second;
              });


    std::ofstream outFile("data/final/results/results.txt");
    outFile << "Global Gain Counts:\n";
    for (const auto &entry: globalGainCounts) {
        const auto &patternPair = entry.first;
        unsigned int gains = entry.second;
        outFile << "Buy Pattern: " << patternPair.first
                << ", Sell Pattern: " << patternPair.second
                << ", Gains: " << gains << "\n";
    }

    outFile << "\nGlobal Loss Counts:\n";
    for (const auto &entry: globalLossCounts) {
        const auto &patternPair = entry.first;
        unsigned int losses = entry.second;
        outFile << "Buy Pattern: " << patternPair.first
                << ", Sell Pattern: " << patternPair.second
                << ", Losses: " << losses << "\n";
    }

    outFile << "\nGlobal Win/Loss Ratios:\n";
    for (const auto &entry: globalWinLossRatios) {
        const auto &patternPair = entry.first;
        double ratio = entry.second;
        outFile << "Buy Pattern: " << patternPair.first
                << ", Sell Pattern: " << patternPair.second
                << ", Win/Loss Ratio: " << ratio << "\n";
    }

    outFile << "\nGlobal Net Data:\n";
    for (const auto &entry: globalNetData) {
        const auto &patternPair = entry.first;
        double net = entry.second;
        outFile << "Buy Pattern: " << patternPair.first
                << ", Sell Pattern: " << patternPair.second
                << ", Net: " << net << "\n";
    }

}

std::map<CSV::Column, std::vector<int>> processFirst() {
    const std::string fileName = "data/intermediate/a.us.txt";
    std::map<CSV::Column, std::vector<double>> data =
            CSV::readStockCSV(fileName);
    auto directions =
            calculateDirectionDataForOne(
                    CSV::extract_symbol(fileName),
                    data
            );
    return directions;
}

void readDataFromFile(
        std::unordered_map<std::pair<ulong, ulong>, uint, PairHash> &globalGainCounts,
        std::unordered_map<std::pair<ulong, ulong>, uint, PairHash> &globalLossCounts,
        std::vector<std::pair<std::pair<ulong, ulong>, double>> &globalWinLossRatios,
        std::unordered_map<std::pair<ulong, ulong>, double, PairHash> &globalNetData,
        const std::string &filename
) {
    std::ifstream inFile(filename);

    if (!inFile) {
        std::cerr << "Error opening file for reading: " << filename << std::endl;
        return;
    }

    std::string line;

    while (std::getline(inFile, line)) {
        if (line.empty()) {
            continue; // Skip empty lines
        }

        bool isCount;
        bool isGain;
        bool isNet;

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

    inFile.close();
}

int main() {

    // aggregate_stock_data_to_single_folder();
    // processFirst();
    Timer::logFunctionTime(
            [] {
                process();
//                std::unordered_map<std::pair<ulong, ulong>, uint, PairHash> globalGainCountsA;
//                std::unordered_map<std::pair<ulong, ulong>, uint, PairHash> globalLossCountsA;
//                std::vector<std::pair<std::pair<ulong, ulong>, double>> globalWinLossRatiosA;
//                std::unordered_map<std::pair<ulong, ulong>, double, PairHash> globalNetDataA;
//                readDataFromFile(
//                        globalGainCountsA,
//                        globalLossCountsA,
//                        globalWinLossRatiosA,
//                        globalNetDataA,
//                        "data/final/results/aggregated_monthly_results.txt"
//                );
//
//                std::unordered_map<std::pair<ulong, ulong>, uint, PairHash> globalGainCounts;
//                std::unordered_map<std::pair<ulong, ulong>, uint, PairHash> globalLossCounts;
//                std::vector<std::pair<std::pair<ulong, ulong>, double>> globalWinLossRatios;
//                std::unordered_map<std::pair<ulong, ulong>, double, PairHash> globalNetData;
//                readDataFromFile(
//                        globalGainCounts,
//                        globalLossCounts,
//                        globalWinLossRatios,
//                        globalNetData,
//                        "data/final/results/results.txt"
//                );
//
//                size_t total = 0;
//                for (auto &pair: globalGainCountsA) {
//                    total += pair.second;
//                }
//                for (auto &pair: globalLossCountsA) {
//                    total += pair.second;
//                }
//                std::cout << "Total counts: " << total << std::endl;
//
//                size_t chanceCountUp = 0;
//                size_t chanceCountDown = 0;
//                for (auto &pair: globalNetDataA) {
//                    auto &patternPair = pair.first;
//                    auto net = pair.second;
//                    if (net > 100.0 && globalNetData[patternPair] > 1.0
//                        // net > 1.0 && globalNetData[patternPair] > 1.0 && (patternPair.first == 26 || patternPair.second == 125)
//                            ) {
//                        chanceCountUp += globalGainCountsA[patternPair];
//                        chanceCountDown += globalLossCountsA[patternPair];
//                        std::cout << "Buy Pattern: " << patternPair.first << ", Sell Pattern: " << patternPair.second
//                                  << ", NetA: " << net << ", Net: " << globalNetData[patternPair] << std::endl;
//                    }
//                }
//                std::cout << "Chance up counts: " << chanceCountUp << std::endl;
//                std::cout << "Chance down counts: " << chanceCountDown << std::endl;
//                printf("");

                std::unordered_map<std::pair<ulong, ulong>, uint, PairHash> globalGainCounts;
                std::unordered_map<std::pair<ulong, ulong>, uint, PairHash> globalLossCounts;
                std::vector<std::pair<std::pair<ulong, ulong>, double>> globalWinLossRatios;
                std::unordered_map<std::pair<ulong, ulong>, double, PairHash> globalNetData;
                readDataFromFile(
                        globalGainCounts,
                        globalLossCounts,
                        globalWinLossRatios,
                        globalNetData,
                        "data/final/results/results.txt"
                );
                for (auto &pair: globalNetData) {
                    auto &patternPair = pair.first;
                    auto net = pair.second;
                    if (net > 1.0
                        // net > 1.0 && globalNetData[patternPair] > 1.0 && (patternPair.first == 26 || patternPair.second == 125)
                            ) {
                        std::cout << "Buy Pattern: " << patternPair.first << ", Sell Pattern: " << patternPair.second
                                  << ", NetA: " << net << ", Net: " << globalNetData[patternPair] << std::endl;
                    }
                }
            },
            ""
    );
    return 0;
}

