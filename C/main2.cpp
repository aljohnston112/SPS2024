#include <cstdio>

#include "tree/ProbabilityTree.h"

int main2() {
    auto node = ProbabilityTree{Hash<int>()};
    node.add({1, 0, 1, 0});
    auto r = node.predict({1, 0});
    printf("");
}
