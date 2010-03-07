/*
 * This file is a part of hildon-extras
 *
 * Copyright (C) 2010 Thomas Perl
 * Copyright (C) 2005, 2008 Nokia Corporation.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version. or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/**
 * SECTION:he-about-dialog
 * @short_description: An about dialog for Hildon-based applications
 *
 * #HeAboutDialog works as a nice default about dialog for Maemo apps
 */

#define _GNU_SOURCE     /* needed for GNU nl_langinfo_l */
#define __USE_GNU       /* needed for locale */

#include <locale.h>

#include <string.h>
#include <stdlib.h>

#include <libintl.h>
#include <langinfo.h>

#include <hildon/hildon.h>

#include <dbus/dbus-glib.h>

#include "he-about-dialog.h"

#define HE_ABOUT_DIALOG_GET_PRIVATE(obj)                           \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), HE_TYPE_ABOUT_DIALOG, HeAboutDialogPrivate))

#define _(String) dgettext("hildon-libs", String)

G_DEFINE_TYPE(HeAboutDialog, he_about_dialog, GTK_TYPE_DIALOG);

struct _HeAboutDialogPrivate
{
    /* Widgets */
    GtkWidget* image_icon;
    GtkWidget* label_app_name;
    GtkWidget* label_version;
    GtkWidget* label_description;
    GtkWidget* label_copyright;
    GtkWidget* table_layout;

    /* Data */
    gchar* website_url;
    gchar* bugtracker_url;
    gchar* donate_url;
};

static void he_about_dialog_finalize (GObject * object);

static void he_about_dialog_response (GtkDialog *dialog, gint response_id, HeAboutDialog *fd);
static void he_about_dialog_class_init (HeAboutDialogClass * class);
static void he_about_dialog_init (HeAboutDialog * fd);
static void he_about_dialog_finalize (GObject *object);

static void open_webbrowser(const gchar* url);

static gpointer                                 parent_class = NULL;


static void
he_about_dialog_class_init (HeAboutDialogClass * klass)
{
    GObjectClass *object_class = (GObjectClass *) klass;
    parent_class = g_type_class_peek_parent (klass);

    g_type_class_add_private (object_class, sizeof (HeAboutDialogPrivate));

    object_class->finalize = he_about_dialog_finalize;
}

