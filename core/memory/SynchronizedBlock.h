#ifndef SYNCHRONIZEDBLOCK_T1HQ0KQX
#define SYNCHRONIZEDBLOCK_T1HQ0KQX

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

#include <memory/OdometryBlock.h>

class MutexRemover {
  public:
    MutexRemover(const std::string &name,bool remove) {
      if (remove)
        boost::interprocess::named_mutex::remove(name.c_str()); 
    }
};

template <class T>
class SynchronizedBlock {
public:
  SynchronizedBlock(MemoryFrame *memory, const std::string &name, bool remove_old = false):
    mutex_remover_(name + memory->suffix_,remove_old),
    data_(NULL),
    mutex_(boost::interprocess::open_or_create,(name + memory->suffix_).c_str())
  {
    memory->getOrAddBlockByName(data_,name,MemoryOwner::SYNC);
  }

  ~SynchronizedBlock() {
    mutex_.unlock();
  }

  void publishData(T *data) {
    boost::interprocess::scoped_lock<boost::interprocess::named_mutex> lock(mutex_);
    *data_ = *data; // copy data in
  }

  void receiveData(T *data) {
    boost::interprocess::scoped_lock<boost::interprocess::named_mutex> lock(mutex_);
    *data = *data_; // copy data out
  }

  void publishUpdate(T *data) {
    // WARNING: only works for odometry for now
    boost::interprocess::scoped_lock<boost::interprocess::named_mutex> lock(mutex_);
    ((OdometryBlock*)data_)->update((OdometryBlock*)data);
  }

  void receiveUpdate(T *data) {
    // WARNING: only works for odometry for now
    boost::interprocess::scoped_lock<boost::interprocess::named_mutex> lock(mutex_);
    *data = *data_; // copy data out
    ((OdometryBlock*)data_)->reset();
  }
  
private:
  MutexRemover mutex_remover_;
  T *data_;
  boost::interprocess::named_mutex mutex_;
};

#endif /* end of include guard: SYNCHRONIZEDBLOCK_T1HQ0KQX */
