#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>
#include <random>

template <class T>
struct Hash {
    static size_t operator()(const T i) {
        return i;
    }

    static T to_element(const T i) {
        return i;
    }

    static size_t number_of_elements() {
        return 2;
    }
};

template <class T>
class ProbabilityTree {
    Hash<T> hash;
    size_t totalCount;
    std::vector<size_t> probabilityVector;
    std::vector<ProbabilityTree> children;

    std::optional<T> sample() {
        if (totalCount == 0) {
            return std::nullopt;
        }

        std::random_device rd;
        std::mt19937 engine(rd());
        std::uniform_real_distribution distribution(0.0, 1.0);
        auto rand = distribution(engine);
        size_t i = 0;
        while (rand >= 0 && i < probabilityVector.size()) {
            rand -= (static_cast<double>(probabilityVector[i]) / totalCount);
            i++;
        }
        --i;
        return hash.to_element(i);;
    }

public:
    explicit ProbabilityTree(
        Hash<T> hash
    )
        : hash(hash),
          totalCount(0),
          probabilityVector{},
          children{} {
        for (int i = 0; i < hash.number_of_elements(); i++) {
            probabilityVector.push_back(0);
        }
    }

    ProbabilityTree(const ProbabilityTree& other) = delete;

    ProbabilityTree(ProbabilityTree&& other) noexcept
        : totalCount(other.totalCount),
          probabilityVector(std::move(other.probabilityVector)),
          children(std::move(other.children)) {
        other.totalCount = 0;
    }

    ~ProbabilityTree() = default;

    ProbabilityTree& operator=(const ProbabilityTree& other) = delete;

    ProbabilityTree& operator=(ProbabilityTree&& other) noexcept {
        if (this != &other) {
            totalCount = other.totalCount;
            probabilityVector = std::move(other.probabilityVector);
            children = std::move(other.children);
            other.totalCount = 0;
        }
        return *this;
    }

    std::optional<T> sample(std::vector<T>&& elements) {
        if (totalCount == 0) {
            return std::nullopt;
        }

        if (children.empty()) {
            for (int i = 0; i < probabilityVector.size(); i++) {
                children.push_back(ProbabilityTree(hash));
            }
        }

        std::unordered_map<T, size_t> sampled{};
        for (int i = 0; i < elements.size(); i++) {
            ProbabilityTree* currentNode = this;
            int j = i;
            while (j < elements.size() && currentNode != nullptr) {
                currentNode = &currentNode->children[elements[j]];
                j++;
            }
            if (currentNode != nullptr) {
                ++sampled[currentNode->sample().value()];
            }
            else {
                return std::nullopt;
            }
        }

        if (sampled.empty()) {
            return sample();
        }

        auto max_it = std::max_element(
            sampled.cbegin(),
            sampled.cend(),
            [](const auto& lhs, const auto& rhs) {
                return lhs.second < rhs.second;
            }
        );
        return max_it->first;
    }

    std::optional<T> predict(std::vector<T>&& elements) {
        if (totalCount == 0) {
            return std::nullopt;
        }

        std::unordered_map<T, size_t> sampled{};
        for (int i = 0; i < elements.size(); i++) {

            ProbabilityTree* currentNode = this;
            int j = i;
            while (j < elements.size() && currentNode->children.size() != 0) {
                currentNode = &currentNode->children[elements[j]];
                j++;
            }
            if (currentNode->children.size() != 0) {
                auto max_it = std::max_element(
                    currentNode->probabilityVector.begin(),
                    currentNode->probabilityVector.end(),
                    [](const auto& lhs, const auto& rhs) {
                        return lhs < rhs;
                    }
                );
                size_t index = std::distance(currentNode->probabilityVector.begin(), max_it);
                ++sampled[index];
            }
            else {
                return std::nullopt;
            }
        }

        if (sampled.empty()) {
            return sample();
        }

        auto max_it = std::max_element(
            sampled.cbegin(),
            sampled.cend(),
            [](const auto& lhs, const auto& rhs) {
                return lhs.second < rhs.second;
            }
        );
        return max_it->first;
    }

    void add(std::vector<T>&& elements) {
        static int maxSize = 21;
        for (size_t end_index = 1; end_index < elements.size() + 1; end_index++) {
            size_t start_index = end_index - maxSize;
            if (end_index < maxSize) {
                start_index = 0;
            }
            for (; start_index < end_index; start_index++) {
                ProbabilityTree* currentNode = this;
                for (size_t k = start_index; k < end_index; k++) {
                    if (currentNode->children.empty()) {
                        for (int i = 0; i < probabilityVector.size(); i++) {
                            currentNode->children.push_back(ProbabilityTree(hash));
                        }
                    }
                    if (
                        currentNode->probabilityVector[elements[k]] == 0 ||
                        start_index == end_index - 1
                    ) {
                        ++currentNode->totalCount;
                        ++currentNode->probabilityVector[elements[k]];
                    }
                    currentNode = &currentNode->children[elements[k]];
                }
            }
        }
    }
};

#endif //NODE_H
