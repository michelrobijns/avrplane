/*
 * joystick.h
 *
 * Created: 07/22/2015
 * Author: Michel Robijns
 */

#ifndef JOYSTICK_H_INCLUDED
#define JOYSTICK_H_INCLUDED

struct joystick {
	int fd;
	int numberOfAxis;
	int numberOfButtons;
	char name[80];
	int *axis;
	char *button;
};

struct joystick initializeJoystick(void);
void terminateJoystick(struct joystick *joystick);
void updateJoystick(struct joystick *joystick);

#endif

