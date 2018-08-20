#include <common/Random.h>

int Random::SEED = 0;
Random Random::inst_ = Random();

Random::Random(int seed) :
  random_(seed),
  ndistribution_(0.0,1.0),
  ngenerator_(std::bind(ndistribution_, random_)) {
}

const Random& Random::inst() { return inst_; }

double Random::sampleN(double mean, double stddev) {
  double v = ngenerator_();
  return v * stddev + mean;
}

double Random::sampleU(double max) {
  return (double)random_()/random_.max() * max;
}

double Random::sampleU(double min, double max) {
  return (double)random_()/random_.max() * (max - min) + min;
}

int Random::sampleU(int max) {
  return random_() % (max + 1);
}

int Random::sampleU(int min, int max) {
  return random_() % (max - min + 1) + min;
}

bool Random::sampleB(double p) {
  return sampleU() < p;
}
