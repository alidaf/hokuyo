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

#define DEBUG 1 // Debugging output switch.

/* Debug output. */
#define PRINT_CMD(x) printf("Sending command: %s\n", x);

/* Error handling. */
#define PRINT_ERROR(x) printf("Error: %s\n", x); exit(1);

/* Length of command code and string. */
#define CMD_CODE_LEN    2
#define CMD_STRING_LEN 14

/* Command codes. */
#define CMD_SET_LASER_ON    "BM"    // Turn laser on.
#define CMD_SET_LASER_OFF   "QT"    // Turn laser off.
#define CMD_SET_LASER_RESET "RS"    // Reset sensor state.
#define CMD_SET_TIME_ADJUST "TM"    // Adjust sensor time to match host.
#define CMD_SET_BIT_RATE    "SS"    // Adjust bit rate for RS232C.
#define CMD_SET_MOTOR_SPEED "CR"    // Adjust sensor motor speed.
#define CMD_SET_SENSITIVITY "HS"    // Set sensitivity mode.
#define CMD_SET_MALFUNCTION "DB"    // Simulate a malfunction.
#define CMD_GET_VERSION     "VV"    // Send version details.
#define CMD_GET_SPEC        "PP"    // Send sensor specification.
#define CMD_GET_RUN_STATE   "II"    // Send sensor run state.
#define CMD_GET_DATA_CONT2  "MS"    // Continuous data acquisition (2-byte).
#define CMD_GET_DATA_CONT3  "MD"    // Continuous data acquisition (3-byte).
#define CMD_GET_DATA_SING2  "GS"    // Measurement data (2-byte).
#define CMD_GET_DATA_SING3  "GD"    // Measurement data (3-byte).

#define BIT_RATE_1 "019200" //  19.2 kbps.
#define BIT_RATE_2 "038400" //  38.4 kbps.
#define BIT_RATE_3 "057600" //  57.6 kbps
#define BIT_RATE_4 "115200" // 115.2 kbps.
#define BIT_RATE_5 "250000" //   250 kbps.
#define BIT_RATE_6 "500000" //   500 kbps.
#define BIT_RATE_7 "750000" //   750 kbps.

/* Number of lines returned for each command. */
#define RET_VERSION_LINES    7

/* Maximum data block size */
#define RET_DATA_BLOCK_MAX  64

// ASCII codes for commands and data.
#define LF "\n" // Line Feed.
#define CR "\r" // Carriage Return.

#define STRING_NULL '\0'
#define STRING_LF   '\n'

#define USB_PORT "/dev/ttyACM0" // Output port for USB.

//  Commands ------------------------------------------------------------------

//  ===========================================================================
//  Initialises serial port.
//  ===========================================================================
int init_port()
{
    struct termios tty;

    int fd;

    fd = open(USB_PORT, O_RDWR | O_NOCTTY | O_NONBLOCK);

    if (fd < 0)
    {
        perror(USB_PORT);
        exit(-1);
    }

    if (tcgetattr(fd, &tty) != 0)
    {
        perror("tcgetattr");
    }
    else
    {
        cfsetospeed(&tty, B9600);
        cfsetispeed(&tty, B9600);

        tty.c_cflag &= ~PARENB;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CSIZE;
        tty.c_cflag |=  CS8;
        tty.c_cflag &= ~CRTSCTS;
        tty.c_cflag |=  CLOCAL | CREAD;

        tty.c_iflag |=  IGNPAR | IGNCR;
        tty.c_iflag &= ~(IXON | IXOFF | IXANY);
        tty.c_iflag |=  ICANON;
        tty.c_iflag &= ~OPOST;

        tcsetattr(fd, TCSANOW, &tty);
    }

    return fd;
}

//  ===========================================================================
//  Turns laser on, returns status.
//  ===========================================================================
int set_laser_on(int fd)
{
    
}

//  ===========================================================================
//  Flushes read buffer.
//  ===========================================================================
void flush_read_buffer(int fd)
{
    tcflush(fd, TCIFLUSH);
}

