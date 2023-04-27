#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OFONO_API_SUBJECT_TO_CHANGE
#include <ofono/dbus.h>

#include "send_email.h"

void receive()
{
   DBusMessage *msg;
   DBusMessageIter args, entry, string, internal;
   DBusConnection *conn;
   DBusError err;
   int ret;
   char *sigvalue;
   char *sender;
   char *message;
   char *sent_time;
   char *local_sent_time;

   // initialise the errors
   dbus_error_init(&err);
   
   // connect to the bus and check for errors
   conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
   if (dbus_error_is_set(&err)) { 
      fprintf(stderr, "Connection Error (%s)\n", err.message);
      dbus_error_free(&err); 
   }
   if (NULL == conn) { 
      exit(1);
   }
   
   // request our name on the bus and check for errors
   ret = dbus_bus_request_name(conn, "test.signal.sink", DBUS_NAME_FLAG_REPLACE_EXISTING , &err);
   if (dbus_error_is_set(&err)) { 
      fprintf(stderr, "Name Error (%s)\n", err.message);
      dbus_error_free(&err); 
   }
   if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret) {
      exit(1);
   }

   // add a rule for which messages we want to see
   dbus_bus_add_match(conn, "type='signal',interface='org.ofono.MessageManager'", &err); // see signals from the given interface
   dbus_connection_flush(conn);
   if (dbus_error_is_set(&err)) { 
      fprintf(stderr, "Match Error (%s)\n", err.message);
      exit(1); 
   }

// from dbus-monitor
//signal time=1675622773.779557 sender=:1.19 -> destination=(null destination) serial=934 path=/quectelqmi_0; interface=org.ofono.MessageManager; member=IncomingMessage
//   string "Vai
// Que vai"
//  array [
//     dict entry(
//       string "LocalSentTime"
//       variant             string "2023-02-05T21:46:12+0300"
//    )
//    dict entry(
//       string "SentTime"
//       variant             string "2023-02-05T21:46:12+0300"
//    )
//    dict entry(
//       string "Sender"
//       variant             string "+79096274557"
//    )
// ]

   // loop listening for signals being emmitted
   while (true) {

      // non blocking read of the next available message
      dbus_connection_read_write(conn, 0);
      msg = dbus_connection_pop_message(conn);

      // loop again if we haven't read a message
      if (NULL == msg) { 
         sleep(1);
         continue;
      }

      const char *interface = dbus_message_get_interface (msg);
      fprintf(stderr, "interface: %s\n", interface);
//      if (!strcmp(interface, "org.ofono.MessageManager"))
      if (dbus_message_is_signal(msg, "org.ofono.MessageManager", "IncomingMessage") ||
          dbus_message_is_signal(msg, "org.ofono.MessageManager", "ImmediateMessage") )
      {
          // read the parameters
         if (!dbus_message_iter_init(msg, &args))
            fprintf(stderr, "Message Has No Parameters\n");
         else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args))
            fprintf(stderr, "Argument is not string!\n"); 
         else
            dbus_message_iter_get_basic(&args, &sigvalue);

         printf("Got Message:\n%s\n", sigvalue);
         message = strdup(interface);

         dbus_message_iter_next(&args);

         if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_ARRAY)
            continue;

         dbus_message_iter_recurse(&args, &entry);

         if (dbus_message_iter_get_arg_type(&entry) != DBUS_TYPE_DICT_ENTRY)
             continue;

//         int items = dbus_message_iter_get_array_len(&entry);
//         printf("length: %d\n", items);

         while (dbus_message_iter_get_arg_type(&entry) == DBUS_TYPE_DICT_ENTRY)
         {
             const char *interface;

             dbus_message_iter_recurse(&entry, &string);
             dbus_message_iter_get_basic(&string, &interface);
             printf("Got value: %s\n", interface);
             if ( !strcmp(interface, "LocalSentTime") || !strcmp(interface, "SentTime") || !strcmp(interface, "Sender") )
             {
                 dbus_message_iter_next(&string);
                 // if (dbus_message_iter_get_arg_type(&string) == DBUS_TYPE_VARIANT)
                 dbus_message_iter_recurse(&string, &internal);
                 dbus_message_iter_get_basic(&internal, &interface);
                 printf("Got value: %s\n", interface);
                 if (!strcmp(interface, "LocalSentTime"))
                     local_sent_time = strdup(interface);
                 if (!strcmp(interface, "SentTime"))
                     sent_time = strdup(interface);
                 if (!strcmp(interface, "Sender"))
                     sender = strdup(interface);

                 dbus_message_iter_next(&string);
             }
             // next string e golazzo!
             dbus_message_iter_next(&entry);
         }

         send_email(sender, sent_time, message);
         // process(sms);
         free(sender);
         free(message);
         free(sent_time);
         free(local_sent_time);
         free(message);
      }

      // free the message
      dbus_message_unref(msg);
   }
   // close the connection
   dbus_connection_close(conn);
}

int main(int argc, char** argv)
{
   if (argc > 1) {
      printf ("Syntax: %s\n", argv[0]);
      return 1;
   }

   receive();

   return 0;
}
