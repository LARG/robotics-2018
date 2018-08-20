#include "CalibratorSettings.h"

CalibratorSettings::CalibratorSettings() {
  boardSize.width = 8;
  boardSize.height = 6;
  flags = 0;
  patternType = CHESSBOARD;
  imageSize.width = 640;
  imageSize.height = 480;
  squareSize = 25.f;
  aspectRatio = 1.f;
}

