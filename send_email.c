/*
 * Copyright (C) 2022 Rhizomatica <rafael@rhizomatica.org>
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
 * SMS processor for Nexmo / Vonage
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>


#include "process_sms.h"
#include "send_email.h"

// mail -s "teste" rafael2k@hermes.radio

#define SUBJECT "SMS HERMES"
#define FROM "default_from@provider.host"
#define DEFAULT_TO "default_to@provider.host"

bool send_email(char *from, char *timestamp, char *body)
{
    char mail_cmd[BUFSIZE] = {0};
    FILE *email_body;
    char dest_email1[512] = {0};
    char dest_email2[512] = {0};
    char dest_email3[512] = {0};


    strcat (mail_cmd, "(mail -r ");
    strcat (mail_cmd, FROM);
    strcat (mail_cmd, " -s \"");
    strcat (mail_cmd, SUBJECT);
    strcat (mail_cmd, "\" -a \"Content-Type: text/plain; charset=UTF-8\" ");

    // maximum 3 dest emails for now
    sscanf(body, "%s %s %s", dest_email1, dest_email2, dest_email3);

    int email_count = 0;
    if(dest_email1[0])
    {
        if (strstr(dest_email1, "@"))
        {
            strcat (mail_cmd, dest_email1); strcat (mail_cmd, " ");
            email_count++;
        }
        else
        {
            strcat (mail_cmd, DEFAULT_TO);
        }
    }
    if(email_count && dest_email2[0])
    {
        if (strstr(dest_email2, "@"))
        {
            strcat (mail_cmd, dest_email2); strcat (mail_cmd, " ");
            email_count++;
        }
    }
    if(email_count && dest_email3[0])
    {
        if (strstr(dest_email3, "@"))
        {
            strcat (mail_cmd, dest_email3); strcat (mail_cmd, " ");
            email_count++;
        }
    }

    int len = strlen(mail_cmd);
    mail_cmd[len] = ')';
    mail_cmd[len+1] = 0;


    printf("Running: %s\n", mail_cmd);

    email_body = popen(mail_cmd, "w");
    fprintf(email_body, "Phone: %s\n", from);
    fprintf(email_body, "%s\n\n", timestamp);

    // skip the emails from the body
    char *needle = body;
    size_t j = 0;
    for (int i = 0; i < email_count; i++)
    {
        while (needle[j] != '@')
            j++;
        j++;
    }

    if (j)
    {
        while ( j && needle[j] != 0 && needle[j] != ' ' && needle[j] != '\n' && j < strlen(body) )
            j++;
        if (needle[j] != 0)
            j++;
    }

    fprintf(email_body, "%s", body+j);
    pclose(email_body);

    return true;
}
