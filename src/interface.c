/* This file is part of Conboy.
 *
 * Copyright (C) 2009 Cornelius Hald
 *
 * Conboy is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Conboy is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Conboy. If not, see <http://www.gnu.org/licenses/>.
 */
#include "localisation.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <libintl.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <gtk/gtkimcontext.h>
#include <glib/gprintf.h>
#include <hildon/hildon-window.h>
#include <hildon/hildon-find-toolbar.h>
#include <hildon/hildon-helper.h>
#include <gdk/gdkx.h>
#include <X11/Xatom.h>
#ifdef HILDON_HAS_APP_MENU
#include <hildon/hildon-text-view.h>
#include <hildon/hildon-pannable-area.h>
#endif

#include "conboy_config.h"
#include "callbacks.h"
#include "metadata.h"
#include "app_data.h"
#include "settings.h"
#include "conboy_note_buffer.h"
#include "conboy_web_sync.h"
#include "conboy_oauth.h"
#include "note.h"
#include "json.h"
#include "ui_helper.h"
#include "he-fullscreen-button.h"
#include "interface.h"

static void initialize_tags(GtkTextBuffer *buffer) {
	/*
	 * The order in which we add the tags here defines also the priority of the tags.
	 * So if we apply, for example, bold and italic to one word, it will result
	 * in <italic><bold>word</bold></italic>. Because bold has a higher priority
	 * than italic.
	 */
	gtk_text_buffer_create_tag(buffer, "centered", "justification", GTK_JUSTIFY_CENTER, NULL);

	gtk_text_buffer_create_tag(buffer, "bold", "weight", PANGO_WEIGHT_BOLD, NULL);
	gtk_text_buffer_create_tag(buffer, "italic", "style", PANGO_STYLE_ITALIC, NULL);
	gtk_text_buffer_create_tag(buffer, "strikethrough", "strikethrough", TRUE, NULL);
	gtk_text_buffer_create_tag(buffer, "underline", "underline", PANGO_UNDERLINE_SINGLE, NULL);
	gtk_text_buffer_create_tag(buffer, "monospace", "family", "monospace", NULL);
	gtk_text_buffer_create_tag(buffer, "highlight", "background", "yellow", NULL);

	gtk_text_buffer_create_tag(buffer, "size:small", "scale", PANGO_SCALE_SMALL, NULL);
	gtk_text_buffer_create_tag(buffer, "size:large", "scale", PANGO_SCALE_LARGE, NULL);
	gtk_text_buffer_create_tag(buffer, "size:huge", "scale", PANGO_SCALE_X_LARGE, NULL);

	gtk_text_buffer_create_tag(buffer, "link:internal", "foreground", "blue", "underline", PANGO_UNDERLINE_SINGLE, NULL);
	gtk_text_buffer_create_tag(buffer, "link:url", "foreground", "blue", "underline", PANGO_UNDERLINE_SINGLE, NULL);
	gtk_text_buffer_create_tag(buffer, "link:broken", "foreground", "gray", "underline", PANGO_UNDERLINE_SINGLE, NULL);

	gtk_text_buffer_create_tag(buffer, "_title", "foreground", "blue", "underline", PANGO_UNDERLINE_SINGLE, "scale", PANGO_SCALE_X_LARGE, NULL);

	gtk_text_buffer_create_tag(buffer, "list-item", NULL);
	gtk_text_buffer_create_tag(buffer, "list", NULL);

	//gtk_text_buffer_create_tag(buffer, "list-item", "foreground", "orange", NULL);
	//gtk_text_buffer_create_tag(buffer, "list", "background", "gray", NULL);
}

static void
set_item_label(GtkContainer *item, const gchar *open_tag, const gchar *text, const gchar *end_tag)
{
	GList *children = gtk_container_get_children(item);
	gchar *string;

	if (children == NULL) {
		g_printerr("ERROR: set_menu_item_label() expects a GtkContainer, which contains one label.\n");
		return;
	}

	string = g_strconcat(open_tag, text, end_tag, NULL);

	/* TODO: Add checks, that it is really a label and the widget is really a Button or a MenuItem */
	gtk_label_set_markup(GTK_LABEL(children->data), string);

	g_list_free(children);
	g_free(string);
}

static gboolean
on_fullscreen_toggled (GtkWidget *widget, GdkEventWindowState *event, gpointer data)
{
	UserInterface *ui = (UserInterface*)data;

	if (event->changed_mask & GDK_WINDOW_STATE_FULLSCREEN) {
		/* Show / Hide toolbar */
		if (event->new_window_state == GDK_WINDOW_STATE_FULLSCREEN) {
			gtk_widget_hide(GTK_WIDGET(ui->toolbar));
		} else {
			gtk_widget_show(GTK_WIDGET(ui->toolbar));
		}
	}

	return FALSE;
}

static void
on_orientation_changed(GdkScreen *screen, gpointer data)
{
	AppData *app_data = app_data_get();
	UserInterface *ui = app_data->note_window;

	app_data->portrait = is_portrait_mode();

	if (app_data->portrait) {
		/* Toolbar */
		gtk_action_set_visible(ui->action_dec_indent, FALSE);
		gtk_action_set_visible(ui->action_inc_indent, FALSE);
		gtk_action_set_visible(ui->action_delete, FALSE);
		gtk_action_set_visible(ui->action_link, FALSE);
		gtk_action_set_visible(ui->action_text_style, FALSE);
		gtk_action_set_visible(ui->action_find, FALSE);

		/* Menu */
		#ifdef HILDON_HAS_APP_MENU
		g_object_ref(ui->app_menu); /* Ref the menu, otherwise it gets destroyed after the next call */
		hildon_window_set_app_menu(ui->window, NULL);
		#endif

		/* Other */
		gtk_text_view_set_editable(ui->view, FALSE);
		gtk_text_view_set_cursor_visible(ui->view, FALSE);

	} else {
		/* Toolbar */
		gtk_action_set_visible(ui->action_dec_indent, TRUE);
		gtk_action_set_visible(ui->action_inc_indent, TRUE);
		gtk_action_set_visible(ui->action_delete, TRUE);
		gtk_action_set_visible(ui->action_link, TRUE);
		gtk_action_set_visible(ui->action_text_style, TRUE);
		gtk_action_set_visible(ui->action_find, TRUE);

		/* Menu */
		#ifdef HILDON_HAS_APP_MENU
		hildon_window_set_app_menu(ui->window, HILDON_APP_MENU(ui->app_menu));
		g_object_unref(ui->app_menu); /* Drop the ref and let HildonWindow handle it again */
		#endif

		/* Other */
		gtk_text_view_set_editable(ui->view, TRUE);
		gtk_text_view_set_cursor_visible(ui->view, TRUE);
	}
}

static void
on_scrollbar_settings_changed(GConfClient *client, guint cnxn_id, GConfEntry *entry, gpointer user_data)
{
	GtkScrolledWindow *win = GTK_SCROLLED_WINDOW(user_data);

	SettingsScrollbarSize size = gconf_value_get_int(entry->value);
	if (size == SETTINGS_SCROLLBAR_SIZE_BIG) {
		hildon_helper_set_thumb_scrollbar(win, TRUE);
	} else {
		hildon_helper_set_thumb_scrollbar(win, FALSE);
	}
}

static void
on_background_color_changed(GConfClient *client, guint cnxn_id, GConfEntry *entry, gpointer user_data)
{
	GtkWidget *view = GTK_WIDGET(user_data);
	GdkColor color;

	const gchar *hex_color = gconf_value_get_string(entry->value);
	gdk_color_parse(hex_color, &color);

	gtk_widget_modify_base(GTK_WIDGET(view), GTK_STATE_NORMAL, &color);

	/* Small stripe on the right side only */
	/*gtk_widget_modify_bg(GTK_WIDGET(window), GTK_STATE_NORMAL, &color);*/
}

