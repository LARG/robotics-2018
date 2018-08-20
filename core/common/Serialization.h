#pragma once

#include <flatbuffers/flatbuffers.h>
#include <flatbuffers/idl.h>
#include <flatbuffers/util.h>
#include <memory>

class Serializable {
};

namespace serialization {
  using data_pointer = const uint8_t*;
  using serializer = std::unique_ptr<flatbuffers::FlatBufferBuilder>;
  template<typename... Ts>
  serializer create_serializer(Ts&&... ts) {
    return std::make_unique<serializer::element_type>(std::forward<Ts>(ts)...);
  }
  template<typename T>
  std::string to_json(const T& data) {
    std::string file;
    if(!flatbuffers::LoadFile(T::schema_source(), false, &file)) {
      printf("Error loading block schema from '%s'\n", T::schema_source());
      exit(1);
    }
    flatbuffers::Parser parser;
    const char* include_paths[] = { SCHEMA_DIR, nullptr };
    if(!parser.Parse(file.c_str(), include_paths)) {
      printf("Error parsing block schema: '%s'\n", T::schema_source());
      exit(1);
    }
    auto serializer = serialization::create_serializer();
    auto location = data.serialize(serializer);
    serializer->Finish(location);

    std::string jsgen;
    GenerateText(parser, serializer->GetBufferPointer(), &jsgen);
    return jsgen;
  }
}

#define DECLARE_INTERNAL_SCHEMA(...) __VA_ARGS__
#define DECLARE_EXTERNAL_SCHEMA(T,...)

#define SCHEMA_FIELD(...) __VA_ARGS__
#define SCHEMA_SOURCE(T) SCHEMA_DIR "/" #T ".fbs"

#define SCHEMA_METHODS(T) \
  inline static flatbuffers::Offset<schema::T> serialize(serialization::serializer& __serializer__, const ::T& source) { \
    return source.serialize(__serializer__); \
  } \
  flatbuffers::Offset<schema::T> serialize(serialization::serializer& __serializer__) const; \
  flatbuffers::Offset<void> serialize_void(serialization::serializer& __serializer__) const { return serialize(__serializer__).Union(); } \
  inline static bool deserialize(const serialization::serializer& __serializer__, ::T& target) { \
    return target.deserialize(__serializer__); \
  } \
  bool deserialize(const serialization::serializer& __serializer__); \
  bool deserialize(serialization::data_pointer __data_pointer__); \
  bool deserialize(const schema::T* data); \
  inline static constexpr const char* schema_source() { return SCHEMA_SOURCE(T); } \
  inline static constexpr bool is_serializable() { return true; }
#define NO_SCHEMA(T) \
  inline flatbuffers::Offset<void> serialize(serialization::serializer& __serializer__) const { return 0; } \
  inline flatbuffers::Offset<void> serialize_void(serialization::serializer& __serializer__) const { return 0; } \
  inline bool deserialize(const serialization::serializer& __serializer__) { return false; } \
  inline bool deserialize(serialization::data_pointer __data_pointer__) { return false; } \
  inline static constexpr const char* schema_source() { return SCHEMA_DIR "/" #T ".fbs"; } \
  inline static constexpr bool is_serializable() { return false; }

#define SCHEMA_SERIALIZATION(block)
#define SCHEMA_DESERIALIZATION(block)

#define SCHEMA_PRE_SERIALIZATION(block)
#define SCHEMA_POST_SERIALIZATION(block)

#define SCHEMA_PRE_DESERIALIZATION(block)
#define SCHEMA_POST_DESERIALIZATION(block)