static void
he_about_dialog_finalize (GObject *object)
{
    g_return_if_fail(HE_IS_ABOUT_DIALOG(object));

    HeAboutDialog *about = HE_ABOUT_DIALOG(object);
    HeAboutDialogPrivate *priv = about->priv;

    g_free(priv->website_url);
    g_free(priv->bugtracker_url);
    g_free(priv->donate_url);

    G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
he_about_dialog_init (HeAboutDialog *ad)
{
    ad->priv = HE_ABOUT_DIALOG_GET_PRIVATE (ad);

    ad->priv->website_url = NULL;
    ad->priv->bugtracker_url = NULL;
    ad->priv->donate_url = NULL;

    gchar* title = NULL;
    const gchar* app_name = NULL;

    app_name = g_get_application_name();

    if (app_name == NULL) {
        gtk_window_set_title(GTK_WINDOW(ad), _("About"));
    } else {
        title = g_strdup_printf(_("About %s"), app_name);
        gtk_window_set_title(GTK_WINDOW(ad), title);
        g_free(title);
    }

    /* Create widgets */
    ad->priv->image_icon = gtk_image_new();
    ad->priv->label_app_name = gtk_label_new(app_name);
    ad->priv->label_version = gtk_label_new(NULL);
    ad->priv->label_description = gtk_label_new(NULL);
    ad->priv->label_copyright = gtk_label_new(NULL);
    ad->priv->table_layout = gtk_table_new(3, 3, FALSE);

    /* Fremantle styling */
    hildon_helper_set_logical_font(ad->priv->label_app_name, "X-LargeSystemFont");
    hildon_helper_set_logical_font(ad->priv->label_version, "LargeSystemFont");
    hildon_helper_set_logical_font(ad->priv->label_copyright, "SmallSystemFont");
    hildon_helper_set_logical_color(ad->priv->label_copyright,
          GTK_RC_FG, GTK_STATE_NORMAL, "SecondaryTextColor");

    /* Alignment and spacing */
    gtk_misc_set_alignment(GTK_MISC(ad->priv->label_app_name), 0., 1.);
    gtk_misc_set_alignment(GTK_MISC(ad->priv->label_version), 0., 1.);
    gtk_misc_set_alignment(GTK_MISC(ad->priv->label_description), 0., 0.);
    gtk_misc_set_alignment(GTK_MISC(ad->priv->label_copyright), 0., 1.);
    gtk_misc_set_padding(GTK_MISC(ad->priv->label_version), 10, 0);
    gtk_misc_set_padding(GTK_MISC(ad->priv->label_copyright), 0, 5);
    gtk_misc_set_padding(GTK_MISC(ad->priv->image_icon), 5, 5);

    GtkWidget* content_area = gtk_dialog_get_content_area(GTK_DIALOG(ad));

    /* Table layout */
    GtkTable* t = GTK_TABLE(ad->priv->table_layout);
    gtk_table_attach(t, ad->priv->image_icon, 0, 1, 0, 2, 0, GTK_EXPAND, 0, 0);
    gtk_table_attach(t, ad->priv->label_app_name, 1, 2, 0, 1, 0, GTK_EXPAND | GTK_FILL, 0, 0);
    gtk_table_attach(t, ad->priv->label_version, 2, 3, 0, 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
    gtk_table_attach(t, ad->priv->label_description, 1, 3, 1, 2, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
    gtk_table_attach(t, ad->priv->label_copyright, 0, 3, 2, 3, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
    gtk_container_add(GTK_CONTAINER(content_area), GTK_WIDGET(t));

    g_signal_connect (G_OBJECT (ad), "response", G_CALLBACK (he_about_dialog_response), ad);
    gtk_widget_show_all (GTK_WIDGET (ad));
}

/* ------------------------------ PRIVATE METHODS ---------------------------- */
static void
he_about_dialog_response (GtkDialog *dialog, gint response_id, HeAboutDialog* ad)
{
    HeAboutDialogPrivate *priv = HE_ABOUT_DIALOG_GET_PRIVATE (ad);

    switch (response_id) {

        case HE_ABOUT_RESPONSE_WEBSITE:
            open_webbrowser(priv->website_url);
            break;
        case HE_ABOUT_RESPONSE_BUGTRACKER:
            open_webbrowser(priv->bugtracker_url);
            break;
        case HE_ABOUT_RESPONSE_DONATE:
            open_webbrowser(priv->donate_url);
            break;
        default:
            /* Dialog closed - do nothing */
            break;
    }
}

static void
open_webbrowser(const gchar* url)
{
    DBusGConnection *connection;
    GError *error = NULL;
    DBusGProxy *proxy;

    connection = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
    if (connection == NULL) {
        /* FIXME: Should show error with URL to user */
        g_error_free(error);
        return;
    }

    proxy = dbus_g_proxy_new_for_name(connection,
            "com.nokia.osso_browser",
            "/com/nokia/osso_browser/request",
            "com.nokia.osso_browser");

    if (!dbus_g_proxy_call(proxy, "load_url", &error, G_TYPE_STRING, url,
                G_TYPE_INVALID, G_TYPE_INVALID)) {
        /* FIXME: Should show error with URL to user */
        g_error_free(error);
    }
}

/* ------------------------------ PUBLIC METHODS ---------------------------- */

/**
 * he_about_dialog_new:
 *
 * Creates a new #HeAboutDialog
 *
 * Returns: a new #HeAboutDialog
 *
 * Since: 2.2
 **/
GtkWidget *
he_about_dialog_new ()
{
    return g_object_new (HE_TYPE_ABOUT_DIALOG, NULL);
}


/**
 * Shows a new #HeAboutDialog with all settings configured.
 *
 * Parameters:
 *   * parent (The parent window that this dialog will be set transient for)
 *   * app_name (Application name, use NULL to use from g_get_application_name())
 *   * icon_name (The application's icon name - must be provided)
 *   * version (The version of the application)
 *   * description (A one-line description what the app does)
 *   * copyright (A one-line copyright notice)
 *   * website_url (The application's homepage URL, or NULL if not available)
 *   * bugtracker_url (The application's bugtracker URL, or NULL if not available)
 *   * donate_url (The application's donations URL, or NULL if not available)
 **/
void
he_about_dialog_present(GtkWindow* parent,
                        const gchar* app_name,
                        const gchar* icon_name,
                        const gchar* version,
                        const gchar* description,
                        const gchar* copyright,
                        const gchar* website_url,
                        const gchar* bugtracker_url,
                        const gchar* donate_url)
{
    HeAboutDialog* ad = HE_ABOUT_DIALOG(he_about_dialog_new());

    if (parent != NULL) {
        gtk_window_set_transient_for(GTK_WINDOW(ad), GTK_WINDOW(parent));
        gtk_window_set_destroy_with_parent(GTK_WINDOW(ad), TRUE);
    }

    if (app_name != NULL) {
        he_about_dialog_set_app_name(ad, app_name);
    }

    he_about_dialog_set_icon_name(ad, icon_name);
    he_about_dialog_set_version(ad, version);
    he_about_dialog_set_description(ad, description);
    he_about_dialog_set_copyright(ad, copyright);

    if (website_url != NULL) {
        he_about_dialog_set_website(ad, website_url);
    }

    if (bugtracker_url != NULL) {
        he_about_dialog_set_bugtracker(ad, bugtracker_url);
    }

    if (donate_url != NULL) {
        he_about_dialog_set_donate_url(ad, donate_url);
    }

    gtk_dialog_run(GTK_DIALOG(ad));
    gtk_widget_destroy(GTK_WIDGET(ad));
}

void
he_about_dialog_set_app_name(HeAboutDialog* ad, const gchar* app_name)
{
    g_return_if_fail (HE_IS_ABOUT_DIALOG(ad));
    gtk_label_set_text(GTK_LABEL(ad->priv->label_app_name), app_name);

    gchar* title = g_strdup_printf(_("About %s"), app_name);
    gtk_window_set_title(GTK_WINDOW(ad), title);
    g_free(title);
}

void
he_about_dialog_set_icon_name(HeAboutDialog* ad, const gchar* icon_name)
{
    g_return_if_fail (HE_IS_ABOUT_DIALOG(ad));

    gtk_image_set_from_icon_name(GTK_IMAGE(ad->priv->image_icon), icon_name, GTK_ICON_SIZE_DIALOG);
}

void
he_about_dialog_set_version(HeAboutDialog* ad, const gchar* version)
{
    g_return_if_fail (HE_IS_ABOUT_DIALOG(ad));
    gtk_label_set_text(GTK_LABEL(ad->priv->label_version), version);
}

void
he_about_dialog_set_description(HeAboutDialog* ad, const gchar* description)
{
    g_return_if_fail (HE_IS_ABOUT_DIALOG(ad));
    gtk_label_set_text(GTK_LABEL(ad->priv->label_description), description);
}

void
he_about_dialog_set_copyright(HeAboutDialog* ad, const gchar* copyright)
{
    g_return_if_fail (HE_IS_ABOUT_DIALOG(ad));
    gtk_label_set_text(GTK_LABEL(ad->priv->label_copyright), copyright);
}

void
he_about_dialog_set_website(HeAboutDialog* ad, const gchar* url)
{
    g_return_if_fail (HE_IS_ABOUT_DIALOG(ad));
    if (ad->priv->website_url == NULL) {
        gtk_dialog_add_button(GTK_DIALOG(ad), _("Visit website"), HE_ABOUT_RESPONSE_WEBSITE);
    } else {
        g_free(ad->priv->website_url);
    }
    ad->priv->website_url = g_strdup(url);
}

void
he_about_dialog_set_bugtracker(HeAboutDialog* ad, const gchar* url)
{
    g_return_if_fail (HE_IS_ABOUT_DIALOG(ad));
    if (ad->priv->bugtracker_url == NULL) {
        gtk_dialog_add_button(GTK_DIALOG(ad), _("Report bug"), HE_ABOUT_RESPONSE_BUGTRACKER);
    } else {
        g_free(ad->priv->bugtracker_url);
    }
    ad->priv->bugtracker_url = g_strdup(url);
}

void
he_about_dialog_set_donate_url(HeAboutDialog* ad, const gchar* url)
{
    g_return_if_fail (HE_IS_ABOUT_DIALOG(ad));
    if (ad->priv->donate_url == NULL) {
        gtk_dialog_add_button(GTK_DIALOG(ad), _("Donate"), HE_ABOUT_RESPONSE_DONATE);
    } else {
        g_free(ad->priv->donate_url);
    }
    ad->priv->donate_url = g_strdup(url);
}


