#pragma once
#ifndef SWIG
template <typename Container, typename Key, typename Value>
class ItemIterator {
  protected:
    Container& _container;
    Key _key;
  public:
    ItemIterator(const ItemIterator& other) = default;
    ItemIterator(ItemIterator&& other) = default;
    ItemIterator& operator=(const ItemIterator& other) {
      _key = other._key;
      return *this;
    }
    ItemIterator& operator=(ItemIterator&& other) {
      _key = std::move(other._key);
      return *this;
    }
    ItemIterator(Container& container, Key key) 
      : _container(container), _key(key) {
    }
    virtual ~ItemIterator() = default;
    virtual Value operator*() const = 0;
    ItemIterator& operator--() {
      --_key;
      return *this;
    }
    ItemIterator& operator++() {
      ++_key;
      return *this;
    }
    inline bool operator==(const ItemIterator& other) {
      return this->_key == other._key;
    }
    inline bool operator!=(const ItemIterator& other) {
      return !(*this == other);
    }
};

template <typename Container, typename Key, typename Value>
class ConstItemIterator {
  protected:
    const Container& _container;
    Key _key;
  public:
    ConstItemIterator(const ConstItemIterator& other) = default;
    ConstItemIterator(ConstItemIterator&& other) = default;
    ConstItemIterator& operator=(const ConstItemIterator& other) {
      _key = other._key;
      return *this;
    }
    ConstItemIterator& operator=(ConstItemIterator&& other) {
      _key = std::move(other._key);
      return *this;
    }
    ConstItemIterator(const Container& container, Key key) 
      : _container(container), _key(key) {
    }
    virtual ~ConstItemIterator() = default;
    virtual Value operator*() const = 0;
    ConstItemIterator& operator--() {
      --_key;
      return *this;
    }
    ConstItemIterator& operator++() {
      ++_key;
      return *this;
    }
    inline bool operator==(const ConstItemIterator& other) {
      return this->_key == other._key;
    }
    inline bool operator!=(const ConstItemIterator& other) {
      return !(*this == other);
    }
};

#endif
