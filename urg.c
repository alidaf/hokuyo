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
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdbool.h>

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


void empty_buffer(int fd)
{
    char c;
    while(read(fd, &c, 1) > 0);
}

/*
uint8_t decode(int fd, uint16_t len)
{



}
*/

//  ===========================================================================
//  Returns data block from sensor.
//  ===========================================================================
void read_data(int fd, char *data)
{

    char    c;
    uint8_t i;

    i = 0;
    while (read(fd, &c, 1) > 0 && c != '\n')
    {
        data[i++] = c;
    }

}

//  ===========================================================================
//  Sends command.
//  ===========================================================================
int send_command(int fd, char command[2], const char *string)
{
    /*
        fd is file descriptor.
        command is 2 chars.
        string + command must be <= 16 chars.
    */

    ssize_t err;
    char    buf[CMD_CODE_LEN+CMD_STRING_LEN+1];

    /* concatenate command with string & line feed. */
    strcpy(buf, command);
    strcat(buf, string);
    strcat(buf, LF);

    if (DEBUG)
    {
        printf("Command  = %s\n", command);
        printf("String   = %s\n", string);
        printf("Combined = %s\n", buf);
    }

    err = write(fd, buf, strlen(buf));
    if (err < 0)
    {
        return (err);
    }

    usleep( 100000 );
    return (0);

//    while(read(fd, &c, 1) > 0 && c != STRING_LF);
}

//  ===========================================================================
//  Returns version information.
//  ===========================================================================
int get_version(int fd, char string[16], version_t *version)
{
    int     err;

    if (DEBUG) PRINT_CMD(CMD_GET_VERSION);

    empty_buffer(fd);

    err = send_command(fd, CMD_GET_VERSION, string);
    if (err < 0)
    {
        if (DEBUG) printf("Error sending command, err = %d.\n", err);
        return (err);
    }

    read_data(fd, version->command);
    read_data(fd, version->string);
    read_data(fd, version->vendor);
    read_data(fd, version->product);
    read_data(fd, version->firmware);
    read_data(fd, version->protocol);
    read_data(fd, version->serial);

    return (0);
}


int main(void)
{

    int fd;
    int err;

    version_t version = {"", "", "", "", "", "", ""};
//    version_t version = malloc(sizeof *version);

    /* Open port. */
    fd = open(USB_PORT, O_RDWR | O_NOCTTY | O_NONBLOCK);

    if (fd < 0) printf("Could not open port\n");
    else printf("Opened USB port\n");
    empty_buffer(fd);

    err = get_version(fd, "jaguar", &version);
    if (err < 0)
    {
        printf("Error %d!\n", err);
    }

    printf("Return command = %s\n", version.command);
    printf("Return string  = %s\n", version.string);
    printf("Vendor: %s\n", version.vendor);
    printf("Product: %s\n", version.product);
    printf("Firmware: %s\n", version.firmware);
    printf("Protocol: %s\n", version.protocol);
    printf("Serial: %s\n", version.serial);

    close(fd);

    return (0);
}
