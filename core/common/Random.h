#ifndef RANDOM_H
#define RANDOM_H

#include <functional>
#include <random>
#include <ctime>

class Random {
  private:
    std::mt19937 random_;
    std::normal_distribution<> ndistribution_;
    std::function<double()> ngenerator_;
    static Random inst_;

  public:
    static int SEED;
    Random(int seed = Random::SEED);
    static const Random& inst();
    double sampleN(double mean, double stddev);
    double sampleU(double max = 1.0);
    double sampleU(double min, double max);
    int sampleU(int max);
    int sampleU(int min, int max);
    bool sampleB(double p = .5);
};
#endif
