#pragma once
#include <memory/StreamBuffer.h>
#include <set>
#include <vector>
#include <list>
#include <fstream>
#include <iostream>
#include <typeinfo>

#include <stdio.h>

#define DEFINE_PRIMITIVE_OVERLOADS(FUNCTION) \
  FUNCTION(double); \
  FUNCTION(float); \
  FUNCTION(int); \
  FUNCTION(char); \
  FUNCTION(bool); 

#define CHECK_BUFFER \
  if(!buffer_) { \
    fprintf(stderr, "Tried to deference a null serialization buffer.\n"); \
    throw -1; \
  }

class BinarySerializable {
  public:
    typedef StreamBuffer Buffer;
    BinarySerializable() : buffer_(NULL){ }
    virtual ~BinarySerializable() {
      if(buffer_) delete buffer_;
    }
    void saveToFile(std::string file) const;
    bool loadFromFile(std::string file);

    friend std::ostream& operator<<(std::ostream& os, const BinarySerializable& s) {
      s.serialize(os);
      return os;
    }

    friend void operator>>(std::istream& is, BinarySerializable& s) {
      s.deserialize(is);
    }

  protected:
    mutable Buffer* buffer_;

    void deserialize(std::istream& is) {
      buffer_ = new Buffer();
      buffer_->read(is);
      buffer_->expand();
      deserialize();
      buffer_->rclear();
      delete buffer_;
      buffer_ = NULL;
    }
    void serialize(std::ostream& os) const {
      buffer_ = new Buffer();
      serialize();
      buffer_->contract();
      buffer_->write(os);
      buffer_->rclear();
      delete buffer_;
      buffer_ = NULL;
    }

    virtual void deserialize() = 0;
    virtual void serialize() const = 0;

    void serializeMember(const std::string& member) const {
      CHECK_BUFFER;
      buffer_->add(member.c_str(), member.length() + 1);
    }
    
    void serializeMember(const BinarySerializable*& member) const {
      if(member) {
        serializeMember(*member);
      } else {
        CHECK_BUFFER;
        Buffer child;
        child.contract();
        buffer_->children.push_back(child);
      }
    }
    
    void serializeMember(const BinarySerializable* const& member) const {
      if(member) {
        serializeMember(*member);
      } else {
        CHECK_BUFFER;
        Buffer child;
        child.contract();
        buffer_->children.push_back(child);
      }
    }

    void serializeMember(const BinarySerializable& member) const {
      CHECK_BUFFER;
      Buffer child;
      member.buffer_ = &child;
      member.serialize();
      member.buffer_ = NULL;
      child.contract();
      buffer_->children.push_back(child);
    }

#define SERIALIZE_PRIMITIVE(T) \
    void serializeMember(const T& member) const { \
      CHECK_BUFFER; \
      buffer_->add((char*)&member, sizeof(T)); \
    }

    DEFINE_PRIMITIVE_OVERLOADS(SERIALIZE_PRIMITIVE);

#define SERIALIZE_CONTAINER(member) { \
    CHECK_BUFFER; \
    auto parent = buffer_; \
    Buffer child; \
    for(const auto& item : member) { \
      buffer_ = &child; \
      serializeMember(item); \
    } \
    child.contract(); \
    parent->children.push_back(child); \
    buffer_ = parent; \
}


    template<typename T>
      void serializeMember(const std::vector<T>& member) const {
        if(std::is_fundamental<T>::value)
          serializePrimitiveVector(member);
        else
          SERIALIZE_CONTAINER(member);
      }
    template<typename T>
      void serializeMember(const std::list<T>& member) const {
        SERIALIZE_CONTAINER(member);
      }

#define DESERIALIZE_PRIMITIVE(T) \
    void deserializeMember(T& member) {\
      CHECK_BUFFER; \
      buffer_->write(member); \
    }

    DEFINE_PRIMITIVE_OVERLOADS(DESERIALIZE_PRIMITIVE);

#define DESERIALIZE_CONTAINER(member,T) { \
    CHECK_BUFFER; \
    auto parent = buffer_; \
    auto child = parent->children.front(); \
    parent->children.pop_front(); \
    child.expand(); \
    while(!child.children.empty()) { \
      member.push_back(T()); \
      buffer_ = &child; \
      deserializeMember(member.back()); \
    } \
    buffer_ = parent; \
    child.rclear(); \
}

    template<typename T>
      void deserializeMember(std::vector<T>& member) {
        if(std::is_fundamental<T>::value)
          deserializePrimitiveVector(member);
        else
          DESERIALIZE_CONTAINER(member,T);
      }
    template<typename T>
      void deserializeMember(std::list<T>& member) {
        DESERIALIZE_CONTAINER(member,T);
      }

    void deserializeMember(std::string& member) {
      CHECK_BUFFER;
      auto child = buffer_->children.front();
      buffer_->children.pop_front();
      member = (const char*)child.buffer;
      child.rclear();
    }

    // Can't use BinarySerializable*& because the compiler can't determine the type to instantiate
    template<typename T>
      void deserializeMember(T*& member) {
        CHECK_BUFFER;
        if(buffer_->children.empty()) {
          member = NULL;
          return;
        }
        auto child = buffer_->children.front();
        buffer_->children.pop_front();
        child.expand();
        if(child.children.size() == 0) {
          member = NULL;
        } else {
          member = new T();
          member->buffer_ = &child;
          member->deserialize();
          member->buffer_ = NULL;
        }
        child.rclear();
      }

    void deserializeMember(BinarySerializable& member) {
      CHECK_BUFFER;
      if(buffer_->children.empty()) return;
      auto child = buffer_->children.front();
      buffer_->children.pop_front();
      child.expand();
      member.buffer_ = &child;
      member.deserialize();
      member.buffer_ = NULL;
      child.rclear();
    }

  private:
    template<typename T>
      void serializePrimitiveVector(const std::vector<T>& member) const {
        CHECK_BUFFER;
        buffer_->add((char*)&member[0], member.size() * sizeof(T));
      }
    template<typename T>
      void deserializePrimitiveVector(std::vector<T>& member) {
        CHECK_BUFFER;
        auto child = buffer_->children.front();
        buffer_->children.pop_front();
        member.assign((const T*)child.buffer, (const T*)(child.buffer + child.size));
        child.rclear();
      }
};

#undef SERIALIZE_CONTAINER
#undef DESERIALIZE_CONTAINER
