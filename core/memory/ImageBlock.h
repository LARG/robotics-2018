#pragma once

#include <memory/MemoryBlock.h>
#include <common/RobotInfo.h>
#include <boost/interprocess/offset_ptr.hpp>
#include <iostream>
#include <sstream>
#include <fstream>
#include <common/ColorConversion.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <schema/gen/ImageBlock_generated.h>

DECLARE_INTERNAL_SCHEMA(struct ImageBlock : public MemoryBlock {
  public:
    SCHEMA_METHODS(ImageBlock);
    static constexpr std::size_t MAX_RAW_IMAGE_SIZE = 960 * 1280 * 2;
    ImageBlock();

    const inline uint8_t* getImgTop() const { return img_top_.get(); }
    const inline uint8_t* getImgBottom() const { return img_bottom_.get(); }
    inline uint8_t* getImgTop() { return img_top_.get(); }
    inline uint8_t* getImgBottom() { return img_bottom_.get(); }
    inline void setImgTop(uint8_t* img) { img_top_ = img; }
    inline void setImgBottom(uint8_t* img) { img_bottom_ = img; }

    bool isLocal() const;
    void setLocal();

    inline bool isLoaded() const { return loaded_; }
    void setLoaded(bool value=true) { loaded_ = value; }

    void writeImage(const uint8_t* imgraw, std::string path, const ImageParams& iparams);
    void readImage(uint8_t* imgraw, std::string path, const ImageParams& iparams);
    void writeImageBinary(const uint8_t* imgraw, std::string path, const ImageParams& iparams);
    void readImageBinary(uint8_t* imgraw, std::string path, const ImageParams& iparams);

    std::string constructPath(std::string id, std::string extension) const;

    SCHEMA_SERIALIZATION({ return _serialize(__serializer__); });
    flatbuffers::Offset<schema::ImageBlock> _serialize(serialization::serializer& __serializer__) const;

    SCHEMA_DESERIALIZATION({ _deserialize(data); });
    void _deserialize(const schema::ImageBlock* data);

  private:
    SCHEMA_FIELD(bool loaded_);
    boost::interprocess::offset_ptr<uint8_t> img_top_;
    boost::interprocess::offset_ptr<uint8_t> img_bottom_;
    mutable SCHEMA_FIELD(std::array<uint8_t,MAX_RAW_IMAGE_SIZE> img_top_local_);
    mutable SCHEMA_FIELD(std::array<uint8_t,MAX_RAW_IMAGE_SIZE> img_bottom_local_);

  public:
    SCHEMA_FIELD(ImageParams top_params_);
    SCHEMA_FIELD(ImageParams bottom_params_);
});
