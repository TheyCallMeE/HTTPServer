#ifndef POSIX_STUFF_HTTPSERVER_WORKQUEUE_H_
#define POSIX_STUFF_HTTPSERVER_WORKQUEUE_H_

#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>

#include "HTTPHeader.h"

class WorkQueue {
 public:
  WorkQueue() {}  // Invokes the constructors of member variables

  void Add(std::unique_ptr<HTTPHeader> data);
  std::unique_ptr<HTTPHeader> Get();

 private:
  std::deque<std::unique_ptr<HTTPHeader>> queue_;
  std::condition_variable cond_;
  std::mutex mutex_;
};

#endif  // POSIX_STUFF_HTTPSERVER_WORKQUEUE_H_
