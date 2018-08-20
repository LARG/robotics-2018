#include <memory/ImageBlock.h>
#include <common/Util.h>
#include <common/SchemaExtensions.h>
#include <yuview/YUVImage.h>
#include <common/Util.h>

constexpr std::size_t ImageBlock::MAX_RAW_IMAGE_SIZE;

using namespace std;
  
ImageBlock::ImageBlock() : top_params_(Camera::TOP), bottom_params_(Camera::BOTTOM) {
  header.version = 4;
  header.size = sizeof(ImageBlock);
  img_top_local_.fill(0);
  img_bottom_local_.fill(0);
  img_top_ = nullptr;
  img_bottom_ = nullptr;
  loaded_ = false;
}

bool ImageBlock::isLocal() const {
  bool isTopLocal = img_top_.get() == img_top_local_.data();
  bool isBottomLocal = img_bottom_.get() == img_bottom_local_.data();
  return isTopLocal && isBottomLocal;
}

void ImageBlock::setLocal() {
  img_top_ = img_top_local_.data();
  img_bottom_ = img_bottom_local_.data();
}

void ImageBlock::writeImage(const uint8_t* imgraw, string path, const ImageParams& iparams) {
  cv::Mat cvimage = color::rawToMat(imgraw, iparams);
  cv::imwrite(path, cvimage);
}

void ImageBlock::readImage(uint8_t* imgraw, string path, const ImageParams& iparams) {
  cv::Mat cvimage = cv::imread(path);
  if(cvimage.data) {
    color::matToRaw(cvimage, imgraw, iparams);
    loaded_ = true;
  }
  else
    loaded_ = false;
}
  
void ImageBlock::writeImageBinary(const uint8_t *imgraw, string path, const ImageParams &iparams) {
  ofstream out(path, ios::out | ios::binary);
  out.write((const char*)imgraw, iparams.rawSize);
}

void ImageBlock::readImageBinary(uint8_t* imgraw, string path, const ImageParams& iparams) {
  ifstream in(path, ios::in | ios::binary);
  in.read((char*)imgraw, iparams.rawSize);
}

string ImageBlock::constructPath(string id, string extension) const {
  return util::ssprintf("%s/%s_%04i.%s", directory_, id.c_str(), frame_id_, extension.c_str());
}
    
flatbuffers::Offset<schema::ImageBlock> ImageBlock::_serialize(serialization::serializer& __serializer__) const {
  flatbuffers::Offset<flatbuffers::Vector<uint8_t>> top_alloc, bottom_alloc;
  if(buffer_logging_) {
    top_alloc = __serializer__->CreateVector(this->img_top_.get(), this->top_params_.rawSize);
    bottom_alloc = __serializer__->CreateVector(this->img_bottom_.get(), this->bottom_params_.rawSize);
  } else {
    auto tfile = yuview::YUVImage::CreateFromRawBuffer(
      this->img_top_.get(), this->top_params_.width, this->top_params_.height
    );
    tfile.save(constructPath("top", "yuv"));
    auto bfile = yuview::YUVImage::CreateFromRawBuffer(
      this->img_bottom_.get(), this->bottom_params_.width, this->bottom_params_.height
    );
    bfile.save(constructPath("bottom", "yuv"));
  }
  auto top_params__alloc = this->top_params_.serialize(__serializer__);
  auto bottom_params__alloc = this->bottom_params_.serialize(__serializer__);
  schema::ImageBlockBuilder __builder__(*__serializer__);
  __builder__.add_loaded_(this->loaded_);
  __builder__.add_top_params_(top_params__alloc);
  __builder__.add_bottom_params_(bottom_params__alloc);
  if(buffer_logging_) {
    __builder__.add_img_top_local_(top_alloc);
    __builder__.add_img_bottom_local_(bottom_alloc);
  }
  return __builder__.Finish();
}

void ImageBlock::_deserialize(const schema::ImageBlock* data) {
  this->loaded_ = data->loaded_();
  this->top_params_.deserialize(data->top_params_());
  this->bottom_params_.deserialize(data->bottom_params_());
  auto tpath = constructPath("top", "yuv");
  if(!buffer_logging_ && util::fexists(tpath)) {
    auto tfile = yuview::YUVImage::ReadSerializedObject(tpath);
    std::copy(tfile.buffer(), tfile.buffer() + tfile.dataSize(), this->img_top_local_.begin());
  } else {
    schema::std::deserialize(data->img_top_local_(), this->img_top_local_);
    if(!buffer_logging_) {
      auto tfile = yuview::YUVImage::CreateFromRawBuffer(
        this->img_top_local_.begin(), this->top_params_.width, this->top_params_.height
      );
      tfile.save(tpath);
    }
  }
  auto bpath = constructPath("bottom", "yuv");
  if(!buffer_logging_ && util::fexists(bpath)) {
    auto bfile = yuview::YUVImage::ReadSerializedObject(bpath);
    std::copy(bfile.buffer(), bfile.buffer() + bfile.dataSize(), this->img_bottom_local_.begin());
  } else {
    schema::std::deserialize(data->img_bottom_local_(), this->img_bottom_local_);
    if(!buffer_logging_) {
      auto bfile = yuview::YUVImage::CreateFromRawBuffer(
        this->img_bottom_local_.begin(), this->bottom_params_.width, this->bottom_params_.height
      );
      bfile.save(bpath);
    }
  }
  setLocal();
}
