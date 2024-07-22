#ifndef NODE_H
#define NODE_H
#include <unordered_map>

template <class T>
class Node {
    size_t totalCount;
    std::unordered_map<T, size_t> probabilityMap;
    std::unordered_map<T, Node> children;

public:
    Node()
        : totalCount(0),
          probabilityMap{},
          children{} {}

    Node(
        size_t count,
        std::unordered_map<T, size_t> probMap,
        std::unordered_map<T, Node> childNodes
    ) = delete;

    Node(const Node& other) = delete;

    Node(Node&& other) noexcept
        : totalCount(other.totalCount),
          probabilityMap(std::move(other.probabilityMap)),
          children(std::move(other.children)) {
        other.totalCount = 0;
    }

    ~Node() = default;

    Node& operator=(const Node& other) = delete;

    Node& operator=(Node&& other) noexcept {
        if (this != &other) {
            totalCount = other.totalCount;
            probabilityMap = std::move(other.probabilityMap);
            children = std::move(other.children);
            other.totalCount = 0;
        }
        return *this;
    }
};

#endif //NODE_H
