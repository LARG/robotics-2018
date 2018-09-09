#include <vision/VisionModule.h>
#include <common/RobotInfo.h>
#include <vision/ImageProcessor.h>
#include <vision/VisionBlocks.h>
#include <memory/FrameInfoBlock.h>
#include <memory/JointBlock.h>
#include <memory/RobotVisionBlock.h>
#include <memory/SensorBlock.h>
#include <memory/ImageBlock.h>
#include <memory/BodyModelBlock.h>
#include <memory/RobotStateBlock.h>
#include <memory/CameraBlock.h>
#include <memory/WorldObjectBlock.h>
#include <memory/RobotInfoBlock.h>
#include <vision/Logging.h>
#include <common/Util.h>
#include <boost/lexical_cast.hpp>

void VisionModule::specifyMemoryDependency() {
  //last_frame_processed_ = 0;

  requiresMemoryBlock("vision_frame_info");
  requiresMemoryBlock("vision_joint_angles");
  requiresMemoryBlock("robot_vision");
  requiresMemoryBlock("vision_sensors");
  requiresMemoryBlock("raw_image");
  requiresMemoryBlock("robot_state");
  requiresMemoryBlock("vision_body_model");
  requiresMemoryBlock("camera_info");
  requiresMemoryBlock("robot_info");
  requiresMemoryBlock("game_state");
  providesMemoryBlock("world_objects");
}

void VisionModule::specifyMemoryBlocks() {
  getOrAddMemoryBlock(cache_.frame_info,"vision_frame_info");
  getOrAddMemoryBlock(cache_.joint,"vision_joint_angles");
  getOrAddMemoryBlock(cache_.robot_vision,"robot_vision");
  getOrAddMemoryBlock(cache_.sensor,"vision_sensors");
  getOrAddMemoryBlock(cache_.image,"raw_image");
  getOrAddMemoryBlock(cache_.world_object,"world_objects");
  getOrAddMemoryBlock(cache_.robot_state,"robot_state");
  getOrAddMemoryBlock(cache_.body_model,"vision_body_model");
  getOrAddMemoryBlock(cache_.camera,"camera_info");
  getOrAddMemoryBlock(cache_.game_state,"game_state");
  getOrAddMemoryBlock(cache_.robot_info,"robot_info");
  *top_params_ = cache_.image->top_params_;
  *bottom_params_ = cache_.image->bottom_params_;
}

bool VisionModule::areFeetOnGround() {
  float total = 0.0f;
  for(int i = fsrLFL; i <= fsrRRR; i++)
    total += cache_.sensor->values_[i];

  return total > .3;
}


void VisionModule::processFrame() {
  // reset world objects
  cache_.world_object->reset();
  
#ifndef TOOL
  if(!areFeetOnGround()) {
    return;
  }
#endif
  tlog(30, "Processing bottom camera");
  bottom_processor_->processFrame();

  tlog(30, "Processing top camera");
  top_processor_->processFrame();
}

void VisionModule::updateTransforms() {
    top_processor_->updateTransform();
    bottom_processor_->updateTransform();
}

bool VisionModule::useSimColorTable() {
  return (cache_.frame_info->source == MEMORY_SIM);
}

string VisionModule::getDataBase() {
  return memory_->data_path_;
}

int VisionModule::getRobotId() {
  return cache_.robot_state->robot_id_;
}

void VisionModule::initSpecificModule() {
  loadColorTables();
  top_processor_ = std::make_unique<ImageProcessor>(*vblocks_, *top_params_, Camera::TOP);
  bottom_processor_ = std::make_unique<ImageProcessor>(*vblocks_, *bottom_params_, Camera::BOTTOM);
  top_processor_->SetColorTable(topColorTable.data());
  bottom_processor_->SetColorTable(bottomColorTable.data());
  top_processor_->init(textlogger);
  bottom_processor_->init(textlogger);
  if(cache_.robot_state->WO_SELF == WO_TEAM_COACH) {
    top_params_->defaultHorizontalStepScale = 0;
    top_params_->defaultVerticalStepScale = 0;
    tlog(30, "Set coach step scales");
  }
}

VisionModule::VisionModule() {
  printf("Creating vision system"); fflush(stdout);

  bottomColorTableName = "none";
  topColorTableName = "none";
  bottomColorTable.fill(c_UNDEFINED);
  topColorTable.fill(c_UNDEFINED);

  printf("."); fflush(stdout);
  top_params_ = std::make_unique<ImageParams>(Camera::TOP);
  bottom_params_ = std::make_unique<ImageParams>(Camera::BOTTOM);
  
  printf("."); fflush(stdout);
  vblocks_ = std::make_unique<VisionBlocks>(cache_);

  printf(".done!\n");
}

VisionModule::~VisionModule() {
}

bool VisionModule::loadColorTables() {
  bool uniqueColorTablesAvailable = true;
  if (useSimColorTable()) {
    loadColorTable(Camera::BOTTOM, "sim.col");
    // just set top to match bottom (esp so both pointers are the same, and editing either edits both in tool)
    topColorTable = bottomColorTable;
    topColorTableName = bottomColorTableName;
  } else {

    bool topOk = false, bottomOk = false;
    if(cache_.robot_state->WO_SELF == WO_TEAM_COACH) {
      std::string file = "coachtop.col";
      topOk = loadColorTable(Camera::TOP, file.c_str(), false);
    } else {
      // Attempt to load robot-specific color tables if available
      std::string bottomColorTableFile = util::ssprintf("%02ibottom.col", getRobotId());
      bottomOk = loadColorTable(Camera::BOTTOM, bottomColorTableFile.c_str(), false);
      
      std::string topColorTableFile = util::ssprintf("%02itop.col", getRobotId());
      topOk = loadColorTable(Camera::TOP, topColorTableFile.c_str(), false);
    }

    // If not available, then load the default top and bottom color tables
    uniqueColorTablesAvailable = bottomOk && topOk;
    if (!bottomOk) {
      bottomOk = loadColorTable(Camera::BOTTOM, "defaultbottom.col");
    }
    if (!topOk) {
      topOk = loadColorTable(Camera::TOP, "defaulttop.col");
    }

    // If still not available, then load the single default color table
    if (!bottomOk) {
      bottomOk = loadColorTable(Camera::BOTTOM, "default.col");
    }
    if (!topOk) {
      topOk = loadColorTable(Camera::TOP, "default.col");
    }


  }
  return uniqueColorTablesAvailable;
}

bool VisionModule::loadColorTable(Camera::Type camera, std::string fileName, bool fullPath) {
  unsigned char* colorTable = bottomColorTable.data();
  if (camera == Camera::TOP) {
    colorTable=topColorTable.data();
  }
  string colorTableName;
  if (!fullPath) {
#ifdef TOOL
    colorTableName = util::ssprintf("%s/current/%s", getDataBase(), fileName);
#else
    colorTableName = util::ssprintf("%s/%s", getDataBase(), fileName);
#endif
  } else {
    colorTableName = fileName;
  }

  FILE* f=fopen(colorTableName.c_str(), "rb");
  if (f==NULL) {
    std::cout << "Vision: *** ERROR can't load " << colorTableName << " *** for camera " << camera << std::endl << std::flush;
    colorTableName = "none";
    return false;
  }
  std::cout << "Vision: Loaded " << colorTableName << " ! for camera " << camera << std::endl << std::flush;
  bool ok = fread(colorTable,LUT_SIZE,1,f);
  fclose(f);

  if (camera==Camera::TOP) {
    topColorTableName = colorTableName;
  } else {
    bottomColorTableName = colorTableName;
  }

 return ok;
}
