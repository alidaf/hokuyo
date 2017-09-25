#include <unistd.h>	    // UNIX standard function definitions.
#include <stdio.h>	    // Standard Input/Output definitions.
#include <stdlib.h>
#include <string.h>	    // String function definitions.
#include <stdint.h>	    // Standard type definitions.
#include <fcntl.h>	    // File control definitions.
#include <stdbool.h>	// Boolean definitions.
#include <termios.h>	// POSIX terminal control definitions.

#include "urg.h"

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
//  Main routine.
//  ===========================================================================
int main(void)
{
    int err;
    sensor_t sensor;

    const char *device = "/dev/ttyACM0";
    long baud = 115200;

    err = serial_open(&sensor.serial, device, baud);
    printf("Error = %d\n", err);

}
