/*
 * This file is a part of conboy is based up on hildon. Original copyright below:
 *
 * Copyright (C) 2008 Nokia Corporation, all rights reserved.
 *
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

#ifndef                                         __CONBOY_CHECK_BUTTON_H__
#define                                         __CONBOY_CHECK_BUTTON_H__

#include                                        <hildon/hildon-gtk.h>

G_BEGIN_DECLS

#define                                         CONBOY_TYPE_CHECK_BUTTON \
                                                (conboy_check_button_get_type())

#define                                         CONBOY_CHECK_BUTTON(obj) \
                                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                                                CONBOY_TYPE_CHECK_BUTTON, ConboyCheckButton))

#define                                         CONBOY_CHECK_BUTTON_CLASS(klass) \
                                                (G_TYPE_CHECK_CLASS_CAST ((klass), \
                                                CONBOY_TYPE_CHECK_BUTTON, ConboyCheckButtonClass))

#define                                         CONBOY_IS_CHECK_BUTTON(obj) \
                                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CONBOY_TYPE_CHECK_BUTTON))

#define                                         CONBOY_IS_CHECK_BUTTON_CLASS(klass) \
                                                (G_TYPE_CHECK_CLASS_TYPE ((klass), CONBOY_TYPE_CHECK_BUTTON))

#define                                         CONBOY_CHECK_BUTTON_GET_CLASS(obj) \
                                                (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                                                CONBOY_TYPE_CHECK_BUTTON, ConboyCheckButtonClass))

typedef struct                                  _ConboyCheckButton ConboyCheckButton;

typedef struct                                  _ConboyCheckButtonClass ConboyCheckButtonClass;

typedef struct                                  _ConboyCheckButtonPrivate ConboyCheckButtonPrivate;

struct                                          _ConboyCheckButtonClass
{
    GtkButtonClass parent_class;

    /* Signal handlers */
    void (* toggled)							(ConboyCheckButton *button);
};

struct                                          _ConboyCheckButton
{
    GtkButton parent;

    /* private */
    ConboyCheckButtonPrivate *priv;
};


/**
 * ConboyCheckButtonArrangement:
 * @CONBOY_CHECK_BUTTON_ARRANGEMENT_HORIZONTAL: Labels are arranged from left to right
 * @CONBOY_CHECK_BUTTON_ARRANGEMENT_VERTICAL: Labels are arranged from top to bottom
 *
 * Describes the arrangement of labels inside a #ConboyCheckButton
 *
 **/
typedef enum {
   CONBOY_CHECK_BUTTON_ARRANGEMENT_HORIZONTAL,
   CONBOY_CHECK_BUTTON_ARRANGEMENT_VERTICAL
}                                               ConboyCheckButtonArrangement;

/**
 * ConboyCheckButtonStyle:
 * @CONBOY_CHECK_BUTTON_STYLE_NORMAL: The button will look like a normal #ConboyCheckButton
 * @CONBOY_CHECK_BUTTON_STYLE_PICKER: The button will look like a #HildonPickerButton
 *
 * Describes the visual style of a #ConboyCheckButton
 **/
typedef enum {
   CONBOY_CHECK_BUTTON_STYLE_NORMAL,
   CONBOY_CHECK_BUTTON_STYLE_PICKER
}                                               ConboyCheckButtonStyle;

GType
conboy_check_button_get_type                          (void) G_GNUC_CONST;

GtkWidget *
conboy_check_button_new                               (HildonSizeType          size,
                                                 ConboyCheckButtonArrangement arrangement);

GtkWidget *
conboy_check_button_new_with_text                     (HildonSizeType           size,
                                                 ConboyCheckButtonArrangement  arrangement,
                                                 const gchar             *title,
                                                 const gchar             *value);

void
conboy_check_button_set_title                         (ConboyCheckButton *button,
                                                 const gchar  *title);

void
conboy_check_button_set_value                         (ConboyCheckButton *button,
                                                 const gchar  *value);

const gchar *
conboy_check_button_get_title                         (ConboyCheckButton *button);

const gchar *
conboy_check_button_get_value                         (ConboyCheckButton *button);

void
conboy_check_button_set_text                          (ConboyCheckButton *button,
                                                 const gchar  *title,
                                                 const gchar  *value);

/*
void
conboy_check_button_set_image                         (ConboyCheckButton *button,
                                                 GtkWidget    *image);

GtkWidget *
conboy_check_button_get_image                         (ConboyCheckButton *button);
*/

/*
void
conboy_check_button_set_image_position                (ConboyCheckButton    *button,
                                                 GtkPositionType  position);
*/

void
conboy_check_button_set_alignment                     (ConboyCheckButton *button,
                                                 gfloat        xalign,
                                                 gfloat        yalign,
                                                 gfloat        xscale,
                                                 gfloat        yscale);
void
conboy_check_button_set_title_alignment               (ConboyCheckButton *button,
                                                 gfloat        xalign,
                                                 gfloat        yalign);

void
conboy_check_button_set_value_alignment               (ConboyCheckButton *button,
                                                 gfloat        xalign,
                                                 gfloat        yalign);

/*
void
conboy_check_button_set_image_alignment               (ConboyCheckButton *button,
                                                 gfloat        xalign,
                                                 gfloat        yalign);
*/

void
conboy_check_button_add_title_size_group              (ConboyCheckButton *button,
                                                 GtkSizeGroup *size_group);
void
conboy_check_button_add_value_size_group              (ConboyCheckButton *button,
                                                 GtkSizeGroup *size_group);
/*
void
conboy_check_button_add_image_size_group              (ConboyCheckButton *button,
                                                 GtkSizeGroup *size_group);
*/

void
conboy_check_button_add_size_groups                   (ConboyCheckButton *button,
                                                 GtkSizeGroup *title_size_group,
                                                 GtkSizeGroup *value_size_group,
                                                 GtkSizeGroup *image_size_group);

void
conboy_check_button_set_style                         (ConboyCheckButton      *button,
                                                 ConboyCheckButtonStyle  style);

ConboyCheckButtonStyle
conboy_check_button_get_style                         (ConboyCheckButton *button);

G_END_DECLS

#endif /* __CONBOY_CHECK_BUTTON_H__ */

#endif /* HILDON_HAS_APP_MENU */