static void
on_text_color_changed(GConfClient *client, guint cnxn_id, GConfEntry *entry, gpointer user_data)
{
	GtkWidget *view = GTK_WIDGET(user_data);
	GdkColor color;

	const gchar *hex_color = gconf_value_get_string(entry->value);
	gdk_color_parse(hex_color, &color);

	g_printerr("Text color changed \n");

	gtk_widget_modify_text(GTK_WIDGET(view), GTK_STATE_NORMAL, &color);
}

static void
on_link_color_changed(GConfClient *client, guint cnxn_id, GConfEntry *entry, gpointer user_data)
{
	if (user_data == NULL || !GTK_IS_TEXT_VIEW(user_data)) {
		g_printerr("ERROR: on_link_color_changed(): user_data is not a GtkTextView.");
		return;
	}
	GtkTextView *view = GTK_TEXT_VIEW(user_data);
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(view);
	GtkTextTag *title = gtk_text_tag_table_lookup(buffer->tag_table, "_title");
	GtkTextTag *link = gtk_text_tag_table_lookup(buffer->tag_table, "link:internal");
	const gchar *hex_color = gconf_value_get_string(entry->value);

	g_object_set(title, "foreground", hex_color, NULL);
	g_object_set(link, "foreground", hex_color, NULL);
}

static void
on_use_custom_colors_changed(GConfClient *client, guint cnxn_id, GConfEntry *entry, gpointer user_data)
{
	GtkTextView *view = GTK_TEXT_VIEW(user_data);

	if (gconf_value_get_bool(entry->value)) {
		/* We just emmit the changed signal on gconf, so the UI will update itself */
		AppData *app_data = app_data_get();
		gconf_client_notify(app_data->client, SETTINGS_BACKGROUND_COLOR);
		gconf_client_notify(app_data->client, SETTINGS_TEXT_COLOR);
		gconf_client_notify(app_data->client, SETTINGS_LINK_COLOR);

	} else {
		/* Reset to default colors */
		GtkTextBuffer *buffer = gtk_text_view_get_buffer(view);
		GtkTextTag *title = gtk_text_tag_table_lookup(buffer->tag_table, "_title");
		GtkTextTag *link = gtk_text_tag_table_lookup(buffer->tag_table, "link:internal");
		g_object_set(title, "foreground", "blue", NULL);
		g_object_set(link, "foreground", "blue", NULL);

		gtk_widget_modify_base(GTK_WIDGET(view), GTK_STATE_NORMAL, NULL);
		gtk_widget_modify_text(GTK_WIDGET(view), GTK_STATE_NORMAL, NULL);
	}
}

static void
on_font_size_changed(GConfClient *client, guint cnxn_id, GConfEntry *entry, gpointer user_data)
{
	GtkWidget *textview = GTK_WIDGET(user_data);
	PangoFontDescription *font = pango_font_description_new();
	pango_font_description_set_size(font, gconf_value_get_int(entry->value));
	gtk_widget_modify_font(GTK_WIDGET(textview), font);
	pango_font_description_free(font);
}

static WebSyncDialogData*
create_sync_dialog(GtkWindow *parent)
{
	GtkWidget *dia = gtk_dialog_new();
	gtk_window_set_modal(GTK_WINDOW(dia), TRUE);
	gtk_window_set_transient_for(GTK_WINDOW(dia), parent);
	gtk_window_set_default_size(GTK_WINDOW(dia), 400, 250);
	gtk_window_set_title(GTK_WINDOW(dia), " ");

	GtkWidget *button = gtk_dialog_add_button(GTK_DIALOG(dia), GTK_STOCK_OK, GTK_RESPONSE_OK);
	gtk_widget_set_sensitive(button, FALSE);

	GtkWidget *txt = gtk_label_new("");
	gtk_label_set_markup(GTK_LABEL(txt), "<b>Synchronization ongoing</b>");
	gtk_label_set_line_wrap(GTK_LABEL(txt), TRUE);
	gtk_widget_show(txt);

	GtkWidget *bar = gtk_progress_bar_new();
	gtk_widget_show(bar);

	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dia)->vbox), bar, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dia)->vbox), txt, TRUE, TRUE, 0);

	g_signal_connect(dia, "response", G_CALLBACK(gtk_widget_destroy), NULL);

	/* TODO: This data must get freed once the dialog is closed */
	WebSyncDialogData *dialog_data = g_new0(WebSyncDialogData, 1);
	dialog_data->dialog = GTK_DIALOG(dia);
	dialog_data->bar = GTK_PROGRESS_BAR(bar);
	dialog_data->label = GTK_LABEL(txt);
	dialog_data->button = GTK_BUTTON(button);

	return dialog_data;
}


static void
on_sync_but_clicked(GtkButton *but, gpointer user_data)
{
	AppData *app_data = app_data_get();

	/* Show dialog */
	GtkWindow *parent = GTK_WINDOW(app_data->note_window->window);
	WebSyncDialogData *dialog_data = create_sync_dialog(parent);
	ui_helper_remove_portrait_support(GTK_WINDOW(dialog_data->dialog));
	gtk_widget_show(GTK_WIDGET(dialog_data->dialog));

	/* Create thread and start sync */
	GThread *thread = g_thread_create(web_sync_do_sync, dialog_data, TRUE, NULL);
	if (!thread) {
		g_printerr("ERROR: Cannot create sync thread\n");
		return;
	}
}

/*
 * This is a hack: We load the first note here, because it has
 * to be run inside the main loop after opening the window.
 * The reason is, that in note_show() we scroll to the last
 * saved cursor position. This scrolling only works releable
 * after some size calculation on the widget have been done.
 *
 * When running directly from main (outside of the mainloop) those
 * calculations are not done yet, so the scrolling to the cursor
 * possition does not work correctly.
 *
 * To make it run only once, and to every time the application
 * is minimized/maximized, we use a boolean flag on app_data.
 */
static gboolean
on_window_visible(GtkWindow *window, GdkEvent *event, gpointer user_data)
{
	AppData *app_data = app_data_get();

	if (app_data->started) {
		return FALSE;
	}

	UserInterface *ui = app_data->note_window;

	/* Open latest note or new one */
	gchar *guid = settings_load_last_open_note();
	ConboyNote *note = NULL;

	app_data->started = TRUE;

	/* Try to open last viewed note */
	if (guid != NULL) {
		note = conboy_note_store_find_by_guid(app_data->note_store, guid);
		g_free(guid);

		if (note) {

			note_show(note, TRUE, FALSE, FALSE);

			/* Now process all pending events like calculating window size,
			 * text amount, scrollbar positions etc. If we don't do this,
			 * we cannot set the scrollbars to the right position as the size
			 * of the widget is still unknown */
			while (gtk_events_pending()) {
				gtk_main_iteration_do(FALSE);
			}

			/* Scroll to saved position */
			GtkAdjustment *adj;
			#ifdef HILDON_HAS_APP_MENU
			adj = hildon_pannable_area_get_vadjustment(HILDON_PANNABLE_AREA(ui->scrolled_window));
			#else
			adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(ui->scrolled_window));
			#endif
			gtk_adjustment_set_value(adj, settings_load_last_scroll_position());

			return FALSE;
		}
	}

	/* If that does not work, use last edited note */
	if (note == NULL) {
		note = conboy_note_store_get_latest(app_data->note_store);
	}

	/* If that does not work, create new note */
	if (note == NULL) {
		gchar title[50];
		g_sprintf(title, _("New Note %i"), 1);
		note = conboy_note_new_with_title(title);
		note_show(note, TRUE, TRUE, TRUE);
	} else {
		note_show(note, TRUE, TRUE, FALSE);
	}



	return FALSE;
}

