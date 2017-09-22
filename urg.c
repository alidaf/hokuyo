//  ===========================================================================
//  Driver for Hokuyo URG-04LX-UG01 laser scanner.
//  ===========================================================================
/*
    Copyright 2017 Darren Faulke <darren@alidaf.co.uk>
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
//  ===========================================================================

#include "urg.h"
#include <unistd.h>	    // UNIX standard function definitions.
#include <stdio.h>	    // Standard Input/Output definitions.
#include <stdlib.h>
#include <string.h>	    // String function definitions.
#include <stdint.h>	    // Standard type definitions.
#include <fcntl.h>	    // File control definitions.
#include <stdbool.h>	// Boolean definitions.
#include <termios.h>	// POSIX terminal control definitions.

//  Commands ------------------------------------------------------------------

//  ===========================================================================
//  Clears serial port.
//  ===========================================================================
void serial_flush(serial_t *serial)
{
    tcdrain(serial->fd);
    tcflush(serial->fd, TCIOFLUSH);
}

//  ===========================================================================
//  Sets serial baud rate.
//  ===========================================================================
int serial_set_baud(serial_t *serial, long baud)
{
    long baud_val;

    switch (baud)
    {
    case 4800:
        baud_val = B4800;
        break;
    case 9600:
        baud_val = B9600;
        break;
    case 19200:
        baud_val = B19200;
        break;
    case 38400:
        baud_val = B38400;
        break;
    case 57600:
        baud_val = B57600;
        break;
    case 115200:
        baud_val = B115200;
        break;
    default:
        return -1;
    }

    cfsetospeed(&serial->settings, baud_val);
    cfsetispeed(&serial->settings, baud_val);

    tcsetattr(serial->fd, TCSADRAIN, &serial->settings);
    serial_flush(serial);

    return (0);
}

//  ===========================================================================
//  Initialises serial port.
//  ===========================================================================
int serial_open(serial_t *serial, const char *device, long baud)
{

    int flags = 0;
    int ret = 0;

    serial->fd = open(device, O_RDWR | O_NOCTTY | O_NONBLOCK);

    if (serial->fd < 0)
    {
        perror("USB open");
        return (-1);
    }

    flags = fcntl(serial->fd, F_GETFL, 0);
    fcntl(serial->fd, F_SETFL, flags & ~O_NONBLOCK);

    // Get current port options.
    tcgetattr(serial->fd, &serial->settings);

    // Set port options (lifted from the urg library source code).
    serial->settings.c_iflag = 0;
    serial->settings.c_oflag = 0;

    serial->settings.c_cflag &= ~(CSIZE | PARENB | CSTOPB);
    serial->settings.c_cflag |= CS8 | CREAD | CLOCAL;
    serial->settings.c_lflag &= ~(ICANON | ECHO | ISIG | IEXTEN);

    serial->settings.c_cc[VMIN] = 0;
    serial->settings.c_cc[VTIME] = 0;

    ret = serial_set_baud(serial, baud);

    return ret;
}

//  ===========================================================================
//  Closes serial port.
//  ===========================================================================
int serial_close(serial_t *serial)
{
    return close(serial->fd);
}

//  ===========================================================================
//  Writes command to port.
//  ===========================================================================
int write_command(serial_t *serial, const char *data, int size)
{
    return write(serial->fd, data, size);
}

//  ===========================================================================
//  Returns data sum.
//  ===========================================================================
char get_data_sum(char *data)
{
    uint8_t  i;
    uint16_t val;
    uint8_t  sum;
    uint8_t  len;

    val = 0;
    len = strlen(data);

    for (i = 0; i < len; i++)
    {
        val += data[i];
    }

    sum = (val & 0x3f) + 0x30;

    return (sum);
}

//  ===========================================================================
//  Returns data block from sensor.
//  ===========================================================================
int get_data(serial_t *serial, char *data)
{
    char c;
    int  i;

    i = 0;

    while (read(serial->fd, &c, 1) > 0 && (c != STRING_LF))
    {
        data[i++] = c;
    }

    return (i-1);

}

//  ===========================================================================
//  Turns laser on, returns status in data.
//  ===========================================================================
int set_laser_on(serial_t *serial, char *data)
{
    int  err;

//    char command[DATA_CMD_LEN + DATA_STRING_LEN];
//    char status[DATA_STATUS_LEN + DATA_SUM_LEN + DATA_EOL_LEN];
    char *cmd;
    char *status;

    serial_flush(serial);

    strcpy(cmd, CMD_SET_LASER_ON);
//    strcat(cmd, string);  // Don't know why this kills the return data!
    strcat(cmd, LF);

    if (DEBUG) PRINT_CMD(cmd);

    err = write(serial->fd, cmd, strlen(cmd));

    if (err < 0)
    {
        printf("Error writing command.\n");
        perror("Set laser on");
        return (err);
    }

    usleep( 100000 ); // Definitely needs this!

    err = get_data(serial, &cmd);
    if (err < 0)
    {
        printf("Error getting command.\n");
        return (err);
    }
    err = get_data(serial, &status);
    if (err < 0)
    {
        printf("Error getting status.\n");
        return (err);
    }

    printf("\tCommand: %s\n", cmd);
    printf("\tStatus: %s\n", status);
    printf("\n");

    return (err);
}

//  ===========================================================================
//  Main routine.
//  ===========================================================================
int main(void)
{
    int     err;

    sensor_t sensor;

    const char *device = "/dev/ttyACM0";
    long baud = 115200;

    char data[64];

    err = serial_open(&sensor.serial, device, baud);
    if (err < 0)
    {
        printf("Error initialising port.\n");
    }

    err = serial_close(&sensor.serial);
    if (err < 0)
    {
        printf("Error closing port.\n");
    }

    err = set_laser_on(&sensor.serial, &data);

    printf("Data = %s\n", data);

    return (0);
}

