#include <Common.h>
#include <stdarg.h>

namespace yuview {
  using namespace std;

  YUVException::YUVException(const char* format, ...) {
    const unsigned int bufsize = 4096;
    thread_local char buffer[bufsize];
    va_list args;
    va_start(args, format);
    vsnprintf (buffer, bufsize, format, args);
    va_end (args);
    _format = format;
    _message = buffer;
  }

  const char* YUVException::what() const throw() {
    return _message.c_str();
  }

  string itoa(int i) {
    ostringstream ss;
    ss << i;
    return ss.str();
  }

  string ftoa(float f) {
    return ssprintf("%2.2f", f);
  }

  string dtoa(double d) {
    return ftoa(d);
  }

  string ssprintf(const char* format, ...) {
    const unsigned int bufsize = 4096;
    thread_local char buffer[bufsize];
    va_list args;
    va_start(args, format);
    vsnprintf (buffer, bufsize, format, args);
    va_end (args);
    return string(buffer);
  }
}
