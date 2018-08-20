/*
 * Erik <br0ke@math.smsu.edu>
 * http://archives.seul.org/linuxgames/Aug-1999/msg00107.html
 * this is the linux 2.2.x way of handling joysticks. It allows an arbitrary
 * number of axis and buttons. It's event driven, and has full signed int
 * ranges of the axis (-32768 to 32767). It also lets you pull the joysticks
 * name. The only place this works of that I know of is in the linux 1.x 
 * joystick driver, which is included in the linux 2.2.x kernels
 */

// Modified for XBOX CONTROLLER

#include "joystickxbox.h"

#define JOY_DEV "/dev/input/js0"

using namespace std;

int joy_fd, *axis=NULL, num_of_axis=0, num_of_buttons=0;
char *button=NULL, name_of_joystick[80];
struct js_event js;

ControllerInfo controllerData;

bool fInitXboxController = false;

int initXboxJoystick() {
	if((joy_fd = open(JOY_DEV , O_RDONLY)) == -1)
	{
		printf( "Couldn't open joystick\n" );
		fInitXboxController = false;
		return -1;
	} else {
	  fInitXboxController = true;
	}

	ioctl( joy_fd, JSIOCGAXES, &num_of_axis );
	ioctl( joy_fd, JSIOCGBUTTONS, &num_of_buttons );
	ioctl( joy_fd, JSIOCGNAME(80), &name_of_joystick );

	axis = (int *) calloc( num_of_axis, sizeof( int ) );
	button = (char *) calloc( num_of_buttons, sizeof( char ) );

	fcntl( joy_fd, F_SETFL, O_NONBLOCK );	/* use non-blocking mode */

	return 0;
}

bool updateXboxJoystick() {
	if(!fInitXboxController) return false;

	/* read the joystick state */
	ssize_t ret = read(joy_fd, &js, sizeof(struct js_event));
		
	/* see what to do with the event */
	switch (js.type & ~JS_EVENT_INIT)
	{
		case JS_EVENT_AXIS:
			axis   [ js.number ] = js.value;
			break;
		case JS_EVENT_BUTTON:
			button [ js.number ] = js.value;
			break;
	}

	float norm = (float) (2 << 14) - 1;
	controllerData.x = axis[0] / norm;
	controllerData.y = -axis[1] / norm;
	controllerData.x2 = axis[2] / norm;
	controllerData.y2 = -axis[3] / norm;
	controllerData.rt = (axis[4] / norm + 1) / 2;
	controllerData.lt = (axis[5] / norm + 1) / 2;
	controllerData.a = button[0];

	//printf("x: %f - y: %f - x2: %f - y2: %f - a:%i - lt:%f rt:%f   \r\n", controllerData.x, controllerData.y, controllerData.x2, controllerData.y2, controllerData.a, controllerData.lt, controllerData.rt);
	//fflush(stdout);
  return true;
}

ControllerInfo getControllerInfo() {
	return controllerData;
}

/*int main() {
	if(initXboxJoystick() < 0) return -1;
	
	while(true) {
		updateXboxJoystick();
	}
	close( joy_fd );
	return 0;	
}*/
