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

/*
    Interface: USB via CDC-ACM built into kernel.

    Data Encoding & Decoding:

    Sensor data is encoded to reduce transmission time using either 2, 3 or 4
    character encoding. In each case the data is split into 6 bit chunks and
    0x30H added to each chunk to convert them to ASCII. Decoding follows the
    reverse process.

    Encoding example (4 character):

    16,000,000 ms
    binary          = 111101000010010000000000
    6-bit chunks    = 111101 000010 010000 000000
    Hexadecimal     =   0x3d   0x02   0x10   0x00
    Add 0x30H       =   0x6d   0x32   0x40   0x30
    ASCII equiv     =      m      2      @      0

    Decoding example (4 character):

    Code            =      m      2      @      0
    Hexadecimal     =   0x6d   0x32   0x40   0x30
    Subtract 0x30H  =   0x3d   0x02   0x10   0x00
    6-bit chunks    = 111101 000010 010000 000000
    Merged          = 111101000010010000000000
                    = 16,000,000

    Format:

    Communication is initiated form the host to the sensor by sending a
    command that consists of a command symbol, parameter, and string
    characters followed by a line feed or carriage return, or both. The
    sensor replies with the command echo, status, sum, line feed, data
    related to the command, sum and two line feeds as a termination code.

    i.e.

    Host -> sensor:

    ,---------------------------------------,
    | CMD | Param | String (Max 16) | LF/CR |
    '---------------------------------------'

    Sensor -> Host (reply):

    ,--------------------------------------------------------------------,
    | CMD | Param String | LF | Status | Sum | LF | Data | Sum | LF | LF |
    '--------------------------------------------------------------------'


    The command sybol is a 2-byte code.
    The string is any combination of alphanumeric chars, space, and the
    symbols .,_+@. A semicolon is used to separate the string from the
    parameter.

    Status is a 2-byte error code, 00 and 99 indicating success.
    Sum is a 1-byte checksum calculated from summing the string or return
    data, taking the lower 6 bits and adding 0x30H, e.g.

    [LF]Hokuyo[LF]  = 0x48+0x6f+0x6b+0x75+0x79+0x6f
                    = 0x27f
                    = 1001111111
    Lower 6 bits    = 111111 = 0x3f
    Add 0x30        = 0x6f
    ASCII           = o

    If the return data exceeds 64 bytes, a LF is inserted and a sum is
    calculated after every 64 bytes.

    Commands:

    There are 13 types of predefined sensor commands in SCIP2.0.
    Multiple commands (must not be of the same type) may be sent at one time,
    with each command being dealt with and replied to, in turn.

*/

//  ===========================================================================

#include <stdint.h>
#include <termios.h>

//  Defines. ------------------------------------------------------------------

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

#define BIT_RATE_0  115200  // Initial baud.

/* Number of lines returned for each command. */
#define RET_VERSION_LINES    7

/* Maximum data block size */
#define DATA_CMD_LEN     2
#define DATA_STRING_LEN 16
#define DATA_BLOCK_LEN  700
#define DATA_SUM_LEN     1
#define DATA_STATUS_LEN  2
#define DATA_EOL_LEN     2 // Accounts for 2 LF for end of data line.

// ASCII codes for commands and data.
#define LF "\n" // Line Feed.
#define CR "\r" // Carriage Return.

#define STRING_NULL '\0'
#define STRING_LF   '\n'
#define STRING_CR   '\r'

#define USB_PORT "/dev/ttyACM0" // Output port for USB.

#define SENSORS_MAX 4   // Max number of sensors.

//  Types. --------------------------------------------------------------------

typedef struct
{
//    char command[64];
//    char string[64];
    char vendor[64];
    char product[64];
    char firmware[64];
    char protocol[64];
    char serial[64];
/*
    char command[DATA_BLOCK_LEN];
    char string[DATA_BLOCK_LEN];
    char vendor[DATA_BLOCK_LEN];
    char product[DATA_BLOCK_LEN];
    char firmware[DATA_BLOCK_LEN];
    char protocol[DATA_BLOCK_LEN];
    char serial[DATA_BLOCK_LEN];
*/
} version_t;

typedef struct
{
    char *buffer;
    int   size;
    int   first;
    int   last;
} buffer_t;

typedef struct
{
    int fd;
    struct termios settings;
} serial_t;

typedef struct
{
    uint8_t id;
    version_t version;
    serial_t serial;
    char data[DATA_BLOCK_LEN];
} sensor_t;

//  Array of sensors.
sensor_t *sensor[SENSORS_MAX];

