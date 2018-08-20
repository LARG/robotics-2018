#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include <boost/shared_ptr.hpp>

#include <memory/Lock.h>

#include <memory/MemoryFrame.h>
#include <memory/Lock.h>

#include <memory/RobotVisionBlock.h>


#include "decode.h"

using namespace std;

class RobotBehavior;

class Parser {
 public:
  
  Parser(RobotBehavior* robot, Lock *vision_lock);

  ~Parser();

  bool parse(const string &input);

 protected:  
  vector<string> tokenise(const string &s);
  vector<string> tokeniseCommaDelim(const string &s);
  bool parseTime(const string &str);
  bool parseGameState(const string &str);
  bool parseGyro(const string &str);
  bool parseAccelerometer(const string &str);
  bool parseHear(const string &str);
  bool parseHingeJoint(const string &str);
  bool parseFRP(const string &str);
  bool parseImage(const string &str);
  bool parseSee(const string &str);
  bool parseMyPos(const string &str);
  vector<string> segment(const string &str, const bool &omitEnds);
  void processVision();

 private: 

  RobotBehavior* robot_;
  Lock *vision_lock_;

  base64::Decoder theBase64Decoder; // Sim camera images are in base64

  int uNum;
  int side;

  string teamName;

  bool fProcessedVision;

  char sim_image_[SIM_IMAGE_SIZE];
};

#endif // PARSER_H

