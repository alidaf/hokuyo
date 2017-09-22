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
void clear_port(int fd)
{
    tcdrain(fd);
    tcflush(fd, TCIOFLUSH);
}

//  ===========================================================================
//  Initialises serial port.
//  ===========================================================================
int init_port()
{
    struct termios options;

    int fd;

    int flags = 0;

    fd = open(USB_PORT, O_RDWR | O_NOCTTY | O_NONBLOCK);

    if (fd < 0)
    {
        perror("Error opening port");
        exit(-1);
    }

    flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);

    // Get current port options.
    tcgetattr(fd, &options);

    // Set port options.
    options.c_iflag = 0;
    options.c_oflag = 0;

    options.c_cflag &= ~(CSIZE | PARENB | CSTOPB);
    options.c_cflag |= CS8 | CREAD | CLOCAL;
    options.c_lflag &= ~(ICANON | ECHO | ISIG | IEXTEN);

    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 0;

    cfsetospeed(&options, B19200);
    cfsetispeed(&options, B19200);

    tcsetattr(fd, TCSADRAIN, &options);
    clear_port(fd);

    return fd;
}

//  ===========================================================================
//  Writes command to port.
//  ===========================================================================
int write_command(int fd, const char *data, int size)
{
    return write(fd, data, size);
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
int get_data(int fd, char *data)
{
    char c;
    int  i;

    i = 0;

/*
    while (read(fd, &c, 1) > 0 && (c != STRING_LF))
    {
        data[i++] = c;
    }
    return (i-1);
*/

    while (read(fd, &c, 1) > 0 && (c != STRING_LF))
    {
        data[i++] = c;
    }

//    data[i+1] = STRING_LF;

    return (i-1);

}

//  ===========================================================================
//  Turns laser on, returns status.
//  ===========================================================================
int set_laser_on(int fd, char string[DATA_STRING_LEN])
{
    int  err;

//    char command[DATA_CMD_LEN + DATA_STRING_LEN];
//    char status[DATA_STATUS_LEN + DATA_SUM_LEN + DATA_EOL_LEN];
    char cmd[DATA_BLOCK_LEN];
    char status[DATA_BLOCK_LEN];

    clear_port(fd);
//    flush_write_buffer(fd);
//    flush_read_buffer(fd);

    strcpy(cmd, CMD_SET_LASER_ON);
//    strcat(cmd, string);  // Don't know why this kills the return data!
    strcat(cmd, LF);

    if (DEBUG) PRINT_CMD(cmd);

    err = write(fd, cmd, strlen(cmd));

    if (err < 0)
    {
        printf("Error writing command.\n");
        perror("Set laser on");
        return (err);
    }

    usleep( 100000 ); // Definitely needs this!

    get_data(fd, cmd);
    get_data(fd, status);

    printf("\tCommand: %s\n", cmd);
    printf("\tStatus: %s\n", status);
    printf("\n");

//    flush_read_buffer(fd);

    return (err);

}

//  ===========================================================================
//  Turns laser off.
//  ===========================================================================
int set_laser_off(int fd, char string[DATA_STRING_LEN])
{
    int  err;

//    char command[DATA_CMD_LEN + DATA_STRING_LEN];
//    char status[DATA_STATUS_LEN + DATA_SUM_LEN + DATA_EOL_LEN];
    char cmd[DATA_BLOCK_LEN];
    char status[DATA_BLOCK_LEN];

    clear_port(fd);
//    flush_write_buffer(fd);
//    flush_read_buffer(fd);

    strcpy(cmd, CMD_SET_LASER_OFF);
//    strcat(cmd, string);  // Don't know why this kills the return data!
    strcat(cmd, LF);

    if (DEBUG) PRINT_CMD(cmd);

    err = write(fd, cmd, strlen(cmd));

    if (err < 0)
    {
        printf("Error writing command.\n");
        perror("Set laser on");
        return (err);
    }

    usleep( 100000 ); // Definitely needs this!

    get_data(fd, cmd);
    get_data(fd, status);

    printf("\tCommand: %s\n", cmd);
    printf("\tStatus: %s\n", status);
    printf("\n");

//    flush_read_buffer(fd);

    return (err);

}

//  ===========================================================================
//  Returns version information in version_t.
//  ===========================================================================
int get_version(int fd, char string[16], version_t *version)
{
    char cmd[DATA_CMD_LEN + DATA_STRING_LEN];
    int  err;

//    flush_write_buffer(fd);
//    flush_read_buffer(fd);

    strcpy(cmd, CMD_GET_VERSION);
//    strcat(buf, string);  // Don't know why this kills the return data!
    strcat(cmd, LF);

    if (DEBUG) PRINT_CMD(cmd);

    clear_port(fd);
    err = write(fd, cmd, strlen(cmd));

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
    get_data(fd, version->command);
    get_data(fd, version->string);
    get_data(fd, version->vendor);
    get_data(fd, version->product);
    get_data(fd, version->firmware);
    get_data(fd, version->protocol);
    get_data(fd, version->serial);

//    flush_read_buffer(fd);

    return (err);
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
    char buf[DATA_CMD_LEN + DATA_STRING_LEN + 1];

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

    clear_port(fd);
    err = write(fd, buf, strlen(buf));

    get_data(fd, command);
    get_data(fd, string_ret);
    get_data(fd, status);
    get_data(fd, sum);

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
    int     fd;
    int     err;



    version_t version = {"", "", "", "", "", "", ""};
//    id = "Jaguar";
//    version_t version = malloc(sizeof *version);

    fd = init_port();

    /* Flush buffers. */
//    flush_write_buffer(fd);
//    flush_read_buffer(fd);

    err = get_version(fd, "Jaguar", &version);
    if (err < 0)
    {
        printf("Error getting version information.\n");
    }
    else
    {
        printf("VERSION INFO.\n\n");
        printf("\tCommand  : %s\n", version.command);
        printf("\tString   : %s\n", version.string);
        printf("\tVendor   : %s\n", version.vendor);
        printf("\tProduct  : %s\n", version.product);
        printf("\tFirmware : %s\n", version.firmware);
        printf("\tProtocol : %s\n", version.protocol);
        printf("\tSerial   : %s\n", version.serial);
        printf("\n");
    }

    err = set_laser_on(fd, "Jaguar");
    err = set_laser_on(fd, "Jaguar");
    set_laser_off(fd, "Jaguar");
    set_laser_off(fd, "Jaguar");
/*
    char sum;
    sum = get_data_sum("FIRM:3.4.03(17/Dec./2012)");
    printf("Sum for FIRM:3.4.03(17/Dec./2012) = %c\n", sum);
    sum = get_data_sum("PROT:SCIP 2.0");
    printf("Sum for PROT:SCIP 2.0 = %c\n", sum);
    sum = get_data_sum("SERI:H1620245");
    printf("Sum for SERI:H1620245 = %c\n", sum);
    sum = get_data_sum("00");
    printf("Sum for 00 = %c\n", sum);
    sum = get_data_sum("01");
    printf("Sum for 01 = %c\n", sum);
    sum = get_data_sum("02");
    printf("Sum for 02 = %c\n", sum);
*/

    close(fd);

    return (0);
}

