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
//  Reads data.
//  ===========================================================================
const char *read_data(int fd)
{

//    char    *ret;     // Causes segfault @ ret[n++] = c;
    char ret[] = {'0'};
    char    c;
    uint8_t n;
    bool    data_end = false;
    bool    line_end = false;
    ssize_t err;

    n = 0;
//    ret = "";

    if (DEBUG) printf("Reading port:\n");

    while (!data_end)
    {
        err = read(fd, &c, 1);
        printf("n = %d, s = %c\n", n, c);
        if (err <= 0)
        {
            printf("Error reading, err = %d.\n", (int)err);
            data_end = true;
        }

        ret[n++] = c;
        if (n > 1)
        {
            if ((c = STRING_LF) && (ret[n-1] = STRING_LF))
            {
                printf("\nEnd of data.\n");
                data_end = true;
            }
        }
    }

    if (DEBUG) printf("Data = %s\n", ret );

/*
        while (!line_end)
        {

            if (DEBUG) printf("n = %d\n", n);
            err = read(fd, &c, 1);
            if (err <= 0)
            {
                if (DEBUG) printf("Read error.\n");
                line_end = true;
                data_end = true;
                break;
            }
            if (DEBUG) printf("%c", c);
            ret[n++] = c;

            // Check for double LF in sequence - end of data.
            if (DEBUG) printf("Checking for double LF.\n");
            if ((n > 0 ) && (c = STRING_LF) && (ret[n-1] = STRING_LF))
            {
                data_end = true;
                printf("End of data.\n");
            }
            // Check whether there is anything on port.
            if (c <= 0 )
            {
                data_end = true;
                if (DEBUG) printf("Nothing there!\n");
            }
            // Check for LF - end of line.
            if ((c <= 0) || (c = STRING_LF))
            {
                line_end = true;
            }
            // Check for maximum line length.
            if (n >= RET_DATA_BLOCK_MAX)
            {
                line_end = true;
                printf("Max block length reached\n");
            }
        }
    if (DEBUG) printf("\n");

    }
*/
    return *ret;

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
    char    c;

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
/*


*/
void get_version()
{
    if (DEBUG) PRINT_CMD(CMD_GET_VERSION);

}


int main(void)
{

    const char *data;
//    char data[];
    int fd;
    int err;

    /* Open port. */
    fd = open(USB_PORT, O_RDWR | O_NOCTTY | O_NONBLOCK);

    if (fd < 0) printf("Could not open port\n");
    else printf("Opened USB port\n");
    empty_buffer(fd);

    err = send_command(fd, CMD_GET_VERSION, "Jaguar");

    if (err < 0)
    {
        printf("Error sending command, err = %d.\n", err);
    }

    data = read_data(fd);

    printf("Return = %s\n", data );

    close(fd);

    return (0);
}
