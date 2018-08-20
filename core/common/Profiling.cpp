#include <common/Profiling.h>
#define FIRST_ID 1
static int __id = FIRST_ID;
static int __lastid = __id;
static std::map<int,timeval> __times;
static std::map<int,TimeList> __timelists;
static bool __enable_profiling = false;

int tic(int id) {
  timeval t;
  gettimeofday(&t, NULL);
  if(id >= FIRST_ID) {
    __times[id] = t;
    return id;
  }
  else {
    __times[__id] = t;
    __lastid = __id;
    return __id++;
  }
}

double toc(int id) {
  if(id < 0) id = __lastid;
  timeval tstart = __times[id];
  timeval tend;
  gettimeofday(&tend, NULL);
  double elapsed = 
    (tend.tv_sec - tstart.tv_sec) +
    (tend.tv_usec - tstart.tv_usec) / 1000000.0;
  return elapsed;
}

int clockavg(int maxSize) {
  TimeList tl;
  tl.maxSize = maxSize;
  __timelists[__id] = tl;
  return __id++;
}

void ticavg(int id) {
  timeval t;
  gettimeofday(&t, NULL);
  __times[id] = t;
}

double tocavg(int id) {
  double elapsed = toc(id);
  TimeList& tl = __timelists[id];
  tl.times.push_back(elapsed);
  if(tl.times.size() > tl.maxSize)
    tl.times.pop_front();
  double avg = 0;
  for(std::list<double>::iterator it = tl.times.begin(); it != tl.times.end(); it++) {
    double tim = *it;
    avg += tim;
  }
  avg /= tl.maxSize;
  return avg;
}

void printtime(int id) {
  if(!__enable_profiling) return;
  double seconds = toc(id);
  printf("time: %2.4f\n", seconds);
}

void printtime(const char* name, int id) {
  if(!__enable_profiling) return;
  printf("%s ", name);
  printtime(id);
}

Timer::Timer() : id_(0), pauseId_(0), elapsed_(0), paused_(0), interval_(1), iterations_(0) { }
Timer::Timer(std::string message, int interval) : Timer() { 
  message_ = message;
  interval_ = interval; 
}

void Timer::start() {
  if(!id_) id_ = tic();
  else tic(id_);
}

void Timer::stop() {
  elapsed_ += toc(id_);
  elapsed_ -= paused_;
  paused_ = 0;
  int i = (iterations_ + 1) % (interval_ + 1);
  if(i == 0) restart();
  else iterations_ = i;
}

void Timer::pause() {
  if(pauseId_) tic(pauseId_);
  else pauseId_ = tic();
}

void Timer::unpause() {
  paused_ += toc(pauseId_);
}

bool Timer::ready() {
  return iterations_ == interval_;
}

void Timer::restart() {
  iterations_ = 0;
  elapsed_ = 0;
  tic(id_);
}

double Timer::avgtime() {
  return elapsed_ / interval_;
}

double Timer::avgrate() {
  return interval_ / elapsed_;
}

double Timer::elapsed() {
  return toc(id_);
}

void Timer::setMessage(std::string message) {
  message_ = message;
}

void Timer::setInterval(int interval) {
  interval_ = interval;
}

void Timer::printAtInterval() {
  if(!__enable_profiling) return;
  if(!ready()) return;
  if(message_.size() > 0) 
    printf("%s: %2.6fms\n", message_.c_str(), elapsed_ / interval_ * 1000);
  else
    printf("%2.6fms\n", elapsed_ / interval_ * 1000);
  restart();
}
