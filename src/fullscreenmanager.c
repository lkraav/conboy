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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "fullscreenmanager.h"
#include "app_data.h"

#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

/* gl stuff */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <hildon/hildon.h>
#include <hildon/hildon-remote-texture.h>

#define DEBUG 0

/* Full screen mode UI related stuff: */
#define FULLSCREEN_UI_BUTTON_WIDTH          80
#define FULLSCREEN_UI_BUTTON_HEIGHT         70
#define FULLSCREEN_UI_BUTTON_HIDE_DELAY     5000
#define FULLSCREEN_UI_BUTTON_FADE_STEP_TIME 100
#define OFFSET 75

#include <hildon/hildon-defines.h>

static GtkWidget *fullscreen_ui_create(FullscreenManager * manager);
static void fullscreen_ui_destroy(FullscreenManager * self);
static void fullscreen_ui_show(FullscreenManager * self);
static void fullscreen_ui_hide(FullscreenManager * self);
static void fullscreen_ui_enable(FullscreenManager * self);
static void fullscreen_ui_disable(FullscreenManager * self);
static gboolean fullscreen_ui_hide_timer_cb(gpointer data);
static void ui_parent_destroy_cb(FullscreenManager * self);
static void ui_parent_size_allocate_cb(GtkWidget *widget, GtkAllocation *allocation,
                                       gpointer user_data);
static gboolean fullscreen_ui_attach(FullscreenManager * self, GtkWidget * parent);
static void fullscreen_ui_detach (FullscreenManager * self);




#if DEBUG
#define DMSG_RAW(...) {fprintf(stdout, __VA_ARGS__);fflush(stdout);}
/* This fails with -Werror if DMSG is not used anywhere..
static gchar G_dmsgbuf[2048];
#define DMSG(...) {g_snprintf(G_dmsgbuf, 2048, __VA_ARGS__); DMSG_RAW(" # # # [%s:%-4d] %s\n", __FILE__, __LINE__, G_dmsgbuf); }
*/
/* NOTE: This version does not accept variables as fmt..: */
#define DMSG(fmt , ...) {fprintf(stdout, " # # # [%s:%-4d] " fmt "\n", __FILE__, __LINE__, ## __VA_ARGS__);}

#define DMSG2(...) {g_snprintf(G_dmsgbuf, 2048, __VA_ARGS__); DMSG_RAW("         %s\n", G_dmsgbuf); }
#define DMSG_FUNC_BEGIN() {DMSG_RAW("\n\n # # # [%s:%d] %s BEGIN\n", __FILE__, __LINE__, __FUNCTION__);}
#define DMSG_FUNC_END() {DMSG_RAW(" # # # [%s:%d] %s END\n", __FILE__, __LINE__, __FUNCTION__);}
#else
#define DMSG(...)
#define DMSG2(...)
#define DMSG_FUNC_BEGIN(...)
#define DMSG_FUNC_END(...)
#endif


G_DEFINE_TYPE(FullscreenManager, fullscreen_manager, G_TYPE_OBJECT)




/**
 * Enable / disable fullscreen for given GtkWindow.
 * NOTE: Always use this function to set browser fullscreen mode!
 * @param aWindow a GtkWindow instance.
 * @param aEnable TRUE of aWindow should be set to fullscreen, FALSE if normal screen.
 */
static void
set_fullscreen (GtkWindow * aWindow, gboolean aEnable)
{
    DMSG_FUNC_BEGIN();
    g_return_if_fail (GTK_IS_WINDOW (aWindow));

    if (aEnable) {
        gtk_window_fullscreen (aWindow);
    } else {
        gtk_window_unfullscreen (aWindow);
    }
    DMSG_FUNC_END();
}


/**
* FullscreenManager object instance initialization.
*
* @param self A FullscreenManager instance.
*/
void
fullscreen_manager_init(FullscreenManager * self)
{
    DMSG_FUNC_BEGIN();
    g_assert(self != NULL);

    self->parent_window = NULL;
    self->overlay = NULL;
    self->release_event = TRUE;
    self->last_event_time = 0;

    self->overlay_x = 0;
    self->overlay_y = 0;
    self->overlay_visible = FALSE;

    DMSG_FUNC_END();
}


