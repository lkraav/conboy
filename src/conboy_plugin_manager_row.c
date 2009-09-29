


#include "localisation.h"

#ifdef HILDON_HAS_APP_MENU

#include <hildon/hildon-gtk.h>

#include "conboy_check_button.h"
#include "conboy_plugin_manager_row.h"


G_DEFINE_TYPE(ConboyPluginManagerRow, conboy_plugin_manager_row, GTK_TYPE_HBOX);


static void
update_row (ConboyPluginInfo *info, ConboyPluginManagerRow *self)
{
	if (conboy_plugin_info_is_active(info)) {
		conboy_check_button_set_active(self->main_but, TRUE);
		gtk_widget_set_sensitive(self->conf_but, conboy_plugin_info_is_configurable(info));
	} else {
		conboy_check_button_set_active(self->main_but, FALSE);
		gtk_widget_set_sensitive(self->conf_but, FALSE);
	}
}

static void
conboy_plugin_manager_row_set_plugin_info (ConboyPluginManagerRow *self, ConboyPluginInfo *info)
{
	g_return_if_fail(self != NULL);
	g_return_if_fail(info != NULL);

	g_return_if_fail(CONBOY_IS_PLUGIN_MANAGER_ROW(self));
	g_return_if_fail(CONBOY_IS_PLUGIN_INFO(info));

	/*g_object_ref(info);*/
	self->plugin_info = info;

	conboy_check_button_set_text(CONBOY_CHECK_BUTTON(self->main_but), info->name, info->desc);

	update_row(info, self);

	/* connect to signals */
	g_signal_connect(info, "plugin-activated",   G_CALLBACK(update_row), self);
	g_signal_connect(info, "plugin-deactivated", G_CALLBACK(update_row), self);

}

GtkWidget*
conboy_plugin_manager_row_new (ConboyPluginInfo *info)
{
	ConboyPluginManagerRow *row = g_object_new(CONBOY_TYPE_PLUGIN_MANAGER_ROW, NULL);
	conboy_plugin_manager_row_set_plugin_info(row, info);
	return GTK_WIDGET(row);
}

gboolean
conboy_plugin_manager_row_get_active (ConboyPluginManagerRow *self)
{
	g_return_val_if_fail(self != NULL, FALSE);
	g_return_val_if_fail(CONBOY_PLUGIN_MANAGER_ROW(self), FALSE);

	return conboy_check_button_get_active(self->main_but);
}

void
conboy_plugin_manager_row_set_active (ConboyPluginManagerRow *self, gboolean active)
{
	g_return_if_fail(self != NULL);
	g_return_if_fail(CONBOY_PLUGIN_MANAGER_ROW(self));

	if (active) {
		conboy_check_button_set_active(self->main_but, TRUE);
		gtk_widget_set_sensitive(self->conf_but, conboy_plugin_info_is_configurable(self->plugin_info));

	} else {
		conboy_check_button_set_active(self->main_but, FALSE);
		gtk_widget_set_sensitive(self->conf_but, FALSE);
	}
}

/*
enum {
	MAIN_BUTTON_TOGGLED,
	CONF_BUTTON_CLICKED,
	INFO_BUTTON_CLICKED,
	LAST_SIGNAL
};
*/


static void
on_main_button_toggled (ConboyCheckButton *button, ConboyPluginManagerRow *self)
{
	if (conboy_check_button_get_active(button)) {
		conboy_plugin_info_activate_plugin(self->plugin_info);
	} else {
		conboy_plugin_info_deactivate_plugin(self->plugin_info);
	}
}

static GtkWindow*
get_parent_window (GtkWidget *widget)
{
	GtkWidget *parent = widget;
	while (parent != NULL && !GTK_IS_WINDOW(parent)) {
		parent = gtk_widget_get_parent(parent);
	}
	return GTK_WINDOW(parent);
}