static void
on_storage_activated (ConboyStorage *storage, UserInterface *ui)
{
	g_printerr("INFO: Storage activated\n");

	AppData *app_data = app_data_get();
	ConboyNote *note = conboy_note_store_get_latest(app_data->note_store);
	gtk_text_buffer_set_modified(ui->buffer, FALSE); /* Prevent note_show() from saving */
	if (note != NULL) {
		note_show(note, TRUE, TRUE, FALSE);
	} else {
		/* TODO: Create new note */
		gtk_text_buffer_set_text(ui->buffer, "", -1);
	}

	gtk_text_view_set_editable(GTK_TEXT_VIEW(ui->view), TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(ui->toolbar), TRUE);
}

static void
on_storage_deactivated (ConboyStorage *storage, UserInterface *ui)
{
	g_printerr("INFO: Storage deactivated\n");

	if (gtk_text_buffer_get_modified(ui->buffer)) {
		note_save(ui);
	}

	/* Clear history */
	AppData *app_data = app_data_get();
	/* Only free note_history because current_element is part of it */
	g_list_free(app_data->note_history);
	app_data->note_history = NULL;
	app_data->current_element = NULL;

	/* Block automatic saving, set text, unblock saving */
	g_signal_handlers_block_matched(ui->buffer, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, ui);
	gtk_text_buffer_set_text(ui->buffer, "No storage backend plugin loaded. Please configure one in 'Settings'.", -1);
	g_signal_handlers_unblock_matched(ui->buffer, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, ui);

	conboy_note_buffer_clear_active_tags(CONBOY_NOTE_BUFFER(ui->buffer));

	gtk_text_view_set_editable(GTK_TEXT_VIEW(ui->view), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(ui->toolbar), FALSE);
}

enum Icon {
	ICON_DEC_INDENT,
	ICON_INC_INDENT,
	ICON_LINK,
	ICON_TEXT_STYLE,
	ICON_DELETE,
	ICON_SEARCH,
	ICON_OPEN,
	ICON_BACK,
	ICON_FORWARD
};

#ifdef HILDON_HAS_APP_MENU
static GtkWidget*
add_icon(const gchar *filename, GtkToolButton *button)
{
	/* If we don't find the icons in these two paths, we return the stock button */
	gchar *full_path = g_build_filename(PREFIX, "/share/icons/hicolor/48x48/hildon", filename, NULL);
	if (!g_file_test(full_path, G_FILE_TEST_EXISTS)) {
		g_printerr("not found: %s\n", full_path);
		g_free(full_path);
		full_path = g_build_filename("/home/conny/workspace/conboy/data/icons/48x48", filename, NULL);
		if (!g_file_test(full_path, G_FILE_TEST_EXISTS)) {
			g_free(full_path);
			return GTK_WIDGET(button);
		}
	}

	/* If we found the icon, we set it and return the button with icon */
	GtkWidget *icon = gtk_image_new_from_file(full_path);
	gtk_widget_show(icon);
	gtk_tool_button_set_icon_widget(button, icon);
	g_free(full_path);
	return GTK_WIDGET(button);
}
#endif

static GtkWidget*
create_tool_button(GtkAction *action, enum Icon icon)
{
	GtkToolButton *button = GTK_TOOL_BUTTON(gtk_action_create_tool_item(action));

#ifdef HILDON_HAS_APP_MENU
	switch (icon)
	{
		case ICON_INC_INDENT:
			return add_icon("conboy-increase_indent.png", button);

		case ICON_DEC_INDENT:
			return add_icon("conboy-decrease_indent.png", button);

		case ICON_LINK:
			return add_icon("conboy-hyperlink.png", button);

		case ICON_TEXT_STYLE:
			return add_icon("conboy-text_transform.png", button);

		case ICON_SEARCH:
			return add_icon("general_search.png", button);

		case ICON_OPEN:
			return add_icon("general_toolbar_folder.png", button);

		case ICON_DELETE:
			return add_icon("general_delete.png", button);

		case ICON_BACK:
			return add_icon("general_back.png", button);

		case ICON_FORWARD:
			return add_icon("general_forward.png", button);

		default:
			return GTK_WIDGET(button);
	}
#else
	return GTK_WIDGET(button);
#endif
}

static void
ungrab_volume_keys(GtkWidget *window)
{
#ifdef HILDON_HAS_APP_MENU
    /* Tell maemo-status-volume daemon to ungrab keys */
    unsigned long val = 1; /* ungrab, use 0 to grab */
    Atom atom;
    GdkDisplay *display = NULL;
    display = gdk_drawable_get_display (GDK_DRAWABLE(window->window));
    atom = gdk_x11_get_xatom_by_name_for_display (display, "_HILDON_ZOOM_KEY_ATOM");
    XChangeProperty (GDK_DISPLAY_XDISPLAY (display),
                     GDK_WINDOW_XID (GDK_DRAWABLE(window->window)), atom, XA_INTEGER, 32,
                     PropModeReplace, (unsigned char *) &val, 1);
#endif
}


/*
 * I'm not sure what is better:
 * 1) Declaring all as GtkWidget, so that Fremantle and Diablo code can share these.
 * 2) Declare all as the actual types, so inside the code it's easy to know which type a widget has.
 */