//  ===========================================================================
//  Flushes write buffer.
//  ===========================================================================
void flush_write_buffer(int fd)
{
    tcflush(fd, TCOFLUSH);
}

//  ===========================================================================
//  Returns decoded data.
//  ===========================================================================
void decode(int fd, char *data)
{

}


//  ===========================================================================
//  Returns data block from sensor.
//  ===========================================================================
int read_data(int fd, char *data)
{
    char c;
    int  i;

    i = 0;
    while (read(fd, &c, 1) > 0 && c != STRING_LF)
    {
        data[i++] = c;
    }
    return (i-1);

}

//  ===========================================================================
//  Returns version information in version_t.
//  ===========================================================================
int get_version(int fd, version_t *version)
{
    char buf[CMD_CODE_LEN+CMD_STRING_LEN+1];
    int  err;

//    flush_write_buffer(fd);
//    flush_read_buffer(fd);

    strcpy(buf, CMD_GET_VERSION);
    strcat(buf, LF);

    if (DEBUG) PRINT_CMD(buf);

    err = write(fd, buf, strlen(buf));

    if (err < 0)
    {
        printf("Error writing command.\n");
        perror("Write to port");
        return (err);
    }

    usleep( 100000 );   // Definitely needs this!

    /*
        Error checking could be applied to
        each read_data but since the data
        is meant for display only, any
        errors will be evident.
    */
    read_data(fd, version->command);
    read_data(fd, version->string);
    read_data(fd, version->vendor);
    read_data(fd, version->product);
    read_data(fd, version->firmware);
    read_data(fd, version->protocol);
    read_data(fd, version->serial);

    return (err);
}

//  ===========================================================================
//  Returns all data until LF LF.
//  ===========================================================================
int get_data(int fd)
{
    char    c;

    printf("Return string = ");
    while (read(fd, &c, 1) > 0)
    {
        printf("%c", c);
    }
    printf("\n");

    return (0);
}

//  ===========================================================================
//  Changes communication bit rate.
//  ===========================================================================
int set_bit_rate(int fd, char rate[6], char string[16])
{
    int  err;
    char command[2] = {'\0'};
    char string_ret[16] = {'\0'};
    char status[2] = {'\0'};
    char sum[2] = {'\0'};
    char buf[CMD_CODE_LEN+CMD_STRING_LEN+1];

    /* concatenate command with string & line feed. */
    strcpy(buf, CMD_SET_BIT_RATE);
    strcat(buf, rate);
    strcat(buf, string);
    strcat(buf, LF);

    if (DEBUG)
    {
        printf("Command  = %s\n", CMD_SET_BIT_RATE);
        printf("Rate     = %s\n", rate);
//        printf("String   = %s\n", string);
        printf("Combined = %s\n", buf);
    }

    if (DEBUG) PRINT_CMD(CMD_SET_BIT_RATE);

    err = write(fd, buf, strlen(buf));

    read_data(fd, command);
    read_data(fd, string_ret);
    read_data(fd, status);
    read_data(fd, sum);

    printf("Command = %s\n", command);
    printf("String  = %s\n", string_ret);
    printf("Status  = %s\n", status);
    printf("Sum     = %s\n", sum);

//    err = get_data(fd);

    return (err); // Need to return error code.
}

//  ===========================================================================
//  Main routine.
//  ===========================================================================
int main(void)
{
    int fd;
    int err;

    version_t version = {"", "", "", "", "", "", ""};
//    version_t version = malloc(sizeof *version);

    fd = init_port();

    /* Flush buffers. */
    flush_write_buffer(fd);
    flush_read_buffer(fd);

    err = get_version(fd, &version);
    if (err < 0)
    {
        printf("Error getting version information.\n");
    }
    else
    {
        printf("VERSION INFO.\n\n");
        printf("\tVendor   : %s\n", version.vendor);
        printf("\tProduct  : %s\n", version.product);
        printf("\tFirmware : %s\n", version.firmware);
        printf("\tProtocol : %s\n", version.protocol);
        printf("\tSerial   : %s\n", version.serial);
        printf("\n");
    }

    close(fd);

    return (0);
}

