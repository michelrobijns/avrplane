/*
 * main.c
 *
 * Created: 7/22/2015
 * Author: Michel Robijns
 *
 * This file is part of avrplane which is released under the MIT license.
 * See the file LICENSE or go to http://opensource.org/licenses/MIT for full
 * license details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>

#include "joystick.h"
#include "serial.h"

// Update frequencies
#define FREQ_JOYSTICK 1000 
#define FREQ_SERIAL_READ 1000
#define FREQ_SERIAL_WRITE 50
#define FREQ_TERMINAL_WRITE 60

// Constants for computing the battery voltage from the received ADC value
#define VCC 4.993
#define DIVISOR 2.938

void *joystickUpdater(void *argument);
void *serialReader(void *argument);
void *serialWriter(void *argument);
void *terminalWriter(void *argument);
uint8_t mapToRange(int number);

uint8_t roll = 50;
uint8_t pitch = 50;
uint8_t yaw = 50;
uint8_t throttle = 0;
uint8_t button1 = 0;
uint8_t button2 = 0;
uint8_t button3 = 0;
uint8_t button4 = 0;
uint8_t button5 = 0;
uint8_t button6 = 0;

double voltage = 0;

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
    pthread_t tids[4];
    
    pthread_create(&tids[0], &attr, joystickUpdater, NULL);
    pthread_create(&tids[1], &attr, serialReader, NULL);
    pthread_create(&tids[2], &attr, serialWriter, NULL);
    pthread_create(&tids[3], &attr, terminalWriter, NULL);
    
    for (int i = 0; i < 4; i++)
    {
        pthread_join(tids[i], NULL);
    }
    
    // Never reached because the threads run forever
    terminateJoystick(&joystick);
    closeSerial(&serialPort);
    
    return 0;
}

void *joystickUpdater(__attribute__ ((unused)) void *argument)
{
    while (1)
    {
        updateJoystick(&joystick);
        
        roll = mapToRange(-1 * joystick.axis[0]);
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
        
        uint16_t temp = (serialPort.bufferRX[3] << 8) | serialPort.bufferRX[4];
        voltage = temp / 1023.0 * VCC * DIVISOR;
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
        
        sendBytes(&serialPort);
        
        usleep(1000000 / FREQ_SERIAL_WRITE);
    }
    
    pthread_exit(0);
}

void* terminalWriter(__attribute__ ((unused)) void *argument)
{
    while (1)
    {
        printf("Throttle: %3d, ", throttle);
        printf("Roll: %3d, ", roll);
        printf("Pitch: %3d, ", pitch);
        printf("Yaw: %3d, ", yaw);
        printf("Engine armed: %1d, ", button1);
        printf("Battery voltage: %2.2f V (charge below 11 V).", voltage);
        printf("\r");
        
        fflush(stdout);
        
        usleep(1000000 / FREQ_TERMINAL_WRITE);
    }
    
    pthread_exit(0);
}

// Maps the reported joystick position to a number from 0 to 100
uint8_t mapToRange(int number)
{
    return (uint8_t) ((number + 32767) / 655.34);
}

