/*
 * joystick.h
 *
 * Created: 7/22/2015
 * Author: Michel Robijns
 *
 * This file is part of avrplane which is released under the MIT license.
 * See the file LICENSE or go to http://opensource.org/licenses/MIT for full
 * license details.
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

