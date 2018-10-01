
############################ PROJECT SOURCES FILES
# Add source files needed to compile this project

SET(RSWALK_SRCS
   
   # Vision
   perception/vision/RSCamera.cpp
   perception/vision/VisionDefs.cpp

   # Kinematics
   perception/kinematics/Kinematics.cpp
   perception/kinematics/Pose.cpp
   perception/kinematics/Parameters.cpp
   # perception/kinematics/CKF.cpp

   # Types
   types/XYZ_Coord.hpp
   types/PostInfo.cpp

   # Misc
   #utils/Logger.cpp
   thread/Thread.cpp

   # YAML
   GyroConfig.cpp

   # Motion
   ActionGenerator.cpp
   BodyModel.cpp
   ClippedGenerator.cpp
   DeadGenerator.cpp
   DistributedGenerator.cpp
   Walk2014Generator.cpp
   WalkCycle.cpp
   PendulumModel.cpp
   WalkEnginePreProcessor.cpp
   RSWalkModule2014.cpp
   HeadGenerator.cpp
   NullGenerator.cpp
   RefPickupGenerator.cpp
   StandGenerator.cpp

   )

############################ CHECK LIBRARY / EXECUTABLE OPTION

ADD_LIBRARY(rswalk2014 STATIC ${RSWALK_SRCS} )

find_package(PythonLibs 2 REQUIRED)


SET_TARGET_PROPERTIES(rswalk2014 PROPERTIES OUTPUT_NAME "rswalk2014")
SET_TARGET_PROPERTIES(rswalk2014 PROPERTIES PREFIX "lib")
SET_TARGET_PROPERTIES(rswalk2014 PROPERTIES CLEAN_DIRECT_OUTPUT 1)

ADD_CUSTOM_COMMAND ( OUTPUT version.cpp
   COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/bin/genversion.pl > version.cpp
)

SET ( SWIG_DEPENDENCIES
   utils/body.hpp
   utils/boostSerializationVariablesMap.hpp
   utils/SPLDefs.hpp
   utils/speech.hpp
   perception/kinematics/Parameters.hpp
   perception/vision/WhichCamera.hpp
   perception/kinematics/Pose.hpp
   types/Point.hpp
   types/BBox.hpp
   types/ActionCommand.hpp
   types/Odometry.hpp
   types/JointValues.hpp
   types/SensorValues.hpp
   types/RRCoord.hpp
   types/AbsCoord.hpp
   types/PostInfo.hpp
)

ADD_DEFINITIONS( -DEIGEN_DONT_ALIGN )


#SET_SOURCE_FILES_PROPERTIES( RobotModule.cpp PROPERTIES GENERATED TRUE )
SET_SOURCE_FILES_PROPERTIES( version.cpp PROPERTIES GENERATED TRUE )
SET_SOURCE_FILES_PROPERTIES( log.cpp PROPERTIES GENERATED TRUE )

############################ SET LIBRARIES TO LINK WITH
# Add any 3rd party libraries to link each target with here
find_package(Boost COMPONENTS system program_options thread serialization regex unit_test_framework REQUIRED)
find_package(ZLIB    REQUIRED)
find_package(BZip2   REQUIRED)
#find_package(Threads REQUIRED)
SET ( RUNSWIFT_BOOST  ${Boost_SYSTEM_LIBRARY}
                      ${Boost_REGEX_LIBRARY}
                      ${Boost_THREAD_LIBRARY}
                      ${Boost_PROGRAM_OPTIONS_LIBRARY}
                      ${Boost_SERIALIZATION_LIBRARY} )

