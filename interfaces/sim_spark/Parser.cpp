#include <cmath>

#include <common/RobotInfo.h>
#include <math/Vector3.h>

#include "RobotBehavior.h"
#include "Parser.h"

using boost::shared_ptr;

#define RAD_T_DEG  180.0/ M_PI
#define DEG_T_RAD  M_PI/180.0

///////////////////////////
//Tak care of side both halves - constructor wrong currently.
///////////////////////////

Parser::Parser(RobotBehavior* robot, Lock* vision_lock) {

  //Both side and UNum not necessarily correct, but stop gap initially.
  this->uNum = 1;

  this->teamName = teamName;
  this->fProcessedVision = false;

  robot_=robot;

  vision_lock_ = vision_lock;
}

Parser::~Parser() {
}

vector<string> Parser::tokenise(const string &s){
  int length = s.length();
  string currentString = "";
  bool currentValid = false;

  vector<string> v;

  for(int i = 0; i < length; ++i){

    char c = s.at(i);
    if(c != '(' && c != ')' && c != ' ') {
      currentString.append(1, c);
    }
    else {
      if(currentString.length() > 0) {
        v.push_back(currentString);
        currentString = "";
      }
    }
  }

  /*
    cout << "Tokens: ";
    for(int i = 0; i < v.size(); ++i){
    cout << v[i] << " ";
    }
    cout << "\n";
  */

  return v;
}

vector<string> Parser::tokeniseCommaDelim(const string &s){
  
  int length = s.length();
  string currentString = "";
  bool currentValid = false;

  vector<string> v;

  for(int i = 0; i < length; ++i){

    char c = s.at(i);
    if(c != '(' && c != ')' && c != ','){
      currentString.append(1, c);
    }
    else{

      if(currentString.length() > 0){
	v.push_back(currentString);
	currentString = "";
      }
    }
  }

  /*
    cout << "Tokens: ";
    for(int i = 0; i < v.size(); ++i){
    cout << v[i] << " ";
    }
    cout << "\n";
  */

  return v;
}

bool Parser::parseTime(const string &str){

  bool valid = false;
  double time = 0;
  this->fProcessedVision = false;
  vector<string> tokens = tokenise(str);
  for(int i = 0; i < tokens.size() - 1; ++i){
    if(!(tokens[i].compare("now"))){
      time = atof(tokens[i + 1].c_str());
      valid = true;
    }
  }

  if(valid){
    //    cout << "Time: " << time << "\n";
    robot_->frame_info_->seconds_since_start = time;
    if (robot_->frame_info_->start_time == -1) {
      robot_->frame_info_->start_time = robot_->frame_info_->seconds_since_start;
      robot_->vision_frame_info_->start_time = robot_->frame_info_->start_time;
    }
  }
  //  else{
  //    cout << str << " not valid.\n";
  //  }

  return valid;
}

