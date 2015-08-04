/*
 * serial.h
 *
 * Created: 07/22/2015
 * Author: Michel Robijns
 */

#ifndef SERIAL_H_INCLUDED
#define SERIAL_H_INCLUDED

#define BUFFER_SIZE 15

struct serialPort {
	int fd;
	char bufferRX[15];
	char bufferTX[15];
};

struct serialPort openSerial(void);
void closeSerial(struct serialPort *serialPort);
void sendBytes(struct serialPort *serialPort);
void readBytes(struct serialPort *serialPort);

#endif

