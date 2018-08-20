#ifndef LOCK_CH9LJB3R
#define LOCK_CH9LJB3R

#include <string>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/named_condition.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/thread/thread_time.hpp>

void cleanLock(const std::string &name);

#define LOCK_MOTION "motion"
#define LOCK_VISION "vision"
#define LOCK_MOTION_VISION "motionvision"

#define SUFFIX_MUTEX "mtx"
#define SUFFIX_CND "cnd"

class MemoryFrame;

class Lock{
public:
  Lock(const std::string &name,bool clean_first = false);
  ~Lock();

  static std::string getLockName(MemoryFrame *memory,const std::string &name);

  void lock();
  bool try_lock();
  bool timed_lock(unsigned long);
  bool timed_lock(boost::system_time &time);
  void notify_one();
  void wait();
  bool timed_wait(unsigned long);
  bool timed_wait(boost::system_time &time);
  void unlock();
  bool owns();

  void recreate(); // probably don't use this

  boost::system_time getTime(long wait_time_in_ms);
  
private:
  std::string name_;
  boost::interprocess::named_mutex *mutex_;
  boost::interprocess::named_condition *cond_;
  boost::interprocess::scoped_lock<boost::interprocess::named_mutex> *lock_;

  void deleteLock();
  void createLock();
};

#endif /* end of include guard: LOCK_CH9LJB3R */