UserInterface* create_mainwin() {

	UserInterface *ui = g_new0(UserInterface, 1);

	GtkWidget *mainwin;
	GtkWidget *vbox1;
	GtkWidget *main_menu;
	GtkWidget *text_style_menu;
	GdkScreen *screen;

	GtkWidget *menu_new;
	GtkWidget *menu_bold;
	GtkWidget *menu_italic;
	GtkWidget *menu_strike;
	GtkWidget *menu_highlight;
	GtkWidget *menu_fixed;
	GtkWidget *menu_bullets;
	GtkWidget *menu_settings;
	GtkWidget *menu_quit;
	GtkWidget *menu_text_style;
	GtkWidget *menu_inc_indent;
	GtkWidget *menu_dec_indent;
	GtkWidget *menu_font_small;
	GtkWidget *menu_font_normal;
	GtkWidget *menu_font_large;
	GtkWidget *menu_font_huge;
	GtkWidget *menu_open;
	GtkWidget *menu_sync;
	GtkWidget *menu_delete;
	GtkWidget *menu_fullscreen;
	GtkWidget *menu_about;

	GtkWidget *toolbar;
	GtkWidget *find_bar;
	GtkWidget *scrolledwindow1;
	GtkWidget *textview;
	GtkTextBuffer *buffer;

	GtkWidget *button_link;
	GtkWidget *button_delete;
	GtkWidget *button_notes;
	GtkWidget *button_inc_indent;
	GtkWidget *button_dec_indent;
	GtkWidget *button_style;
	GtkWidget *button_find;
	GtkWidget *button_back;
	GtkWidget *button_forward;

	GtkAction *action_new;
	GtkAction *action_delete;
	GtkAction *action_settings;
	GtkAction *action_quit;
	GtkAction *action_notes;
	GtkAction *action_bold;
	GtkAction *action_fixed;
	GtkAction *action_highlight;
	GtkAction *action_italic;
	GtkAction *action_strike;
	GtkAction *action_bullets;
	GtkAction *action_link;
	GtkAction *action_inc_indent;
	GtkAction *action_dec_indent;
	GtkAction *action_text_style;
	GtkAction *action_zoom_in;
	GtkAction *action_zoom_out;
	GtkAction *action_font_small;
	GtkAction *action_font_normal;
	GtkAction *action_font_large;
	GtkAction *action_font_huge;
	GtkAction *action_find;
	GtkAction *action_sync;
	GtkAction *action_back;
	GtkAction *action_forward;
	GtkAction *action_fullscreen;
	GtkAction *action_about;

	GtkActionGroup *action_group;

	GtkTextTag *link_internal_tag;
	GtkTextTag *link_url_tag;

	GSList *radio_group = NULL;

	GtkAccelGroup *accel_group;

	PangoFontDescription *font;
	AppData *app_data = app_data_get();

#ifdef HILDON_HAS_APP_MENU
	mainwin = hildon_stackable_window_new();
#else
	mainwin = hildon_window_new();
#endif
	gtk_window_set_title(GTK_WINDOW(mainwin), "Conboy");
	ui->window = HILDON_WINDOW(mainwin);

	screen = gdk_screen_get_default();

	accel_group = gtk_accel_group_new();
	gtk_window_add_accel_group(GTK_WINDOW(mainwin), accel_group);
	g_object_unref(accel_group);

	vbox1 = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox1);
	gtk_container_add(GTK_CONTAINER(mainwin), vbox1);

	/* ACTIONS */
	action_bold = GTK_ACTION(gtk_toggle_action_new("bold", _("Bold"), NULL, NULL));
	action_bullets = GTK_ACTION(gtk_toggle_action_new("bullets", _("Bullets"), NULL, NULL));
	action_dec_indent = GTK_ACTION(gtk_action_new("dec_indent", _("Decrease indent"), NULL, GTK_STOCK_UNINDENT));
	action_delete = GTK_ACTION(gtk_action_new("delete", _("Delete note"), NULL, GTK_STOCK_DELETE));
	action_fixed = GTK_ACTION(gtk_toggle_action_new("monospace", _("Fixed width"), NULL, NULL));
	action_highlight = GTK_ACTION(gtk_toggle_action_new("highlight", _("Highlight"), NULL, NULL));
	action_inc_indent = GTK_ACTION(gtk_action_new("inc_indent", _("Increase indent"), NULL, GTK_STOCK_INDENT));
	action_link = GTK_ACTION(gtk_action_new("link", _("Link"), NULL, GTK_STOCK_REDO));
	action_new = GTK_ACTION(gtk_action_new("new", _("New note"), NULL, NULL));
	action_notes = GTK_ACTION(gtk_action_new("open", _("Open note"), NULL, GTK_STOCK_OPEN));
	action_settings = GTK_ACTION(gtk_action_new("settings", _("Settings"), NULL, NULL));
	action_quit = GTK_ACTION(gtk_action_new("quit", _("Quit"), NULL, NULL));
	action_italic = GTK_ACTION(gtk_toggle_action_new("italic", _("Italic"), NULL, GTK_STOCK_ITALIC));
	action_strike = GTK_ACTION(gtk_toggle_action_new("strikethrough", _("Strikeout"), NULL, NULL));
	action_text_style = GTK_ACTION(gtk_action_new("style", _("Style"), NULL, GTK_STOCK_SELECT_FONT));
	action_zoom_in = GTK_ACTION(gtk_action_new("zoom_in", _("Zoom in"), NULL, GTK_STOCK_ZOOM_IN));
	action_zoom_out = GTK_ACTION(gtk_action_new("zoom_out", _("Zoom out"), NULL, GTK_STOCK_ZOOM_OUT));
	action_find = GTK_ACTION(gtk_action_new("find", _("Find in note"), NULL, GTK_STOCK_FIND));
	gchar *sync_label = g_strconcat(_("Synchronize"), " (Alpha)", NULL);
	action_sync = GTK_ACTION(gtk_action_new("sync", sync_label, NULL, NULL));
	action_back = GTK_ACTION(gtk_action_new("back", _("Back"), NULL, GTK_STOCK_GO_BACK));
	action_forward = GTK_ACTION(gtk_action_new("forward", _("Forward"), NULL, GTK_STOCK_GO_FORWARD));
	action_fullscreen = GTK_ACTION(gtk_action_new("fullscreen", _("Fullscreen"), NULL, NULL));
	action_about = GTK_ACTION(gtk_action_new("about", _("About"), NULL, NULL));
	/* TODO: Use an enum instead of 0 to 3 */
	action_font_small = GTK_ACTION(gtk_radio_action_new("size:small", _("Small"), NULL, NULL, 0));
	action_font_normal = GTK_ACTION(gtk_radio_action_new("size:normal", _("Normal"), NULL, NULL, 1));
	action_font_large = GTK_ACTION(gtk_radio_action_new("size:large", _("Large"), NULL, NULL, 2));
	action_font_huge = GTK_ACTION(gtk_radio_action_new("size:huge", _("Huge"), NULL, NULL, 3));

	gtk_action_set_sensitive(action_link, FALSE);
	/*gtk_action_set_sensitive(action_inc_indent, FALSE);*/
	gtk_action_set_sensitive(action_dec_indent, FALSE);
	gtk_action_set_sensitive(action_back, FALSE);
	gtk_action_set_sensitive(action_forward, FALSE);

	/* Build the radio action group */
	gtk_radio_action_set_group(GTK_RADIO_ACTION(action_font_small), radio_group);
	radio_group = gtk_radio_action_get_group(GTK_RADIO_ACTION(action_font_small));
	gtk_radio_action_set_group(GTK_RADIO_ACTION(action_font_normal), radio_group);
	radio_group = gtk_radio_action_get_group(GTK_RADIO_ACTION(action_font_normal));
	gtk_radio_action_set_group(GTK_RADIO_ACTION(action_font_large), radio_group);
	radio_group = gtk_radio_action_get_group(GTK_RADIO_ACTION(action_font_large));
	gtk_radio_action_set_group(GTK_RADIO_ACTION(action_font_huge), radio_group);
	radio_group = gtk_radio_action_get_group(GTK_RADIO_ACTION(action_font_huge));
	gtk_radio_action_set_current_value(GTK_RADIO_ACTION(action_font_normal), 1);

	action_group = gtk_action_group_new("notewin");
	/* TODO: This 3 blocks should be one function call. Write a function which executs all three commands */
	gtk_action_group_add_action_with_accel(action_group, action_bold,      "<Ctrl>b");
	gtk_action_group_add_action_with_accel(action_group, action_italic,    "<Ctrl>i");
	gtk_action_group_add_action_with_accel(action_group, action_fixed,     "<Ctrl>m");
	gtk_action_group_add_action_with_accel(action_group, action_highlight, "<Ctrl>h");
	gtk_action_group_add_action_with_accel(action_group, action_strike,    "<Ctrl>s");
	gtk_action_group_add_action_with_accel(action_group, action_quit,      "<Ctrl>q");
	gtk_action_group_add_action_with_accel(action_group, action_new,       "<Ctrl>n");
	gtk_action_group_add_action_with_accel(action_group, action_find,      "<Ctrl>f");
	gtk_action_group_add_action_with_accel(action_group, action_fullscreen,"<Ctrl>u");

	gtk_action_set_accel_group(action_bold,      accel_group);
	gtk_action_set_accel_group(action_italic,    accel_group);
	gtk_action_set_accel_group(action_fixed,     accel_group);
	gtk_action_set_accel_group(action_highlight, accel_group);
	gtk_action_set_accel_group(action_strike,    accel_group);
	gtk_action_set_accel_group(action_quit,      accel_group);
	gtk_action_set_accel_group(action_new,       accel_group);
	gtk_action_set_accel_group(action_find,      accel_group);
	gtk_action_set_accel_group(action_new,       accel_group);
	gtk_action_set_accel_group(action_quit,      accel_group);
	gtk_action_set_accel_group(action_fullscreen,accel_group);

	gtk_action_connect_accelerator(action_bold);
	gtk_action_connect_accelerator(action_italic);
	gtk_action_connect_accelerator(action_fixed);
	gtk_action_connect_accelerator(action_highlight);
	gtk_action_connect_accelerator(action_strike);
	gtk_action_connect_accelerator(action_find);
	gtk_action_connect_accelerator(action_new);
	gtk_action_connect_accelerator(action_quit);
	gtk_action_connect_accelerator(action_fullscreen);


	/* FORMAT MENU */
