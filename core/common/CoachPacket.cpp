#include <common/CoachPacket.h>
#include <common/RobotInfo.h>
#include <common/Util.h>

const char* CoachPacket::OUR_GOAL_MESSAGE =       "ball at our goal";
const char* CoachPacket::OUR_SIDE_MESSAGE =       "ball on our end";
const char* CoachPacket::OUR_SIDE_MID_MESSAGE =   "ball on our side";
const char* CoachPacket::MID_MESSAGE =            "ball near middle";
const char* CoachPacket::THEIR_SIDE_MID_MESSAGE = "ball on their side";
const char* CoachPacket::THEIR_SIDE_MESSAGE =     "ball on their end";
const char* CoachPacket::THEIR_GOAL_MESSAGE =     "ball at their goal"; // 18

const char* CoachPacket::REAL_CLOSE_MESSAGE =     "so close"; // 8
const char* CoachPacket::CLOSE_MESSAGE =          "close";
const char* CoachPacket::CENTER_MESSAGE =         "center";
const char* CoachPacket::FAR_MESSAGE =            "far side";
const char* CoachPacket::REAL_FAR_MESSAGE =       "very far";

// "left open" "right open" "front open" "back open"
// ball comma +1
// pnctuation +1
// ball loctn +28
// oppn comma +1
// oppont msg <= 9
// TOTAL SIZE 31

const char* CoachPacket::UNKNOWN_MESSAGE =       "i have no idea what is going on";

const vector<string> CoachPacket::XMESSAGES = {
  OUR_GOAL_MESSAGE, OUR_SIDE_MESSAGE, OUR_SIDE_MID_MESSAGE,
  MID_MESSAGE,
  THEIR_SIDE_MID_MESSAGE, THEIR_SIDE_MESSAGE, THEIR_GOAL_MESSAGE
};
const vector<string> CoachPacket::YMESSAGES = {
  REAL_CLOSE_MESSAGE, CLOSE_MESSAGE, CENTER_MESSAGE, FAR_MESSAGE, REAL_FAR_MESSAGE
};
const vector<string> CoachPacket::IDENTIFIERS = { ".", "!", "?", ";" };

const float CoachPacket::XINC = FIELD_X / CoachPacket::XMESSAGES.size();
const float CoachPacket::YINC = FIELD_Y / CoachPacket::YMESSAGES.size();
const float CoachPacket::XSTART = -FIELD_X / 2;
const float CoachPacket::YSTART = -FIELD_Y / 2;

CoachPacket::CoachPacket() {
  id = -1;
  frameUpdated = -100000;
}

string CoachPacket::encodeMessage() {
  string xpart, ypart, idpart;
  for(int i = 0; i < XMESSAGES.size(); i++) {
    if(ballPos.x < XSTART + XINC * (i + 1)) {
      xpart = XMESSAGES[i];
      break;
    }
  }
  for(int i = 0; i < YMESSAGES.size(); i++) {
    if(ballPos.y < YSTART + YINC * (i + 1)) {
      ypart = YMESSAGES[i];
      break;
    }
  }
  id = (id + 1) % IDENTIFIERS.size();
  idpart = IDENTIFIERS[id];
  string message = xpart + ',' + ypart + idpart;
  return message;
}

bool CoachPacket::decodeMessage(const char* message) {
  string smessage(message);
  return decodeMessage(smessage);
}

bool CoachPacket::decodeMessage(string message) {
  string xpart, ypart, idpart;
  vector<string> parts;
  parts = util::split(message, ',');
  if(parts.size() < 2) return false;
  xpart = parts[0];
  ypart = parts[1];
  idpart = ypart[ypart.size() - 1];
  ypart = ypart.substr(0, ypart.size() - 1);
  for(int i = 0; i < XMESSAGES.size(); i++) {
    if(xpart == XMESSAGES[i]) {
      float xmid = XSTART + i * XINC + XINC/2;
      ballPos.x = xmid;
      break;
    }
  }
  for(int i = 0; i < YMESSAGES.size(); i++) {
    if(ypart == YMESSAGES[i]) {
      float ymid = YSTART + i * YINC + YINC/2;
      ballPos.y = ymid;
      break;
    }
  }
  for(int i = 0; i < IDENTIFIERS.size(); i++) {
    if(idpart == IDENTIFIERS[i]) {
      id = i;
      break;
    }
  }
  ballCov << XINC, 0, 0, YINC;
  ballCov *= ballCov;
  return true;
}
