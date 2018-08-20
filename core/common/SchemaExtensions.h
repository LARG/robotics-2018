#include <array>
#include <vector>

namespace schema {
  namespace std {
    template<typename T, ::std::size_t N>
    inline void deserialize(const flatbuffers::Vector<T>* input, ::std::array<T,N>& a) {
      ::std::copy(input->begin(), input->end(), a.begin());
    }
    template<typename T>
    inline void deserialize(const flatbuffers::Vector<T>* input, ::std::vector<T>& v) {
      v.resize(input->size());
      ::std::copy(input->begin(), input->end(), v.begin());
    }
    template<typename T, typename U, ::std::size_t N>
    inline void deserialize_composite_external(const flatbuffers::Vector<T>* input, ::std::array<U,N>& a) {
      assert(input->size() <= a.size());
      for(int i = 0; i < input->size(); i++) {
        const auto& source = (*input)[i];
        U& target = a[i];
        deserialize(source, target);
      }
    }
    template<typename T, typename U>
    inline void deserialize_composite_external(const flatbuffers::Vector<T>* input, ::std::vector<U>& v) {
      v.resize(input->size());
      for(int i = 0; i < input->size(); i++) {
        const auto& source = (*input)[i];
        U& target = v[i];
        deserialize(source, target);
      }
    }
    template<typename T, typename U, ::std::size_t N>
    inline void deserialize_composite_internal(const flatbuffers::Vector<T>* input, ::std::array<U,N>& a) {
      assert(input->size() <= a.size());
      for(int i = 0; i < input->size(); i++) {
        const auto& source = (*input)[i];
        U& target = a[i];
        target.deserialize(source);
      }
    }
    template<typename T, typename U>
    inline void deserialize_composite_internal(const flatbuffers::Vector<T>* input, ::std::vector<U>& v) {
      v.resize(input->size());
      for(int i = 0; i < input->size(); i++) {
        const auto& source = (*input)[i];
        U& target = v[i];
        target.deserialize(source);
      }
    }
  }
}
