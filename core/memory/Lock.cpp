#include "Lock.h"
#include <iostream>
#include "MemoryFrame.h"

void cleanLock(const std::string &name){
  boost::interprocess::named_mutex::remove((name + SUFFIX_MUTEX).c_str());
  boost::interprocess::named_condition::remove((name + SUFFIX_CND).c_str());
}

Lock::Lock(const std::string &name, bool clean_first):
  name_(name)
{
  if (clean_first)
    cleanLock(name);
  createLock();
}

Lock::~Lock() {
  deleteLock();
}

std::string Lock::getLockName(MemoryFrame *memory,const std::string &name) {
  return name + memory->suffix_;
}

void Lock::createLock() {
  mutex_ = new boost::interprocess::named_mutex(boost::interprocess::open_or_create,(name_ + SUFFIX_MUTEX).c_str());
  cond_ = new boost::interprocess::named_condition(boost::interprocess::open_or_create,(name_ + SUFFIX_CND).c_str());
  lock_ = new boost::interprocess::scoped_lock<boost::interprocess::named_mutex>(*mutex_,boost::interprocess::defer_lock);
}

void Lock::deleteLock() {
  delete mutex_;
  delete cond_;
  delete lock_;
}

void Lock::lock(){
  lock_->lock();
}

bool Lock::try_lock() {
  return lock_->try_lock();
}

bool Lock::timed_lock(unsigned long wait_time) {
  return lock_->timed_lock(getTime(wait_time));
}

bool Lock::timed_lock(boost::system_time &time) {
  return lock_->timed_lock(time);
}

void Lock::notify_one() {
  cond_->notify_one();
}

void Lock::wait() {
  cond_->wait(*lock_);
}

bool Lock::timed_wait(unsigned long wait_time) {
  return cond_->timed_wait(*lock_,getTime(wait_time));
}

bool Lock::timed_wait(boost::system_time &time) {
  return cond_->timed_wait(*lock_,time);
}

void Lock::unlock() {
  lock_->unlock();
}

bool Lock::owns() {
  return lock_->owns();
}

boost::system_time Lock::getTime(long wait_time_in_ms) {
  return boost::get_system_time() + boost::posix_time::milliseconds(wait_time_in_ms);
}

void Lock::recreate(){
  deleteLock();
  cleanLock(name_.c_str());
  createLock();
}
