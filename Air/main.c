/*
 * main.c
 *
 * Created: 7/23/2015
 * Author: Michel Robijns
 * 
 * This file is part of avrplane which is released under the MIT license.
 * See the file LICENSE or go to http://opensource.org/licenses/MIT for full
 * license details.
 */

#define F_CPU 16000000L

#include <avr/io.h>
#include <string.h>
#include <avr/interrupt.h>

#include "timers.h"
#include "serial.h"
#include "adc.h"

// Change these values to adjust the neutral position of the control surfaces
#define L_AILR_ADJ -5
#define R_AILR_ADJ -3
#define ELEVTR_ADJ -6
#define RUDDER_ADJ -2
#define NS_WHL_ADJ -27

// Declare external global variables
extern uint16_t pwm[10];
extern char rxBuffer[15];
extern char txBuffer[15];
extern uint16_t voltage;

// Declare external functions
extern void doAt500Hz(void);

uint8_t allowThrottle = 0;
uint8_t flapsUp = 0;
uint8_t flapsDown = 0;
uint16_t counter1 = 0;
uint16_t counter2 = 0;
uint8_t aileronOffset = 0;
int16_t elevatorOffset = 0;

void updateAxes(void);
void updateButtons(void);

int main(void)
{
    // Call setup functions
    setupSerial();
    setupTimers();
    setupADC();
    
    // Set external interrupts
    sei();
    
    while (1)
    {
        // Infinite loop
    }
    
    return 0;
}

// This function is called 500 times per second
void doAt500Hz(void)
{
    updateAxes();
    updateButtons();

    counter1++;
    counter2++;

    if (counter1 == 10) // Triggered exactly 50 times per second
    {
        updateADCs();
        counter1 = 0;
    }

    if (counter2 == 500) // Triggered exactly 1 time per second
    {
        txBuffer[3] = voltage >> 8;
        txBuffer[4] = voltage & 0xFF;

        // Sends whatever is in the TX buffer
        sendTxBuffer();

        counter2 = 0;
    }
}

// Process input from the joystick axes
void updateAxes(void)
{
    // Left aileron
    pwm[0] = 1000 + 10 * ((int) rxBuffer[0] + L_AILR_ADJ + aileronOffset);
    
    // Right aileron
    pwm[1] = 1000 + 10 * ((int) rxBuffer[0] + R_AILR_ADJ - aileronOffset);
    
    // Elevator
    pwm[2] = 1000 + 10 * ((int) rxBuffer[1] + ELEVTR_ADJ - elevatorOffset / 10);
    
    // Rudder
    pwm[3] = 1000 + 10 * ((int) rxBuffer[2] + RUDDER_ADJ);
    
    // Nose wheel
    pwm[4] = 1300 + 4 * (100 - (int) rxBuffer[2] + NS_WHL_ADJ);
    
    // Throttle
    if (allowThrottle) // Dead man's switch
    {
        pwm[5] = 1050 + 9 * (int) rxBuffer[3];
    }
}

// Process input from the joystick buttons
void updateButtons(void)
{
    // Set the joystick hat switch as a dead man's switch for the throttle
    if (rxBuffer[4] == 1 && rxBuffer[3] == 0 && allowThrottle == 0)
    {
        allowThrottle = 1;
    }
    else if (rxBuffer[4] == 1 && allowThrottle == 1)
    {
        allowThrottle = 1;
    }
    else
    {
        allowThrottle = 0;
        pwm[5] = 1000;
    }
    
    // Set the flaperons with joystick buttons 5 (down) and 6 (up)
    if (rxBuffer[8] == 1 && flapsDown == 0)
    {
        aileronOffset += (aileronOffset < 39) ? 13 : 0;
    }
    if (rxBuffer[9] == 1 && flapsUp == 0)
    {
        aileronOffset -= (aileronOffset > 0) ? 13 : 0;
    }
    
    // The following two variables record the state of buttons 5 and 6. Doing
    // so, it is possible to detect a CHANGE in the state of these buttons,
    // rather than simply detecting whether the buttons are being pressed.
    flapsDown = rxBuffer[8];
    flapsUp = rxBuffer[9];
    
    // Set the elevator trim with joystick buttons 3 (up) and 4 (down)
    if (rxBuffer[6] == 1)
    {
        elevatorOffset += (elevatorOffset < 400) ? 1 : 0;
    }
    if (rxBuffer[7] == 1)
    {
        elevatorOffset -= (elevatorOffset > -400) ? 1 : 0;
    }
    
    // Reset the elevator trim on pressing joystick button 2
    if (rxBuffer[5] == 1)
    {
        elevatorOffset = 0;
    }
}
