#ifndef ROBOTINFO_R3BSIHAC
#define ROBOTINFO_R3BSIHAC

#define IGNORE_COACH_DELAYS true

#include <string>
#include <cmath>
#include <vector>
#include <sstream>
#include <iostream>
#include <sys/time.h>
#include <common/Enum.h>
#include <vision/VisionConstants.h>

#define FUNCTION_IS_NOT_USED __attribute__ ((unused))

//#define RAD_T_DEG  180.0 / M_PI
//#define DEG_T_RAD  M_PI / 180.0f

#define SIGMA 1.0

// piyushk: added these #defines for vision port
#define HIPOFFSETZ 85.0
#define NECKOFFSETZ 230

#define LUT_BIT 7
#define LUT_SIZE 128*128*128

//#define SIM_IMAGE_X 640
//#define SIM_IMAGE_Y 480
// I think these are actually 320x240 - SAM
#define SIM_IMAGE_X 320
#define SIM_IMAGE_Y 240
#define SIM_IMAGE_SIZE (SIM_IMAGE_X * SIM_IMAGE_Y * 3)

#define NUM_SONAR_VALS 10

ENUM_CLASS(SupportBase,
  SensorFoot,
  LeftFoot,
  RightFoot,
  TorsoBase
);

enum Joint {
  HeadYaw,
  HeadPitch,
  LHipYawPitch,
  LHipRoll,
  LHipPitch,
  LKneePitch,
  LAnklePitch,
  LAnkleRoll,
  RHipYawPitch,
  RHipRoll,
  RHipPitch,
  RKneePitch,
  RAnklePitch,
  RAnkleRoll,
  LShoulderPitch,
  LShoulderRoll,
  LElbowYaw,
  LElbowRoll,
  RShoulderPitch,
  RShoulderRoll,
  RElbowYaw,
  RElbowRoll,
  NUM_JOINTS
};


static std::string JointNames[] = {
  "HeadYaw",
  "HeadPitch",
  "LHipYawPitch",
  "LHipRoll",
  "LHipPitch",
  "LKneePitch",
  "LAnklePitch",
  "LAnkleRoll",
  "RHipYawPitch",
  "RHipRoll",
  "RHipPitch",
  "RKneePitch",
  "RAnklePitch",
  "RAnkleRoll",
  "LShoulderPitch",
  "LShoulderRoll",
  "LElbowYaw",
  "LElbowRoll",
  "RShoulderPitch",
  "RShoulderRoll",
  "RElbowYaw",
  "RElbowRoll"
};
enum Sensor {
  gyroX,
  gyroY,
  gyroZ,
  accelX,
  accelY,
  accelZ,
  angleX,
  angleY,
  angleZ,
  battery,
  fsrLFL,
  fsrLFR,
  fsrLRL,
  fsrLRR,
  fsrRFL,
  fsrRFR,
  fsrRRL,
  fsrRRR,
  bumperLL,
  bumperLR,
  bumperRL,
  bumperRR,
  centerButton,
  headFront,
  headMiddle,
  headRear,
  NUM_SENSORS
};

static int FIRST_ANGLE_SENSOR = gyroX;
static int LAST_ANGLE_SENSOR = angleZ;
static int NUM_ANGLE_SENSORS = LAST_ANGLE_SENSOR - FIRST_ANGLE_SENSOR + 1;

static std::string SensorNames[] = {
  "gyroX",
  "gyroY",
  "gyroZ",
  "accelX",
  "accelY",
  "accelZ",
  "angleX",
  "angleY",
  "angleZ",
  "battery",
  "fsrLFL",
  "fsrLFR",
  "fsrLRL",
  "fsrLRR",
  "fsrRFL",
  "fsrRFR",
  "fsrRRL",
  "fsrRRR",
  "centerButton",
  "bumperLL",
  "bumperLR",
  "bumperRL",
  "bumperRR",
  "headFront",
  "headMiddle",
  "headRear"
};

enum {
  HeadPan = HeadYaw,
  HeadTilt = HeadPitch,
  NUM_HEAD_JOINTS = 2,
  BODY_JOINT_OFFSET = NUM_HEAD_JOINTS,
  NUM_BODY_JOINTS = NUM_JOINTS - NUM_HEAD_JOINTS,
  ARM_JOINT_FIRST = LShoulderPitch,
  ARM_JOINT_LAST = RElbowRoll,
  NUM_ARM_JOINTS = ARM_JOINT_LAST - ARM_JOINT_FIRST + 1,
  LEG_JOINT_FIRST = LHipYawPitch,
  LEG_JOINT_LAST = RAnkleRoll,
  NUM_LEG_JOINTS = LEG_JOINT_LAST - LEG_JOINT_FIRST + 1
};

