/*
 * This file is a part of hildon-extras
 *
 * Copyright (C) 2009 Cornelius Hald <hald@icandy.de>
 *
 * Alsmost completely based on code by:
 * Copyright (C) 2008, 2009 Nokia Corporation
 * Contact: Rodrigo Novo <rodrigo.novo@nokia.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser Public License as published by
 * the Free Software Foundation; version 2 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser Public License for more details.
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef HILDON_HAS_APP_MENU

#ifndef                                         __HE_CHECK_BUTTON_H__
#define                                         __HE_CHECK_BUTTON_H__

#include                                        <hildon/hildon-gtk.h>

G_BEGIN_DECLS

#define                                         HE_TYPE_CHECK_BUTTON \
                                                (he_check_button_get_type())

#define                                         HE_CHECK_BUTTON(obj) \
                                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                                                HE_TYPE_CHECK_BUTTON, HeCheckButton))

#define                                         HE_CHECK_BUTTON_CLASS(klass) \
                                                (G_TYPE_CHECK_CLASS_CAST ((klass), \
                                                HE_TYPE_CHECK_BUTTON, HeCheckButtonClass))

#define                                         HE_IS_CHECK_BUTTON(obj) \
                                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HE_TYPE_CHECK_BUTTON))

#define                                         HE_IS_CHECK_BUTTON_CLASS(klass) \
                                                (G_TYPE_CHECK_CLASS_TYPE ((klass), HE_TYPE_CHECK_BUTTON))

#define                                         HE_CHECK_BUTTON_GET_CLASS(obj) \
                                                (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                                                HE_TYPE_CHECK_BUTTON, HeCheckButtonClass))

typedef struct                                  _HeCheckButton HeCheckButton;

typedef struct                                  _HeCheckButtonClass HeCheckButtonClass;

typedef struct                                  _HeCheckButtonPrivate HeCheckButtonPrivate;

struct                                          _HeCheckButtonClass
{
    GtkButtonClass parent_class;

    /* Signal handlers */
    void (* toggled)							(HeCheckButton *button);
};

struct                                          _HeCheckButton
{
    GtkButton parent;

    /* private */
    HeCheckButtonPrivate *priv;
};


/**
 * HeCheckButtonArrangement:
 * @HE_CHECK_BUTTON_ARRANGEMENT_HORIZONTAL: Labels are arranged from left to right
 * @HE_CHECK_BUTTON_ARRANGEMENT_VERTICAL: Labels are arranged from top to bottom
 *
 * Describes the arrangement of labels inside a #HeCheckButton
 *
 **/
typedef enum {
   HE_CHECK_BUTTON_ARRANGEMENT_HORIZONTAL,
   HE_CHECK_BUTTON_ARRANGEMENT_VERTICAL
}                                               HeCheckButtonArrangement;

/**
 * HeCheckButtonStyle:
 * @HE_CHECK_BUTTON_STYLE_NORMAL: The button will look like a normal #HeCheckButton
 * @HE_CHECK_BUTTON_STYLE_PICKER: The button will look like a #HildonPickerButton
 *
 * Describes the visual style of a #HeCheckButton
 **/
typedef enum {
   HE_CHECK_BUTTON_STYLE_NORMAL,
   HE_CHECK_BUTTON_STYLE_PICKER
}                                               HeCheckButtonStyle;

GType
he_check_button_get_type                        (void) G_GNUC_CONST;

GtkWidget *
he_check_button_new                              (HildonSizeType          size,
                                                 HeCheckButtonArrangement arrangement);

GtkWidget *
he_check_button_new_with_text                    (HildonSizeType           size,
                                                 HeCheckButtonArrangement  arrangement,
                                                 const gchar              *title,
                                                 const gchar              *value);

void
he_check_button_set_title                        (HeCheckButton *button,
                                                 const gchar    *title);

void
he_check_button_set_value                        (HeCheckButton *button,
                                                 const gchar    *value);

const gchar *
he_check_button_get_title                        (HeCheckButton *button);

const gchar *
he_check_button_get_value                        (HeCheckButton *button);

void
he_check_button_set_text                         (HeCheckButton *button,
                                                 const gchar    *title,
                                                 const gchar    *value);

void
he_check_button_set_alignment                    (HeCheckButton *button,
                                                 gfloat          xalign,
                                                 gfloat          yalign,
                                                 gfloat          xscale,
                                                 gfloat          yscale);
void
he_check_button_set_title_alignment              (HeCheckButton *button,
                                                 gfloat          xalign,
                                                 gfloat          yalign);

void
he_check_button_set_value_alignment              (HeCheckButton *button,
                                                 gfloat          xalign,
                                                 gfloat          yalign);

void
he_check_button_add_title_size_group             (HeCheckButton *button,
                                                 GtkSizeGroup   *size_group);
void
he_check_button_add_value_size_group             (HeCheckButton *button,
                                                 GtkSizeGroup   *size_group);

void
he_check_button_add_size_groups                  (HeCheckButton *button,
                                                 GtkSizeGroup   *title_size_group,
                                                 GtkSizeGroup   *value_size_group,
                                                 GtkSizeGroup   *image_size_group);

void
he_check_button_set_style                        (HeCheckButton      *button,
                                                 HeCheckButtonStyle  style);

HeCheckButtonStyle
he_check_button_get_style                        (HeCheckButton *button);

void
he_check_button_toggled                          (HeCheckButton *button);

void
he_check_button_set_active                       (HeCheckButton *button,
                                                 gboolean       is_active);

gboolean
he_check_button_get_active                       (HeCheckButton *button);

G_END_DECLS

#endif /* __HE_CHECK_BUTTON_H__ */

#endif /* HILDON_HAS_APP_MENU */
