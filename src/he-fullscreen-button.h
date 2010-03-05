/*
 * This file is a part of hildon-extras
 *
 * Copyright (C) 2009 Cornelius Hald <hald@icandy.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser Public License as published by
 * the Free Software Foundation; version 2 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser Public License for more details.
 */

#ifndef _HE_FULLSCREEN_BUTTON_
#define _HE_FULLSCREEN_BUTTON_

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define						HE_TYPE_FULLSCREEN_BUTTON \
							(he_fullscreen_button_get_type())

#define						HE_FULLSCREEN_BUTTON(object) \
							(G_TYPE_CHECK_INSTANCE_CAST((object), \
							HE_TYPE_FULLSCREEN_BUTTON, HeFullscreenButton))

#define						HE_FULLSCREEN_BUTTON_CLASS(klass) \
							(G_TYPE_CHECK_CLASS_CAST((klass), \
							HE_TYPE_FULLSCREEN_BUTTON, HeFullscreenButtonClass))

#define						HE_IS_FULLSCREEN_BUTTON(object) \
							(G_TYPE_CHECK_INSTANCE_TYPE((object), \
							HE_TYPE_FULLSCREEN_BUTTON))

#define						HE_IS_FULLSCREEN_BUTTON_CLASS(klass) \
							(G_TYPE_CHECK_CLASS_TYPE((klass), \
							HE_TYPE_FULLSCREEN_BUTTON))

#define						HE_FULLSCREEN_BUTTON_GET_CLASS(obj) \
							(G_TYPE_INSTANCE_GET_CLASS((obj), \
							HE_TYPE_FULLSCREEN_BUTTON, HeFullscreenButtonClass))


typedef struct				_HeFullscreenButton HeFullscreenButton;
typedef struct				_HeFullscreenButtonClass HeFullscreenButtonClass;


struct _HeFullscreenButton
{
        GObject      parent;

        GtkWindow   *parent_window;
};

struct _HeFullscreenButtonClass
{
        GObjectClass parent_class;

        void (*clicked) (HeFullscreenButton *manager);
};


GType
he_fullscreen_button_get_type (void);

HeFullscreenButton *
he_fullscreen_button_new (GtkWindow *window);

void
he_fullscreen_button_disable (HeFullscreenButton *self);

void
he_fullscreen_button_enable (HeFullscreenButton *self);

GtkWidget *
he_fullscreen_button_get_overlay (HeFullscreenButton *self);

GtkWindow *
he_fullscreen_button_get_window (HeFullscreenButton *self);

G_END_DECLS

#endif /* _HE_FULLSCREEN_BUTTON_ */