typedef float Joints[NUM_JOINTS];

struct JointRequest
{
	Joints angles;
	int hardness[NUM_JOINTS];
};

// Returns the joint name
#ifndef SWIG
static const char* getJointName(Joint joint) FUNCTION_IS_NOT_USED;
#endif
static const char* getJointName(Joint joint) {
	switch(joint)
	{
	case HeadYaw: return "HeadYaw";
	case HeadPitch: return "HeadPitch";
	case LShoulderPitch: return "LShoulderPitch";
	case LShoulderRoll: return "LShoulderRoll";
	case LElbowYaw: return "LElbowYaw";
	case LElbowRoll: return "LElbowRoll";
	case RShoulderPitch: return "RShoulderPitch";
	case RShoulderRoll: return "RShoulderRoll";
	case RElbowYaw: return "RElbowYaw";
	case RElbowRoll: return "RElbowRoll";
	case LHipYawPitch: return "LHipYawPitch";
	case LHipRoll: return "LHipRoll";
	case LHipPitch: return "LHipPitch";
	case LKneePitch: return "LKneePitch";
	case LAnklePitch: return "LAnklePitch";
	case LAnkleRoll: return "LAnkleRoll";
	case RHipYawPitch: return "RHipYawPitch";
	case RHipRoll: return "RHipRoll";
	case RHipPitch: return "RHipPitch";
	case RKneePitch: return "RKneePitch";
	case RAnklePitch: return "RAnklePitch";
	case RAnkleRoll: return "RAnkleRoll";
	default:
		return "unknown";
	}
}

// Returns the string location in fast access for the sensor
#ifndef SWIG
const char* getSensorString(Sensor s) FUNCTION_IS_NOT_USED;
#endif
const char* getSensorString(Sensor s);

// joint limits
const float minJointLimits[NUM_JOINTS] = {
  DEG_T_RAD * (-110.0 + SIGMA), //(-120.0 + SIGMA), // HeadYaw // Todd:  doc says 120, get an error if we go over 60
  DEG_T_RAD * (-45.0 + SIGMA), // HeadPitch

	DEG_T_RAD * (-90.0 + SIGMA), // LHipYawPitch
	DEG_T_RAD * (-25.0 + SIGMA), // LHipRoll
	DEG_T_RAD * (-100.0 + SIGMA), // LHipPitch
	DEG_T_RAD * (0.0 + SIGMA), // LKneePitch
	DEG_T_RAD * (-75.0 + SIGMA), // LAnklePitch
	DEG_T_RAD * (-45.0 + SIGMA), // LAnkleRoll

	DEG_T_RAD * (-90.0 + SIGMA), // RHipYawPitch
	DEG_T_RAD * (-45.0 + SIGMA), // RHipRoll
	DEG_T_RAD * (-100.0 + SIGMA), // RHipPitch
	DEG_T_RAD * (0.0 + SIGMA), // RKneePitch
	DEG_T_RAD * (-75.0 + SIGMA), // RAnklePitch
	DEG_T_RAD * (-25.0 + SIGMA), // RAnkleRoll

	DEG_T_RAD * (-115.0 + SIGMA), //(-120.0 + SIGMA), // LShoulderPitch
	DEG_T_RAD * (0.0 + SIGMA), // LShoulderRoll
	DEG_T_RAD * (-120.0 + SIGMA), // LElbowYaw
	DEG_T_RAD * (-90.0 + SIGMA), // LElbowRoll


	DEG_T_RAD * (-115.0 + SIGMA), //(-120.0 + SIGMA), // seems to be 115 // RShoulderPitch
	DEG_T_RAD * (-95.0 + SIGMA), // RShoulderRoll
	DEG_T_RAD * (-120.0 + SIGMA), // RElbowYaw
	DEG_T_RAD * (0.0 + SIGMA) // RElbowRoll
};