bool Parser::parseGameState(const string &str){

  bool gameTimeValid = false;
  bool playModeValid = false;
  bool uNumValid = false;
  bool sideValid = false;

  double gameTime = 0;
  string playModeStr = "";

  int playMode = -1;

  vector<string> tokens = tokenise(str);
  for(int i = 0; i < tokens.size() - 1; ++i){
    
    if(!(tokens[i].compare("t"))){
      gameTime = atof(tokens[i + 1].c_str());
      gameTimeValid = true;
      ++i;
    }
    else if(!(tokens[i].compare("pm"))){
      
      playModeStr = tokens[i + 1];
      
      if(!(playModeStr.compare("BeforeKickOff"))){
        //robot_->game_state_->state=READY; 
        playModeValid = true;
      }
      else if(!(playModeStr.compare("KickOff_Left"))){
        //robot_->game_state_->state=PLAYING; 
        playModeValid = true;
      }
      else if(!(playModeStr.compare("KickOff_Right"))){
        //robot_->game_state_->state=PLAYING; 
        playModeValid = true;
      }
      else if(!(playModeStr.compare("PlayOn"))){
        //robot_->game_state_->state=PLAYING; 
        playModeValid = true;
      }
      else if(!(playModeStr.compare("Kickin_Left"))){
        playModeValid = true;
      }
      else if(!(playModeStr.compare("KickIn_Right"))){
        playModeValid = true;
      }
      else if(!(playModeStr.compare("Goal_Left"))){
        playModeValid = true;
      }
      else if(!(playModeStr.compare("Goal_Right"))){
        playModeValid = true;
      }
      else if(!(playModeStr.compare("GameOver"))){
        playModeValid = true;
      }
      //Extra
      else if(!(playModeStr.compare("corner_kick_left"))){
        playModeValid = true;
      }
      else if(!(playModeStr.compare("corner_kick_right"))){
        playModeValid = true;
      }
      else if(!(playModeStr.compare("goal_kick_left"))){
        playModeValid = true;
      }
      else if(!(playModeStr.compare("goal_kick_right"))){
        playModeValid = true;
      }
      else if(!(playModeStr.compare("PM_OFFSIDE_LEFT"))){
        playModeValid = true;
      }
      else if(!(playModeStr.compare("PM_OFFSIDE_RIGHT"))){
        playModeValid = true;
      }
      else if(!(playModeStr.compare("free_kick_left"))){
        playModeValid = true;
      }
      else if(!(playModeStr.compare("free_kick_right"))){
        playModeValid = true;
      }
      else{
        playModeValid = false;
        cout << "Unknown play mode: " << playModeStr << "\n";
      }
      ++i;
    }
    else if(!(tokens[i].compare("unum"))){
      
      uNum = atoi(tokens[i + 1].c_str());
      if(1 <= uNum && uNum <= 11){
        uNumValid = true;
      }
      ++i;
    }
    else if(!(tokens[i].compare("team"))){
      // Todd: left is blue, right is red
      cout << "Robot team: " << robot_->robot_state_->team_ << " sim team: " << tokens[i+1] << endl;

      if(!(tokens[i + 1].compare("left"))){
        sideValid = true;
        if (robot_->robot_state_->team_ != TEAM_BLUE){
          cout << "ERROR: Sim team is BLUE, robot thinks its RED" << endl;
        }
      }
      else if(!(tokens[i + 1].compare("right"))){
        sideValid = true;
        if (robot_->robot_state_->team_ != TEAM_RED){
          cout << "ERROR: Sim team is RED, robot thinks its BLUE" << endl;
        }
      }
      ++i;
    }
    
  }
  
  if(gameTimeValid){
    //    cout << "Game Time: " << gameTime << "\n";
    //worldModel->setGameTime(gameTime);
  }
  
  if(playModeValid){
    //    cout << "Play Mode: " << playMode << "\n";
    //worldModel->setPlayMode(playMode);
  }

  if(uNumValid){
    //    cout << "UNum: " << uNum << "\n";
    //worldModel->setUNum(uNum);
    //worldModel->setUNumSet(true);
  }

  if(sideValid){
    //    cout << "Side: " << side << "\n";
    //worldModel->setSide(side);
    //worldModel->setSideSet(true);
  }

  bool valid = gameTimeValid || playModeValid;

  //  if(!valid){
  //    cout << str << " not valid.\n";
  //  }

  return valid;
}

bool Parser::parseGyro(const string &str){
  bool valid = false;
  
  double rateX, rateY, rateZ;

  vector<string> tokens = tokenise(str);
  for(int i = 0; i < tokens.size(); ++i){
    if(!tokens[i].compare("rt")){
      if(i + 3 < tokens.size()){
        
        rateX = atof(tokens[i + 1].c_str());
        rateY = atof(tokens[i + 2].c_str());
        rateZ = atof(tokens[i + 3].c_str());
        valid = true;
      }
    }
    
  }
  
  if(valid){
    //cout << "Gyro: " << rateX << " " << rateY << " " << rateZ << "\n";
    // 
    // Apparently X and Y need to swap, also may need to negate something
    robot_->raw_sensor_block_->values_[gyroX] = rateY; 
    robot_->raw_sensor_block_->values_[gyroY] = rateX;
    robot_->raw_sensor_block_->values_[gyroZ] = rateZ;
  }
  
  return valid;
}

