#pragma once
#include <vector>

class ImageBuffer : public std::vector<unsigned char> {
  public:
    using std::vector<unsigned char>::vector;
    static const ImageBuffer Null;
};
