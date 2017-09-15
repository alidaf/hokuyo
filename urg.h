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


// Functions
/*
    encode_command
    send_command
    decode_data
    check_errors
*/