bool Parser::parseAccelerometer(const string &str){
  bool valid = false;

  double rateX, rateY, rateZ;

  vector<string> tokens = tokenise(str);
  for(int i = 0; i < tokens.size(); ++i){
    if(!tokens[i].compare("a")){
      if(i + 3 < tokens.size()){
        
        rateX = atof(tokens[i + 1].c_str());
        rateY = atof(tokens[i + 2].c_str());
        rateZ = atof(tokens[i + 3].c_str());
        //cout << "Robot side = " << worldModel->getSide() << ", num = " << worldModel->getUNum() << ", acc = " << VecPosition(rateX, rateY, rateZ) << endl;  
        valid = true;
      }
    }
  }
  
  if(valid){
    
    // Sometimes spurious (very high or NaN) readings come through. Clip them.
    double spuriousThreshold = 20.0;
    if(rateX != rateX || (fabs(rateX) > spuriousThreshold)){
      rateX = 0;
    }
    if(rateY != rateY || (fabs(rateY) > spuriousThreshold)){
      rateY = 0;
    }
    if(rateZ != rateZ || (fabs(rateZ) > spuriousThreshold)){
      rateZ = 0;
    }
    
    // Make signs compatible with agent's local axes.
    // Apparently X and Y need to swap
    double correctedRateX = rateY;
    double correctedRateY = -rateX;
    double correctedRateZ = rateZ;

    robot_->raw_sensor_block_->values_[accelX] = correctedRateX;
    robot_->raw_sensor_block_->values_[accelY] = correctedRateY;
    robot_->raw_sensor_block_->values_[accelZ] = correctedRateZ;
    
    
  }
  //  else{
  //    cout << str << " not valid.\n";
  //  }
  
  return valid;
}

//to handle -- do when needed.
bool Parser::parseHear(const string &str){

  bool valid = false;
  double hearTime;
  bool self;
  double angle;
  string message;
  vector<string> tokens = tokenise(str);
  valid = (tokens.size() == 4);
  
  hearTime = atof(tokens[1].c_str());
  if(!(tokens[2].compare("self"))){
    self = true;
    angle = 0;
  }
  else{
    self = false;
    angle = atof(tokens[2].c_str());
  }

  if (self) return true;

  message = tokens[3];
  TeamPacket tp;
  char temp[sizeof(TeamPacket)];
  int dl = theBase64Decoder.decode(message, temp, sizeof(TeamPacket));
  memcpy(&tp,temp,sizeof(TeamPacket));

  if (tp.robotNumber != robot_->robot_state_->WO_SELF &&
      tp.gcTeam == robot_->game_state_->gameContTeamNum &&
      tp.rbTeam == robot_->robot_state_->team_ &&
      tp.robotNumber > 0 && tp.robotNumber < 5) {
    
    
    // reset the counter
    robot_->team_packets_->sinceLastTeamPacketIn=0;
    
    // copy team packet to memory
    TeamPacket* tpMem = &(robot_->team_packets_->tp[tp.robotNumber]);
    memcpy(tpMem, &tp, sizeof(TeamPacket));
    
    // set some flags about when we received it, etc
    robot_->team_packets_->frameReceived[tp.robotNumber] = robot_->vision_frame_info_->frame_id;
    robot_->team_packets_->ballUpdated[tp.robotNumber] = true;
    robot_->team_packets_->oppUpdated[tp.robotNumber] = true;

    // Populate world objects for team mate position
    robot_->world_objects_->objects_[tp.robotNumber].loc.x = tp.locData.robotX;
    robot_->world_objects_->objects_[tp.robotNumber].loc.y = tp.locData.robotY;
    robot_->world_objects_->objects_[tp.robotNumber].orientation = tp.locData.orient;

    robot_->world_objects_->objects_[tp.robotNumber].sd.x = tp.locData.robotSDX;
    robot_->world_objects_->objects_[tp.robotNumber].sd.y = tp.locData.robotSDY;
    robot_->world_objects_->objects_[tp.robotNumber].sdOrientation = tp.locData.sdOrient;
    
  }
  
  return valid;
}