#ifdef HILDON_HAS_APP_MENU
	text_style_menu = hildon_app_menu_new();

	menu_bold = gtk_toggle_button_new();
	menu_italic = gtk_toggle_button_new();
	menu_strike = gtk_toggle_button_new();
	menu_highlight = gtk_toggle_button_new();
	menu_fixed = gtk_toggle_button_new();
	menu_bullets = gtk_toggle_button_new();
	menu_font_small = gtk_toggle_button_new();
	menu_font_normal = gtk_toggle_button_new();
	menu_font_large = gtk_toggle_button_new();
	menu_font_huge = gtk_toggle_button_new();

	gtk_action_connect_proxy(action_bold, menu_bold);
	gtk_action_connect_proxy(action_italic, menu_italic);
	gtk_action_connect_proxy(action_strike, menu_strike);
	gtk_action_connect_proxy(action_highlight, menu_highlight);
	gtk_action_connect_proxy(action_fixed, menu_fixed);
	gtk_action_connect_proxy(action_bullets, menu_bullets);
	gtk_action_connect_proxy(action_font_small, menu_font_small);
	gtk_action_connect_proxy(action_font_normal, menu_font_normal);
	gtk_action_connect_proxy(action_font_large, menu_font_large);
	gtk_action_connect_proxy(action_font_huge, menu_font_huge);

	set_item_label(GTK_CONTAINER(menu_bold),       "<b>", _("Bold"), "</b>");
	set_item_label(GTK_CONTAINER(menu_italic),     "<i>", _("Italic"), "</i>");
	set_item_label(GTK_CONTAINER(menu_strike),     "<s>", _("Strikeout"), "</s>");
	set_item_label(GTK_CONTAINER(menu_highlight),  "<span background=\"yellow\" foreground=\"black\">", _("Highlight"), "</span>");
	set_item_label(GTK_CONTAINER(menu_fixed),      "<tt>", _("Fixed width"), "</tt>");
	set_item_label(GTK_CONTAINER(menu_font_small), "<span size=\"small\">", _("Small"), "</span>");
	set_item_label(GTK_CONTAINER(menu_font_large), "<span size=\"large\">", _("Large"), "</span>");
	set_item_label(GTK_CONTAINER(menu_font_huge),  "<span size=\"x-large\">", _("Huge"), "</span>");

	hildon_app_menu_append(HILDON_APP_MENU(text_style_menu), GTK_BUTTON(menu_bold));
	hildon_app_menu_append(HILDON_APP_MENU(text_style_menu), GTK_BUTTON(menu_italic));
	hildon_app_menu_append(HILDON_APP_MENU(text_style_menu), GTK_BUTTON(menu_strike));
	hildon_app_menu_append(HILDON_APP_MENU(text_style_menu), GTK_BUTTON(menu_highlight));
	hildon_app_menu_append(HILDON_APP_MENU(text_style_menu), GTK_BUTTON(menu_fixed));
	hildon_app_menu_append(HILDON_APP_MENU(text_style_menu), GTK_BUTTON(menu_bullets));

	hildon_app_menu_add_filter(HILDON_APP_MENU(text_style_menu), GTK_BUTTON(menu_font_small));
	hildon_app_menu_add_filter(HILDON_APP_MENU(text_style_menu), GTK_BUTTON(menu_font_normal));
	hildon_app_menu_add_filter(HILDON_APP_MENU(text_style_menu), GTK_BUTTON(menu_font_large));
	hildon_app_menu_add_filter(HILDON_APP_MENU(text_style_menu), GTK_BUTTON(menu_font_huge));


#else
	text_style_menu = GTK_WIDGET(gtk_menu_new());

	menu_bold = gtk_action_create_menu_item(action_bold);
	menu_italic = gtk_action_create_menu_item(action_italic);
	menu_strike = gtk_action_create_menu_item(action_strike);
	menu_highlight = gtk_action_create_menu_item(action_highlight);
	menu_fixed = gtk_action_create_menu_item(action_fixed);
	menu_bullets = gtk_action_create_menu_item(action_bullets);
	menu_inc_indent = gtk_action_create_menu_item(action_inc_indent);
	menu_dec_indent = gtk_action_create_menu_item(action_dec_indent);
	menu_font_small = gtk_action_create_menu_item(action_font_small);
	menu_font_normal = gtk_action_create_menu_item(action_font_normal);
	menu_font_large = gtk_action_create_menu_item(action_font_large);
	menu_font_huge = gtk_action_create_menu_item(action_font_huge);
	/* TODO: Write a function, which takes GtkActions that contain markup code and that creates the correct widgets */
	set_item_label(GTK_CONTAINER(menu_bold),       "<b>", _("Bold"), "</b>");
	set_item_label(GTK_CONTAINER(menu_italic),     "<i>", _("Italic"), "</i>");
	set_item_label(GTK_CONTAINER(menu_strike),     "<s>", _("Strikeout"), "</s>");
	set_item_label(GTK_CONTAINER(menu_highlight),  "<span background=\"yellow\">", _("Highlight"), "</span>");
	set_item_label(GTK_CONTAINER(menu_fixed),      "<tt>", _("Fixed width"), "</tt>");
	set_item_label(GTK_CONTAINER(menu_font_small), "<span size=\"small\">", _("Small"), "</span>");
	set_item_label(GTK_CONTAINER(menu_font_large), "<span size=\"large\">", _("Large"), "</span>");
	set_item_label(GTK_CONTAINER(menu_font_huge),  "<span size=\"x-large\">", _("Huge"), "</span>");

	gtk_menu_shell_append(GTK_MENU_SHELL(text_style_menu), menu_bold);
	gtk_menu_shell_append(GTK_MENU_SHELL(text_style_menu), menu_italic);
	gtk_menu_shell_append(GTK_MENU_SHELL(text_style_menu), menu_strike);
	gtk_menu_shell_append(GTK_MENU_SHELL(text_style_menu), menu_highlight);
	gtk_menu_shell_append(GTK_MENU_SHELL(text_style_menu), menu_fixed);
	gtk_menu_shell_append(GTK_MENU_SHELL(text_style_menu), gtk_separator_menu_item_new());
	gtk_menu_shell_append(GTK_MENU_SHELL(text_style_menu), menu_font_small);
	gtk_menu_shell_append(GTK_MENU_SHELL(text_style_menu), menu_font_normal);
	gtk_menu_shell_append(GTK_MENU_SHELL(text_style_menu), menu_font_large);
	gtk_menu_shell_append(GTK_MENU_SHELL(text_style_menu), menu_font_huge);
	gtk_menu_shell_append(GTK_MENU_SHELL(text_style_menu), gtk_separator_menu_item_new());
	gtk_menu_shell_append(GTK_MENU_SHELL(text_style_menu), menu_bullets);
	gtk_menu_shell_append(GTK_MENU_SHELL(text_style_menu), menu_inc_indent);
	gtk_menu_shell_append(GTK_MENU_SHELL(text_style_menu), menu_dec_indent);