const float maxJointLimits[NUM_JOINTS] = {
	DEG_T_RAD * (110.0 - SIGMA), //(120.0 - SIGMA) // HeadYaw,
	DEG_T_RAD * (45.0 - SIGMA), // HeadPitch

	DEG_T_RAD * (0.0 - SIGMA), // LHipYawPitch
	DEG_T_RAD * (45.0 - SIGMA), // LHipRoll
	DEG_T_RAD * (25.0 - SIGMA), // LHipPitch
	DEG_T_RAD * (130.0 - SIGMA), // LKneePitch
	DEG_T_RAD * (45.0 - SIGMA), // LAnklePitch
	DEG_T_RAD * (25.0 - SIGMA), // LAnkleRoll

	DEG_T_RAD * (0.0 - SIGMA), // RHipYawPitch
	DEG_T_RAD * (25.0 - SIGMA), // RHipRoll
	DEG_T_RAD * (25.0 - SIGMA), // RHipPitch
	DEG_T_RAD * (130.0 - SIGMA), // RKneePitch
	DEG_T_RAD * (45.0 - SIGMA), // RAnklePitch
	DEG_T_RAD * (45.0 - SIGMA), // RAnkleRoll

	DEG_T_RAD * (115.0 - SIGMA), //(120.0 - SIGMA) // LShoulderPitch
	DEG_T_RAD * (95.0 - SIGMA), // LShoulderRoll
	DEG_T_RAD * (120.0 - SIGMA), // LElbowYaw
	DEG_T_RAD * (0.0 - SIGMA), // LElbowRoll

	DEG_T_RAD * (115.0 - SIGMA), //(120.0 - SIGMA), // RShoulderPitch
	DEG_T_RAD * (0.0 - SIGMA), // RShoulderRoll
	DEG_T_RAD * (120.0 - SIGMA), // RElbowYaw
	DEG_T_RAD * (90.0 - SIGMA) // RElbowRoll
};

// All the body parts that have weight
class BodyPart {
  public:
	enum Part {
		neck,
		head,
    top_camera,
    bottom_camera,
    left_shoulder,
    left_bicep,
    left_elbow,
    left_forearm,
    left_hand,
    right_shoulder,
    right_bicep,
    right_elbow,
    right_forearm,
    right_hand,
    left_pelvis,
    left_hip,
    left_thigh,
    left_tibia,
    left_ankle,
    left_foot, // rotated at ankle (used for weight)
    left_bottom_foot, // translated (used for pose)
    right_pelvis,
    right_hip,
    right_thigh,
    right_tibia,
    right_ankle,
    right_foot,
    right_bottom_foot,
		torso,
    virtual_base,
		NUM_PARTS
	};
};

// Location of key body parts
class BodyFrame {
  public:
  enum Part {
    origin,
    head,
    left_shoulder,
    left_elbow,
    left_wrist,
    right_shoulder,
    right_elbow,
    right_wrist,
    torso,
    left_hip,
    left_knee,
    left_ankle,
    left_foot,
    right_hip,
    right_knee,
    right_ankle,
    right_foot,
    left_foot_front_left,
    left_foot_front_right,
    left_foot_rear_left,
    left_foot_rear_right,
    right_foot_front_left,
    right_foot_front_right,
    right_foot_rear_left,
    right_foot_rear_right,
    NUM_POINTS,
  };
};

enum LED {
  EarLeft0,
  EarLeft36,
  EarLeft72,
  EarLeft108,
  EarLeft144,
  EarLeft180,
  EarLeft216,
  EarLeft252,
  EarLeft288,
  EarLeft324,

  EarRight0,
  EarRight36,
  EarRight72,
  EarRight108,
  EarRight144,
  EarRight180,
  EarRight216,
  EarRight252,
  EarRight288,
  EarRight324,

  FaceRedLeft0,
  FaceRedLeft45,
  FaceRedLeft90,
  FaceRedLeft135,
  FaceRedLeft180,
  FaceRedLeft225,
  FaceRedLeft270,
  FaceRedLeft315,

  FaceGreenLeft0,
  FaceGreenLeft45,
  FaceGreenLeft90,
  FaceGreenLeft135,
  FaceGreenLeft180,
  FaceGreenLeft225,
  FaceGreenLeft270,
  FaceGreenLeft315,

  FaceBlueLeft0,
  FaceBlueLeft45,
  FaceBlueLeft90,
  FaceBlueLeft135,
  FaceBlueLeft180,
  FaceBlueLeft225,
  FaceBlueLeft270,
  FaceBlueLeft315,

  FaceRedRight0,
  FaceRedRight45,
  FaceRedRight90,
  FaceRedRight135,
  FaceRedRight180,
  FaceRedRight225,
  FaceRedRight270,
  FaceRedRight315,

  FaceGreenRight0,
  FaceGreenRight45,
  FaceGreenRight90,
  FaceGreenRight135,
  FaceGreenRight180,
  FaceGreenRight225,
  FaceGreenRight270,
  FaceGreenRight315,

  FaceBlueRight0,
  FaceBlueRight45,
  FaceBlueRight90,
  FaceBlueRight135,
  FaceBlueRight180,
  FaceBlueRight225,
  FaceBlueRight270,
  FaceBlueRight315,