bool Parser::parseHingeJoint(const string &str){

  bool valid;
  
  string name;
  double angle;

  bool validName = false;
  bool validAngle = false;
  
  int hingeJointIndex = -1;
  

  vector<string> tokens = tokenise(str);
  for(int i = 0; i < tokens.size(); ++i){
    if(!tokens[i].compare("n")){
      if(i + 1 < tokens.size()){
        
        name = tokens[i + 1];
        
        if(!(name.compare("hj1"))){
          hingeJointIndex = HeadYaw; // HJ_H1;
          validName = true;
        }
        else if(!(name.compare("hj2"))){
          hingeJointIndex = HeadPitch; //HJ_H2;
          validName = true;
        }
        else if(!(name.compare("laj1"))){
          hingeJointIndex = LShoulderPitch; ///HJ_LA1;
          validName = true;
        }
        else if(!(name.compare("laj2"))){
          hingeJointIndex = LShoulderRoll; //HJ_LA2;
          validName = true;
        }
        else if(!(name.compare("laj3"))){
          hingeJointIndex = LElbowYaw; //HJ_LA3;
          validName = true;
        }
        else if(!(name.compare("laj4"))){
          hingeJointIndex = LElbowRoll; //HJ_LA4;
          validName = true;
        }
        else if(!(name.compare("raj1"))){
          hingeJointIndex =RShoulderPitch; //HJ_RA1;
          validName = true;
        }
        else if(!(name.compare("raj2"))){
          hingeJointIndex = RShoulderRoll; //HJ_RA2;
          validName = true;
        }
        else if(!(name.compare("raj3"))){
          hingeJointIndex = RElbowYaw; //HJ_RA3;
          validName = true;
        }
        else if(!(name.compare("raj4"))){
          hingeJointIndex = RElbowRoll; //HJ_RA4;
          validName = true;
        }
        else if(!(name.compare("llj1"))){
          hingeJointIndex = LHipYawPitch; //HJ_LL1;
          validName = true;
        }
        else if(!(name.compare("llj2"))){
          hingeJointIndex = LHipRoll; //HJ_LL2;
          validName = true;
        }
        else if(!(name.compare("llj3"))){
          hingeJointIndex = LHipPitch; //HJ_LL3;
          validName = true;
        }
        else if(!(name.compare("llj4"))){
          hingeJointIndex = LKneePitch; //HJ_LL4;
          validName = true;
        }
        else if(!(name.compare("llj5"))){
          hingeJointIndex = LAnklePitch; //HJ_LL5;
          validName = true;
        }
        else if(!(name.compare("llj6"))){
          hingeJointIndex = LAnkleRoll; //HJ_LL6;
          validName = true;
        }
        else if(!(name.compare("rlj1"))){
          hingeJointIndex = RHipYawPitch; //HJ_RL1;
          validName = true;
        }
        else if(!(name.compare("rlj2"))){
          hingeJointIndex = RHipRoll; //HJ_RL2;
          validName = true;
        }
        else if(!(name.compare("rlj3"))){
          hingeJointIndex = RHipPitch; //HJ_RL3;
          validName = true;
        }
        else if(!(name.compare("rlj4"))){
          hingeJointIndex = RKneePitch; //HJ_RL4;
          validName = true;
        }
        else if(!(name.compare("rlj5"))){
          hingeJointIndex = RAnklePitch; //HJ_RL5;
          validName = true;
        }
        else if(!(name.compare("rlj6"))){
          hingeJointIndex = RAnkleRoll; //HJ_RL6;
          validName = true;
        }
      }
    }

    if(!tokens[i].compare("ax")){
      if(i + 1 < tokens.size()){

        angle = atof(tokens[i + 1].c_str());
        validAngle = true;
      }
    }

  }

  valid = validName && validAngle;

  if(valid) {    
    robot_->raw_joint_angles_->values_[hingeJointIndex]=DEG_T_RAD*angle;
  }

  return valid;
}

