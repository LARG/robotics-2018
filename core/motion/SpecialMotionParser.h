#ifndef SPECIALMOTION_PARSER_H
#define SPECIAL_MOTION_PARSER_H

#include <string>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <cctype>
#include <fstream>
//#include <motion/SpecialMotion.h>
#include <common/RobotInfo.h>


using namespace std;


const float VOID_NUM=-720;

namespace Motions {
enum mType {
  null=0,
  start,
  end
};

enum motionType{
  Null=0,
  Hardness,
  JointAngle
};
}

class SpecialMotion{
   public:
     SpecialMotion();
     SpecialMotion(int type);
    // SpecialMotion(mType m_t, float t, float stf, jointMotions);
     
     Motions::motionType mType;
     float time;
     float stiffness;
     float jointMotions[NUM_JOINTS];
     
   //  bool executeMotion();
};


class SpecialMotionParser
{
   public:
       SpecialMotionParser();
      // void loadMotionFile();
       static bool ParseMotionFile(const string& mofFilePath,vector<SpecialMotion>& MotionSequence);


    
   private:
       static int JointMapping[NUM_JOINTS];
       bool isCommentedOut(string s);
       string getMotionIdName(string& s);
       
};

#endif
