#ifndef PROBABILITY_H
#define PROBABILITY_H
#include <unordered_map>
#include <vector>

template <typename K, typename V>
class ProbabilityMap {
    std::unordered_map<K, std::unordered_map<V, int>> counts{};
    int total = 0;
};

template <typename K, typename T>
class ProbabilityCalculator {
    ProbabilityMap<K, T> countOccurrences(std::vector<K> samples, std::vector<T> outcomes) {
        ProbabilityMap<K, T> map = ProbabilityMap<K, T>{};

        for (int i = 0; i < samples.size(); i++) {
            const auto& sample = samples[i];
            const auto& outcome = outcomes[i];
            map.counts.try_emplace(sample, std::unordered_map<T, int>{});
            map.counts[sample].try_emplace(outcome, 0);
            ++map.counts[sample][outcome];
            ++map.total;
        }

        return map;
    }
};

#endif //PROBABILITY_H
