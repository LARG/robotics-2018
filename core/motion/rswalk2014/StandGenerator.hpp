#pragma once

#include "Generator.hpp"
#include <string>

class StandGenerator : Generator {
   public:
      explicit StandGenerator(std::string path);
      ~StandGenerator();
      virtual JointValues makeJoints(ActionCommand::All* request,
                                     Odometry* odometry,
                                     const SensorValues &sensors,
                                     BodyModel &bodyModel,
                                     float ballX,
                                     float ballY);
      virtual bool isActive();
      void reset();
      void stop();
      void readOptions(std::string path); //const boost::program_options::variables_map &config);

   private:
      float phi;
      Generator *posGen;
};
