#pragma once

#include <assert.h>
#include <sstream>
#include <string>
#include <list>
#include <map>
#include <vector>
#include <set>
#include <stack>
#include <unordered_set>
#include <unordered_map>
#include <memory>

namespace yuview {

  std::string itoa(int i);
  std::string ftoa(float f);
  std::string dtoa(double d);
  std::string ssprintf(const char* format, ...);
  inline bool endswith(std::string const & value, std::string const & ending) {
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
  }


  class YUVException : public std::exception {
    public:
      YUVException(const char* format, ...);
      const char* what() const throw ();
      virtual ~YUVException() throw() = default;
    private:
      std::string _format, _message;
  };

  using std::shared_ptr;
  using std::unique_ptr;
  using std::make_shared;
  using std::make_unique;

  using std::static_pointer_cast;
  using std::dynamic_pointer_cast;
}
