#include "WorkQueue.h"

void WorkQueue::Add(std::unique_ptr<HTTPHeader> data) {
  // Unique lock for the wait function of condition variable, unlocked on
  // destructor call
  std::unique_lock<std::mutex> unqlck(mutex_);

  // Add to the deque
  queue_.push_back(std::move(data));

  // Lock the mutex then signal the threads with the condition variable
  cond_.notify_one();
}

std::unique_ptr<HTTPHeader> WorkQueue::Get() {
  // Unique lock for the wait function of condition variable, unlocked on
  // destructor call
  std::unique_lock<std::mutex> unqlck(mutex_);

  // Waiting for the condition
  cond_.wait(unqlck);

  auto x = std::move(queue_.front());
  queue_.pop_front();
  return x;
}
