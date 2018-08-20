#include <memory/LogViewer.h>
#include <common/Util.h>
#include <memory/LogReader.h>
#include <memory/LogWriter.h>
#include <memory/LogMetadata.h>

LogViewer::LogViewer(std::string directory, int start, int finish) : directory_(directory), start_(start), finish_(finish) {
  start_ = start;
  reader_ = std::make_unique<LogReader>(directory);
  mdata_ = std::make_unique<LogMetadata>(reader_->mdata());
  if(finish > 0)
    finish_ = std::min(static_cast<int>(mdata_->frames) - 1, finish);
  else
    finish_ = mdata_->frames - 1;
}

LogViewer::~LogViewer() { }

ImageParams LogViewer::getTopParams(int frame) const {
  MemoryFrame* m = &getFrame(frame);
  ImageBlock* block = nullptr;
  m->getBlockByName(block, "raw_image", false);
  if (block != nullptr)
    return ImageBlock(*block).top_params_;
  return ImageParams::Empty;
}

std::vector<ImageParams> LogViewer::getTopParams() const {
  std::vector<ImageParams> params;
  int count = size();
  for(int i = 0; i < count; i++){
    params.push_back(getTopParams(i));
  }
  return params;
}

ImageParams LogViewer::getBottomParams(int frame) const {
  MemoryFrame* m = &getFrame(frame);
  ImageBlock* block = nullptr;
  m->getBlockByName(block, "raw_image", false);
  if (block != nullptr)
    return ImageBlock(*block).bottom_params_;
  return ImageParams::Empty;
}

std::vector<ImageParams> LogViewer::getBottomParams() const {
  std::vector<ImageParams> params;
  int count = size();
  for(int i = 0; i < count; i++){
    params.push_back(getBottomParams(i));
  }
  return params;
}

ImageBuffer LogViewer::getRawTopImage(int frame) const {
  MemoryFrame* m = &getFrame(frame);
  ImageBlock* block = nullptr;
  m->getBlockByName(block, "raw_image", false);
  if (block != nullptr) {
    ImageBuffer buffer(block->top_params_.rawSize);
    std::copy(block->getImgTop(), block->getImgTop() + buffer.size(), buffer.begin());
    return buffer;
  }
  return ImageBuffer::Null;
}

std::vector<ImageBuffer> LogViewer::getRawTopImages() const {
  std::vector<ImageBuffer> images;
  int count = size();
  for(int i = 0; i < count; i++) {
    images.push_back(getRawTopImage(i));
  }
  return images;
}

ImageBuffer LogViewer::getRawBottomImage(int frame) const {
  MemoryFrame* m = &getFrame(frame);
  ImageBlock* block = nullptr;
  m->getBlockByName(block, "raw_image", false);
  if (block != nullptr) {
    ImageBuffer buffer(block->bottom_params_.rawSize);
    std::copy(block->getImgBottom(), block->getImgBottom() + buffer.size(), buffer.begin());
    return buffer;
  }
  return ImageBuffer::Null;
}

std::vector<ImageBuffer> LogViewer::getRawBottomImages() const {
  std::vector<ImageBuffer> images;
  int count = size();
  for(int i = 0; i < count; i++){
    images.push_back(getRawBottomImage(i));
  }
  return images;
}

MemoryFrame& LogViewer::getFrame(unsigned int idx) const {
  int frame = idx + start_;
  if(frame >= mdata_->frames || frame < 0)
    throw std::runtime_error(util::format("INVALID LOG FRAME REQUESTED: %i --> %i\n", idx, frame));
  mframe_ = std::unique_ptr<MemoryFrame>(reader_->readFrame(frame));
  return *mframe_;
}
    
MemoryFrame& CachedLogViewer::getFrame(unsigned int idx) const {
  int frame = idx + start_;
  auto it = cache_.find(frame);
  if(it == cache_.end()) {
    auto mframe = reader().readFrame(frame);
    cache_.emplace(frame, std::unique_ptr<MemoryFrame>(mframe));
    return *mframe;
  }
  auto& mframe = it->second;;
  return *mframe;
}
