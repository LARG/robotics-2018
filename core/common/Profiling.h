#ifndef PROFILING_H
#define PROFILING_H

#include <sys/time.h>
#include <map>
#include <list>
#include <stdio.h>
#include <iostream>

struct TimeList {
  int maxSize;
  std::list<double> times;
};

int tic(int id = -1);
double toc(int id = -1);
void printtime(int id = -1);
void printtime(const char* message, int id = -1);

int clockavg(int = 10);
void ticavg(int id);
double tocavg(int id);

class Timer {
  public:
    Timer();
    Timer(std::string message, int interval);
    void start();
    void stop();
    void pause();
    void unpause();
    void restart();
    void setMessage(std::string message);
    void setInterval(int interval);
    void printAtInterval();
    bool ready();
    double avgrate();
    double avgtime();
    double elapsed();
  private:
    int id_, pauseId_;
    int interval_, iterations_;
    double elapsed_, paused_;
    std::string message_;
};

#endif