/**
* FullscreenManager object class initialization.
*
* @param klass A FullscreenManager class object instance.
*/
void
fullscreen_manager_class_init(FullscreenManagerClass * klass)
{
    DMSG_FUNC_BEGIN();
    g_assert(klass != NULL);
    DMSG_FUNC_END();
}


static gboolean
fullscreen_on_toggled (GtkWidget *widget, GdkEventWindowState *event, gpointer data)
{
	FullscreenManager *self = FULLSCREEN_MANAGER(data);

	if (event->changed_mask & GDK_WINDOW_STATE_FULLSCREEN) {

		if (event->new_window_state == GDK_WINDOW_STATE_FULLSCREEN) {
			g_printerr("INFO: FullscreenManager: fullscreen enabled \n");
			fullscreen_ui_enable(self);
		} else {
			g_printerr("INFO: FullscreenManager: fullscreen disabled \n");
			fullscreen_ui_disable(self);
		}

	}
}


FullscreenManager *
fullscreen_create_manager(GtkWindow * view)
{
    DMSG_FUNC_BEGIN();
    g_assert(view != NULL);

    FullscreenManager *instance = g_object_new(FULLSCREEN_MANAGER_TYPE, NULL);
    g_return_val_if_fail(instance != NULL, NULL);

    instance->parent_window = view;
    instance->overlay = fullscreen_ui_create(instance);

    instance->button_press_signal_id = 0;
    instance->button_release_signal_id = 0;

    instance->button_press_hook_id = 0;
    instance->button_release_hook_id = 0;

    g_signal_connect_swapped(G_OBJECT(view),
                             "destroy",
                             G_CALLBACK(fullscreen_destroy_manager),
                             instance);

    g_signal_connect(view, "window-state-event", G_CALLBACK(fullscreen_on_toggled), instance);

    DMSG_FUNC_END();
    return instance;
}


void
fullscreen_destroy_manager(FullscreenManager * self)
{
	g_printerr("destroy_manager()\n");
    DMSG_FUNC_BEGIN();
    g_assert(self != NULL);

    if (self->parent_window != NULL) {
        g_signal_handlers_disconnect_by_func (self->parent_window, fullscreen_destroy_manager, self);
    }

    fullscreen_ui_disable (self);

    fullscreen_ui_destroy (self);
    self->overlay = NULL;

    DMSG_FUNC_END();
}




static void
fullscreen_set_overlay_position (FullscreenManager *self)
{
	GtkWidget *parent = GTK_WIDGET(self->parent_window);
	GtkWidget *overlay = GTK_WIDGET(self->overlay);

	gint x = parent->allocation.width - overlay->allocation.width;
	gint y = parent->allocation.height - overlay->allocation.height - OFFSET;

	g_printerr("Set overlay position %i / %i \n", x, y);
	hildon_animation_actor_set_position(HILDON_ANIMATION_ACTOR(overlay), x, y);

	self->overlay_x = x;
	self->overlay_y = y;
}

/**
 * Called when the size allocation of FSM UI parent changes while UI is attached.
 *
 * TODO: Why is this called so often???
 */
static void
ui_parent_size_allocate_cb (GtkWidget     *widget,
                            GtkAllocation *allocation,
                            gpointer       user_data)
{
    g_return_if_fail (widget != NULL);
    g_return_if_fail (allocation != NULL);

    FullscreenManager *self = FULLSCREEN_MANAGER(user_data);
    g_return_if_fail (self != NULL);

    GtkWidget *ui_win = GTK_WIDGET(self->overlay);
    g_return_if_fail (ui_win != NULL);

    fullscreen_set_overlay_position(self);

    /*
    gint x = allocation->width - ui_win->allocation.width;
    gint y = allocation->height - ui_win->allocation.height - OFFSET;

    g_printerr("Set position x:%i  y:%i\n", x, y);
    hildon_animation_actor_set_position(HILDON_ANIMATION_ACTOR(ui_win), x, y);
    self->overlay_x = x;
    self->overlay_y = y;
    */

}


/**
 * Helper callback that is called if a browser window is closed while
 * full screen mode UI is still attached to it.
 *
 * @param self A FullscreenManager instance.
 */
