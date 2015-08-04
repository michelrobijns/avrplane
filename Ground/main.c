/*
 * main.c
 *
 * Created: 07/22/2015
 * Author: Michel Robijns
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>

#include "joystick.h"
#include "serial.h"

#define FREQ_JOYSTICK 1000 
#define FREQ_SERIAL_READ 1000
#define FREQ_SERIAL_WRITE 50
#define VCC 4.993
#define DIVISOR 2.938

void *joystickUpdater(void *argument);
void *serialReader(void *argument);
void *serialWriter(void *argument);
uint8_t mapToRange(int number);

uint8_t roll;
uint8_t pitch;
uint8_t yaw;
uint8_t throttle;
uint8_t button1;
uint8_t button2;
uint8_t button3;
uint8_t button4;
uint8_t button5;
uint8_t button6;

struct joystick joystick;
struct serialPort serialPort;

int main(void)
{
    joystick = initializeJoystick();
	serialPort = openSerial();
    
    printf("Joystick's file descriptor: %d\n\r", joystick.fd);
	printf("Serial port's file descriptor: %d\n\r", serialPort.fd);
	
	// Launch threads
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_t tids[3];

	pthread_create(&tids[0], &attr, joystickUpdater, NULL);
	pthread_create(&tids[1], &attr, serialReader, NULL);
	pthread_create(&tids[2], &attr, serialWriter, NULL);

	for (int i = 0; i < 3; i++)
	{
		pthread_join(tids[i], NULL);
	}

	terminateJoystick(&joystick);
	closeSerial(&serialPort);

    return 0;
}

void *joystickUpdater(__attribute__ ((unused)) void *argument)
{
	while (1)
	{
		updateJoystick(&joystick);
		
		roll = mapToRange(joystick.axis[0]);
		pitch = mapToRange(-1 * joystick.axis[1]);
		yaw = mapToRange(joystick.axis[2]);
		throttle = mapToRange(-1 * joystick.axis[3]);
		button1 = joystick.button[0];
		button2 = joystick.button[1];
		button3 = joystick.button[2];
		button4 = joystick.button[3];
		button5 = joystick.button[4];
		button6 = joystick.button[5];

		usleep(1000000 / FREQ_JOYSTICK);
	}

	pthread_exit(0);
}

void* serialReader(__attribute__ ((unused)) void *argument)
{
	while (1)
	{
        readBytes(&serialPort);
		usleep(1000000 / FREQ_SERIAL_READ);
        
        uint16_t ans = (serialPort.bufferRX[3] << 8) | serialPort.bufferRX[4];

        printf("TEST: %d\n", ans);

        printf("Battery voltage: %.2f\n", ans / 1023.0 * VCC * DIVISOR);
	}

	pthread_exit(0);
}

void* serialWriter(__attribute__ ((unused)) void *argument)
{
	while (1)
	{
		serialPort.bufferTX[0] = roll;
		serialPort.bufferTX[1] = pitch;
		serialPort.bufferTX[2] = yaw;
		serialPort.bufferTX[3] = throttle;
		serialPort.bufferTX[4] = button1;
		serialPort.bufferTX[5] = button2;
		serialPort.bufferTX[6] = button3;
		serialPort.bufferTX[7] = button4;
		serialPort.bufferTX[8] = button5;
		serialPort.bufferTX[9] = button6;
		serialPort.bufferTX[10] = 1;
		serialPort.bufferTX[11] = 1;
		serialPort.bufferTX[12] = 1;
		serialPort.bufferTX[13] = 1;
		serialPort.bufferTX[14] = 'e';

		printf("Throttle: %d, ", throttle);
		printf("Roll: %d, ", roll);
		printf("Pitch: %d, ", pitch);
		printf("Yaw: %d, ", yaw);
		printf("Toggle: %d.", button1);
		printf("\r");
		fflush(stdout);

		sendBytes(&serialPort);

		usleep(1000000 / FREQ_SERIAL_WRITE);
	}
	
	pthread_exit(0);
}

uint8_t mapToRange(int number)
{
	return (uint8_t) ((number + 32767) / 655.34);
}

