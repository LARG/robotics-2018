#pragma once

#include <cmath>
#include <cstdlib>

//const float M_2PI = 2.0f * M_PI;
#ifndef M_2PI
#define M_2PI (2.0f * M_PI)
#endif

namespace static_math {

  // Get the integral log2
  template <class V>
  inline constexpr int log2(V v) {
    return v < 2 ? 1 : 1 + log2(v/2);
  }

  // Round up to the nearest power of 2
  template <class V>
  inline constexpr V round2(V v) {
    return 1 << log2(v);
  }

  /**
  * Returns the sign of a value.
  * \param a The value.
  * \return The sign of \c a.
  */
  template <class V> 
  inline constexpr V sgn(V v) {
    return v < 0 ? -1 : v == 0 ? 0 : 1;
  }
  template <class V> 
  inline constexpr V sign(V v) {
    return sgn(v);
  }

  /**
  * Calculates the square of a value.
  * \param a The value.
  * \return The square of \c a.
  */
  template <class V> 
  inline constexpr V sqr(V v) {
    return v * v;
  }

  // Calculate i % n with the result always being greater than 0
  template <class V, typename = std::enable_if_t<std::is_integral<V>::value>>
  inline constexpr V posmod(V i, V n) { 
    return (i % n + n) % n; 
  }

  template <class V, typename = std::enable_if_t<std::is_floating_point<V>::value>>
  inline constexpr V posfmod(V i, V n) { 
    return fmod(fmod(i, n) + n, n); 
  }
  
  // Force the value v to fall within the specified range
  template <class V> 
  inline constexpr V crop(V v, V lower, V upper) {
    assert(lower <= upper);
    if(lower > upper) return v;
    if(v < lower) v = lower;
    if(v > upper) v = upper;
    return v;
  }
}
/**
* Calculates the sec of a value.
* \param a The value.
* \return The sec of \c a.
*/
template <class V> 
inline V sec(const V& a) {
  return 1/cosf(a);
}

/**
* Calculates the cosec of a value.
* \param a The value.
* \return The cosec of \c a.
*/
template <class V> 
inline V cosec(const V& a) {
  return 1/sinf(a);
}


/** 
* reduce angle to [-pi..+pi]
* \param data angle coded in rad
* \return normalized angle coded in rad
*/
template <class V> 
inline constexpr V normalize(V v) {
  v = fmod(v, M_2PI);
  if(v >  M_PI) v -= M_2PI;
  else if(v < -M_PI) v += M_2PI;
  return v;
}
