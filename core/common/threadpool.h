#pragma once

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

class threadpool {
  public:
    threadpool(std::size_t);
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) 
    -> std::future<std::result_of_t<F(Args...)>>;
    ~threadpool();
  private:
    std::vector<std::thread> _workers;
    std::queue<std::function<void()>> _tasks;
    std::mutex _queue_mutex;
    std::condition_variable _queue_cv;
    bool _stop = false;
};

// Launch as many worker threads as are requested
inline threadpool::threadpool(std::size_t threads) {
  for(std::size_t i = 0; i < threads; i++) {
    _workers.emplace_back([this]{
      while(true) {
        std::function<void()> task;
        {
          std::unique_lock<std::mutex> lock(_queue_mutex);
          _queue_cv.wait(lock, 
            // Make this worker wait until the pool is being stopped
            // or until a task is ready for it
            [this]{ return _stop || !_tasks.empty(); }
          );

          // If there are no tasks and the pool is being stopped, just return out
          if(_stop && _tasks.empty())
            return;

          // At this point the worker has the queue lock and a task.
          task = std::move(_tasks.front());
          _tasks.pop();
        }
        // Run the task we pulled from the queue
        task();
      }
    });
  }
}

// Queue up a new task
template<class F, class... Args>
auto threadpool::enqueue(F&& f, Args&&... args) 
  -> std::future<std::result_of_t<F(Args...)>> {
  using return_type = std::result_of_t<F(Args...)>;

  auto task = std::make_shared<std::packaged_task<return_type()>>(
    std::bind(std::forward<F>(f), std::forward<Args>(args)...)
  );

  std::future<return_type> res = task->get_future();
  {
    std::unique_lock<std::mutex> lock(_queue_mutex);

    // Don't allow enqueueing after stopping the pool
    if(_stop)
      throw std::runtime_error("Attempted calling threadpool::enqueue on stopped threadpool");

    _tasks.emplace([task](){ (*task)(); });
  }
  _queue_cv.notify_one();
  return res;
}

// Join all threads in the pool on destruction
inline threadpool::~threadpool() {
  {
    std::unique_lock<std::mutex> lock(_queue_mutex);
    _stop = true;
  }
  _queue_cv.notify_all();
  for(auto& w : _workers)
    w.join();
}
