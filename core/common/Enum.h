#pragma once

/*
* @file Tools/Enum.h
* Defines a macro that declares an enum and provides
* a function to access the names of its elements.
*
* @author Thomas Röfer
*/

#include <string>
#include <vector>
#include <iostream>
#include <common/Util.h>

/**
* @class EnumName
* The class converts a single comma-separated string of enum names
* into single entries that can be accessed in constant time.
* It is the worker class for the templated version below.
*/
class EnumName
{
private:
  std::vector<std::string> names_; /**< The vector of enum names. */
  
  /**
  * A method that trims a string, i.e. removes spaces from its
  * beginning and end.
  * @param s The string that is trimmed.
  * @return The string without spaces at the beginning and at its end.
  */
  static std::string trim(const std::string& s);
  
public:
  /**
  * Constructor.
  * @param enums A string that contains a comma-separated list of enum
  *              elements. It is allowed that an element is initialized
  *              with the value of its predecessor. Any other 
  *              initializations are forbidden.
  *              "a, b, numOfLettersBeforeC, c = numOfLettersBeforeC, d" 
  *              would be a legal parameter.
  * @param numOfEnums The number of enums in the string. Reassignments do
  *                   not count, i.e. in the example above, this 
  *                   parameter had to be 4.
  */
  EnumName(const std::string& enums, size_t numOfEnums);

  /**
  * The method returns the name of an enum element.
  * @param index The index of the enum element.
  * @return Its name.
  */
  inline const char* getName(std::size_t e) { 
    const char* result =  valid(e) ? names_[e].c_str() : nullptr;
    return result;
  }
  inline bool valid(size_t e) const {
    return e < names_.size(); 
  } 
  size_t fromName(const char* s) { 
    for(int i = 0; i < names_.size(); i++)
      if(names_[i] == s)
        return (size_t)i;
    return -1;
  }
  inline const std::vector<std::string>& names() const { return names_; }
};

/**
* Defining an enum and a function get<Enum>Name(<Enum>) that can return
* the name of each enum element. The enum will automatically
* contain an element NUM_<Enum>s that reflects the number of
* elements defined.
*/
#define ENUM(Enum, ...) \
  enum Enum {__VA_ARGS__, NUM_##Enum##s}; \
  inline static const char* getName(Enum e) {static EnumName en(#__VA_ARGS__, (size_t) NUM_##Enum##s); return en.getName(static_cast<std::size_t>(e)); } \
\
  inline static Enum fromName_##Enum(const char* s) { \
    static EnumName en(#__VA_ARGS__, static_cast<std::size_t>(NUM_##Enum##s)); \
    Enum e = static_cast<Enum>(en.fromName(s)); \
    if(!en.valid(e)) \
      throw std::runtime_error(util::format("Invalid " #Enum " value supplied for conversion: %s", s)); \
    return e; \
  } \
  inline static Enum fromName_##Enum(const std::string& s) { \
    return fromName_##Enum(s.c_str()); \
  } \
  inline static const char* toName_##Enum(int i) { \
    Enum e = static_cast<Enum>(i); \
    return toName_##Enum(e); \
  } \
  inline static const char* toName_##Enum(Enum e) { \
    static EnumName en(#__VA_ARGS__, static_cast<std::size_t>(NUM_##Enum##s)); \
    const char* result = getName(e); \
    if(result == nullptr) \
      throw std::runtime_error(util::format("Invalid " #Enum " value supplied for conversion: %s", result)); \
    return result; \
  } \
  inline static std::vector<std::string> names_##Enum() { \
    static EnumName en(#__VA_ARGS__, static_cast<std::size_t>(NUM_##Enum##s)); \
    return en.names(); \
  } \

#define ENUM_STREAMS(Enum) \
inline YAML::Emitter& operator<<(YAML::Emitter& emitter, Enum e) { \
  return emitter << toName_##Enum(e); \
} \
inline void operator>>(const YAML::Node& node, Enum& e) { \
  std::string s; \
  node >> s; \
  e = fromName_##Enum(s); \
} \
inline std::ostream& operator<<(std::ostream& os, Enum e) { \
  return os << toName_##Enum(e); \
} \
inline std::istream& operator>>(std::istream& is, Enum& e) { \
  std::string s; \
  is >> s; \
  e = fromName_##Enum(s); \
  return is; \
}

#ifdef SWIG
#define ENUM_CLASS(Enum, ...) \
class Enum { \
  enum Type {__VA_ARGS__, NUM_##Enum##s}; \
};
#else
#define ENUM_CLASS(Enum, ...) \
enum class Enum {__VA_ARGS__, NUM_##Enum##s}; \
class Enum##Methods { \
  public:\
    inline static constexpr int index(Enum e) { return static_cast<int>(e); } \
    inline static constexpr std::size_t Num##Enum##s() { return static_cast<std::size_t>(Enum::NUM_##Enum##s); } \
    inline static const char* getName(Enum e) { \
      static EnumName en(#__VA_ARGS__, static_cast<std::size_t>(Enum::NUM_##Enum##s)); \
      return en.getName(static_cast<std::size_t>(e)); \
    } \
    inline static const char* toName(Enum e) { \
      static EnumName en(#__VA_ARGS__, static_cast<std::size_t>(Enum::NUM_##Enum##s)); \
      return en.getName(static_cast<std::size_t>(e)); \
    } \
    template <typename T, typename = std::enable_if_t<std::is_integral<T>::value>> \
    inline static const char* toName(T t) { \
      static EnumName en(#__VA_ARGS__, static_cast<std::size_t>(Enum::NUM_##Enum##s)); \
      return en.getName(static_cast<std::size_t>(t)); \
    } \
    inline static Enum fromName(const char* s) { \
      static EnumName en(#__VA_ARGS__, static_cast<std::size_t>(Enum::NUM_##Enum##s)); \
      return static_cast<Enum>(en.fromName(s)); \
    } \
    inline static Enum fromName(const std::string& s) { \
      return fromName(s.c_str()); \
    } \
    inline static bool valid(Enum e) { \
      static EnumName en(#__VA_ARGS__, static_cast<std::size_t>(Enum::NUM_##Enum##s)); \
      return en.valid(static_cast<std::size_t>(e)); \
    } \
    inline static std::vector<Enum> values() { \
      static std::vector<Enum> v; \
      if(v.size() == 0) \
        for(int i = 0; i < static_cast<std::size_t>(Enum::NUM_##Enum##s); i++) \
          v.push_back(static_cast<Enum>(i)); \
      return v; \
    } \
    inline static std::vector<std::string> names() { \
      static EnumName en(#__VA_ARGS__, static_cast<std::size_t>(Enum::NUM_##Enum##s)); \
      return en.names(); \
    } \
};

#define ENUM_CLASS_STREAMS(Enum) \
inline YAML::Emitter& operator<<(YAML::Emitter& emitter, Enum e) { \
  return emitter << Enum##Methods::toName(e); \
} \
inline void operator>>(const YAML::Node& node, Enum& e) { \
  std::string s; \
  node >> s; \
  e = Enum##Methods::fromName(s); \
} \
inline std::ostream& operator<<(std::ostream& os, Enum e) { \
  return os << Enum##Methods::toName(e); \
} \
inline std::istream& operator>>(std::istream& is, Enum& e) { \
  std::string s; \
  is >> s; \
  e = Enum##Methods::fromName(s); \
  return is; \
}
#endif
