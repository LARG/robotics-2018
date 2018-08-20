#pragma once

#include <type_traits>

namespace concepts {
  /// Map an arbitrary argument list to void - this is needed for SFINAE-style concepts
  template<class...>
  using void_t = void;

  /// Verify that the template argument is an std container:
  //    1. The container must have a begin() iterator
  //    2. The container must have an end() iterator
  //    3. The container must have a contained value type

  template <typename T, typename = void>
  struct is_std_container : std::false_type { };

  template <typename T>
  struct is_std_container<T,
    void_t<
      decltype(std::declval<T&>().begin()),
      decltype(std::declval<T&>().end()),
      typename T::value_type
    >
  > : std::true_type {};

  template<typename T>
  constexpr bool is_std_container_v = is_std_container<T>::value;
  
  /// is_any_of is a variadic version of std::is_same. It can be
  // used for restricting functions to a predefined set of types, e.g.
  // std::is_any_of_v<T, std::string, const char*, char*> will return true
  // if T is some sort of string type.
  template<typename T, typename... Rest>
  struct is_any_of : std::false_type {};

  template<typename T, typename First>
  struct is_any_of<T, First> : std::is_same<T, First> {};

  template<typename T, typename First, typename... Rest>
  struct is_any_of<T, First, Rest...>
      : std::integral_constant<bool, std::is_same<T, First>::value || is_any_of<T, Rest...>::value>
  {};

  template<typename T, typename First, typename... Rest>
  constexpr bool is_any_of_v = is_any_of<T, First, Rest...>::value;
}
