#ifndef STREAM_BUFFER_H
#define STREAM_BUFFER_H

#include <stdio.h>
#include <iostream>
#include <string.h>
#include <assert.h>
#include <vector>
#include <list>

struct StreamBuffer {
  using IntT = uint64_t;
  IntT size;
  IntT capacity;
  unsigned char* buffer;
  std::list<StreamBuffer> children;

  StreamBuffer();
  StreamBuffer(unsigned char* b, IntT n);
  StreamBuffer(char* b, IntT n);
  void clear();
  void rclear();
  void resize(IntT n);
  void reset();
  void add(const unsigned char* data, IntT n);
  void add(const char* data, IntT n);
  void add(std::istream& is, IntT n);
  void add(std::istream& is);
  void read(const unsigned char* data, IntT n);
  void read(const char* data, IntT n);
  void read(std::istream& is, IntT n);
  void read(std::istream& is);
  template<typename T>
  void read(const std::vector<T>& v) {
    read((unsigned char*)v.data(), sizeof(T) * v.size());
  }
  template<typename T>
  void write(std::vector<T>& dest) {
    int n = this->size / sizeof(T);
    dest.resize(n);
    memcpy((unsigned char*)dest.data(), this->buffer, this->size);
  }
  template<typename T, typename = std::enable_if_t<!std::is_base_of<std::ostream,T>::value>>
  void write(T& dest) {
    if(children.size()) {
      auto sb = children.front();
      children.pop_front();
      sb.write(dest);
    } else {
      memcpy((unsigned char*)&dest, this->buffer, this->size);
    }
  }
  template<typename T, typename = std::enable_if_t<!std::is_base_of<std::ostream,T>::value>>
  void write(T* dest) {
    if(children.size()) {
      auto sb = children.front();
      children.pop_front();
      sb.write(dest);
    } else {
      memcpy((unsigned char*)dest, this->buffer, this->size);
    }
  }
  void write(std::ostream& os);
  void write(std::ostream& os, IntT n);
  void contract();
  void expand();
  std::vector<StreamBuffer> separate() const;
  static StreamBuffer combine(const std::vector<StreamBuffer>& buffers);
  static void combine(const std::vector<StreamBuffer>& buffers, StreamBuffer& combined);
  static void separate(const StreamBuffer& buffer, std::vector<StreamBuffer>& separated);
  static void clear(std::vector<StreamBuffer>& buffers);
  static void combine(const std::list<StreamBuffer>& buffers, StreamBuffer& combined);
  static void separate(const StreamBuffer& buffer, std::list<StreamBuffer>& separated);
  static void clear(std::list<StreamBuffer>& buffers);
};

#endif