bool Parser::parseFRP(const string &str){
  // these give the force and the center position of the force, but
  // i'm only populating us with binary on/offs for mean time
  bool valid;
  //std::cout << "PARSE FRP" << std::endl;

  string name;
  vector<string> tokens = tokenise(str);
  if(tokens.size() != 11) return false;
  
  int ind = 0;
  if (tokens[ind++] != "FRP") return false;
  if (tokens[ind++] != "n") return false;
  name = tokens[ind++];
  bool lf = (name == "lf");
  bool rf = (name == "rf");

  if (!lf && !rf) { // Only parse out foot collisions
    //std::cout << "Other collision: " << name << std::endl;
    return true;
  }
  if (tokens[ind++] != "c") return false;
  Vector3<float> coords;
  for (int i = 0; i < 3; i++)
    coords[i] = atof(tokens[ind++].c_str()) * 1000; // convert from m to mm
  if (tokens[ind++] != "f") return false;
  Vector3<float> force;
  for (int i = 0; i < 3; i++)
    force[i] = atof(tokens[ind++].c_str()) * 1000 / 10.0; // convert from kg to g (divide by 10 for some reason)
  if (ind != tokens.size()) return false;

  int startInd = fsrLFL;
  int sign = 1;
  float totalForce = force.abs();
  if (rf) {
    startInd = fsrRFL;
    sign = -1;
  }
  // determine distances from coordinates
  float dists[4];
  float totalDist;
  for (int i = 0; i < 4; i++) {
    Vector3<float> offset;
    for (int j = 0; j < 3; j++)
      offset[j] = robot_->robot_info_->dimensions_.values_[RobotDimensions::FSR_LFL_Offset1+3*i+j];
    offset.y *= sign;
    dists[i] = (coords - offset).abs();
    totalDist += dists[i];
  }

  for (int i = 0; i < 4; i++) {
    robot_->raw_sensor_block_->values_[startInd+i] = totalForce * dists[i] / totalDist;
  }

  //std::cout << "coords: ";
  //for (int i = 0; i < 3; i++) {
    //std::cout << coords[i] << " ";
  //}
  //std::cout << std::endl;
  //std::cout << "force: ";
  //for (int i = 0; i < 3; i++) {
    //std::cout << force[i] << " ";
  //}
  //std::cout << std::endl;

  return true;
}

inline void rgbToYuv444 (int r, int g, int b, int &y, int &u, int &v) {
  y = ( ( (66 * r + 129 * g +  25 * b + 128) >> 8) +  16);
  u = ( ( ( -38 * r -  74 * g + 112 * b + 128) >> 8) + 128);
  v = ( ( ( 112 * r -  94 * g -  18 * b + 128) >> 8) + 128);
}

unsigned char clip255(int x) {
  if (x > 255) return 255;
  if (x < 0) return 0;
  else return (unsigned char)x;
}

