#include "VisionModule.h"
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
  getOrAddMemoryBlock(vision_frame_info_,"vision_frame_info");
  getOrAddMemoryBlock(joint_angles_,"vision_joint_angles");
  getOrAddMemoryBlock(robot_vision_,"robot_vision");
  getOrAddMemoryBlock(sensors_,"vision_sensors");
  getOrAddMemoryBlock(image_,"raw_image");
  getOrAddMemoryBlock(world_objects_,"world_objects");
  getOrAddMemoryBlock(robot_state_,"robot_state");
  getOrAddMemoryBlock(body_model_,"vision_body_model");
  getOrAddMemoryBlock(camera_info_,"camera_info");
  getOrAddMemoryBlock(game_state_, "game_state");
  getOrAddMemoryBlock(robot_info_,"robot_info");
  *top_params_ = image_->top_params_;
  *bottom_params_ = image_->bottom_params_;
}

bool VisionModule::areFeetOnGround() {
  float total = 0.0f;
  for(int i = fsrLFL; i <= fsrRRR; i++)
    total += sensors_->values_[i];

  return total > .3;
}


void VisionModule::processFrame() {
  // reset world objects
  world_objects_->reset();
  
  if(!areFeetOnGround()) {
    return;
  }

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
  return (vision_frame_info_->source == MEMORY_SIM);
}

string VisionModule::getDataBase() {
  return memory_->data_path_;
}

int VisionModule::getRobotId() {
  return robot_state_->robot_id_;
}

void VisionModule::initSpecificModule() {
  loadColorTables();
  if(top_processor_) delete top_processor_;
  top_processor_ = new ImageProcessor(*vblocks_, *top_params_, Camera::TOP);
  if(bottom_processor_) delete bottom_processor_;
  bottom_processor_ = new ImageProcessor(*vblocks_, *bottom_params_, Camera::BOTTOM);
  top_processor_->SetColorTable(topColorTable);
  bottom_processor_->SetColorTable(bottomColorTable);
  top_processor_->init(textlogger);
  bottom_processor_->init(textlogger);
  if(robot_state_->WO_SELF == WO_TEAM_COACH) {
    top_params_->defaultHorizontalStepScale = 0;
    top_params_->defaultVerticalStepScale = 0;
    tlog(30, "Set coach step scales");
  }
}

VisionModule::VisionModule() {

  printf("Creating vision system");
  fflush(stdout);

  bottomColorTable = new unsigned char [LUT_SIZE];
  topColorTable = new unsigned char [LUT_SIZE];
  bottomColorTableName = "none";
  topColorTableName = "none";
  memset(bottomColorTable, c_UNDEFINED, LUT_SIZE);
  memset(topColorTable, c_UNDEFINED, LUT_SIZE);
  top_params_ = new ImageParams(Camera::TOP), bottom_params_ = new ImageParams(Camera::BOTTOM);
  top_processor_ = bottom_processor_ = NULL;

  vblocks_ = new VisionBlocks(world_objects_, body_model_, joint_angles_, image_, robot_vision_, vision_frame_info_, robot_state_, robot_info_, sensors_, game_state_);


  puts(" Done!");
  fflush(stdout);
}

VisionModule::~VisionModule() {
  if (topColorTable != bottomColorTable)
    delete [] topColorTable;
  delete [] bottomColorTable;
  if(top_processor_) delete top_processor_;
  if(bottom_processor_) delete bottom_processor_;
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
    if(robot_state_->WO_SELF == WO_TEAM_COACH) {
      std::string file = "coachtop.col";
      topOk = loadColorTable(Camera::TOP, file.c_str(), false);
    } else {
      // Attempt to load robot-specific color tables if available
      std::string bottomColorTableFile =
        boost::lexical_cast<std::string>(getRobotId()) +
        "bottom.col";
      std::string topColorTableFile =
        boost::lexical_cast<std::string>(getRobotId()) +
        "top.col";
      bottomOk = loadColorTable(Camera::BOTTOM, bottomColorTableFile.c_str(), false);
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
  unsigned char* colorTable = bottomColorTable;
  if (camera == Camera::TOP) {
    colorTable=topColorTable;
  }
  string colorTableName;
  if (!fullPath) {
#ifdef TOOL
    colorTableName = getDataBase() + "current/" + fileName;
#else
    colorTableName = getDataBase() + fileName;
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
