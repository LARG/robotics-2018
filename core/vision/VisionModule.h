#ifndef VISION_99KDYIX5
#define VISION_99KDYIX5

#include <Module.h>
#include <common/Camera.h>
#include <memory/MemoryCache.h>

class VisionBlocks;
class ImageParams;
class ImageProcessor;

/// @ingroup vision
class VisionModule: public Module {

  public:
    VisionModule();
    ~VisionModule();

    void specifyMemoryBlocks();
    void specifyMemoryDependency();
    void initSpecificModule();
    void processFrame();
    void updateTransforms();

    bool loadColorTables();
    bool loadColorTable(Camera::Type camera, std::string fileName, bool fullpath=false);

    inline ImageProcessor* top_processor() { return top_processor_.get(); }
    inline ImageProcessor* bottom_processor() { return bottom_processor_.get(); }
    inline const MemoryCache& cache() { return cache_; }

#ifndef SWIG
    std::unique_ptr<ImageParams> top_params_, bottom_params_;

    std::array<unsigned char, LUT_SIZE> bottomColorTable;
    std::array<unsigned char, LUT_SIZE> topColorTable;
#endif

    std::string bottomColorTableName;
    std::string topColorTableName;

  private:
#ifndef SWIG
    MemoryCache cache_;
    std::unique_ptr<VisionBlocks> vblocks_;
    std::unique_ptr<ImageProcessor> top_processor_ , bottom_processor_;
#endif

    bool isBottomCamera();
    bool useSimColorTable();
    std::string getDataBase();
    int getRobotId();
    int getTeamColor();
    bool areFeetOnGround();
};

#endif /* end of include guard: VISION_99KDYIX5 */
