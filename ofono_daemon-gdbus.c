/*
 * bluez_list_devices.c - Find the Bluetooth devices
 * 	- The example uses GDBUS to get the list of bluetooth devices using DBUS
 * 	  interfaces provided by bluez. This lists only the devices which are already or
 *	  currently enumerated by the Adapter.
 * 	- The device may be appeared in followind conditions,
 *		- Previously paired with Adapter
 *		- Previously paired and connected with Adapter
 *		- Appeared after StartDiscovery session (but not yet used/removed)
 * gcc `pkg-config --cflags glib-2.0 gio-2.0` -Wall -Wextra -o ./bin/bluez_list_devices ./bluez_list_devices.c `pkg-config --libs glib-2.0 gio-2.0`
 */

#include <glib.h>
#include <gio/gio.h>

GDBusConnection *con;
static void bluez_property_value(const gchar *key, GVariant *value)
{
	const gchar *type = g_variant_get_type_string(value);

	g_print("\t%s : ", key);
	switch(*type) {
		case 's':
			g_print("%s\n", g_variant_get_string(value, NULL));
			break;
		case 'b':
			g_print("%d\n", g_variant_get_boolean(value));
			break;
		case 'u':
			g_print("%d\n", g_variant_get_uint32(value));
			break;
		case 'a':
			g_print("\n");
			const gchar *uuid;
			GVariantIter i;
			g_variant_iter_init(&i, value);
			while(g_variant_iter_next(&i, "s", &uuid))
				g_print("\t\t%s\n", uuid);
			break;
		default:
			g_print("Other\n");
			break;
	}
}

static void bluez_list_devices(GDBusConnection *con,
				GAsyncResult *res,
				gpointer data)
{
	(void)data;
	GVariant *result = NULL;
	GVariantIter i;
	const gchar *object_path;
	GVariant *ifaces_and_properties;

	result = g_dbus_connection_call_finish(con, res, NULL);
	if(result == NULL)
		g_print("Unable to get result for GetManagedObjects\n");

	/* Parse the result */
	if(result) {
		result = g_variant_get_child_value(result, 0);
		g_variant_iter_init(&i, result);
		while(g_variant_iter_next(&i, "{&o@a{sa{sv}}}", &object_path, &ifaces_and_properties)) {
			const gchar *interface_name;
			GVariant *properties;
			GVariantIter ii;
			g_variant_iter_init(&ii, ifaces_and_properties);
			while(g_variant_iter_next(&ii, "{&s@a{sv}}", &interface_name, &properties)) {
				if(g_strstr_len(g_ascii_strdown(interface_name, -1), -1, "device")) {
					g_print("[ %s ]\n", object_path);
 					const gchar *property_name;
					GVariantIter iii;
					GVariant *prop_val;
					g_variant_iter_init(&iii, properties);
					while(g_variant_iter_next(&iii, "{&sv}", &property_name, &prop_val))
						bluez_property_value(property_name, prop_val);
					g_variant_unref(prop_val);
				}
				g_variant_unref(properties);
			}
			g_variant_unref(ifaces_and_properties);
		}
		g_variant_unref(result);
	}
	g_main_loop_quit((GMainLoop *)data);
}

int main(void)
{
	GMainLoop *loop;

	con = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL);
	if(con == NULL) {
		g_print("Not able to get connection to system bus\n");
		return 1;
	}

	loop = g_main_loop_new(NULL, FALSE);
	g_dbus_connection_call(con,
				"org.ofono",
				"/",
				"org.freedesktop.DBus.ObjectManager",
				"GetManagedObjects",
				NULL,
				G_VARIANT_TYPE("(a{oa{sa{sv}}})"),
				G_DBUS_CALL_FLAGS_NONE,
				-1,
				NULL,
				(GAsyncReadyCallback)bluez_list_devices,
				loop);

	g_main_loop_run(loop);
	g_object_unref(con);
	return 0;
}
