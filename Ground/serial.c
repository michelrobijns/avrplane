/*
 * serial.c
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
#include <string.h>
#include <fcntl.h>
#include <termios.h>

#include "serial.h"

#define PORT "/dev/ttyAMA0"
#define BAUD B19200

struct serialPort openSerial(void)
{
	struct serialPort serialPort = {-1, "", ""};

	struct termios options; // Serial options
	
	// Open serial port
	serialPort.fd = open(PORT, O_RDWR | O_NOCTTY | O_NDELAY);
	
	if (serialPort.fd >= 0)
	{
		// Get current options
		fcntl(serialPort.fd, F_SETFL, 0);
		
		if (tcgetattr(serialPort.fd, &options) != 0)
		{
			//return -1;
		}
		
		memset(&options, 0, sizeof(options)); // Erase the options struct
		
		// Set baudrate
		cfsetispeed(&options, BAUD);
		cfsetospeed(&options, BAUD);
		
		// Set options
		options.c_cflag &= ~PARENB; // No parity bit
		options.c_cflag &= ~CSTOPB; // 1 stop bit
		options.c_cflag &= ~CSIZE; // 8 data bits
		options.c_cflag |= CS8;
		options.c_cflag |= (CLOCAL | CREAD); // Allow reading
		options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // No echo, no control signals, and no interrupts
		options.c_iflag &= IGNPAR; // Ignore parity errors
		options.c_oflag &= ~OPOST; // Enable raw input
		options.c_cc[VMIN] = 1; // Number of bytes to wait for
		options.c_cc[VTIME] = 0; // Timeout
		
		tcflush(serialPort.fd, TCIOFLUSH);
		
		if (tcsetattr(serialPort.fd, TCSAFLUSH, &options) != 0)
		{
			//return 0;
		}
	}

	return serialPort;
}

void closeSerial(struct serialPort *serialPort)
{
	close(serialPort->fd);
}

void sendBytes(struct serialPort *serialPort)
{
	int sent;

	sent = write(serialPort->fd, serialPort->bufferTX, 15);
}

void readBytes(struct serialPort *serialPort)
{
    int i = 0;
    int bytes;
    char c;

    do {
        bytes = read(serialPort->fd, &c, 1);

        if (bytes > 0)
        {
            if (c != '\0')
            {
                serialPort->bufferRX[i] = c;
            } 

            i++;
        }
    } while (c != '\0' && i < BUFFER_SIZE);
}

