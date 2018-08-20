#pragma once

#include <exception>
#include <string>

class CoreException : public std::exception {
  public:
    CoreException(const std::string& format, ...);
    CoreException(const char* format, ...);
    const char* what() const throw ();
    virtual ~CoreException() throw() = default;
  private:
    std::string _format, _message;
};