static void
ui_parent_destroy_cb(FullscreenManager * self)
{
	g_printerr("parent_destroy_cb\n");
    g_return_if_fail (self);
    g_return_if_fail (self->overlay);

    fullscreen_ui_hide (self);
}


/**
* Helper function to hide full screen mode UI.
*
* @param self A FullscreenManager instance.
*/
static void
fullscreen_ui_hide(FullscreenManager * self)
{
    DMSG_FUNC_BEGIN();
    g_return_if_fail (FULLSCREEN_IS_MANAGER(self));

    /* Reset timer */
    g_source_remove_by_user_data((gpointer) self);

    hildon_animation_actor_set_parent(HILDON_ANIMATION_ACTOR(self->overlay), NULL);
    hildon_animation_actor_set_show(HILDON_ANIMATION_ACTOR(self->overlay), FALSE);
    self->overlay_visible = FALSE;

    /*fullscreen_ui_detach (self);*/

    DMSG_FUNC_END();
}


/**
* This timer callback function hides the full screen mode UI as a result of
* the timer expiring.
*
* @param data A FullscreenManager instance.
* @return FALSE to unregister this timer callback.
*/
static gboolean
fullscreen_ui_hide_timer_cb(gpointer data)
{
    DMSG_FUNC_BEGIN();
    g_return_val_if_fail(data != NULL, FALSE);
    FullscreenManager *manager = (FullscreenManager *) data;

    fullscreen_ui_hide(manager);

    DMSG_FUNC_END();
    return FALSE;
}



/**
* This function makes the full screen mode UI visible if it isn't already.
* Fade or hide timers are also reset.
*
* @param data A FullscreenManager instance.
*/
static void
fullscreen_ui_show(FullscreenManager * self)
{
	g_printerr("ui_show()\n");
    DMSG_FUNC_BEGIN();
    g_return_if_fail (self != NULL);
    g_return_if_fail (GTK_IS_WIDGET (self->overlay));

    /* Stop return button hide timeout */
    g_source_remove_by_user_data((gpointer) self);

    /* Only show overlay if we come here through a button release event, not a button press event */
    if (self->release_event) {

        /*fullscreen_ui_attach(self, GTK_WIDGET(self->parent_window));*/
    	g_printerr("Make visible\n");

    	fullscreen_set_overlay_position(self);

        hildon_animation_actor_set_parent(HILDON_ANIMATION_ACTOR(self->overlay), GTK_WINDOW(self->parent_window));
        hildon_animation_actor_set_show(HILDON_ANIMATION_ACTOR(self->overlay), TRUE);
        self->overlay_visible = TRUE;

        /* Set the return button hide timeout */
        g_timeout_add(FULLSCREEN_UI_BUTTON_HIDE_DELAY,
                    fullscreen_ui_hide_timer_cb, (gpointer) self);
    }

    DMSG_FUNC_END();
}


/**
* Input activity hook function.
*
* This hook function is called as a result of mouse or key input event
* emissions on parent window. See GSignalEmissionHook.
*
* @param ihint Signal invocation hint.
* @param n_param_values number of parameters.
* @param param_values instance on which the signal was emitted, followed by the parameters of the emission.
* @param data A FullscreenManager instance.
*/
static gboolean
fullscreen_ui_input_activity_hook(GSignalInvocationHint * ihint,
                                  guint n_param_values,
                                  const GValue * param_values,
                                  gpointer data)
{
    DMSG_FUNC_BEGIN();
    (void) ihint;

    FullscreenManager *self = FULLSCREEN_MANAGER (data);
    g_return_val_if_fail (self, FALSE);

    g_printerr("*** Icon is visible: %i\n", self->overlay_visible);

    GdkEventAny * event = NULL;
    if (n_param_values >= 2)
        event = (GdkEventAny*)g_value_peek_pointer(&(param_values[1]));
    g_return_val_if_fail (event, TRUE);

    guint32 time = 0;
    switch (event->type) {
        case GDK_BUTTON_PRESS:
        case GDK_BUTTON_RELEASE:
            time = ((GdkEventButton*)event)->time;
            break;
        case GDK_KEY_PRESS:
        case GDK_KEY_RELEASE:
            time = ((GdkEventKey*)event)->time;
            break;
        default:
            /* Filter out unexpected messages */
            return TRUE;
    }

    /* We're likely to get events multiple times as they're propagated, so
       filter out events that we've already seen. */
    if (time == self->last_event_time) {
        return TRUE;
    }
    self->last_event_time = time;

    if (event && (event->type == GDK_BUTTON_PRESS)) {
        self->release_event = FALSE;
    } else {
        self->release_event = TRUE;

        /* button was released */
        /* Find out whether or not it was a click on our overlay */
        gdouble click_x, click_y;
        gdk_event_get_coords((GdkEvent*)event, &click_x, &click_y);


        gint overlay_x = self->overlay_x;
        gint overlay_y = self->overlay_y;


        if (self->overlay_visible) {

			if (click_x > overlay_x && click_x < overlay_x + FULLSCREEN_UI_BUTTON_WIDTH &&
					click_y > overlay_y && click_y < overlay_y + FULLSCREEN_UI_BUTTON_HEIGHT)
			{
				g_printerr("############# BUTTON CLICKED ####################\n");
				gtk_window_unfullscreen(self->parent_window);
			}

        }


    }

    fullscreen_ui_show(self);

    DMSG_FUNC_END();
    return TRUE;
}



