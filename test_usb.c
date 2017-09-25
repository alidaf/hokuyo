//  ===========================================================================
//  App for testing return of laser scanner data via USB.
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

#include <unistd.h>	    // UNIX standard function definitions.
#include <stdio.h>	    // Standard Input/Output definitions.
#include <stdlib.h>
#include <string.h>	    // String function definitions.
#include <stdint.h>	    // Standard type definitions.
#include <fcntl.h>	    // File control definitions.
#include <stdbool.h>	// Boolean definitions.
#include <termios.h>	// POSIX terminal control definitions.

#include "urg.h"

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
int serial_set_baud(serial_t *serial, uint16_t baud)
{
    uint16_t baud_val;

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
    serial_flush(serial->fd);

    return (0);
}

//  ===========================================================================
//  Initialises serial port.
//  ===========================================================================
int serial_open(serial_t *serial, const char *device, uint16_t baud)
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
//  Closes port.
//  ===========================================================================
int serial_close(serial_t *serial)
{
    return close(serial->fd);
}

//  ===========================================================================
//  Opens instance of URG sensor.
//  ===========================================================================
int sensor_open(sensor_t *sensor, const char *device, uint16_t baud)
{
    sensor->active = false;
    return serial_open(&sensor->serial, device, baud);

}
//  ===========================================================================
//  Writes command to port.
//  ===========================================================================
int serial_write(serial_t *serial, const char *cmd, uint8_t len)
{

    if (DEBUG) PRINT_CMD(cmd);

    err = write(serial->fd, cmd, len);

    if (err < 0)
    {
        printf("Error writing command.\n");
        perror("Write to port");
        return (err);
    }

    usleep( 100000 );   // Definitely needs this!

    return err;
}

//  ===========================================================================
//  Waits until data is available on serial port.
//  ===========================================================================
int serial_wait(sensor_serial_t *serial, int timeout)
{
    fd_set rfds;
    struct timeval tv;

    FD_ZERO(&rfds);
    FD_SET(serial->fd, &rfds);

    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;

    if (select(serial->fd + 1, &rfds, NULL, NULL,
              (timeout < 0) ? NULL : &tv) <= 0) return (0);

    return 1;
}

//  ===========================================================================
//  Returns data block from sensor.
//  ===========================================================================
int serial_read(serial_t *serial, char *data)
{
    char c;
    int  i;

    i = 0;

    while (read(fd, &c, 1) > 0 && (c != STRING_LF))
    {
        data[i++] = c;
    }

//    data[i+1] = STRING_LF;

    return (i-1);
}

//  ===========================================================================
//  Main routine.
//  ===========================================================================
int main(void)
{
    sensor_t sensor;

    int     fd;
    int     err;
    char    id[16] = "Jaguar";
    char   *data;

    const char *device = "/dev/ttyACM0";
    long baud = 115200;

    err = sensor_open(&sensor.serial, device, baud);
    printf("Open sensor error = %d\n", err);

    err = serial_write(&sensor, "VV", 3);
    printf("Send command error = %d\n", err);
    err = serial_read(&sensor.serial, data);
    printf("Data = %s\n", data);

    err = serial_write(&sensor, "BM", 3);
    printf("Send command error = %d\n", err);
    err = serial_read(&sensor.serial, data);
    printf("Data = %s\n", data);
    err = serial_write(&sensor, "BM", 3);
    printf("Send command error = %d\n", err);
    err = serial_read(&sensor.serial, data);
    printf("Data = %s\n", data);

    err = serial_write(&sensor, "QT", 3);
    printf("Send command error = %d\n", err);
    err = serial_read(&sensor.serial, data);
    printf("Data = %s\n", data);
    err = serial_write(&sensor, "QT", 3);
    printf("Send command error = %d\n", err);
    err = serial_read(&sensor.serial, data);
    printf("Data = %s\n", data);

    err = serial_write(&sensor, "VV", 3);
    printf("Send command error = %d\n", err);
    err = serial_read(&sensor.serial, data);
    printf("Data = %s\n", data);

    close(fd);

    return (0);
}