  ChestRed,
  ChestGreen,
  ChestBlue,

  LFootRed,
  LFootGreen,
  LFootBlue,

  RFootRed,
  RFootGreen,
  RFootBlue,

  LHead0,
  LHead1,
  LHead2,
  LHead3,
  LHead4,
  LHead5,

  RHead5,
  RHead4,
  RHead3,
  RHead2,
  RHead1,
  RHead0,

  NUM_LEDS
};

const int LEDS_PER_EYE = FaceRedLeft315 - FaceRedLeft0 + 1;

#ifndef SWIG
static const char* getLEDString(LED l) FUNCTION_IS_NOT_USED;
#endif
static const char* getLEDString(LED l) {
	switch(l)	{
	case EarLeft0: return "Ears/Led/Left/0Deg/Actuator/Value";
  case EarLeft36: return "Ears/Led/Left/36Deg/Actuator/Value";
  case EarLeft72: return "Ears/Led/Left/72Deg/Actuator/Value";
  case EarLeft108: return "Ears/Led/Left/108Deg/Actuator/Value";
  case EarLeft144: return "Ears/Led/Left/144Deg/Actuator/Value";
  case EarLeft180: return "Ears/Led/Left/180Deg/Actuator/Value";
  case EarLeft216: return "Ears/Led/Left/216Deg/Actuator/Value";
  case EarLeft252: return "Ears/Led/Left/252Deg/Actuator/Value";
  case EarLeft288: return "Ears/Led/Left/288Deg/Actuator/Value";
  case EarLeft324: return "Ears/Led/Left/324Deg/Actuator/Value";

	case EarRight0: return "Ears/Led/Right/0Deg/Actuator/Value";
  case EarRight36: return "Ears/Led/Right/36Deg/Actuator/Value";
  case EarRight72: return "Ears/Led/Right/72Deg/Actuator/Value";
  case EarRight108: return "Ears/Led/Right/108Deg/Actuator/Value";
  case EarRight144: return "Ears/Led/Right/144Deg/Actuator/Value";
  case EarRight180: return "Ears/Led/Right/180Deg/Actuator/Value";
  case EarRight216: return "Ears/Led/Right/216Deg/Actuator/Value";
  case EarRight252: return "Ears/Led/Right/252Deg/Actuator/Value";
  case EarRight288: return "Ears/Led/Right/288Deg/Actuator/Value";
  case EarRight324: return "Ears/Led/Right/324Deg/Actuator/Value";

  case FaceRedLeft0: return "Face/Led/Red/Left/0Deg/Actuator/Value";
  case FaceRedLeft45: return "Face/Led/Red/Left/45Deg/Actuator/Value";
  case FaceRedLeft90: return "Face/Led/Red/Left/90Deg/Actuator/Value";
  case FaceRedLeft135: return "Face/Led/Red/Left/135Deg/Actuator/Value";
  case FaceRedLeft180: return "Face/Led/Red/Left/180Deg/Actuator/Value";
  case FaceRedLeft225: return "Face/Led/Red/Left/225Deg/Actuator/Value";
  case FaceRedLeft270: return "Face/Led/Red/Left/270Deg/Actuator/Value";
  case FaceRedLeft315: return "Face/Led/Red/Left/315Deg/Actuator/Value";

  case FaceGreenLeft0: return "Face/Led/Green/Left/0Deg/Actuator/Value";
  case FaceGreenLeft45: return "Face/Led/Green/Left/45Deg/Actuator/Value";
  case FaceGreenLeft90: return "Face/Led/Green/Left/90Deg/Actuator/Value";
  case FaceGreenLeft135: return "Face/Led/Green/Left/135Deg/Actuator/Value";
  case FaceGreenLeft180: return "Face/Led/Green/Left/180Deg/Actuator/Value";
  case FaceGreenLeft225: return "Face/Led/Green/Left/225Deg/Actuator/Value";
  case FaceGreenLeft270: return "Face/Led/Green/Left/270Deg/Actuator/Value";
  case FaceGreenLeft315: return "Face/Led/Green/Left/315Deg/Actuator/Value";

  case FaceBlueLeft0: return "Face/Led/Blue/Left/0Deg/Actuator/Value";
  case FaceBlueLeft45: return "Face/Led/Blue/Left/45Deg/Actuator/Value";
  case FaceBlueLeft90: return "Face/Led/Blue/Left/90Deg/Actuator/Value";
  case FaceBlueLeft135: return "Face/Led/Blue/Left/135Deg/Actuator/Value";
  case FaceBlueLeft180: return "Face/Led/Blue/Left/180Deg/Actuator/Value";
  case FaceBlueLeft225: return "Face/Led/Blue/Left/225Deg/Actuator/Value";
  case FaceBlueLeft270: return "Face/Led/Blue/Left/270Deg/Actuator/Value";
  case FaceBlueLeft315: return "Face/Led/Blue/Left/315Deg/Actuator/Value";

  case FaceRedRight0: return "Face/Led/Red/Right/0Deg/Actuator/Value";
  case FaceRedRight45: return "Face/Led/Red/Right/45Deg/Actuator/Value";
  case FaceRedRight90: return "Face/Led/Red/Right/90Deg/Actuator/Value";
  case FaceRedRight135: return "Face/Led/Red/Right/135Deg/Actuator/Value";
  case FaceRedRight180: return "Face/Led/Red/Right/180Deg/Actuator/Value";
  case FaceRedRight225: return "Face/Led/Red/Right/225Deg/Actuator/Value";
  case FaceRedRight270: return "Face/Led/Red/Right/270Deg/Actuator/Value";
  case FaceRedRight315: return "Face/Led/Red/Right/315Deg/Actuator/Value";

  case FaceGreenRight0: return "Face/Led/Green/Right/0Deg/Actuator/Value";
  case FaceGreenRight45: return "Face/Led/Green/Right/45Deg/Actuator/Value";
  case FaceGreenRight90: return "Face/Led/Green/Right/90Deg/Actuator/Value";
  case FaceGreenRight135: return "Face/Led/Green/Right/135Deg/Actuator/Value";
  case FaceGreenRight180: return "Face/Led/Green/Right/180Deg/Actuator/Value";
  case FaceGreenRight225: return "Face/Led/Green/Right/225Deg/Actuator/Value";
  case FaceGreenRight270: return "Face/Led/Green/Right/270Deg/Actuator/Value";
  case FaceGreenRight315: return "Face/Led/Green/Right/315Deg/Actuator/Value";

  case FaceBlueRight0: return "Face/Led/Blue/Right/0Deg/Actuator/Value";
  case FaceBlueRight45: return "Face/Led/Blue/Right/45Deg/Actuator/Value";
  case FaceBlueRight90: return "Face/Led/Blue/Right/90Deg/Actuator/Value";
  case FaceBlueRight135: return "Face/Led/Blue/Right/135Deg/Actuator/Value";
  case FaceBlueRight180: return "Face/Led/Blue/Right/180Deg/Actuator/Value";
  case FaceBlueRight225: return "Face/Led/Blue/Right/225Deg/Actuator/Value";
  case FaceBlueRight270: return "Face/Led/Blue/Right/270Deg/Actuator/Value";
  case FaceBlueRight315: return "Face/Led/Blue/Right/315Deg/Actuator/Value";

  case ChestRed: return "ChestBoard/Led/Red/Actuator/Value";
  case ChestGreen: return "ChestBoard/Led/Green/Actuator/Value";
  case ChestBlue: return "ChestBoard/Led/Blue/Actuator/Value";

  case LFootRed: return "LFoot/Led/Red/Actuator/Value";
  case LFootGreen: return "LFoot/Led/Green/Actuator/Value";
  case LFootBlue: return "LFoot/Led/Blue/Actuator/Value";

  case RFootRed: return "RFoot/Led/Red/Actuator/Value";
  case RFootGreen: return "RFoot/Led/Green/Actuator/Value";
  case RFootBlue: return "RFoot/Led/Blue/Actuator/Value";
  
  case LHead0: return "Head/Led/Front/Left/1/Actuator/Value";
  case LHead1: return "Head/Led/Front/Left/0/Actuator/Value";
  case LHead2: return "Head/Led/Middle/Left/0/Actuator/Value";
  case LHead3: return "Head/Led/Rear/Left/0/Actuator/Value";
  case LHead4: return "Head/Led/Rear/Left/1/Actuator/Value";
  case LHead5: return "Head/Led/Rear/Left/2/Actuator/Value";

  case RHead5: return "Head/Led/Rear/Right/2/Actuator/Value";
  case RHead4: return "Head/Led/Rear/Right/1/Actuator/Value";
  case RHead3: return "Head/Led/Rear/Right/0/Actuator/Value";
  case RHead2: return "Head/Led/Middle/Right/0/Actuator/Value";
  case RHead1: return "Head/Led/Front/Right/0/Actuator/Value";
  case RHead0: return "Head/Led/Front/Right/1/Actuator/Value";
	default:
		return "unknown";
	}
}

double getSystemTime();

#endif /* end of include guard: ROBOTINFO_R3BSIHAC */