bool Parser::parseImage(const string &str) {
  //vector<string> tokens = tokenise(str);
  //int w=atoi(tokens[2].c_str());
  //int h=atoi(tokens[3].c_str());
  //cout << tokens[5].length() << " " << w*h*4 << " " << w << " " << h << endl << flush;
  //for (int i=0; i<tokens.size(); i++) {
  //  cout << tokens[i].length() << endl;
  //}
  //std::cout << "PARSE IMAGE" << std::endl << std::flush;
  int dl = theBase64Decoder.decode(str.data() + 20, SIM_IMAGE_X * SIM_IMAGE_Y * 4,&(sim_image_[0]));
  //std::cout << str.data() << std::endl;
  //std::cout << dl << " " << SIM_IMAGE_SIZE << std::endl << std::flush;
  assert(dl == SIM_IMAGE_SIZE);
  int y, u, v;
  int r, g, b;
  int sim_ind;
  int raw_ind;
  unsigned int sim_resolution = SIM_IMAGE_X * SIM_IMAGE_Y;
  
  assert(SIM_IMAGE_X == 320); // the code below assumes the size of the sim image is 320x240
  assert(IMAGE_X == 640); // the code below assumes the size of the raw image is 640x480
  for (int xind = 0; xind < SIM_IMAGE_X; xind++) {
    for (int yind = 0; yind < SIM_IMAGE_Y; yind++) {
      //sim_ind = xind * SIM_IMAGE_Y * 3 + yind * 3;
      //sim_ind = yind * SIM_IMAGE_X * 3 + xind * 3;
      sim_ind = (sim_resolution - SIM_IMAGE_X * (yind + 1) + xind) * 3;
      r = (unsigned char)sim_image_[sim_ind+0];
      g = (unsigned char)sim_image_[sim_ind+1];
      b = (unsigned char)sim_image_[sim_ind+2];
      rgbToYuv444(r,g,b,y,u,v);
      //v = (int)sim_image_[sim_ind+0];
      //u = (int)sim_image_[sim_ind+1];
      //y = (int)sim_image_[sim_ind+2];
      
      //std::cout << xind << " " << yind << " " << sim_ind << ": " << y << " " << u << " " << v << std::endl;
      for (int i = 0; i < 2; i++) {
        raw_ind = (2 * yind + i) * SIM_IMAGE_X * 4 + xind * 4;
        //raw_ind = 2 * yind * SIM_IMAGE_X + xind;
        robot_->raw_image_->img_top_local_[raw_ind] = clip255(y);
        robot_->raw_image_->img_top_local_[raw_ind+1] = clip255(u);
        robot_->raw_image_->img_top_local_[raw_ind+2] = clip255(y);
        robot_->raw_image_->img_top_local_[raw_ind+3] = clip255(v);
      }
    }
  }
  robot_->raw_image_->img_top_ = &(robot_->raw_image_->img_top_local_[0]);


  return true;
}


