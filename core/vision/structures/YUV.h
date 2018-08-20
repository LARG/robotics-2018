#ifndef YUV_H
#define YUV_H

#include <inttypes.h>

/// @ingroup vision
struct YUV {
  uint8_t y;
  uint8_t u;
  uint8_t y2;
  uint8_t v;
};
#endif
