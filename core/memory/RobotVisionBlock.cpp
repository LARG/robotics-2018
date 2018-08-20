#include <memory/RobotVisionBlock.h>

constexpr std::size_t RobotVisionBlock::MAX_SEG_IMAGE_SIZE;

RobotVisionBlock::RobotVisionBlock() : top_params_(Camera::TOP), bottom_params_(Camera::BOTTOM) {
  header.version = 6;
  header.size = sizeof(RobotVisionBlock);
  reported_head_stop_time = 0.0;
  reported_head_moving = false;
  seg_img_top_local_.fill(0);
  seg_img_bottom_local_.fill(0);
  seg_img_top_ = nullptr;
  seg_img_bottom_ = nullptr;
  loaded_ = false;
  bottomGreenPct = topGreenPct = 0;
  bottomUndefPct = topUndefPct = 0;
}

bool RobotVisionBlock::isLocal() const {
  bool isTopLocal = seg_img_top_.get() == seg_img_top_local_.data();
  bool isBottomLocal = seg_img_bottom_.get() == seg_img_bottom_local_.data();
  return isTopLocal && isBottomLocal;
}

void RobotVisionBlock::setLocal() {
  seg_img_top_ = seg_img_top_local_.data();
  seg_img_bottom_ = seg_img_bottom_local_.data();
}
