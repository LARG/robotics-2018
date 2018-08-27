#include <common/Random.h>
#include <assert.h>

using namespace std;

int Random::SEED = 0;
Random Random::_inst = Random();
random_device Random::_device;

Random::Random(int seed, Random::SeedType type) {
  if(type == SeedType::Number)
    _engine = Engine(seed);
  else
    _engine = Engine(_device());
}

Random& Random::inst() { return _inst; }
