#pragma once

#include "Generator.hpp"


class ClippedGenerator : Generator {
   public:
      explicit ClippedGenerator(Generator* g);
      ~ClippedGenerator();
      virtual JointValues makeJoints(ActionCommand::All* request,
                                     Odometry* odometry,
                                     const SensorValues &sensors,
                                     BodyModel &bodyModel,
                                     float ballX,
                                     float ballY);
      virtual bool isActive();
      bool isStanding();
      bool isWalkActive();
      void reset();
      void readOptions(std::string path);

      Generator* generator;
      JointValues old_j;
      bool old_exists;
};
