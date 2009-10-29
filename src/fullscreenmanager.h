/* This file is part of Conboy.
 *
 * Copyright (C) 2009 Cornelius Hald
 *
 * The code is based on example code taken from the maemoexamples project.
 * Original copyright is below.
 *
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the Server EAL package.
 *
 * The Initial Developer of the Original Code is Nokia Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2008
 * the Initial Developer. All Rights Reserved.
 *
 * Contact: Sudarsana Nagineni <sudarsana.nagineni@nokia.com>
 */

#ifndef _FULLSCREENMANAGER_H_
#define _FULLSCREENMANAGER_H_

typedef struct _FullscreenManager FullscreenManager;
typedef struct _FullscreenManagerClass FullscreenManagerClass;

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

typedef void (* Callback) (GtkWindow *window);

struct _FullscreenManager {
        GObject      parent;

        GtkWindow   *parent_window;
        GtkWidget   *overlay;

        gboolean     release_event;
        guint32      last_event_time;

        guint        button_press_signal_id;
        guint        button_release_signal_id;

        gulong       button_press_hook_id;
        gulong       button_release_hook_id;

        Callback     callback;
};

struct _FullscreenManagerClass {
        GObjectClass parent_class;

        guint        signal_id_fullscreen_mode_enabled;
        guint        signal_id_fullscreen_mode_disabled;
};

GType fullscreen_manager_get_type(void);
#define FULLSCREEN_MANAGER_TYPE (fullscreen_manager_get_type())

#define FULLSCREEN_MANAGER(object) (G_TYPE_CHECK_INSTANCE_CAST((object), FULLSCREEN_MANAGER_TYPE, FullscreenManager))
#define FULLSCREEN_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), FULLSCREEN_MANAGER_TYPE, FullscreenManagerClass))
#define FULLSCREEN_IS_MANAGER(object) (G_TYPE_CHECK_INSTANCE_TYPE((object), FULLSCREEN_MANAGER_TYPE))
#define FULLSCREEN_IS_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), FULLSCREEN_MANAGER_TYPE))
#define FULLSCREEN_MANAGER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), FULLSCREEN_MANAGER_TYPE, FullscreenManagerClass))



/**
 * Create new full screen manager instance.
 * Note: This function also attaches the full screen manager to the view's
 * destroy signal so that the full screen manager is destroyed automatically,
 * i.e. under normal circumstances there's no need to do explicit call to
 * fullscreen_destroy_manager.
 *
 * @param window A GtkWindow instance.
 * @param callback A function that should be called whenever the overlay is clicked if null, gtk_window_unfullscreen() is called.
 * @return New FullscreenManager instance.
 */
FullscreenManager *
fullscreen_manager_new(GtkWindow *window, Callback callback);



#endif /* _FULLSCREENMANAGER_H_ */
