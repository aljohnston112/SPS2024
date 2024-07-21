#ifndef GENTHREETEAMBUILDER_TIMER_H
#define GENTHREETEAMBUILDER_TIMER_H

#include <chrono>
#include <functional>
#include <iostream>

class timer {

    static std::chrono::milliseconds timeFunction(
            auto function
    ) {
        const auto start = std::chrono::high_resolution_clock::now();
        function();
        const auto stop = std::chrono::high_resolution_clock::now();
        const auto duration = duration_cast<std::chrono::milliseconds>(stop - start);
        return duration;
    }

public:
    static void logFunctionTime(
            auto function,
            const std::string &messageHeader
    ) {
        auto duration = timeFunction < > (function);
        std::cout << messageHeader << duration.count() << "ms" << std::endl;
    }

};

#endif //GENTHREETEAMBUILDER_TIMER_H
