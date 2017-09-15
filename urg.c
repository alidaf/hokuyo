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
#include <string.h>
#include <stdint.h>
#include <fcntl.h>

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

// ASCII codes for commands and data.
#define LF "\n" // Line Feed.
#define CR "\r" // Carriage Return.

#define USB_PORT "/dev/ttyACM0" // Output port for USB.

//  Commands ------------------------------------------------------------------


//  ===========================================================================
//  Encodes and sends command.
//  ===========================================================================
const char *encode_command(int fd, char command[2], const char *string)
{
    /*
        fd is file descriptor.
        command is 2 chars.
        string + command must be <= 16 chars.
    */


    if (DEBUG) {
        printf("Command = %s\n", command);
        printf("String  = %s\n", string);
    }

    ssize_t err;
    char    c;
    char    buf[CMD_CODE_LEN+CMD_STRING_LEN+1];
    char   *ret;
    uint16_t n;

    /* concatenate command with string & line feed. */
    strcpy(buf, command);
    strcat(buf, string);
    strcat(buf, LF);

    err = write(fd, buf, strlen(buf));
    /* Add error trapping. */

    /* Read in return data. */
    n = 0;
    ret = "";
    while (read(fd, &c, 1) > 0 && c != '\n') {
        ret[n++] = c;
    }
}

//  ===========================================================================
//  Returns version information.
//  ===========================================================================
/*


*/
uint8_t get_version()
{
    if (DEBUG) PRINT_CMD(CMD_GET_VERSION);


}


int main()
{

    const char *data;

    /* Open port. */
    int fd = open(USB_PORT, O_RDWR | O_NOCTTY | O_NONBLOCK);

    data = encode_command(fd, CMD_GET_VERSION, "Nothing");

    close(fd);
    return 0;
}
