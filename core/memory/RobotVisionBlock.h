#ifndef ROBOTVISIONBLOCK_
#define ROBOTVISIONBLOCK_

#include <memory/MemoryBlock.h>
#include <common/RobotInfo.h>
#include <vision/VisionConstants.h>
#include <math/Geometry.h>
#include <vision/structures/HorizonLine.h>
#include <boost/interprocess/offset_ptr.hpp>
#include <vision/enums/Colors.h>

#include <schema/gen/RobotVisionBlock_generated.h>

DECLARE_INTERNAL_SCHEMA(struct RobotVisionBlock : public MemoryBlock {
  public:
    SCHEMA_METHODS(RobotVisionBlock);
    static constexpr std::size_t MAX_SEG_IMAGE_SIZE = 960 * 1280;

    enum VisionBodyPoints {
      LEFT_SHOULDER,
      RIGHT_SHOULDER,
      LEFT_FOOT_TL,
      LEFT_FOOT_TR,
      LEFT_FOOT_BL,
      LEFT_FOOT_BR,
      RIGHT_FOOT_TL,
      RIGHT_FOOT_TR,
      RIGHT_FOOT_BL,
      RIGHT_FOOT_BR,
      NUM_VISION_BODY_POINTS
    };

  public:
    RobotVisionBlock();

    const inline uint8_t* getSegImgTop() const { return seg_img_top_.get(); }
    const inline uint8_t* getSegImgBottom() const { return seg_img_bottom_.get(); }
    inline uint8_t* getSegImgTop() { return seg_img_top_.get(); }
    inline uint8_t* getSegImgBottom() { return seg_img_bottom_.get(); }
    inline void setSegImgTop(uint8_t* img) { seg_img_top_ = img; }
    inline void setSegImgBottom(uint8_t* img) { seg_img_bottom_ = img; }
    
    bool isLocal() const;
    void setLocal();
    
    inline bool isLoaded() const { return loaded_; }
    void setLoaded(bool value=true) { loaded_ = value; }
    
    SCHEMA_PRE_SERIALIZATION({
      ::std::copy(seg_img_top_.get(), seg_img_top_.get() + top_params_.size, seg_img_top_local_.data());
      ::std::copy(seg_img_bottom_.get(), seg_img_bottom_.get() + bottom_params_.size, seg_img_bottom_local_.data());
    });
    SCHEMA_POST_DESERIALIZATION({
      setLocal();
    });

  private:
    SCHEMA_FIELD(bool loaded_);
    boost::interprocess::offset_ptr<uint8_t> seg_img_top_;
    boost::interprocess::offset_ptr<uint8_t> seg_img_bottom_;
    mutable SCHEMA_FIELD(std::array<uint8_t,MAX_SEG_IMAGE_SIZE> seg_img_top_local_);
    mutable SCHEMA_FIELD(std::array<uint8_t,MAX_SEG_IMAGE_SIZE> seg_img_bottom_local_);

  public:
    SCHEMA_FIELD(ImageParams top_params_);
    SCHEMA_FIELD(ImageParams bottom_params_);

    SCHEMA_FIELD(bool doHighResBallScan);
    SCHEMA_FIELD(bool lookForCross);

    SCHEMA_FIELD(double reported_head_stop_time);
    SCHEMA_FIELD(double reported_head_moving);
    SCHEMA_FIELD(float bottomGreenPct);
    SCHEMA_FIELD(float topGreenPct);
    SCHEMA_FIELD(float bottomUndefPct);
    SCHEMA_FIELD(float topUndefPct);
    SCHEMA_FIELD(std::array<Point2D,NUM_VISION_BODY_POINTS> bodyPointsImage);
    SCHEMA_FIELD(HorizonLine horizon);
});

#endif
