/*
 * main.c
 *
 * Created: 23/7/2015 19:33 PM
 * Author: Michel Robijns
 */

#define F_CPU 16000000L

#include <avr/io.h>
#include <string.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "timers.h"
#include "serial.h"
#include "adc.h"

#define AILERON_ADJ -5
#define ELEVATOR_ADJ 0
#define RUDDER_ADJ 0
#define NOSE_WHEEL_ADJ -27

uint8_t allowThrust = 0;
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
    DDRD |= 0b11100000;
    PORTD |= 0b00100000;

    setupSerial();
    setupTimers();
    setupADC();

    sei();

    while (1)
    {
        ;
    }
    
    return 0;
}

// Process signals from the joystick axes
void updateAxes(void)
{
    channelPWM[0] = 1000 + 10 * ((int) rxBuffer[0] + AILERON_ADJ - aileronOffset); // Left aileron
    channelPWM[1] = 1000 + 10 * ((int) rxBuffer[0] + AILERON_ADJ + aileronOffset); // Right aileron
    channelPWM[2] = 1000 + 10 * ((int) rxBuffer[1] + ELEVATOR_ADJ - elevatorOffset / 10); // Elevator
    channelPWM[3] = 1000 + 10 * ((int) rxBuffer[2] + RUDDER_ADJ); // Rudder
    channelPWM[4] = 1300 + 4 * (100 - (int) rxBuffer[2] + NOSE_WHEEL_ADJ); // Nose wheel
    if (allowThrust) channelPWM[5] = 1050 + 9 * (int) rxBuffer[3]; // Throttle
}

// Process signals from the joystick buttons
void updateButtons(void)
{
    // Set the hat switch as a dead man's switch for the throttle
    if (rxBuffer[4] == 1 && rxBuffer[3] == 0 && allowThrust == 0)
    {
        allowThrust = 1;
    }
    else if (rxBuffer[4] == 1 && allowThrust == 1)
    {
        allowThrust = 1;
    }
    else
    {
        allowThrust = 0;
        channelPWM[5] = 1000;
    }

    // Set the position of the flaperons with joystick buttons 5 (down) and 6 (up)
    if (rxBuffer [8] == 1 && flapsDown == 0)
    {
        aileronOffset += (aileronOffset < 39) ? 13 : 0;
    }
    if (rxBuffer [9] == 1 && flapsUp == 0)
    {
        aileronOffset -= (aileronOffset > 0) ? 13 : 0;
    }

    flapsDown = rxBuffer[8];
    flapsUp = rxBuffer[9];

    // Set the elevator trim with joystick buttons 3 (up) and 4 (down)
    if (rxBuffer [6] == 1)
    {
        elevatorOffset += (elevatorOffset < 400) ? 1 : 0;
    }
    if (rxBuffer [7] == 1)
    {
        elevatorOffset -= (elevatorOffset > -400) ? 1 : 0;
    }

    // Reset the elevator trim if joystick button 2 is pressed
    if (rxBuffer [5] == 1)
    {
        elevatorOffset = 0;
    }
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

        sendtxBuffer();

        counter2 = 0;
    }
}
