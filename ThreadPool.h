
#pragma once
#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

class ThreadPool
{
private:
    std::vector<std::thread> m_workers;
    std::queue<std::function<void()>> m_tasks;
    std::mutex m_mutex;
    std::condition_variable m_cv_workers;
    bool m_stop = false;
    void Workers();
public:
    explicit ThreadPool(size_t threads);
    ~ThreadPool();
    void AddTask(std::function<void()> task);
};
