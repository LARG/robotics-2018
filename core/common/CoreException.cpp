#include <common/CoreException.h>
#include <stdarg.h>

using namespace std;

CoreException::CoreException(const std::string& format, ...) {
  const unsigned int bufsize = 4096;
  thread_local char buffer[bufsize];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, bufsize, format.c_str(), args);
  va_end(args);
  _format = format;
  _message = buffer;
}

CoreException::CoreException(const char* format, ...) {
  const unsigned int bufsize = 4096;
  thread_local char buffer[bufsize];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, bufsize, format, args);
  va_end(args);
  _format = format;
  _message = buffer;
}

const char* CoreException::what() const throw() {
  return _message.c_str();
}
