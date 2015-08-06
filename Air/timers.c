/*
 * timers.c
 *
 * Created: 8/4/2015
 * Author: Michel Robijns
 * 
 * This file is part of avrplane which is released under the MIT license.
 * See the file LICENSE or go to http://opensource.org/licenses/MIT for full
 * license details.
 */

#include <avr/io.h>
#include <string.h>
#include <avr/interrupt.h>

#include "timers.h"

uint8_t channel = 0;

// Declare the signal pin of each servo
uint8_t pwmPins[10] = {0b00100000, 0b00010000, 0b00001000, 0b00000100, 
                       0b00000010, 0b00000001, 0, 0, 0, 0};

// Declare the initial pulse width in microseconds of each servo
uint16_t pwm[10] = {1450, 1450, 1500, 1500, 1340, 
                    1000, 1500, 1500, 1500, 1500};

// Declare the bank of each signal pin
volatile uint8_t *pwmBanks[10] = {&PORTB, &PORTB, &PORTB, &PORTB, &PORTB, 
                                  &PORTB, &PORTB, &PORTB, &PORTB, &PORTB};

void setupTimers(void)
{
    // Set pin 8 to 13 to output
    DDRB |= 0b00111111;
    
    // 8-bit Timer/Counter0
    
    // Enable Clear Timer on Compare Match (CTC) mode
    TCCR0A = (1 << WGM01);
    
    // Set the prescaler to 256
    TCCR0B = (1 << CS02);
    
    // Set the Output Compare Register to a counter value (TCNT0) of 125
    OCR0A = 125;
    
    // Enable Output Compare A Match interrupt
    TIMSK0 = (1 << OCIE0A);
    
    // 16-bit Timer/Counter1
    
    // Enable Clear Timer on Compare Match (CTC) mode
    TCCR1B = (1 << WGM12);
    
    // Set the prescaler to 1
    TCCR1B |= (1 << CS10);
    
    // Set the Output Compare Register to a counter value (TCNT1) of 24000
    OCR1A = 24000;
    
    // Enable Output Compare A Match interrupt
    TIMSK1 = (1 << OCIE1A);
}

// Counter0 Output Compare A Interrupt
// This Interrupt Service Routine (ISR) is triggered exactly 500 times per second
ISR(TIMER0_COMPA_vect)
{
    // Call doAt500Hz() defined in main.c
    doAt500Hz();
    
    // Set the counter value to 0
    TCNT1 = 0;
    
    // Set the Output Compare Register to the desired counter value (TCNT1)
    OCR1A = 16 * pwm[channel];
    
    // Set the state of the current channel to high
    *pwmBanks[channel] = pwmPins[channel];
    
    // Switch to the next channel
    channel > 8 ? channel = 0 : channel++;
}

// Counter1 Output Compare A Interrupt
ISR(TIMER1_COMPA_vect)
{
    PORTB = 0b00000000;
}
