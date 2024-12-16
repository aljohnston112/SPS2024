#ifndef TREE_H
#define TREE_H
#include <algorithm>
#include <concepts>
#include <optional>
#include <vector>
#include <random>

template <class T>
struct IHash {
    virtual ~IHash() = default;

    virtual size_t operator()(T i) = 0;

    virtual T to_element(size_t i) = 0;

    virtual size_t number_of_elements() = 0;
};

template <class U, class T>
    requires std::derived_from<U, IHash<T>>
class ProbabilityTree {
    static U hash_;
    size_t number_of_elements_;
    size_t totalCount = 0;
    std::vector<size_t> probabilityVector{};
    std::vector<ProbabilityTree> children{};

    std::optional<T> sample() {
        if (totalCount == 0) {
            return std::nullopt;
        }

        std::random_device rd;
        std::mt19937 engine(rd());
        std::uniform_real_distribution distribution(0.0, 1.0);
        auto rand = distribution(engine);
        size_t i = -1;
        while (rand > 0 && i < probabilityVector.size()) {
            i++;
            rand -= static_cast<double>(probabilityVector[i]) /
                totalCount;
        }
        return hash_.to_element(i);
    }

public:
    explicit ProbabilityTree() {
        number_of_elements_ = hash_.number_of_elements();
        for (int i = 0; i < number_of_elements_; i++) {
            probabilityVector.push_back(0);
        }
    }

    void add(std::vector<T>& ts) {
        auto* currentNode = this;
        for (const T& t : ts) {
            if (currentNode->totalCount > 0 && currentNode->probabilityVector[hash_(t)] > 0) {
                currentNode = &currentNode->children[hash_(t)];
            } else {
                for (size_t i = 0; i < number_of_elements_; i++) {
                    currentNode->children.emplace_back(ProbabilityTree{});
                }
                ++currentNode->probabilityVector[hash_(t)];
                ++currentNode->totalCount;
                currentNode = &currentNode->children[hash_(t)];
            }
        }
    }

    std::optional<T> sample(std::vector<T>& elements) {
        std::optional<T> sample{};
        std::vector<size_t> sampled{};
        sampled.emplace_back(0);
        sampled.emplace_back(0);
        auto end_index = elements.size();
        for (size_t start_index = 0; start_index < end_index; start_index++) {
            size_t currentIndex = start_index;
            bool end = false;
            auto& currentNode = this;
            while (!end && currentIndex < end_index) {
                if (
                    const auto t = elements[currentIndex];
                    currentNode.totalCount > 0 && currentNode.probabilityVector[hash_(t)] > 0) {
                    currentNode = currentNode.children[hash_(t)];
                } else {
                    end = true;
                }
                ++currentIndex;
            }
            if (!end && currentIndex < end_index) {
                ++sampled[currentNode.sample()];
            }
        }
        if (
            const auto max_it = std::ranges::max_element(
                sampled.cbegin(),
                sampled.cend()
            );
            max_it != sampled.cend()) {
            sample = hash_.to_element(*max_it);
        }
        return sample;
    }

    std::optional<T> predict(std::vector<T>&& elements) {
        std::optional<T> sample{};
        std::vector<size_t> sampled{};
        sampled.emplace_back(0);
        sampled.emplace_back(0);
        auto end_index = elements.size();
        for (size_t start_index = 0; start_index < end_index; start_index++) {
            size_t currentIndex = start_index;
            bool end = false;
            auto* currentNode = this;
            while (!end && currentIndex < end_index) {
                if (
                    const auto t = elements[currentIndex];
                    currentNode->totalCount > 0 && currentNode->probabilityVector[hash_(t)] > 0) {
                    currentNode = &currentNode->children[hash_(t)];
                } else {
                    end = true;
                }
                ++currentIndex;
            }
            if (!end) {
                if (
                    const auto max_it = std::ranges::max_element(
                        currentNode->probabilityVector.cbegin(),
                        currentNode->probabilityVector.cend()
                    );
                    max_it != currentNode->probabilityVector.cend()) {
                    ++sampled[hash_.to_element(*max_it)];
                }
            }
        }
        if (
            const auto max_it = std::ranges::max_element(
                sampled.cbegin(),
                sampled.cend()
            );
            max_it != sampled.cend()) {
            sample = hash_.to_element(*max_it);
        }
        return sample;
    }
};

template <class U, class T>
    requires std::derived_from<U, IHash<T>>
U ProbabilityTree<U, T>::hash_;

#endif //TREE_H
