#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_


#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

class thread_pool {
public:
  thread_pool(std::size_t n);
  ~thread_pool();

  std::size_t size() const { return workers.size(); }
  
  template<class F, class... Args>
  auto enqueue(F&& f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type>;

private:
  std::vector<std::thread> workers;
  std::queue<std::function<void()> > tasks;

  std::mutex queue_mutex;
  std::condition_variable condition;
  bool stop;
};


inline thread_pool::thread_pool(std::size_t threads)
  : stop(false) {
  for(size_t i = 0;i<threads;++i)
    workers.emplace_back(
      [this] {
	while (true) {
	  std::function<void()> task;

	  {
	    std::unique_lock<std::mutex> lock(this->queue_mutex);
	    while (not this->stop and
		   this->tasks.empty())
	      this->condition.wait(lock);
	    
	    if(this->stop && this->tasks.empty())
	      return;
	    
	    task = std::move(this->tasks.front());

	    this->tasks.pop();
	  }
	  task();
	}
      }
   );
}


template<class F, class... Args>
auto thread_pool::enqueue(F&& f, Args&&... args)
  -> std::future<typename std::result_of<F(Args...)>::type> {

  using return_type = typename std::result_of<F(Args...)>::type;

  auto task = std::make_shared< std::packaged_task<return_type()> >(
    std::bind(std::forward<F>(f), std::forward<Args>(args)...));

  std::future<return_type> res = task->get_future();

  {
    std::unique_lock<std::mutex> lock(queue_mutex);

    // don't allow enqueueing after stopping the pool
    if(stop)
      throw std::runtime_error("enqueue on stopped thread_pool");

    tasks.emplace([task](){ (*task)(); });
  }
  
  condition.notify_one();
  
  return res;
}


inline thread_pool::~thread_pool() {
  {
    std::unique_lock<std::mutex> lock(queue_mutex);
    stop = true;
  }
  
  condition.notify_all();

  for(std::thread &worker: workers)
    worker.join();
}

#endif /* _THREAD_POOL_H_ */
