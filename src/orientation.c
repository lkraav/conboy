
#ifdef HILDON_HAS_APP_MENU

#include <hildon/hildon.h>
#include <mce/dbus-names.h>
#include <mce/mode-names.h>

#include "orientation.h"

#define MCE_MATCH_RULE "type='signal',interface='" MCE_SIGNAL_IF "',member='" MCE_DEVICE_ORIENTATION_SIG "'"

static void
set_orientation(const gchar* orientation)
{
	g_printerr("INFO: Setting to orientation: %s\n", orientation);

	HildonPortraitFlags flags;
	AppData *app_data = app_data_get();

	if (strcmp(orientation, "landscape") == 0) {
		flags = HILDON_PORTRAIT_MODE_SUPPORT;
		app_data->portrait = FALSE;
	} else if (strcmp(orientation, "portrait") == 0) {
		flags = HILDON_PORTRAIT_MODE_REQUEST;
		app_data->portrait = TRUE;
	} else {
		g_printerr("ERROR: Orientation must be 'landscape' or 'portrait', not '%s'.\n", orientation);
		return;
	}

	/* Switch the orientation of all open note windows */
	hildon_gtk_window_set_portrait_flags(GTK_WINDOW(app_data->note_window->window), flags);

	/* Switch the orientation of the search window */
	if (app_data->search_window != NULL) {
		hildon_gtk_window_set_portrait_flags(GTK_WINDOW(app_data->search_window), flags);
	}

}

static DBusHandlerResult
dbus_handle_mce_message(DBusConnection *con, DBusMessage *msg, gpointer data)
{
	DBusMessageIter iter;
	const gchar *mode = NULL;
	/* TODO: Maybe handle other MCE message here. Like memory too high... */
	if (dbus_message_is_signal(msg, MCE_SIGNAL_IF, MCE_DEVICE_ORIENTATION_SIG)) {
		if (dbus_message_iter_init(msg, &iter)) {
			dbus_message_iter_get_basic(&iter, &mode);
			set_orientation(mode);
		}
	}
	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static gboolean
dbus_is_portrait_mode(osso_context_t* ctx)
{
	osso_rpc_t ret;
	gboolean result = FALSE;

	if (osso_rpc_run_system(ctx, MCE_SERVICE, MCE_REQUEST_PATH,
			MCE_REQUEST_IF,	MCE_DEVICE_ORIENTATION_GET, &ret, DBUS_TYPE_INVALID) == OSSO_OK) {

		g_printerr("INFO: DBus said orientation is: %s\n", ret.value.s);

		if (strcmp(ret.value.s, "portrait") == 0) {
			result = TRUE;
		}

		osso_rpc_free_val(&ret);

	} else {
		g_printerr("ERROR: Call do DBus failed\n");
	}

	return result;
}

void
orientation_init(AppData *app_data)
{
	/* Get the DBus connection */
	osso_context_t *ctx = app_data->osso_ctx;
	DBusConnection *con = osso_get_sys_dbus_connection(ctx);

	/* Get current orientation from DBus and save into app_data */
	app_data->portrait = dbus_is_portrait_mode(ctx);

	/* Add a matchin rule */
	dbus_bus_add_match(con, MCE_MATCH_RULE, NULL);

	/* Add the callback, which should be called, once the device is rotated */
	dbus_connection_add_filter(con, dbus_handle_mce_message, NULL, NULL);
}

#endif /* HILDON_HAS_APP_MENU */
