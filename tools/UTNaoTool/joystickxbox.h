#include <iostream>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>
#include <stdlib.h>

struct ControllerInfo {
	float x, y, x2, y2, lt, rt;
  int a;
};

int initXboxJoystick();
bool updateXboxJoystick();
ControllerInfo getControllerInfo();
