#ifndef LINESTACKRET_H
#define LINESTACKRET_H
#define MAX_BLOBS_PER_LINE 50

/// @ingroup vision
struct LineStackRet {
  bool isLine;
  uint16_t pointCount;
  uint16_t lbIndex[MAX_BLOBS_PER_LINE];     // Stores the blob index of all blobs used to form the line. 
  uint16_t lbCount;   // Counts number of blobs in this line
};
#endif
