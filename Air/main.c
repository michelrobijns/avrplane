/*
 * main.c
 *
 * Created: 23/7/2015 19:33 PM
 * Author: Michel Robijns
 */

#define F_CPU 16000000L
#define BAUD 19200
#define BRC ((F_CPU / 16 / BAUD) - 1)

#define AILERON_ADJ -5
#define ELEVATOR_ADJ 0
#define RUDDER_ADJ 0
#define NOSE_WHEEL_ADJ -27

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

#include <avr/io.h>
#include <string.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define RX_BUFFER_SIZE 15
#define TX_BUFFER_SIZE 15

uint8_t channelPins[10] = {0b00100000, 0b00010000, 0b00001000, 0b00000100, 0b00000010, 0b00000001, 0, 0, 0, 0};
uint16_t channelPWM[10] = {1450, 1450, 1500, 1500, 1340, 1000, 1500, 1500, 1500, 1500};
volatile uint8_t *channelBanks[10] = {&PORTB, &PORTB, &PORTB, &PORTB, &PORTB, &PORTB, &PORTB, &PORTB, &PORTB, &PORTB};

uint8_t allowThrust = 0;
uint8_t flapsUp = 0;
uint8_t flapsDown = 0;

uint8_t aileronOffset = 0;
int16_t elevatorOffset = 0;

uint8_t channel = 0;

char rxBuffer[RX_BUFFER_SIZE] = {50, 50, 50, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'e'};
char txBuffer[TX_BUFFER_SIZE] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, '\0'};

uint8_t rxWritePos = 0;
uint8_t txReadPos = 0;
uint8_t txWritePos = 0;

uint16_t average = 0;
uint16_t samples = 0;
uint16_t voltage;

void setupSerial(void);
void setupTimers(void);
void setupADC(void);
void updateAxes(void);
void updateButtons(void);
void updateADCs(void);
//void appendSerial(char c);
//void serialWrite(char c[]);
void sendtxBuffer(void);
void startConversion(void);

int main(void)
{
    DDRB |= 0b00111111;
    DDRD |= 0b11100000;
    PORTD |= 0b00100000;

    setupSerial();
    setupTimers();
    setupADC();

    sei();

    _delay_ms(1000);

    while (1)
    {
        ;
    }
    
    return 0;
}

void setupSerial(void)
{
    // Set baud rate
    UBRR0H = (BRC >> 8);
    UBRR0L = BRC;
    
    // Enable the USART Receiver and Transmitter
    UCSR0B |= (1 << RXEN0);
    UCSR0B |= (1 << TXEN0);
    
    // Enable the RX Complete Interrupt and the TX Complete Interrupt
    UCSR0B |= (1 << RXCIE0);
    UCSR0B |= (1 << TXCIE0);
    
    // Set the Character Size to 8 bits. There is one stop bit, which the default setting.
    UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);
}

void setupTimers(void)
{
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

    // Set the position of the flaperons with buttons 5 (down) and 6 (up)
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

    // Set the elevator trim with buttons 3 (up) and 4 (down)
    if (rxBuffer [6] == 1)
    {
        elevatorOffset += (elevatorOffset < 400) ? 1 : 0;
    }
    if (rxBuffer [7] == 1)
    {
        elevatorOffset -= (elevatorOffset > -400) ? 1 : 0;
    }

    // Reset the elevator trim if button 2 is pressed
    if (rxBuffer [5] == 1)
    {
        elevatorOffset = 0;
    }
}

// Sample readings from the ADC, send the result to ground, and start a new conversion
void updateADCs(void)
{
    if (samples == 50)
    {
        voltage = average / samples;
        average = 0;
        samples = 0;

        txBuffer[3] = voltage >> 8;
        txBuffer[4] = voltage & 0xFF;

        sendtxBuffer();
    }

    startConversion();
}

// RX Complete Interrupt
ISR(USART_RX_vect)
{
    rxBuffer[rxWritePos] = UDR0; 
    rxWritePos++;

    if ((rxWritePos >= RX_BUFFER_SIZE) || (rxBuffer[rxWritePos - 1] == 'e'))
    {
        rxWritePos = 0;
    }
}

// Counter0 Output Compare A Interrupt
// This Interrupt Service Routine (ISR) is triggered exactly 500 times per second
ISR(TIMER0_COMPA_vect)
{
    updateAxes();
    updateButtons();

    if (channel == 0) // Triggered exactly 50 times per second
    {
        updateADCs();
    }

    // Set the counter value to 0
    TCNT1 = 0;

    // Set the Output Compare Register to the desired counter value (TCNT1)
    OCR1A = 16 * channelPWM[channel];

    // Set the state of the current channel to high
    *channelBanks[channel] = channelPins[channel];

    // Switch to the next channel
    channel > 8 ? channel = 0 : channel++;
    
}

// Counter1 Output Compare A Interrupt
ISR(TIMER1_COMPA_vect)
{
    PORTB = 0b00000000;
}

// ADC Conversion Complete Interrupt
ISR(ADC_vect)
{
    average += ADC;
    samples++;
}

// TX Complete Interrupt
ISR(USART_TX_vect)
{
     if (txReadPos != txWritePos)
     {
         UDR0 = txBuffer[txReadPos];
         txReadPos++;

         if (txReadPos > TX_BUFFER_SIZE)
         {
             txReadPos = 0;
         }
     } else {
         txReadPos = 0;
     }
}

/*void serialWrite(char c[])
{
    for (uint8_t i = 0; i < strlen(c); i++)
    {
        appendSerial(c[i]);
    }

    if (UCSR0A & (1 << UDRE0))
    {
        UDR0 = 0;
    }
}

void appendSerial(char c)
{
    txBuffer[txWritePos] = c;
    txWritePos++;

    if (txWritePos >= TX_BUFFER_SIZE)
    {
        txWritePos = 0;
    }
}*/

void sendtxBuffer()
{
    if (UCSR0A & (1 << UDRE0))
    {
        UDR0 = txBuffer[0];

        txWritePos = 15;
        txReadPos = 1;
    }
}

