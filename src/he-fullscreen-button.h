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
        GtkWidget   *overlay;

        gboolean     release_event;
        guint32      last_event_time;

        guint        button_press_signal_id;
        guint        button_release_signal_id;

        gulong       button_press_hook_id;
        gulong       button_release_hook_id;
};

struct _HeFullscreenButtonClass
{
        GObjectClass parent_class;

        guint        signal_id_fullscreen_mode_enabled;
        guint        signal_id_fullscreen_mode_disabled;

        void (*clicked) (HeFullscreenButton *manager);
};


GType
he_fullscreen_button_get_type (void);

HeFullscreenButton *
he_fullscreen_button_new (GtkWindow *window);

GtkWindow *
he_fullscreen_button_get_window (HeFullscreenButton *manager);


#endif /* _HE_FULLSCREEN_BUTTON_ */
