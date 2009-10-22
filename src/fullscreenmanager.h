/* ***** BEGIN LICENSE BLOCK *****
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
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef _FULLSCREENMANAGER_H_
#define _FULLSCREENMANAGER_H_

typedef struct _FullscreenManager FullscreenManager;
typedef struct _FullscreenManagerClass FullscreenManagerClass;


#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#define FSM_WIN_STATE_KEY "is_fullscreen"

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

        gint		 overlay_x;
        gint		 overlay_y;

        gboolean	 overlay_visible;
        gboolean	 fullscreen;
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
 * @param view A MaemopadWindow instance.
 * @return New FullscreenManager instance.
 */
FullscreenManager *
fullscreen_create_manager(GtkWindow * view);

/**
 * Explicitly destroy the full screen manager.
 * NOTE: Normally the full screen manager is destroyed automatically so there's
 * no need to call this function.
 *
 * @param self A FullscreenManager instance.
 */
void
fullscreen_destroy_manager(FullscreenManager * self);

/**
 * Enable full screen mode.
 *
 * @param self A FullscreenManager instance.
 */
void
fullscreen_enable(FullscreenManager * self);

/**
 * Disable full screen mode.
 *
 * @param self A FullscreenManager instance.
 */
void
fullscreen_disable(FullscreenManager * self);

/**
 * This function returns true if the application is currently on full screen
 * mode (according to full screen manager).
 *
 * @param self A FullscreenManager instance.
 * @return TRUE if application is on full screen mode, else FALSE.
 */
gboolean
fullscreen_is_active(FullscreenManager * self);


#endif /* _FULLSCREENMANAGER_H_ */
