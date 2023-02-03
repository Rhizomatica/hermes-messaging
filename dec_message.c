/* dec_message
 * Copyright (C) 2023 Rhizomatica
 * Author: Rafael Diniz <rafael@riseup.net>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

#define FROM "15013812345"
#define NEXMO_API_KEY "yyyyyyyy"
#define NEXMO_API_SECRET "xxxxxxxxxxxxxxxx"
#define SMS_EMAL_FALLBACK "sms@hermes.radio"

#define BUF_SIZE 4096
#define MAX_FILENAME 4096
#define CMD_LENGTH 4096

int main(int argc, char *argv[])
{
    // read the compressed payload of the sensor
    size_t buffer_size;
    size_t message_size;
    uint8_t *message_payload; // dynamic size, read from stdin
    char tmp_buffer[BUF_SIZE];
    message_size = fread(tmp_buffer, 1, BUF_SIZE, stdin);
    message_payload = malloc(message_size);
    memcpy(message_payload, tmp_buffer, message_size);
    while ( !feof(stdin) )
    {
        size_t needle = message_size;
        buffer_size = fread(tmp_buffer, 1, BUF_SIZE, stdin);
        message_size += buffer_size;
        message_payload = realloc(message_payload, message_size);
        memcpy(message_payload + needle, tmp_buffer, buffer_size);
    }


    // get the phone msisdn
    bool number_error = false;
    char *number_needle = strstr((char *) message_payload, "\n");
    number_needle++;
    char number[BUF_SIZE];
    sscanf(number_needle, "%s", number);
    if (number[0] == '+')
        memmove(number, number+1, strlen(number));

    // verify number
    int number_pos = 0;
    while (number[number_pos] != 0)
    {
        if ( (number[number_pos] < '0' || number[number_pos] > '9' ))
        {
            number_error = true;
            break;
        }
        number_pos++;
    }

    if (number_error)
    {
        printf("Error in identifying the destination number\n");
        // send an error email?
        return EXIT_SUCCESS;
    }

    // now create the message
    char message[BUF_SIZE];
    // get the "FROM"
    sscanf((char *)message_payload, "%[^\n]\n", message);

    strcat (message,"%0A");

    char *body = strstr((char *) message_payload, number);
    body = body + strlen(number) + 1;

    size_t message_needle = strlen(message);
    for (size_t k = 0; k < (message_size - (body - (char *)message_payload)); k++)
    {
        if (body[k] == '\n')
        {
            message[message_needle] = '%'; message_needle++;
            message[message_needle] = '0'; message_needle++;
            message[message_needle] = 'A'; message_needle++;
        }
        else
        {
            message[message_needle] = body[k];
            message_needle++;
        }

    }
    message[message_needle] = 0;

    char cmd[BUF_SIZE];

    sprintf(cmd, "curl -X \"POST\" \"https://rest.nexmo.com/sms/json\" -d \"from=%s\" -d \"to=%s\" -d \"api_key=%s\" -d \"api_secret=%s\" -d \"text=%s\"\n",
            FROM, number, NEXMO_API_KEY, NEXMO_API_SECRET, message);

//    printf("cmd\n%s", cmd);

    system(cmd);

    free(message_payload);
    return EXIT_SUCCESS;
}
