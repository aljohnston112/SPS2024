#include <future>
#include <iostream>

#include "thread_pool.h"

namespace thread_pool {
    ThreadPool::ThreadPool(const int numThreads) {
        for (int i = 0; i < numThreads; ++i) {
            std::cout << "Created thread" << std::endl;
            threads.emplace_back(
                [this] {
                    while (true) {
                        Function task;
                        {
                            std::unique_lock lock(mutex);
                            task_condition.wait(
                                lock,
                                [this] {
                                    return stop || !tasks.empty();
                                }
                            );
                            if (tasks.empty()) {
                                return;
                            }
                            task = std::move(tasks.front());
                            tasks.pop();
                        }
                        task();
                    }
                }
            );
        }
    }

    void ThreadPool::addTask(Function&& function) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            tasks.emplace(std::move(function));
            task_condition.notify_all();
        }
    }

    void ThreadPool::addTasks(std::vector<Function>& functions) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            for (Function& function : functions) {
                tasks.emplace(std::move(function));
            }
            task_condition.notify_all();
        }
    }

    ThreadPool::~ThreadPool() {
        {
            std::lock_guard lock(mutex);
            stop = true;
            task_condition.notify_all();
        }
        for (std::jthread& thread : threads) {
            thread.join();
        }
    }

    unsigned int ThreadPool::getNumberOfThreads() {
        static unsigned int numThreads = std::thread::hardware_concurrency();
        return numThreads;
    }

    std::shared_ptr<ThreadPool> ThreadPool::getCPUWorkInstance() {
        static auto thread_pool =
            std::make_shared<ThreadPool>(getNumberOfThreads());
        return thread_pool;
    }

    std::shared_ptr<ThreadPool> ThreadPool::getDiskReadInstance() {
        static auto thread_pool =
            std::make_shared<ThreadPool>(getNumberOfThreads());
        return thread_pool;
    }

    std::shared_ptr<ThreadPool> ThreadPool::getDiskWriteInstance() {
        static auto thread_pool =
            std::make_shared<ThreadPool>(getNumberOfThreads());
        return thread_pool;
    }
}