/**
* Enable full screen mode UI.
*
* This function makes the full screen mode UI visible and hooks mouse and
* key event signal emissions. The UI is hidden after predetermined time and
* is reshown when ever aforementioned signal hooks are activated.
*
* @param self A FullscreenManager instance.
*/
static void
fullscreen_ui_enable(FullscreenManager * self)
{
    DMSG_FUNC_BEGIN();
    g_return_if_fail(FULLSCREEN_IS_MANAGER(self));

    if (self->button_press_hook_id == 0) {
        self->button_press_signal_id =
            g_signal_lookup("button-press-event", GTK_TYPE_WIDGET);
        self->button_press_hook_id =
            g_signal_add_emission_hook(self->button_press_signal_id, 0,
                                       fullscreen_ui_input_activity_hook,
                                       (gpointer) self, NULL);
    }

    if (self->button_release_hook_id == 0) {
        self->button_release_signal_id =
            g_signal_lookup("button-release-event", GTK_TYPE_WIDGET);
        self->button_release_hook_id =
            g_signal_add_emission_hook(self->button_release_signal_id, 0,
                                       fullscreen_ui_input_activity_hook,
                                       (gpointer) self, NULL);
    }

    fullscreen_ui_show(self);
    DMSG_FUNC_END();
}


/**
* Disable full screen mode UI.
*
* This function hides the full screen UI (if visible) and releases mouse and
* key event signal emission hooks.
*
* @param self A FullscreenManager instance.
*/
static void
fullscreen_ui_disable(FullscreenManager * self)
{
    DMSG_FUNC_BEGIN();
    g_return_if_fail(FULLSCREEN_IS_MANAGER(self));

    fullscreen_ui_hide(self);

    if (self->button_release_hook_id > 0) {
        g_signal_remove_emission_hook(self->button_release_signal_id,
                                      self->button_release_hook_id);
        self->button_release_hook_id = 0;
    }

    if (self->button_press_hook_id > 0) {
        g_signal_remove_emission_hook(self->button_press_signal_id,
                                      self->button_press_hook_id);
        self->button_press_hook_id = 0;
    }

    DMSG_FUNC_END();
}


/**
* Creates the full screen mode UI.
*
* @param manager A FullscreenManager instance.
* @return New full screen mode UI as GtkWidget pointer.
*/
static GtkWidget *
fullscreen_ui_create(FullscreenManager * manager)
{
    DMSG_FUNC_BEGIN();
    g_return_val_if_fail(FULLSCREEN_IS_MANAGER(manager), NULL);

    /* Create texture */

    GtkWidget *img = gtk_image_new_from_file("/usr/share/icons/hicolor/48x48/hildon/general_fullsize.png");
    gtk_widget_show(img);

    HildonAnimationActor *actor = HILDON_ANIMATION_ACTOR(hildon_animation_actor_new());
    gtk_widget_set_size_request(GTK_WIDGET(actor), FULLSCREEN_UI_BUTTON_WIDTH, FULLSCREEN_UI_BUTTON_HEIGHT);

    GdkColor color;
    gdk_color_parse("#f00", &color);
    gtk_widget_modify_bg(GTK_WIDGET(actor), GTK_STATE_NORMAL, &color);
    gtk_widget_show(GTK_WIDGET(actor));

    gtk_container_add(GTK_CONTAINER(actor), img);


    hildon_animation_actor_set_opacity(actor, 127);

    DMSG_FUNC_END();

    return GTK_WIDGET(actor);
}


