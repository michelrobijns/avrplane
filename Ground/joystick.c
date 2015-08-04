/*
 * joystick.c
 *
 * Created: 07/22/2015
 * Author: Michel Robijns
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>

#include "joystick.h"

#define JS "/dev/input/js0"

struct joystick initializeJoystick(void)
{
	struct joystick joystick = {1, 0, 0, "", NULL, NULL};
    
    if ((joystick.fd = open(JS, O_RDONLY)) == -1)
    {
        printf("Could not open joystick\n\r");
    }
    
    ioctl(joystick.fd, JSIOCGAXES, &joystick.numberOfAxis);
    ioctl(joystick.fd, JSIOCGBUTTONS, &joystick.numberOfButtons);
    ioctl(joystick.fd, JSIOCGNAME(80), &joystick.name);
    
    joystick.axis = (int *) calloc(joystick.numberOfAxis, sizeof(int));
    joystick.button = (char *) calloc(joystick.numberOfButtons, sizeof(char));
    
    printf("Joystick detected: %s, %d axis, %d buttons\n\r", joystick.name, joystick.numberOfAxis, joystick.numberOfButtons);
    
    fcntl(joystick.fd, F_SETFL, O_NONBLOCK);
    
	return joystick;
}

void terminateJoystick(struct joystick *joystick)
{
	close(joystick->fd);
}

void updateJoystick(struct joystick *joystick)
{
	struct js_event js;

	// Read the joystick sate
	read(joystick->fd, &js, sizeof(struct js_event));
	
	// See what to do with the event
	switch (js.type & ~JS_EVENT_INIT)
	{
		case JS_EVENT_AXIS:
			joystick->axis[js.number] = js.value;
			break;
		case JS_EVENT_BUTTON:
			joystick->button[js.number] = js.value;
			break;
	}
}

