#ifndef COACH_PACKET_H
#define COACH_PACKET_H

#include <Eigen/Core>
#include <common/Field.h>
#include <math/Geometry.h>
#include <string>
#include <sstream>

struct CoachPacket {
  public:
    CoachPacket();
    bool decodeMessage(const char* message);
    bool decodeMessage(std::string message);
    std::string encodeMessage();

    bool send;
    int frameUpdated;
    int id;
    Point2D ballPos;
    Eigen::Matrix<float, 2, 2, Eigen::DontAlign> ballCov;

  private:
    static const char* OUR_GOAL_MESSAGE;
    static const char* THEIR_GOAL_MESSAGE;
    static const char* THEIR_SIDE_MESSAGE;
    static const char* OUR_SIDE_MESSAGE;
    static const char* THEIR_SIDE_MID_MESSAGE;
    static const char* OUR_SIDE_MID_MESSAGE;
    static const char* MID_MESSAGE;

    static const char* REAL_CLOSE_MESSAGE;
    static const char* CLOSE_MESSAGE;
    static const char* FAR_MESSAGE;
    static const char* REAL_FAR_MESSAGE;
    static const char* CENTER_MESSAGE;
    
    static const char* UNKNOWN_MESSAGE;

    static const std::vector<std::string> XMESSAGES, YMESSAGES, IDENTIFIERS;

    static const float XINC, YINC, XSTART, YSTART;
};

#endif