bool Parser::parseSee(const string &str){
  vision_lock_->lock();

  bool valid = false;
  vector<string> tokens = tokenise(str);
  robot_->world_objects_->reset();

  // Init ultrasounds to max distance
  robot_->raw_sensor_block_->sonar_left_[0]=2.55;
  robot_->raw_sensor_block_->sonar_right_[0]=2.55;
  
  for (int i=0; i<tokens.size(); i++) {
    //std::cout << tokens[i] << std::endl;

    bool isWO = false;
    int woID = WO_BALL;
    if (tokens[i]=="B") {
      isWO = true;
      woID = WO_BALL;
    }
    // apparently L is blue and R is yellow
    // and 1 is top post and 2 is bottom post
    else if (tokens[i]=="G1R"){
      isWO = true;
      woID = WO_UNKNOWN_LEFT_GOALPOST;
    }
    else if (tokens[i]=="G2R"){
      isWO = true;
      woID = WO_UNKNOWN_RIGHT_GOALPOST;
    }
    else if (tokens[i]=="G1L"){
      isWO = true;
      woID = WO_UNKNOWN_RIGHT_GOALPOST;
    }
    else if (tokens[i]=="G2L"){
      isWO = true;
      woID = WO_UNKNOWN_LEFT_GOALPOST;
    }
    // Todd: just using these as ambiguous ints for now
    else if (tokens[i]=="F1R"){
      isWO = true;
      woID = WO_UNKNOWN_L_1;
      //woID = WO_YELLOW_FIELD_LEFT_L;
    }
    else if (tokens[i]=="F2R"){
      isWO = true;
      woID = WO_UNKNOWN_L_2;
      //woID = WO_YELLOW_FIELD_RIGHT_L;
    }
    else if (tokens[i]=="F1L"){
      isWO = true;
      woID = WO_UNKNOWN_L_1;
      //woID = WO_BLUE_FIELD_RIGHT_L;
    }
    else if (tokens[i]=="F2L"){
      isWO = true;
      woID = WO_UNKNOWN_L_2;
      //woID = WO_BLUE_FIELD_LEFT_L;
    }

    // fill in the next few tokens as a truth world object
    if (isWO){
      i+=2;
      robot_->world_objects_->objects_[woID].seen = true;
      robot_-> world_objects_->objects_[woID].visionDistance = atof(tokens[i].c_str())*1000;
      i++;
      robot_->world_objects_->objects_[woID].visionBearing = atof(tokens[i].c_str())*DEG_T_RAD + robot_->raw_joint_angles_->values_[HeadYaw];
      i++;
      robot_->world_objects_->objects_[woID].visionElevation = atof(tokens[i].c_str())*DEG_T_RAD;
      // check that distance is ok
      if (robot_->world_objects_->objects_[woID].visionDistance > 8000)
        robot_->world_objects_->objects_[woID].seen = false;
    }
    if (tokens[i]== "team"){
      // other robots
      i++;
      // vision and sonar of opponents
      bool opponent = false;
      
      if ((robot_->robot_state_->team_ == 0 && tokens[i] == "1") ||
          (robot_->robot_state_->team_ == 1 && tokens[i] == "0")){
        opponent = true;
      }
      
      i+=3;
      if (tokens[i] == "head"){
        i+=2;
        // dist, bearing, elev
        float dist = atof(tokens[i].c_str())*1000;
        i++;
        float bearing = atof(tokens[i].c_str())*DEG_T_RAD + robot_->raw_joint_angles_->values_[HeadYaw];
        // convert to x,z
        float x = dist * sinf(bearing);
        float z = dist * cosf(bearing);
        /*
        if (opponent) {
          robot_->robot_vision_->relOppoPosVisionX[robot_->robot_vision_->numOppoVision]=x;
          robot_->robot_vision_->relOppoPosVisionZ[robot_->robot_vision_->numOppoVision]=z;
          robot_->robot_vision_->numOppoVision++;
        }
        */
        // fake sonar
        // currently hacked for left, right angles. Should be done more correctly in future
        float sonar_dist = dist/1000.0;
        if ((bearing > DEG_T_RAD*-15) && (bearing < DEG_T_RAD*45)) { // in left cone
          if (sonar_dist < robot_->raw_sensor_block_->sonar_left_[0]) {
            robot_->raw_sensor_block_->sonar_left_[0]=sonar_dist;
          }
        }
        if ((bearing > DEG_T_RAD*-45) && (bearing < DEG_T_RAD*15)) { // in right cone
          if (sonar_dist < robot_->raw_sensor_block_->sonar_right_[0]) {
            robot_->raw_sensor_block_->sonar_right_[0]=sonar_dist;
          }
        }
      }
    }
      
    if (tokens[i]=="mypos") {
      i++;
      double x = atof(tokens[i++].c_str());
      double y = atof(tokens[i++].c_str());
      double z = atof(tokens[i++].c_str());
      double ori = atof(tokens[i++].c_str());
      double bx = atof(tokens[i++].c_str());
      double by = atof(tokens[i++].c_str());
      double bz = atof(tokens[i++].c_str());
      
      robot_->sim_truth_data_->robot_pos_.translation.x = x*1000.0; 
      robot_->sim_truth_data_->robot_pos_.translation.y = y*1000.0;
      robot_->sim_truth_data_->robot_pos_.rotation = ori - robot_->raw_joint_angles_->values_[HeadYaw];
      robot_->sim_truth_data_->ball_pos_.translation.x = bx*1000.0; 
      robot_->sim_truth_data_->ball_pos_.translation.y = by*1000.0;
      double dx = bx - x;
      double dy = by - y;
      robot_->sim_truth_data_->ball_pos_.rotation = ori - atan2(dy,dx); // TODO, check this
      //sim_truth_data_->log_block = true;
    }

  }
  robot_->vision_frame_info_->frame_id++;
  robot_->vision_frame_info_->seconds_since_start = robot_->frame_info_->seconds_since_start;

  vision_lock_->notify_one();
  vision_lock_->unlock();

  return true;
}

