#include <filesystem>
#include <future>
#include <fstream>
#include <ranges>

#include "config.h"
#include "data_sanitizer.h"
#include "directions.h"
#include "file_util.h"
#include "thread_pool.h"
#include "timer.h"

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
    const std::string folder = sps_config::intermediate_data_folder;
    auto filePaths = getAllFilesPaths(folder);

    auto directions = calculateAllDirectionData();

    std::vector<std::future<std::map<ulong, uint> > > countFutures;
    std::map<ulong, uint> countResultsForAll{};
    for (
        const auto &direction_data:
        std::views::values(directions)
    ) {
        auto promise = std::make_shared<
            std::promise<std::map<ulong, uint> >
        >();
        countFutures.emplace_back(promise->get_future());


        thread_pool::ThreadPool::getCPUWorkInstance()->addTask(
            [&direction_data, p = std::move(promise)] {
                std::map<ulong, uint> countResults{};
                const auto length = direction_data.begin()->second.size();
                for (int i = 0; i < length; ++i) {
                    uint bits = 0;
                    int j = 0;
                    for (
                        const auto &direction_series:
                        std::views::values(direction_data)
                    ) {
                        bits += direction_series[i] << j++;
                    }
                    countResults[bits]++;
                }
                p->set_value(countResults);
            }
        );
    }

    for (auto &count: countFutures) {
        for (const auto &[key, value]: count.get()) {
            countResultsForAll[key] += value;
        }
    }

    auto resultFileName = "data/final/results/AllPatternCounts.txt";
    std::ofstream resultFile(resultFileName);
    size_t totalCount = 0;
    for (const auto &[key, value]: countResultsForAll) {
        resultFile << "Pattern: " << key << ", Count: " << value << "\n";
        totalCount += value;
    }
    resultFile << "Total Count: " << totalCount << "\n";
    resultFile.close();

    std::vector<
        std::future<
            std::pair<
                std::string,
                std::map<CSV::Column, std::vector<double> >
            >
        >
    > stockFutures{};

    std::unordered_map<std::string, std::map<CSV::Column, std::vector<double> > > stockFiles{};
    for (const auto &directionPair: directions) {
        auto promise = std::make_shared<
            std::promise<
                std::pair<
                    std::string,
                    std::map<CSV::Column, std::vector<double> >
                >
            >
        >();
        stockFutures.emplace_back(promise->get_future());

        thread_pool::ThreadPool::getDiskReadInstance()->addTask(
            [&directionPair, &folder, p = std::move(promise)] {
                auto symbol = directionPair.first;
                p->set_value(
                    {
                        symbol,
                        CSV::readStockCSV(folder + symbol + ".us.txt")
                    }
                );
            }
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
    std::vector<std::pair<std::pair<ulong, ulong>, double> > globalWinLossRatios;
    std::unordered_map<std::pair<ulong, ulong>, double, PairHash> globalNetData;


    std::mutex mutex;
    std::condition_variable cv;
    size_t completedTasks = 0;

    for (const auto &sellInt: std::views::keys(countResultsForAll)) {
        for (const auto &buyInt: std::views::keys(countResultsForAll)) {
            if (sellInt != buyInt) {
                thread_pool::ThreadPool::getCPUWorkInstance()->addTask(
                    [
                        buyInt, sellInt,
                        &mutex, &cv, &completedTasks,
                        &stockFiles, &directions,
                        &countResultsForAll,
                        &globalNetData, &globalGainCounts, &globalLossCounts
                    ] {
                        double globalPercent = 1.0;

                        for (auto &[symbol, directionData]: directions) {
                            auto &stockData = stockFiles[symbol];

                            const auto globalLength = directionData.begin()->second.size();

                            const auto &highDataVector = stockData[CSV::high];
                            const auto &lowDataVector = stockData[CSV::low];
                            const auto &monthDirectionVector = directionData[CSV::Column::month];
                            const auto &dayDirectionVector = directionData[CSV::Column::day];
                            const auto &openDirectionVector = directionData[CSV::Column::open];
                            const auto &highDirectionVector = directionData[CSV::Column::high];
                            const auto &lowDirectionVector = directionData[CSV::Column::low];
                            const auto &closeDirectionVector = directionData[CSV::Column::close];
                            const auto &volumeDirectionVector = directionData[CSV::Column::volume];

                            constexpr int daysPerYear = 252;
                            // const int daysPerMonth = 21;
                            constexpr size_t originalLength = daysPerYear;
                            size_t length = globalLength;
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
                                double buyPrice = 0;
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
                            } {
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
    } {
        std::unique_lock lock(mutex);
        cv.wait(
            lock,
            [&completedTasks, &countResultsForAll] {
                return completedTasks == countResultsForAll.size() * (countResultsForAll.size() - 1);
            }
        );
    }

    for (const auto &pattern: std::views::keys(globalNetData)) {
        globalNetData[pattern] /= static_cast<double>(directions.size());
    }

    for (const auto &[patternPair, gains]: globalGainCounts) {
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
        [](const auto &lhs, const auto &rhs) {
            return lhs.second > rhs.second;
        }
    );


    std::filesystem::create_directory(sps_config::results_data_folder);
    std::string filePath = sps_config::results_data_folder + "results.txt";
    std::ofstream outFile(filePath);
    outFile << "Global Gain Counts:\n";
    for (const auto &[patternPair, gains]: globalGainCounts) {
        outFile << "Buy Pattern: " << patternPair.first
                << ", Sell Pattern: " << patternPair.second
                << ", Gains: " << gains << "\n";
    }

    outFile << "\nGlobal Loss Counts:\n";
    for (const auto &[patternPair, losses]: globalLossCounts) {
        outFile << "Buy Pattern: " << patternPair.first
                << ", Sell Pattern: " << patternPair.second
                << ", Losses: " << losses << "\n";
    }

    outFile << "\nGlobal Win/Loss Ratios:\n";
    for (const auto &[patternPair, ratio]: globalWinLossRatios) {
        outFile << "Buy Pattern: " << patternPair.first
                << ", Sell Pattern: " << patternPair.second
                << ", Win/Loss Ratio: " << ratio << "\n";
    }

    outFile << "\nGlobal Net Data:\n";
    for (const auto &[patternPair, net]: globalNetData) {
        outFile << "Buy Pattern: " << patternPair.first
                << ", Sell Pattern: " << patternPair.second
                << ", Net: " << net << "\n";
    }
}

std::map<CSV::Column, std::vector<int> > processFirst() {
    const std::string stockDataFilePath = sps_config::intermediate_data_folder + "a.us.txt";
    std::map<CSV::Column, std::vector<double> > data = CSV::readStockCSV(stockDataFilePath);
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
    std::unordered_map<std::pair<ulong, ulong>, uint, PairHash> &globalGainCounts,
    std::unordered_map<std::pair<ulong, ulong>, uint, PairHash> &globalLossCounts,
    std::vector<std::pair<std::pair<ulong, ulong>, double> > &globalWinLossRatios,
    std::unordered_map<std::pair<ulong, ulong>, double, PairHash> &globalNetData,
    const std::string &filename
) {
    std::ifstream inFile(filename);

    if (!inFile) {
        std::cerr << "Error opening file for reading: " << filename << std::endl;
        return;
    }

    std::string line;
    bool isCount;
    bool isGain;
    bool isNet;
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

int main() {
    move_stock_data_to_single_folder();
    // processFirst();
    timer::logFunctionTime(
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
            std::vector<std::pair<std::pair<ulong, ulong>, double> > globalWinLossRatios;
            std::unordered_map<std::pair<ulong, ulong>, double, PairHash> globalNetData;
            readDataFromFile(
                globalGainCounts,
                globalLossCounts,
                globalWinLossRatios,
                globalNetData,
                "data/final/results/results.txt"
            );
            for (auto &[patternPair, net]: globalNetData) {
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