#endif



	/* MAIN MENU */
#ifdef HILDON_HAS_APP_MENU
	main_menu = hildon_app_menu_new();

	menu_new = gtk_button_new();
	menu_open = gtk_button_new();
	menu_settings = gtk_button_new();
	menu_sync = gtk_button_new();
	menu_quit = gtk_button_new();
	menu_delete = gtk_button_new();
	menu_fullscreen = gtk_button_new();
	menu_about = gtk_button_new();

	gtk_action_connect_proxy(action_new, menu_new);
	gtk_action_connect_proxy(action_notes, menu_open);
	gtk_action_connect_proxy(action_settings, menu_settings);
	gtk_action_connect_proxy(action_sync, menu_sync);
	gtk_action_connect_proxy(action_quit, menu_quit);
	gtk_action_connect_proxy(action_delete, menu_delete);
	gtk_action_connect_proxy(action_fullscreen, menu_fullscreen);
	gtk_action_connect_proxy(action_about, menu_about);

	hildon_app_menu_append(HILDON_APP_MENU(main_menu), GTK_BUTTON(menu_new));
	hildon_app_menu_append(HILDON_APP_MENU(main_menu), GTK_BUTTON(menu_delete));
	hildon_app_menu_append(HILDON_APP_MENU(main_menu), GTK_BUTTON(menu_sync));
	hildon_app_menu_append(HILDON_APP_MENU(main_menu), GTK_BUTTON(menu_settings));
	hildon_app_menu_append(HILDON_APP_MENU(main_menu), GTK_BUTTON(menu_fullscreen));
	hildon_app_menu_append(HILDON_APP_MENU(main_menu), GTK_BUTTON(menu_about));

	if (app_data->portrait) {
		gtk_widget_hide(menu_settings);
	} else {
		gtk_widget_hide(menu_open);
	}

	hildon_window_set_app_menu(HILDON_WINDOW(mainwin), HILDON_APP_MENU(main_menu));

#else
	main_menu = gtk_menu_new();

	menu_new = gtk_action_create_menu_item(action_new);
	menu_delete = gtk_action_create_menu_item(action_delete);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_delete), NULL);
	/*menu_text_style = gtk_menu_item_new_with_label(_("Text Style"));
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_text_style), text_style_menu);*/
	menu_settings = gtk_action_create_menu_item(action_settings);
	menu_sync = gtk_action_create_menu_item(action_sync);
	menu_about = gtk_action_create_menu_item(action_about);
	menu_quit = gtk_action_create_menu_item(action_quit);

	gtk_menu_shell_append(GTK_MENU_SHELL(main_menu), menu_new);
	gtk_menu_shell_append(GTK_MENU_SHELL(main_menu), menu_delete);
	/*gtk_menu_shell_append(GTK_MENU_SHELL(main_menu), gtk_separator_menu_item_new());
	gtk_menu_shell_append(GTK_MENU_SHELL(main_menu), menu_text_style);*/
	gtk_menu_shell_append(GTK_MENU_SHELL(main_menu), gtk_separator_menu_item_new());
	gtk_menu_shell_append(GTK_MENU_SHELL(main_menu), menu_settings);
	/*gtk_menu_shell_append(GTK_MENU_SHELL(main_menu), gtk_separator_menu_item_new());*/
	gtk_menu_shell_append(GTK_MENU_SHELL(main_menu), menu_sync);
	gtk_menu_shell_append(GTK_MENU_SHELL(main_menu), gtk_separator_menu_item_new());
	gtk_menu_shell_append(GTK_MENU_SHELL(main_menu), menu_about);
	gtk_menu_shell_append(GTK_MENU_SHELL(main_menu), menu_quit);


	/* Must be at the end of the menu definition */
	hildon_window_set_menu(HILDON_WINDOW(mainwin), GTK_MENU(main_menu));
#endif


	/* TOOLBAR */
	toolbar = gtk_toolbar_new();
	ui->toolbar = GTK_TOOLBAR(toolbar);
	button_dec_indent = create_tool_button(action_dec_indent, ICON_DEC_INDENT);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(button_dec_indent), -1);
#ifdef HILDON_HAS_APP_MENU
	gtk_widget_set_size_request(GTK_WIDGET(button_dec_indent), 95, -1);
#else
	gtk_widget_set_size_request(GTK_WIDGET(button_dec_indent), 80, -1);
#endif

	button_inc_indent = create_tool_button(action_inc_indent, ICON_INC_INDENT);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(button_inc_indent), -1);

	button_link = create_tool_button(action_link, ICON_LINK);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(button_link), -1);

	button_style = create_tool_button(action_text_style, ICON_TEXT_STYLE);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(button_style), -1);

	button_find = create_tool_button(action_find, ICON_SEARCH);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(button_find), -1);

	GtkToolItem *separator = gtk_separator_tool_item_new();
	gtk_tool_item_set_expand(separator, TRUE);
	gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(separator), FALSE);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), separator, -1);

	button_back = create_tool_button(action_back, ICON_BACK);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(button_back), -1);

	button_forward = create_tool_button(action_forward, ICON_FORWARD);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(button_forward), -1);

	button_notes = create_tool_button(action_notes, ICON_OPEN);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(button_notes), -1);

	gtk_widget_show_all(toolbar);

	/*
	if (app_data->portrait) {
		on_orientation_changed(NULL, NULL);
	}
	*/

	hildon_window_add_toolbar(HILDON_WINDOW(mainwin), GTK_TOOLBAR(toolbar));

	/* FIND TOOL BAR */
	find_bar = hildon_find_toolbar_new(_("Search:"));
	hildon_window_add_toolbar(HILDON_WINDOW(mainwin), GTK_TOOLBAR(find_bar));

	/* SCROLLED WINDOW */
#ifdef HILDON_HAS_APP_MENU
	scrolledwindow1 = hildon_pannable_area_new();
#else
	scrolledwindow1 = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
#endif
	gtk_widget_show(scrolledwindow1);
	gtk_box_pack_start(GTK_BOX (vbox1), scrolledwindow1, TRUE, TRUE, 0);

	/* TEXT VIEW */
	buffer = GTK_TEXT_BUFFER(conboy_note_buffer_new());
#ifdef HILDON_HAS_APP_MENU
	textview = hildon_text_view_new();
	hildon_text_view_set_buffer(HILDON_TEXT_VIEW(textview), buffer);
#else
	textview = gtk_text_view_new_with_buffer(buffer);
