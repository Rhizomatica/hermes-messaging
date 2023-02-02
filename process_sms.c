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
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "process_sms.h"
#include "send_email.h"

void urldecode(char *dst, char *src)
{
    char a, b;
    while (*src) {
        if ((*src == '%') &&
            ((a = src[1]) && (b = src[2])) &&
            (isxdigit(a) && isxdigit(b))) {
            if (a >= 'a')
                a -= 'a'-'A';
            if (a >= 'A')
                a -= ('A' - 10);
            else
                a -= '0';
            if (b >= 'a')
                b -= 'a'-'A';
            if (b >= 'A')
                b -= ('A' - 10);
            else
                b -= '0';
            *dst++ = 16*a+b;
            src+=3;
        } else if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst++ = '\0';
}


bool process_sms(char *uri)
{
    char *char_ptr = NULL;

    int index;

    char from[BUFSIZE];
    char dest[BUFSIZE];
    char messageId[BUFSIZE];
    char type[BUFSIZE];
    char api_key[BUFSIZE];

    char keyword[BUFSIZE];
    char keyword_dec[BUFSIZE];

    char timestamp[BUFSIZE];
    char timestamp_dec[BUFSIZE];

    char message[BUFSIZE];
    char message_dec[BUFSIZE];

    bool concat_message_ready;
    bool concat_message_flag;

    static int concat_ref;
    static char concat_message[BUFSIZE];


    printf("Is SMS!\n");
    printf("URI = %s\n\n", uri);

    // get msisdn (aka: from)
    char_ptr =  strstr(uri, "?msisdn=") + strlen("?msisdn=");
    if (char_ptr)
    {
        for(index = 0; char_ptr[index] != '&' && char_ptr[index] != 0; index++)
            from[index] = char_ptr[index];
        from[index] = 0;
        // printf("msisdn=%s\n", from);
    }
    else
        return false;

    // get to
    char_ptr =  strstr(uri, "&to=") + strlen("&to=");
    if (char_ptr)
    {
        for(index = 0; char_ptr[index] != '&' && char_ptr[index] != 0; index++)
            dest[index] = char_ptr[index];
        dest[index] = 0;
        // printf("to=%s\n", dest);
    }
    else
        return false;

    // get messageId
    char_ptr =  strstr(uri, "&messageId=") + strlen("&messageId=");
    if (char_ptr)
    {
        for(index = 0; char_ptr[index] != '&' && char_ptr[index] != 0; index++)
            messageId[index] = char_ptr[index];
        messageId[index] = 0;
        // printf("messageId=%s\n", messageId);
    }
    else
        return false;

    // get type
    char_ptr =  strstr(uri, "&type=") + strlen("&type=");
    if (char_ptr)
    {
        for(index = 0; char_ptr[index] != '&' && char_ptr[index] != 0; index++)
            type[index] = char_ptr[index];
        type[index] = 0;
        // printf("type=%s\n", type);
    }
    else
        return false;

    // get keyword
    char_ptr =  strstr(uri, "&keyword=") + strlen("&keyword=");
    if (char_ptr)
    {
        for(index = 0; char_ptr[index] != '&' && char_ptr[index] != 0; index++)
            keyword[index] = char_ptr[index];
        keyword[index] = 0;
        // printf("keyword raw = %s\n", keyword);

        urldecode(keyword_dec, keyword);
        printf("keyword=%s\n", keyword_dec);
    }
    else
        return false;

    // get api-key
    char_ptr =  strstr(uri, "&api-key=") + strlen("&api-key=");
    if (char_ptr)
    {
        for(index = 0; char_ptr[index] != '&' && char_ptr[index] != 0; index++)
            api_key[index] = char_ptr[index];
        api_key[index] = 0;
        // printf("api-key=%s\n", api_key);
    }
    else
        return false;

    // get message-timestamp
    char_ptr =  strstr(uri, "&message-timestamp=") + strlen("&message-timestamp=");
    if (char_ptr)
    {
        for(index = 0; char_ptr[index] != '&' && char_ptr[index] != 0; index++)
            timestamp[index] = char_ptr[index];
        timestamp[index] = 0;
        // printf("message-timestamp raw = %s\n", timestamp);

        urldecode(timestamp_dec, timestamp);
        // printf("message-timestamp=%s\n", timestamp_dec);

    }
    else
        return false;

    char_ptr =  strstr(uri, "&text=") + strlen("&text=");
    if (char_ptr)
    {
        for(index = 0; char_ptr[index] != '&' && char_ptr[index] != 0; index++)
            message[index] = char_ptr[index];
        message[index] = 0;
        //printf("message raw = %s\n", message);

        // decode message

        urldecode(message_dec, message);
        printf("text=%s\n", message_dec);
    }
    else
        return false;

    // now we check if we are dealing with a concatenated messages...
    char_ptr =  strstr(uri, "&concat=true");
    if (char_ptr)
    {
        concat_message_flag = true;

        // &concat-ref=222&concat-total=3&concat-part=3
        char_ptr =  strstr(uri, "&concat-ref=") + strlen("&concat-ref=");
        int concat_ref_now = atoi(char_ptr);
        // decode message

        char_ptr =  strstr(uri, "&concat-total=") + strlen("&concat-total=");
        int concat_total = atoi(char_ptr);

        char_ptr =  strstr(uri, "&concat-part=") + strlen("&concat-part=");
        int concat_part = atoi(char_ptr);

        printf("concat-ref=%d\nconcat-total=%d\nconcat-part=%d\n", concat_ref_now, concat_total, concat_part);

        if (!concat_part || !concat_total)
        {
            printf("Error dealing with concatenated messages!\n");
            return false;
        }

        if (concat_part == 1)
            concat_ref = concat_ref_now;

        if (concat_part > 1 && concat_ref != concat_ref_now)
        {
            printf("Error dealing with concatenated messages!\n");
            return false;
        }

        if (concat_part == 1)
            strcpy(concat_message, message_dec);
        else
            strcat(concat_message, message_dec);

        if (concat_total == concat_part)
        {
            concat_message_ready = true;
            printf("concatenated-message=%s\n",concat_message);
        }
        else
        {
            concat_message_ready = false;
        }
    }
    else
    {
        concat_message_flag = false;
    }


    if (concat_message_flag && !concat_message_ready)
        return true;

    send_email(from, timestamp_dec, concat_message_flag? concat_message : message_dec);

    return true;
}

#if 0

normal:
GET /?msisdn=5511998728111&to=5511953259200&messageId=2800000005569122&text=L%C3%A1+vai+https&type=unicode&keyword=L%C3%81&api-key=cd906872&message-timestamp=2022-07-11+09%3A58%3A54 HTTP/1.1



concat:
/?msisdn=5511998728111&to=5511953259200&messageId=28000000054225B9&concat=true&concat-ref=222&concat-total=3&concat-part=1&text=Siiiiiiim+senhor+meu+patr%C3%A3o+de+longa+data+vamos+fazer+uma+mensagem+&type=unicode&keyword=SIIIIIIIM&api-key=cd906872&message-timestamp=2022-07-08+20%3A38%3A36

/?msisdn=5511998728111&to=5511953259200&messageId=28000000054225DB&concat=true&concat-ref=222&concat-total=3&concat-part=2&text=at%C3%A9+que+vem+longuinha+mesmo+ser%C3%A1+que+vain123445677889+aasssss+looon&type=unicode&keyword=AT%C3%89&api-key=cd906872&message-timestamp=2022-07-08+20%3A38%3A40 HTTP/1.1

/?msisdn=5511998728111&to=5511953259200&messageId=26000000012C9274&concat=true&concat-ref=222&concat-total=3&concat-part=3&text=ga%C3%A9+ajdjduejdjdjeuudjdjjdjdjdjjj+23456777553224565324664335534&type=unicode&keyword=GA%C3%89&api-key=cd906872&message-timestamp=2022-07-08+20%3A38%3A43


#endif
