#pragma once

#include "Walk2014Generator.hpp"
#include "Generator.hpp"
#include "utils/Timer.hpp"
#include <motion/RSWalkParameters.h>


class WalkEnginePreProcessor : Generator {
   public:
      explicit WalkEnginePreProcessor();
      ~WalkEnginePreProcessor();
      JointValues makeJoints(ActionCommand::All* request,
            Odometry* odometry,
            const SensorValues &sensors,
            BodyModel &bodyModel,
            float ballX,
            float ballY);
      bool isActive();
      void readOptions(std::string path);
      void reset();
      void stop();
      bool linedUp;

   public:

      class LineUpEngine {

	public:
         explicit LineUpEngine(Walk2014Generator* walkEngine);
         Walk2014Generator* walkEngine;
         bool hasStarted;
         ActionCommand::Body::Foot foot;
         void reset();
         void start(ActionCommand::Body::Foot foot);
         bool hasEnded(ActionCommand::All* request, float ballX, float ballY);
         void preProcess(ActionCommand::All* request,
               float ballX,
               float ballY);
	 bool linedUp;
         int linedUpCt;
      };

      class DribbleEngine {
	public:
         explicit DribbleEngine(Walk2014Generator* walkEngine);

         enum DribbleState {
            INIT, TURN, FORWARD, END
         };
	
         DribbleState dribbleState;
         Timer dribbleTimer;
         Walk2014Generator* walkEngine;
         ActionCommand::Body::Foot foot;
         void reset();
         bool hasEnded();
         void start(ActionCommand::Body::Foot foot);
         void preProcess(ActionCommand::All* request,
               BodyModel &bodyModel);
      };

public:
      bool isKicking;
      LineUpEngine* lineUpEngine;
      DribbleEngine* dribbleEngine;
      Walk2014Generator* walkEngine;
};
