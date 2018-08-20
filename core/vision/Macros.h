#ifndef MACROS_H
#define MACROS_H

/// @addtogroup vision
/// @{
#define getTopSegPixelValueAt(x, y) *(vblocks_.robot_vision->getSegImgTop() + y * iparams_.width + x)
#define getBottomSegPixelValueAt(x, y) *(vblocks_.robot_vision->getSegImgBottom() + y * iparams_.width + x)
#define getSegPixelValueAt(x, y) (camera_ == Camera::TOP ? getTopSegPixelValueAt(x, y) : getBottomSegPixelValueAt(x, y))
#define SQ(x) (x * x)
#define ROUND(v, step) ((v) - (v % step))
#define FOCUS_RANGE_VERTICAL(c) (c == c_ORANGE ? 8 : c == c_YELLOW ? 16 : 8)
#define FOCUS_RANGE_HORIZONTAL(c) 8
/// @}

#endif
