#include <boost/program_options.hpp>
#include <initializer_list>
#include <sstream>

template<typename T>
class StreamableVector : public std::vector<T> {
  public:
    StreamableVector() = default;
    StreamableVector(const std::vector<T>& v) : std::vector<T>(v) { }
    StreamableVector(const std::initializer_list<T>& l) : std::vector<T>(l) { }
    static constexpr StreamableVector<T>* from_vector(std::vector<T>* v) {
      return static_cast<StreamableVector<T>*>(v);
    }
    friend std::ostream& operator<<(std::ostream& os, const StreamableVector& sv) {
      for(int i = 0; i < sv.size(); i++) {
        if(i > 0)
          os << ",";
        os << sv[i];
      }
      return os;
    }
};

template<typename T>
void validate(boost::any& v,
  const std::vector<std::string>& values,
  StreamableVector<T>*, int) {
  StreamableVector<T> sv;
  for(const auto& v : values) {
    T tval;
    std::stringstream ss(v);
    ss >> tval;
    sv.push_back(tval);
  }
  v = sv;
}

template<typename T, typename charT>
using FixedBase = boost::program_options::typed_value<StreamableVector<T>, charT>;

template<typename T>
std::ostream &operator <<(std::ostream &os, const StreamableVector<T> &v) {
   using namespace std;
   copy(v.begin(), v.end(), ostream_iterator<T>(os));
   return os;
}

template<typename T, int N, typename charT = char>
class FixedMultiValueToken : public FixedBase<T,charT> {
  private:
    StreamableVector<T>* sv_;
  public:
    using base = FixedBase<T,charT>;
    FixedMultiValueToken(std::vector<T>* v) 
     : base(StreamableVector<T>::from_vector(v)), sv_(StreamableVector<T>::from_vector(v)) {
      base::multitoken();
    }

    template<typename... Ts>
    base* default_value(Ts... ts) {
      static_assert(N == sizeof...(Ts), 
        "The number of default values must match the fixed token count specifcatons."
      );
      auto values = StreamableVector<T>{ts...};
      return base::default_value(values);
    }

    unsigned min_tokens() const { return N; }
    unsigned max_tokens() const { return N; }

    base* zero_tokens() {
      base::zero_tokens();
      return *this;
    }
};

template<typename T, int N>
FixedMultiValueToken<T,N>* FixedTokensValue(std::vector<T>* v) {
  auto r = new FixedMultiValueToken<T,N>(v);
  return r; 
}
