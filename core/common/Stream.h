#pragma once

#include <iostream>
#include <common/Concepts.h>

template<typename T, typename = std::enable_if_t<
  concepts::is_std_container_v<T> && !concepts::is_any_of_v<T, std::string>
>>
std::ostream& operator<<(std::ostream& os, const T& container) {
  bool first = true;
  for(const auto& item : container) {
    if(!first) os << ", ";
    os << item;
    first = false;
  }
  return os;
}
