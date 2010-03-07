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

#ifndef					__HE_ABOUT_DIALOG_H__
#define					__HE_ABOUT_DIALOG_H__

#include				<gtk/gtk.h>

typedef enum {
    HE_ABOUT_RESPONSE_WEBSITE = 0,
    HE_ABOUT_RESPONSE_BUGTRACKER,
    HE_ABOUT_RESPONSE_DONATE,
} HeAboutDialogResponseType;


G_BEGIN_DECLS
  #define                                         HE_TYPE_ABOUT_DIALOG \
                                                  (he_about_dialog_get_type ())

  #define                                         HE_ABOUT_DIALOG(obj) \
                                                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                                                  HE_TYPE_ABOUT_DIALOG, HeAboutDialog))

  #define                                         HE_ABOUT_DIALOG_CLASS(vtable) \
                                                  (G_TYPE_CHECK_CLASS_CAST ((vtable), \
                                                  HE_TYPE_ABOUT_DIALOG, HeAboutDialogClass))

  #define                                         HE_IS_ABOUT_DIALOG(obj) \
                                                  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HE_TYPE_ABOUT_DIALOG))

  #define                                         HE_IS_ABOUT_DIALOG_CLASS(vtable) \
                                                  (G_TYPE_CHECK_CLASS_TYPE ((vtable), HE_TYPE_ABOUT_DIALOG))

  #define                                         HE_ABOUT_DIALOG_GET_CLASS(inst) \
                                                  (G_TYPE_INSTANCE_GET_CLASS ((inst), \
                                                  HE_TYPE_ABOUT_DIALOG, HeAboutDialogClass))

typedef struct			_HeAboutDialog HeAboutDialog;

typedef struct			_HeAboutDialogClass HeAboutDialogClass;

typedef struct          _HeAboutDialogPrivate HeAboutDialogPrivate;

struct					_HeAboutDialog
{
	GtkDialog parent;
	/*< private > */
	HeAboutDialogPrivate *priv;
};

struct					_HeAboutDialogClass
{
	GtkDialogClass parent_class;
};

GType G_GNUC_CONST
he_about_dialog_get_type	(void);

GtkWidget*
he_about_dialog_new			(void);

void
he_about_dialog_present(GtkWindow* parent,
                        const gchar* app_name,
                        const gchar* icon_name,
                        const gchar* version,
                        const gchar* description,
                        const gchar* copyright,
                        const gchar* website_url,
                        const gchar* bugtracker_url,
                        const gchar* donate_url);

void
he_about_dialog_set_app_name(HeAboutDialog* dialog, const gchar* app_name);

void
he_about_dialog_set_icon_name(HeAboutDialog* dialog, const gchar* icon_name);

void
he_about_dialog_set_version(HeAboutDialog* dialog, const gchar* version);

void
he_about_dialog_set_description(HeAboutDialog* ad, const gchar* description);

void
he_about_dialog_set_copyright(HeAboutDialog* ad, const gchar* copyright);

void
he_about_dialog_set_website(HeAboutDialog* ad, const gchar* url);

void
he_about_dialog_set_bugtracker(HeAboutDialog* ad, const gchar* url);

void
he_about_dialog_set_donate_url(HeAboutDialog* ad, const gchar* url);

G_END_DECLS

#endif						/* __HE_ABOUT_DIALOG_H__ */
