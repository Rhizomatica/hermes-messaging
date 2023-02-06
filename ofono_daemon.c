#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define OFONO_API_SUBJECT_TO_CHANGE
#include <ofono/dbus.h>

/**
 * Listens for signals on the bus
 */
void receive()
{
   DBusMessage* msg;
   DBusMessageIter args, entry, string;
   DBusConnection* conn;
   DBusError err;
   int ret;
   char* sigvalue;

   printf("Listening for signals\n");

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

      char *interface = dbus_message_get_interface (msg);
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

         dbus_message_iter_next(&args);

         if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_ARRAY)
            continue;

         dbus_message_iter_recurse(&args, &entry);

         if (dbus_message_iter_get_arg_type(&entry) != DBUS_TYPE_DICT_ENTRY)
             continue;

         int items = dbus_message_iter_get_array_len(&entry);

         printf("length: %d\n", items);

         while (dbus_message_iter_get_arg_type(&entry) == DBUS_TYPE_DICT_ENTRY)
         {
             char *interface;
             //dbus_message_iter_get_basic(&entry, &interface);
             //printf("Got interface: %s\n", interface);

             dbus_message_iter_recurse(&entry, &string);
             dbus_message_iter_get_basic(&string, &interface);
             printf("Got value: %s\n", interface);

             dbus_message_iter_next(&entry);
         }

         // what to do here?

//        dbus_message_iter_open_container(&args, DBUS_MESSAGE_ITER_TYPE_DICT,
 //                                         OFONO_PROPERTIES_ARRAY_SIGNATURE,
  //                                        &dict);


//int 	dbus_message_iter_get_arg_type (DBusMessageIter *iter)
// 	Returns the argument type of the argument that the message iterator points to. More...

//int 	dbus_message_iter_get_element_type (DBusMessageIter *iter)
// 	Returns the element type of the array that the message iterator points to. More...

//void 	dbus_message_iter_recurse (DBusMessageIter *iter, DBusMessageIter *sub)
// 	Recurses into a container value when reading values from a message, initializing a sub-iterator

//         int elem_count = dbus_message_iter_get_element_count(&dict);
 //        fprintf(stderr, "Element count: %d\n", elem_count);

         // fprintf(stderr, "container: %s\n",dbus_message_get_container_instance(msg));
//         fprintf(stderr,"container: %s\n",dbus_message_get_data(msg,0));

//         dbus_message_iter_next(&args);

//         if (dbus_message_iter_get_arg_type(&args) == DBUS_MESSAGE_ITER_TYPE_DICT)
//             fprintf(stderr, "-- dict type\n");
//         if (dbus_message_iter_get_arg_type(&args) == DBUS_MESSAGE_ITER_TYPE_ARRAY)
//             fprintf(stderr, "-- array type\n");

//         dbus_message_iter_get_basic(&args, &sigvalue);

//         printf("Got Signal with value %s\n", sigvalue);

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