static void
on_conf_button_clicked (GtkButton *button, ConboyPluginManagerRow *self)
{
	ConboyPluginInfo *info = self->plugin_info;

	g_return_if_fail(conboy_plugin_info_is_configurable(info));

	/* Find parent window */
	GtkWindow *parent = get_parent_window(GTK_WIDGET(self));

	GtkWidget *dialog = gtk_dialog_new_with_buttons(_("Plugin settings"),
				NULL,
				GTK_DIALOG_MODAL,
				GTK_STOCK_OK,
				GTK_RESPONSE_OK,
				NULL);

	GtkWidget *content_area = GTK_DIALOG(dialog)->vbox;

	if (info->plugin == NULL) {
		g_printerr("ERROR: PLUGIN IS NULL\n");
		return;
	}

	GtkWidget *settings = conboy_plugin_get_settings_widget(info->plugin);
	if (settings == NULL) {
		g_printerr("ERROR: Settings widget it NULL\n");
		return;
	}
	gtk_widget_show(settings);

	/* Add the widget to the dialog */
	gtk_box_pack_start(GTK_BOX(content_area), settings, TRUE, TRUE, 10);

	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

static void
on_info_button_clicked (GtkButton *button, ConboyPluginManagerRow *self)
{
	ConboyPluginInfo *info = self->plugin_info;

	g_return_if_fail (info != NULL);

	GtkAboutDialog *dia = g_object_new (GTK_TYPE_ABOUT_DIALOG,
			#ifdef HILDON_HAS_APP_MENU
			"program-name", conboy_plugin_info_get_name (info),
			#else
			"name", conboy_plugin_info_get_name (info),
			#endif
			"copyright", conboy_plugin_info_get_copyright (info),
			"authors", conboy_plugin_info_get_authors (info),
			"comments", conboy_plugin_info_get_description (info),
			"version", conboy_plugin_info_get_version (info),
			NULL);

	gtk_window_set_destroy_with_parent (GTK_WINDOW(dia), TRUE);
	gtk_window_set_modal(GTK_WINDOW(dia), TRUE);

	gtk_dialog_run(GTK_DIALOG(dia));
	gtk_widget_destroy(GTK_WIDGET(dia));
}


/*static guint signals[LAST_SIGNAL] = {0};*/


static void
conboy_plugin_manager_row_dispose (GObject *obj)
{
	if (obj == NULL || !CONBOY_IS_PLUGIN_MANAGER_ROW(obj)) {
		return;
	}

	ConboyPluginManagerRow *self = CONBOY_PLUGIN_MANAGER_ROW(obj);
	ConboyPluginInfo *info = self->plugin_info;

	/* Remove signal handlers */
	if (info != NULL && CONBOY_IS_PLUGIN_INFO(info)) {
		g_signal_handlers_disconnect_by_func(info, update_row, self);
	}

	/* Chain up */
	G_OBJECT_CLASS(conboy_plugin_manager_row_parent_class)->dispose(obj);
}



static void
conboy_plugin_manager_row_class_init (ConboyPluginManagerRowClass *klass)
{
	G_OBJECT_CLASS(klass)->dispose = conboy_plugin_manager_row_dispose;

	klass->info_pix = gdk_pixbuf_new_from_file("/usr/share/icons/hicolor/48x48/hildon/general_information.png", NULL);
	klass->conf_pix = gdk_pixbuf_new_from_file("/usr/share/icons/hicolor/48x48/hildon/general_settings.png", NULL);

	/* We dont need those signals
	klass->main_button_toggled = NULL;
	klass->conf_button_clicked = NULL;
	klass->info_button_clicked = NULL;

	signals[MAIN_BUTTON_TOGGLED] =
			g_signal_new(
					"main-button-toggled",
					CONBOY_TYPE_PLUGIN_MANAGER_ROW,
					G_SIGNAL_RUN_LAST,
					G_STRUCT_OFFSET(ConboyPluginManagerRowClass, main_button_toggled),
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID,
					G_TYPE_NONE,
					0);

	signals[CONF_BUTTON_CLICKED] =
			g_signal_new(
					"conf-button-clicked",
					CONBOY_TYPE_PLUGIN_MANAGER_ROW,
					G_SIGNAL_RUN_LAST,
					G_STRUCT_OFFSET(ConboyPluginManagerRowClass, conf_button_clicked),
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID,
					G_TYPE_NONE,
					0);

	signals[INFO_BUTTON_CLICKED] =
			g_signal_new(
					"info-button-clicked",
					CONBOY_TYPE_PLUGIN_MANAGER_ROW,
					G_SIGNAL_RUN_LAST,
					G_STRUCT_OFFSET(ConboyPluginManagerRowClass, info_button_clicked),
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID,
					G_TYPE_NONE,
					0);
	*/

}

static void
conboy_plugin_manager_row_init (ConboyPluginManagerRow *self)
{
	ConboyPluginManagerRowClass *klass = CONBOY_PLUGIN_MANAGER_ROW_GET_CLASS(self);

	gtk_box_set_homogeneous(GTK_BOX(self), FALSE);
	gtk_box_set_spacing(GTK_BOX(self), 0);

	/* Main button */
	GtkWidget *main_but = conboy_check_button_new(HILDON_SIZE_FINGER_HEIGHT, CONBOY_CHECK_BUTTON_ARRANGEMENT_VERTICAL);
	gtk_widget_show(main_but);
	gtk_box_pack_start(GTK_BOX(self), main_but, TRUE, TRUE, 0);

	/* Config button */
	GtkWidget *conf_but = hildon_gtk_button_new(HILDON_SIZE_FINGER_HEIGHT);
	gtk_button_set_image(GTK_BUTTON(conf_but), gtk_image_new_from_pixbuf(klass->conf_pix));
	gtk_widget_show(conf_but);
	gtk_box_pack_start(GTK_BOX(self), conf_but, FALSE, FALSE, 0);

	/* Info/About button */
	GtkWidget *info_but = hildon_gtk_button_new(HILDON_SIZE_FINGER_HEIGHT);
	gtk_button_set_image(GTK_BUTTON(info_but), gtk_image_new_from_pixbuf(klass->info_pix));
	gtk_widget_show(info_but);
	gtk_box_pack_start(GTK_BOX(self), info_but, FALSE, FALSE, 0);

	/* Save for later usage */
	self->main_but = main_but;
	self->info_but = info_but;
	self->conf_but = conf_but;
	self->plugin_info = NULL;

	/* Connect signal handlers */
	g_signal_connect(main_but, "toggled", G_CALLBACK(on_main_button_toggled), self);
	g_signal_connect(conf_but, "clicked", G_CALLBACK(on_conf_button_clicked), self);
	g_signal_connect(info_but, "clicked", G_CALLBACK(on_info_button_clicked), self);

}

#endif /* #ifdef HILDON_HAS_APP_MENU */
