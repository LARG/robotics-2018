#pragma once

#include <stdint.h>

namespace yuview {
  struct YUVFile {
    uint32_t width;
    uint32_t height;
    std::vector<char> buffer;
  };
}
