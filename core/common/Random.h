#pragma once

#include <random>
#include <map>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cassert>
#include <common/Concepts.h>

class Random {
  public:
    static int SEED;
#ifndef SWIG
  private:
    using Engine = std::mt19937;
    static std::random_device _device;
    Engine _engine;
    static Random _inst;

  public:
    enum class SeedType {
      Device,
      Number
    };
    Random(int seed = Random::SEED, SeedType type = SeedType::Number);
    static Random& inst();
    inline static Engine& engine() { return _inst._engine; }
    
    /// Sample from a normal distribution with the specified mean and standard deviation
    //template<typename T, typename = std::enable_if_t<
    //  std::is_floating_point<T>::value
    //>>

    /// Sample from a uniform distribution in the range [min,max]
    template<typename T, typename = std::enable_if_t<
      std::is_floating_point<T>::value
    >, typename = T>
    inline T sampleN(T mean, T stddev) {
      return std::normal_distribution<T>(mean, stddev)(_engine);
    }

    inline float sampleN() {
      return std::normal_distribution<double>(0.0, 1.0)(_engine);
    }

    /// Sample from a uniform distribution in the range [0,max]
    template<typename T = float, typename = std::enable_if_t<
      std::is_floating_point<T>::value
    >, typename = T>
    inline T sampleU(T max = static_cast<T>(1.0)) {
      assert(max > static_cast<T>(0.0));
      return sampleU(static_cast<T>(0.0), max);
    }

    /// Sample from a uniform distribution in the range [min,max]
    template<typename T, typename = std::enable_if_t<
      std::is_floating_point<T>::value
    >, typename = T>
    inline T sampleU(T min, T max) {
      return std::uniform_real_distribution<T>(min, max)(_engine);
    }

    /// Sample from a uniform distribution in the range [0,max]
    template<typename T, typename = std::enable_if_t<
      std::is_integral<T>::value
    >>
    inline T sampleU(T max) {
      assert(max >= 1);
      return sampleU(static_cast<T>(0), max);
    }
    
    /// Sample from a uniform distribution in the range [min,max]
    template<typename T, typename = std::enable_if_t<
      std::is_integral<T>::value
    >>
    inline T sampleU(T min, T max) {
      return std::uniform_int_distribution<T>(min, max)(_engine);
    }

    /// Sample from a Bernoulli distribution with success probability p
    inline bool sampleB(float p = .5f) {
      return sampleU(1.0f) < p;
    }

    /// Choose a random item from the container
    /// Required: T is an STL container
    template <typename T, typename = std::enable_if_t<concepts::is_std_container_v<T>>>
    auto choice(const T& items) {
      auto it = items.begin();
      auto n = sampleU(items.size() - 1);
      std::advance(it, n);
      return *it;
    }

    /// Create a random subset from the container
    /// Required: T is an STL container
    template <typename T, typename = std::enable_if_t<concepts::is_std_container_v<T>>>
    auto subset(const T& items, float p) {
      T subs;
      for(const auto& i : items)
        if(sampleB(p)) 
          subs.insert(subs.end(), i);
      return subs;
    }

    /// Shuffle a copy of the supplied container
    /// Required: T is an STL container
    template <typename T, typename = std::enable_if_t<concepts::is_std_container_v<T>>>
    auto shuffle_copy(const T& items) {
      std::vector<typename T::value_type> shuffled(items.begin(), items.end());
      std::shuffle(shuffled.begin(), shuffled.end(), _engine);
      return shuffled;
    }
    
    /// Shuffle the supplied container in-place
    /// Required: T is an STL container
    template <typename T, typename = std::enable_if_t<concepts::is_std_container_v<T>>>
    void shuffle_inplace(T& items) {
      std::shuffle(items.begin(), items.end(), _engine);
    }
#endif
};
