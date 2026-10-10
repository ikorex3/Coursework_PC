#include "ThreadPool.h"
using namespace std;

ThreadPool::ThreadPool(size_t threads) {
  for (int i = 0; i < threads; i++) {
    m_workers.emplace_back(&ThreadPool::Workers, this);
  }
}

ThreadPool::~ThreadPool() {
  {
    unique_lock<mutex> lock(m_mutex);
    m_stop = true;
  }
  m_cv_workers.notify_all();
  for (thread &worker : m_workers) {
    if (worker.joinable()) {
      worker.join();
    }
  }
}

void ThreadPool::Workers() {
  while (true) {
    function<void()> task;
    {
      unique_lock<mutex> lock(m_mutex);
      m_cv_workers.wait(lock, [this]() { return m_stop || !m_tasks.empty(); });
      if (m_stop && m_tasks.empty()) {
        return;
      }
      task = move(m_tasks.front());
      m_tasks.pop();
    }
    task();
  }
}

void ThreadPool::AddTask(std::function<void()> task) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_tasks.push(task);
  m_cv_workers.notify_one();
}