bool Parser::parseMyPos(const string &str){
  bool valid = false;
  vector<string> tokens = tokenise(str);
  //  TODO: Daniel changed
  //if(!tokens[0].compare("mypos") && tokens.size() == 4){
  if(!tokens[0].compare("mypos") && tokens.size() >= 4 ){
    double x = atof(tokens[1].c_str());
    double y = atof(tokens[2].c_str());
    double z = atof(tokens[3].c_str());
    std::cout << x << " " << y << " " << z << std::endl;
    //worldModel->setMyPositionGroundTruth(VecPosition(x, y, z));

    // if sent the angle as well
    if( tokens.size() >=5 ) {
      //double angle = Rad2Deg( atof(tokens[4].c_str()) );
      //worldModel->setMyAngDegGroundTruth( angle );
    }

    // if sent ball position as well
    if( tokens.size() >=8 ) {
      double bx = atof(tokens[5].c_str());
      double by = atof(tokens[6].c_str());
      double bz = atof(tokens[7].c_str());
      //VecPosition ballPos = VecPosition(bx, by, bz);
      //worldModel->setBallGroundTruth(ballPos);
    }
  
    valid = true;
    }
  
  return valid;
}

vector<string> Parser::segment(const string &str, const bool &omitEnds){

  //cout << "Segmenting " << str << "\n";

  vector<string> v;

  int ptr = 0;
  int length = str.length();

  if(omitEnds){
    ptr = 1;
    length = str.length() - 1;
  }

  int bracCount = 0;

  string currentString = "";

  do{
    
    while(ptr < length && str.at(ptr) != '('){
      ptr++;
    }

    if(ptr < length){

      currentString = "";

      do{
        char c = str.at(ptr);

        if(c == '('){
          bracCount++;
        }
        else if(c == ')'){
          bracCount--;
        }

        currentString.append(1, c);
        ptr++;

      }while(bracCount != 0 && ptr < length);

      if(bracCount == 0){
        v.push_back(currentString);
        currentString = "";
      }
    }
  }while(ptr < length);

  return v;
}

bool Parser::parse(const string &input){
  bool valid = true;
  vector<string> inputSegments = segment(input, false);
  for (int i = fsrLFL; i<=fsrRRR; i++) {
    robot_->raw_sensor_block_->values_[i]=0.0;
  }
  
  for(int i = 0; i < inputSegments.size(); ++i){
    //if (inputSegments[i].at(1)!= 'I') std::cout << inputSegments[i] << std::endl;
    //Time
    if(inputSegments[i].at(1)== 't'){
      valid = parseTime(inputSegments[i]) && valid;
    }
    else if(inputSegments[i].at(1) == 'G'){
      //GameState
      if(inputSegments[i].at(2) == 'S'){
        valid = parseGameState(inputSegments[i]) && valid;
      }
      //Gyro
      else{
        valid = parseGyro(inputSegments[i]) && valid;
      }
    }
    //Hear
    else if(inputSegments[i].at(1) == 'h'){
      valid = parseHear(inputSegments[i]) && valid;
    }
    //Hinge Joint
    else if(inputSegments[i].at(1) == 'H'){
      valid = parseHingeJoint(inputSegments[i]) && valid;
    }
    //See
    else if(inputSegments[i].at(1) == 'S'){
      valid = parseSee(inputSegments[i]);
    }
    //Image
    else if(inputSegments[i].at(1) == 'I'){
      valid = parseImage(inputSegments[i]);
    }
    //FRP
    else if(inputSegments[i].at(1) == 'F'){
      valid = parseFRP(inputSegments[i]) && valid;
    }
    //Accelerometer
    else if(inputSegments[i].at(1) == 'A') {
      valid = parseAccelerometer(inputSegments[i]) && valid;
    }
    else{
      valid = false;
    }
    
  }
  return valid;
}
