#ifndef EXPONENTIALFILTER_4Y6AXBK7
#define EXPONENTIALFILTER_4Y6AXBK7

class ExponentialFilter {
public:
  ExponentialFilter(float decayRate, float factor = 1.0, float initialValue = 0):
    value(initialValue),
    decayRate(decayRate),
    factor(factor)
  {
  }

  float update(float val) {
    value = decayRate * value + (1.0 - decayRate) * val;
    return getValue();
  }

  float getValue() {
    return factor * value;
  }

  void reset(float initialValue = 0) {
    value = initialValue;
  }

  void setParams(float decayRate, float factor, float initialValue = 0) {
    this->value = initialValue;
    this->decayRate = decayRate;
    this->factor = factor;
  }

private:
  float value;
  float decayRate;
  float factor;
};

#endif /* end of include guard: EXPONENTIALFILTER_4Y6AXBK7 */