/**
 * Destroy full screen mode UI.
 *
 * @param self A FullscreenManager instance.
 */
static void
fullscreen_ui_destroy (FullscreenManager * self)
{
    DMSG_FUNC_BEGIN();
    g_return_if_fail (self != NULL);

    if (self->overlay != NULL) {
        gtk_widget_destroy (GTK_WIDGET(self->overlay));
        self->overlay = NULL;
    }

    DMSG_FUNC_END();
}


/**
 * Attach full screen mode UI (i.e. return to normal mode button) to a
 * parent widget.
 *
 * @param self A FullscreenManager instance.
 * @param parent The parent widget.
 * @return TRUE if attaching succeeded, else FALSE.
 */
static gboolean
fullscreen_ui_attach (FullscreenManager * self, GtkWidget * parent)
{
	g_printerr("ui_attach\n");
    DMSG_FUNC_BEGIN();
    g_return_val_if_fail (FULLSCREEN_IS_MANAGER (self), FALSE);
    g_return_val_if_fail (GTK_IS_WIDGET (parent), FALSE);

    if (!GTK_WIDGET_REALIZED (parent)) {
        gtk_widget_realize (parent);
        g_return_val_if_fail (GTK_WIDGET_REALIZED (parent), FALSE);
    }

    if (self->overlay == NULL) {
        self->overlay = fullscreen_ui_create (self);
        g_return_val_if_fail (self->overlay != NULL, FALSE);
    }

    GtkWidget * wid = GTK_WIDGET(self->overlay);
    g_return_val_if_fail (wid != NULL, FALSE);

    gint x = parent->allocation.width - wid->allocation.width;
    gint y = parent->allocation.height - wid->allocation.height + OFFSET;


    hildon_animation_actor_set_parent(HILDON_ANIMATION_ACTOR(wid), GTK_WINDOW(parent));
    self->overlay_visible = TRUE;

    hildon_animation_actor_set_position(HILDON_ANIMATION_ACTOR(wid), x, y);
    self->overlay_x = x;
    self->overlay_y = y;


    /* Make sure that FSM ui is detached from the parent window before it is
       destroyed */

    g_signal_connect_swapped(G_OBJECT(parent),
                             "destroy",
                             G_CALLBACK(ui_parent_destroy_cb),
                             self);

    g_signal_connect (G_OBJECT(parent),
                      "size-allocate",
                      G_CALLBACK(ui_parent_size_allocate_cb),
                      (gpointer)self);

    DMSG_FUNC_END();
    return TRUE;
}


/**
 * Detach full screen mode UI from the current parent.
 *
 * @param self A FullscreenManager instance.
 */
static void
fullscreen_ui_detach (FullscreenManager * self)
{
    DMSG_FUNC_BEGIN();
    g_return_if_fail (FULLSCREEN_IS_MANAGER (self));
    g_return_if_fail (self->overlay != NULL);

    GtkWidget * ui_wid = GTK_WIDGET (self->overlay);
    g_return_if_fail (ui_wid != NULL);

    hildon_animation_actor_set_parent(HILDON_ANIMATION_ACTOR(self->overlay), NULL);
    self->overlay_visible = FALSE;

	/* Disconnect "size-allocate" and "destroy" signals */
	g_signal_handlers_disconnect_by_func (G_OBJECT(self->parent_window),
										  G_CALLBACK(ui_parent_size_allocate_cb),
										  (gpointer)ui_wid);

	g_signal_handlers_disconnect_by_func (G_OBJECT(self->parent_window),
										  G_CALLBACK(ui_parent_destroy_cb),
										  (gpointer)self);


    DMSG_FUNC_END();
}
