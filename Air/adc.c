/*
 * adc.c
 *
 * Created: 8/4/2015
 * Author: Michel Robijns
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "adc.h"
#include "serial.h"

uint16_t accumulator = 0;
uint8_t samples = 0;
uint16_t voltage;

void setupADC(void)
{
    // Select the reference voltage to VCC //the internal 1.1 V reference
    ADMUX = (1 << REFS0);// | (1 << REFS1);
    
    // Set ADC5 to as input channel
    ADMUX |= (1 << MUX0) | (1 << MUX2);
    
    // Enable the ADC
    ADCSRA = (1 << ADEN);
    
    // Enable the ADC Conversion Complete Interrupt
    ADCSRA |= (1 << ADIE);
    
    // Set the division factor between the system clock frequency and the input clock to the ADC to 128
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    
    // Disable the digital input buffer
    DIDR0 = (1 << ADC5D);
    
    // ADC start conversion
    startConversion();
}

// Conversion must be started manually
void startConversion(void)
{
    // ADC start conversion
    ADCSRA |= (1 << ADSC);
}

// Sample readings from the ADC, send the result to ground, and start a new conversion
void updateADCs(void)
{
    if (samples == 50)
    {
        voltage = accumulator / samples;
        accumulator = 0;
        samples = 0;
    }

    startConversion();
}

// ADC Conversion Complete Interrupt
ISR(ADC_vect)
{
    accumulator += ADC;
    samples++;
}
