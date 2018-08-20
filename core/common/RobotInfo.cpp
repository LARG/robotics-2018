#include "RobotInfo.h"

double getSystemTime() {
  struct timezone tz;
  timeval timeT;
  gettimeofday(&timeT, &tz);
  return timeT.tv_sec + (timeT.tv_usec / 1000000.0);
}

const char* getSensorString(Sensor s) {
	switch(s)
	{
	case gyroX: return "InertialSensor/GyrX";
	case gyroY: return "InertialSensor/GyrY";
  case gyroZ: return "InertialSensor/GyrZ";
	case accelX: return "InertialSensor/AccX";
	case accelY: return "InertialSensor/AccY";
	case accelZ: return "InertialSensor/AccZ";
  case angleX: return "InertialSensor/AngleX";
	case angleY: return "InertialSensor/AngleY";
	case angleZ: return "InertialSensor/AngleZ";
	case fsrLFL: return "LFoot/FSR/FrontLeft";
	case fsrLFR: return "LFoot/FSR/FrontRight";
	case fsrLRL: return "LFoot/FSR/RearLeft";
	case fsrLRR: return "LFoot/FSR/RearRight";
	case fsrRFL: return "RFoot/FSR/FrontLeft";
	case fsrRFR: return "RFoot/FSR/FrontRight";
	case fsrRRL: return "RFoot/FSR/RearLeft";
	case fsrRRR: return "RFoot/FSR/RearRight";
	//case ultrasoundL: return "US/Left";
	//case ultrasoundR: return "US/Right";
	case battery: return "Battery/Charge";
  case centerButton: return "ChestBoard/Button";
  case bumperLL: return "LFoot/Bumper/Left";
  case bumperLR: return "LFoot/Bumper/Right";
  case bumperRL: return "RFoot/Bumper/Left";
  case bumperRR: return "RFoot/Bumper/Right";
  case headFront: return "Head/Touch/Front";
  case headMiddle: return "Head/Touch/Middle";
  case headRear: return "Head/Touch/Rear";
	default:
		return "unknown";
	}
}
