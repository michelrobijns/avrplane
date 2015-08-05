/*
 * serial.c
 *
 * Created: 8/4/2015
 * Author: Michel Robijns
 */

#define F_CPU 16000000UL
#define BAUD 19200UL
#define BRC ((F_CPU / 16 / BAUD) - 1)

#include <avr/io.h>
#include <string.h>
#include <avr/interrupt.h>
#include "serial.h"

char rxBuffer[15] = {50, 50, 50, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'e'};
char txBuffer[15] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, '\0'};

uint8_t rxWritePos = 0;
uint8_t txReadPos = 0;
uint8_t txWritePos = 0;

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
    
    // Set the Character Size to 8 bits. There is one stop bit, which is
    // the default setting.
    UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);
}

// RX Complete Interrupt
ISR(USART_RX_vect)
{
    rxBuffer[rxWritePos] = UDR0; 
    rxWritePos++;
    
    if ((rxWritePos >= 15) || (rxBuffer[rxWritePos - 1] == 'e'))
    {
        rxWritePos = 0;
    }
}

// TX Complete Interrupt
ISR(USART_TX_vect)
{
    if (txReadPos != txWritePos)
    {
        UDR0 = txBuffer[txReadPos];
        txReadPos++;
        
        if (txReadPos > 15)
        {
            txReadPos = 0;
        }
    } else {
        txReadPos = 0;
    }
}

void sendTxBuffer(void)
{
    if (UCSR0A & (1 << UDRE0))
    {
        UDR0 = txBuffer[0];
        
        txWritePos = 15;
        txReadPos = 1;
    }
}