#endif
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_WORD);
	gtk_widget_show(textview);
	gtk_container_add(GTK_CONTAINER (scrolledwindow1), textview);


	/* Create basic set of tags */
	initialize_tags(buffer);

	/* Set initial font size */
	font = pango_font_description_new();
	pango_font_description_set_size(font, settings_load_font_size());
	gtk_widget_modify_font(GTK_WIDGET(textview), font);
	pango_font_description_free(font);

	/* Set initial scrollbar size */
	if (settings_load_scrollbar_size() == SETTINGS_SCROLLBAR_SIZE_BIG) {
		hildon_helper_set_thumb_scrollbar(GTK_SCROLLED_WINDOW(scrolledwindow1), TRUE);
	}

	/* Set initial colors */
	if (settings_load_use_costum_colors()) {
		GdkColor color;
		GtkTextTag *title = gtk_text_tag_table_lookup(buffer->tag_table, "_title");
		GtkTextTag *link = gtk_text_tag_table_lookup(buffer->tag_table, "link:internal");

		/* Background color */
		settings_load_color(&color, SETTINGS_COLOR_TYPE_BACKGROUND);
		gtk_widget_modify_base(textview, GTK_STATE_NORMAL, &color);

		/* Text color */
		settings_load_color(&color, SETTINGS_COLOR_TYPE_TEXT);
		gtk_widget_modify_text(textview, GTK_STATE_NORMAL, &color);

		/* Link color */
		settings_load_color(&color, SETTINGS_COLOR_TYPE_LINKS);
		g_object_set(title, "foreground-gdk", &color, NULL);
		g_object_set(link, "foreground-gdk", &color, NULL);
	}


	/* Enable support for tap and hold on the textview */
	/* TODO: This is deactivated for now, because it's not included in upstream Gtk
	 * and therefore does not work on exist in ubuntu */
	/*gtk_widget_tap_and_hold_setup(textview, NULL, NULL, 0);*/

	/* Save for later usage */
	ui->window = HILDON_WINDOW(mainwin);
	ui->view = GTK_TEXT_VIEW(textview);
	ui->buffer = buffer;
	ui->find_bar = HILDON_FIND_TOOLBAR(find_bar);
	ui->find_bar_is_visible = FALSE;
	ui->style_menu = text_style_menu;
	ui->app_menu = main_menu;
	ui->scrolled_window = scrolledwindow1;

	ui->action_bold = GTK_TOGGLE_ACTION(action_bold);
	ui->action_bullets = GTK_TOGGLE_ACTION(action_bullets);
	ui->action_highlight = GTK_TOGGLE_ACTION(action_highlight);
	ui->action_italic = GTK_TOGGLE_ACTION(action_italic);
	ui->action_strike = GTK_TOGGLE_ACTION(action_strike);
	ui->action_fixed = GTK_TOGGLE_ACTION(action_fixed);
	ui->action_link = GTK_ACTION(action_link);
	ui->action_font_small = GTK_RADIO_ACTION(action_font_small);
	ui->action_inc_indent = GTK_ACTION(action_inc_indent);
	ui->action_dec_indent = GTK_ACTION(action_dec_indent);
	ui->action_back = GTK_ACTION(action_back);
	ui->action_forward = GTK_ACTION(action_forward);
	ui->action_delete = GTK_ACTION(action_delete);
	ui->action_text_style = GTK_ACTION(action_text_style);
	ui->action_find = GTK_ACTION(action_find);

	ui->menu_open = menu_open;

	if (app_data->portrait) {
		on_orientation_changed(NULL, NULL);
	}

	/* Window signals */
	g_signal_connect(mainwin, "delete-event",
	        G_CALLBACK(on_window_delete),
	        ui);

	/* Action signals */

	g_signal_connect(action_new, "activate",
			G_CALLBACK(on_new_button_clicked),
			ui);

	g_signal_connect(action_bold, "activate",
			G_CALLBACK(on_format_button_clicked),
			ui);

	g_signal_connect(action_italic, "activate",
			G_CALLBACK(on_format_button_clicked),
			ui);

	g_signal_connect(action_strike, "activate",
			G_CALLBACK(on_format_button_clicked),
			ui);

	g_signal_connect(action_highlight, "activate",
			G_CALLBACK(on_format_button_clicked),
			ui);

	g_signal_connect(action_fixed, "activate",
			G_CALLBACK(on_format_button_clicked),
			ui);

	/* It is enough to listen to one action of the radio group */
	g_signal_connect(action_font_small, "changed",
			G_CALLBACK(on_font_size_radio_group_changed),
			ui);

	g_signal_connect(action_bullets, "activate",
			G_CALLBACK(on_bullets_button_clicked),
			ui);

	g_signal_connect(action_quit, "activate",
			G_CALLBACK(on_quit_button_clicked),
			ui);

	g_signal_connect(action_settings, "activate",
			G_CALLBACK(on_settings_button_clicked),
			mainwin);

	g_signal_connect(action_inc_indent, "activate",
			G_CALLBACK(on_inc_indent_button_clicked),
			ui);

	g_signal_connect(action_dec_indent, "activate",
			G_CALLBACK(on_dec_indent_button_clicked),
			ui);

	g_signal_connect(action_link, "activate",
			G_CALLBACK(on_link_button_clicked),
			ui);

	g_signal_connect(action_notes, "activate",
			G_CALLBACK(on_notes_button_clicked),
			mainwin);

	g_signal_connect(action_delete, "activate",
			G_CALLBACK(on_delete_button_clicked),
			ui);

	g_signal_connect(action_zoom_in, "activate",
			G_CALLBACK(on_smaller_button_clicked),
			NULL);

	g_signal_connect(action_zoom_in, "activate",
			G_CALLBACK(on_bigger_button_clicked),
			NULL);

	g_signal_connect(action_text_style, "activate",
			G_CALLBACK(on_style_button_clicked),
			ui);

	g_signal_connect(action_find, "activate",
			G_CALLBACK(on_find_button_clicked),
			ui);

	g_signal_connect(action_sync, "activate",
			G_CALLBACK(on_sync_but_clicked),
			ui);

	g_signal_connect(action_back, "activate",
			G_CALLBACK(on_back_button_clicked),
			ui);

	g_signal_connect(action_forward, "activate",
			G_CALLBACK(on_forward_button_clicked),
			ui);

	g_signal_connect(action_fullscreen, "activate",
			G_CALLBACK(on_fullscreen_button_clicked),
			ui);

	g_signal_connect(action_about, "activate",
			G_CALLBACK(on_about_button_clicked),
			ui);

	/* OTHER SIGNALS */
	g_signal_connect ((gpointer) buffer, "mark-set",
			G_CALLBACK (on_textview_cursor_moved),
			ui);

	g_signal_connect ((gpointer) buffer, "changed",
			G_CALLBACK (on_textbuffer_changed),
			mainwin);

	g_signal_connect ((gpointer)buffer, "modified-changed",
			G_CALLBACK(on_text_buffer_modified_changed),
			ui);

	g_signal_connect ((gpointer)textview, "tap-and-hold",
			G_CALLBACK(on_textview_tap_and_hold),
			NULL);

	g_signal_connect ((gpointer)mainwin, "key_press_event",
			G_CALLBACK(on_hardware_key_pressed),
			ui);

	g_signal_connect ((gpointer)textview, "key-press-event",
			G_CALLBACK(on_text_view_key_pressed),
			ui);

	g_signal_connect_after (GTK_TEXT_BUFFER(buffer), "insert-text",
			G_CALLBACK(on_text_buffer_insert_text),
			ui);

	g_signal_connect_after ((gpointer)buffer, "delete-range",
			G_CALLBACK(on_text_buffer_delete_range),
			ui);

	g_signal_connect((gpointer)find_bar, "search",
			G_CALLBACK(on_find_bar_search),
			ui);

	g_signal_connect((gpointer)find_bar, "close",
			G_CALLBACK(on_find_bar_close),
			ui);

	g_signal_connect((gpointer)mainwin, "map-event",
			G_CALLBACK(on_window_visible),
			NULL);

	g_signal_connect((gpointer)mainwin, "window-state-event",
			G_CALLBACK(on_fullscreen_toggled),
			ui);

	/* Listening to activation / deactivation of storage */
	ConboyStorage *storage = app_data->storage;
	g_signal_connect(storage, "activated",
			G_CALLBACK(on_storage_activated), ui);

	g_signal_connect(storage, "deactivated",
			G_CALLBACK(on_storage_deactivated), ui);


	/* Listen to changes in the settings */
	/* TODO: Use an array instead of the list */
	ui->listeners = g_list_prepend(ui->listeners, GINT_TO_POINTER(gconf_client_notify_add(app_data->client, SETTINGS_SCROLLBAR_SIZE, on_scrollbar_settings_changed, scrolledwindow1, NULL, NULL)));
	ui->listeners = g_list_prepend(ui->listeners, GINT_TO_POINTER(gconf_client_notify_add(app_data->client, SETTINGS_BACKGROUND_COLOR, on_background_color_changed, textview, NULL, NULL)));
	ui->listeners = g_list_prepend(ui->listeners, GINT_TO_POINTER(gconf_client_notify_add(app_data->client, SETTINGS_TEXT_COLOR, on_text_color_changed, textview, NULL, NULL)));
	ui->listeners = g_list_prepend(ui->listeners, GINT_TO_POINTER(gconf_client_notify_add(app_data->client, SETTINGS_LINK_COLOR, on_link_color_changed, textview, NULL, NULL)));
	ui->listeners = g_list_prepend(ui->listeners, GINT_TO_POINTER(gconf_client_notify_add(app_data->client, SETTINGS_USE_CUSTOM_COLORS, on_use_custom_colors_changed, textview, NULL, NULL)));
	ui->listeners = g_list_prepend(ui->listeners, GINT_TO_POINTER(gconf_client_notify_add(app_data->client, SETTINGS_FONT_SIZE, on_font_size_changed, textview, NULL, NULL)));


	g_signal_connect((gpointer)screen, "size-changed",
			G_CALLBACK(on_orientation_changed),
			NULL);

	link_internal_tag = gtk_text_tag_table_lookup(buffer->tag_table, "link:internal");
	g_signal_connect ((gpointer) link_internal_tag, "event",
			G_CALLBACK (on_link_internal_tag_event),
			ui);

	link_url_tag = gtk_text_tag_table_lookup(buffer->tag_table, "link:url");
	g_signal_connect ((gpointer) link_url_tag, "event",
			G_CALLBACK (on_link_url_tag_event),
			ui);

	/* Set approximate window size to make scrolling work correctly */
	gtk_window_set_default_size(GTK_WINDOW(mainwin), 700, 450);

	/* Before ungrabbing the keys, we need to show it */
	gtk_widget_show(mainwin);
	ungrab_volume_keys(mainwin);

	/* Adding the transparent fullscreen button */
