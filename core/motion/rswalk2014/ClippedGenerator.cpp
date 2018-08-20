#include "ClippedGenerator.hpp"
//#include "libagent/AgentData.hpp"
#include "utils/body.hpp"
#include "utils/clip.hpp"
//#include "utils/Logger.hpp"
#include "DistributedGenerator.hpp"

using boost::program_options::variables_map;

ClippedGenerator::ClippedGenerator(Generator* g)
   : generator(g),
     old_exists(false) {
}

ClippedGenerator::~ClippedGenerator() {
   delete generator;
}

bool ClippedGenerator::isActive() {
   return generator->isActive();
}


bool ClippedGenerator::isStanding() {
  return ((DistributedGenerator*)generator)->isStanding();
}

bool ClippedGenerator::isWalkActive() {
  return ((DistributedGenerator*)generator)->walkIsActive();
}
      

void ClippedGenerator::reset() {
   generator->reset();
   old_exists = false;
}



void ClippedGenerator::readOptions(std::string path) { //const boost::program_options::variables_map &config) {
   generator->readOptions(path);
}

JointValues ClippedGenerator::makeJoints(ActionCommand::All* request,
                                         Odometry* odometry,
                                         const SensorValues &sensors,
                                         BodyModel &bodyModel,
                                         float ballX,
                                         float ballY) {

   JointValues j = generator->makeJoints(request, odometry, sensors, bodyModel, ballX, ballY);
   for (uint8_t i = 0; i < RSJoints::NUMBER_OF_JOINTS; ++i) {
      // Clip stifnesses
      if (j.stiffnesses[i] >= 0.0f) {
         j.stiffnesses[i] = CLIP(j.stiffnesses[i], 0.0f, 1.0f);
      } else {
         j.stiffnesses[i] = -1.0f;
      }

      // Clip angles
      if (!std::isnan(j.angles[i])) {
         j.angles[i] = RSJoints::limitJointRadians(RSJoints::jointCodes[i],
                                                 j.angles[i]);
      }

      // Clip velocities
      if (old_exists) {
         j.angles[i] = CLIP(j.angles[i],
                            old_j.angles[i] - RSJoints::Radians::MaxSpeed[i],
                            old_j.angles[i] + RSJoints::Radians::MaxSpeed[i]);
      }
   }
   old_exists = true;
   old_j = j;
   return j;
}