#ifdef HILDON_HAS_APP_MENU
	HeFullscreenButton *but = he_fullscreen_button_new(GTK_WINDOW(mainwin));
	g_signal_connect_swapped(but, "clicked", G_CALLBACK(ui_helper_toggle_fullscreen), he_fullscreen_button_get_window(but));
#endif

	/* Disable word completion (no HILDON_GTK_INPUT_MODE_DICTIONARY and no HILDON_GTK_INPUT_MODE_MULTILINE) */
	hildon_gtk_text_view_set_input_mode(GTK_TEXT_VIEW(textview), HILDON_GTK_INPUT_MODE_FULL | HILDON_GTK_INPUT_MODE_AUTOCAP);

	return ui;
}

void
conboy_note_window_show_note(UserInterface *ui, ConboyNote *note)
{
	conboy_note_buffer_set_xml(CONBOY_NOTE_BUFFER(ui->buffer), note->content);
	ui->note = note;
}

void
conboy_note_window_update_button_states(UserInterface *ui)
{
	/* Blocking signals here because the ..set_active() method makes the buttons
	 * emit the clicked signal. And because of this the formatting changes.
	 */
	g_signal_handlers_block_by_func(ui->action_bold, on_format_button_clicked, ui);
	g_signal_handlers_block_by_func(ui->action_italic, on_format_button_clicked, ui);
	g_signal_handlers_block_by_func(ui->action_strike, on_format_button_clicked, ui);
	g_signal_handlers_block_by_func(ui->action_highlight, on_format_button_clicked, ui);
	g_signal_handlers_block_by_func(ui->action_fixed, on_format_button_clicked, ui);
	g_signal_handlers_block_by_func(ui->action_bullets, on_bullets_button_clicked, ui);
	g_signal_handlers_block_by_func(ui->action_font_small, on_font_size_radio_group_changed, ui);
	g_signal_handlers_block_by_func(ui->action_dec_indent, on_dec_indent_button_clicked, ui);



	/* TODO: This can be optimized: Note disable all and then enable selected, but determine state and then
	 * set the state. */
	gtk_toggle_action_set_active(ui->action_bold, FALSE);
	gtk_toggle_action_set_active(ui->action_italic, FALSE);
	gtk_toggle_action_set_active(ui->action_strike, FALSE);
	gtk_toggle_action_set_active(ui->action_highlight, FALSE);
	gtk_toggle_action_set_active(ui->action_fixed, FALSE);
	gtk_toggle_action_set_active(ui->action_bullets, FALSE);
	gtk_radio_action_set_current_value(ui->action_font_small, 1); /* Enable normal font size */

	/* Disable indent actions */
	/*gtk_action_set_sensitive(ui->action_inc_indent, TRUE);*/
	gtk_action_set_sensitive(ui->action_dec_indent, FALSE);

	/* Copy pointer for iteration */
	GSList *tags = conboy_note_buffer_get_active_tags(CONBOY_NOTE_BUFFER(ui->buffer));
	while (tags != NULL) {
		GtkTextTag *tag = GTK_TEXT_TAG(tags->data);
		if (strcmp(tag->name, "bold") == 0) {
			gtk_toggle_action_set_active(ui->action_bold, TRUE);
		} else if (strcmp(tag->name, "italic") == 0) {
			gtk_toggle_action_set_active(ui->action_italic, TRUE);
		} else if (strcmp(tag->name, "strikethrough") == 0) {
			gtk_toggle_action_set_active(ui->action_strike, TRUE);
		} else if (strcmp(tag->name, "highlight") == 0) {
			gtk_toggle_action_set_active(ui->action_highlight, TRUE);
		} else if (strcmp(tag->name, "monospace") == 0) {
			gtk_toggle_action_set_active(ui->action_fixed, TRUE);
		} else if (strncmp(tag->name, "list-item", 9) == 0) {
			gtk_toggle_action_set_active(ui->action_bullets, TRUE);
		} else if (strcmp(tag->name, "size:small") == 0) {
			gtk_radio_action_set_current_value(ui->action_font_small, 0);
		} else if (strcmp(tag->name, "size:large") == 0) {
			gtk_radio_action_set_current_value(ui->action_font_small, 2);
		} else if (strcmp(tag->name, "size:huge") == 0) {
			gtk_radio_action_set_current_value(ui->action_font_small, 3);
		} else if (strcmp(tag->name, "list") == 0) {
			/*gtk_action_set_sensitive(ui->action_inc_indent, TRUE);*/
			gtk_action_set_sensitive(ui->action_dec_indent, TRUE);
		}

		tags = tags->next;
	}

	/* unblock signals */
	g_signal_handlers_unblock_by_func(ui->action_bold, on_format_button_clicked, ui);
	g_signal_handlers_unblock_by_func(ui->action_italic, on_format_button_clicked, ui);
	g_signal_handlers_unblock_by_func(ui->action_strike, on_format_button_clicked, ui);
	g_signal_handlers_unblock_by_func(ui->action_highlight, on_format_button_clicked, ui);
	g_signal_handlers_unblock_by_func(ui->action_fixed, on_format_button_clicked, ui);
	g_signal_handlers_unblock_by_func(ui->action_bullets, on_bullets_button_clicked, ui);
	g_signal_handlers_unblock_by_func(ui->action_font_small, on_font_size_radio_group_changed, ui);
	g_signal_handlers_unblock_by_func(ui->action_dec_indent, on_dec_indent_button_clicked, ui);
}
