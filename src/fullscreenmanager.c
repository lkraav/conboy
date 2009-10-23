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

#ifdef HILDON_HAS_APP_MENU

#include <hildon/hildon.h>
#include <hildon/hildon-animation-actor.h>
#include <hildon/hildon-defines.h>

#include "fullscreenmanager.h"

/* Full screen mode UI related stuff: */
#define FULLSCREEN_UI_BUTTON_WIDTH          80
#define FULLSCREEN_UI_BUTTON_HEIGHT         70
#define FULLSCREEN_UI_BUTTON_OPACITY		255
#define FULLSCREEN_UI_BUTTON_HIDE_DELAY		5000
#define OFFSET 0


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

    g_return_if_fail (GTK_IS_WINDOW (aWindow));

    if (aEnable) {
        gtk_window_fullscreen (aWindow);
    } else {
        gtk_window_unfullscreen (aWindow);
    }

}

/**
* Helper function to hide full screen mode UI.
*
* @param self A FullscreenManager instance.
*/
static void
fullscreen_ui_hide(FullscreenManager * self)
{
    g_return_if_fail (FULLSCREEN_IS_MANAGER(self));

    /* Reset timer */
    g_source_remove_by_user_data((gpointer) self);

    /*hildon_animation_actor_set_parent(HILDON_ANIMATION_ACTOR(self->overlay), NULL);*/
    hildon_animation_actor_set_show(HILDON_ANIMATION_ACTOR(self->overlay), FALSE);
    self->overlay_visible = FALSE;
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
* This timer callback function hides the full screen mode UI as a result of
* the timer expiring.
*
* @param data A FullscreenManager instance.
* @return FALSE to unregister this timer callback.
*/
static gboolean
fullscreen_ui_hide_timer_cb(gpointer data)
{

    g_return_val_if_fail(data != NULL, FALSE);
    FullscreenManager *manager = (FullscreenManager *) data;

    fullscreen_ui_hide(manager);


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
    g_return_if_fail (self != NULL);
    g_return_if_fail (GTK_IS_WIDGET (self->overlay));

    /* Stop return button hide timeout */
    g_source_remove_by_user_data((gpointer) self);

    /* Only show overlay if we come here through a button release event, not a button press event */
    if (self->release_event) {

    	fullscreen_set_overlay_position(self);
        /*hildon_animation_actor_set_parent(HILDON_ANIMATION_ACTOR(self->overlay), GTK_WINDOW(self->parent_window));*/
        hildon_animation_actor_set_show(HILDON_ANIMATION_ACTOR(self->overlay), TRUE);
        self->overlay_visible = TRUE;

        /* Set the return button hide timeout */
        g_timeout_add(FULLSCREEN_UI_BUTTON_HIDE_DELAY,
                    fullscreen_ui_hide_timer_cb, (gpointer) self);
    }

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
    (void) ihint;

    FullscreenManager *self = FULLSCREEN_MANAGER (data);
    g_return_val_if_fail (self, FALSE);

    GtkWidget *widget;
    GdkEventAny *event = NULL;

    if (n_param_values >= 2)
    	widget = GTK_WIDGET(g_value_peek_pointer(&(param_values[0])));
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
        /* Find out whether or not it was a click on the overlay
         * We have to check for the coordinates on the parent window
         * because the HildonAnimationActor does not fire events at all
         */
        gdouble click_x, click_y;
        gdk_event_get_coords((GdkEvent*)event, &click_x, &click_y);

        gint trans_x, trans_y;
        gtk_widget_translate_coordinates(widget, GTK_WIDGET(self->parent_window), click_x, click_y, &trans_x, &trans_y);

        gint overlay_x = self->overlay_x;
        gint overlay_y = self->overlay_y;

        g_printerr("Clicked: %f / %f\n", click_x, click_y);
        g_printerr("Trans  : %i / %i\n", trans_x, trans_y);
        g_printerr("Overlay: %i / %i\n", overlay_x, overlay_y);

        if (self->overlay_visible) {
        	/* If the click was in the region of the overlay */
			if (click_x > overlay_x && click_x < overlay_x + FULLSCREEN_UI_BUTTON_WIDTH &&
					click_y > overlay_y && click_y < overlay_y + FULLSCREEN_UI_BUTTON_HEIGHT)
			{
				g_printerr("INFO: Overlay clicked\n");
				gtk_window_unfullscreen(self->parent_window);
			}

        }
    }

    fullscreen_ui_show(self);

    /* Does not work :( TODO: Find replacement*/
    /*g_signal_stop_emission(widget, ihint->signal_id, ihint->detail);*/

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
fullscreen_ui_enable(FullscreenManager *self)
{

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
}

static void
realize (GtkWidget *widget)
{
    GdkScreen *screen = gtk_widget_get_screen (widget);
    gtk_widget_set_colormap (widget, gdk_screen_get_rgba_colormap (screen));
}


gboolean
on_expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    cairo_t *cr;
    GdkPixbuf *pixbuf = (GdkPixbuf *) data;

    cr = gdk_cairo_create (widget->window);
    gdk_cairo_region (cr, event->region);
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    gdk_cairo_set_source_pixbuf (cr, pixbuf, 0.0, 0.0);
    cairo_paint(cr);
    cairo_destroy(cr);
    return TRUE;
}



/**
* Creates the full screen mode UI.
*
* @param manager A FullscreenManager instance.
* @return New full screen mode UI as GtkWidget pointer.
*/
static GtkWidget *
fullscreen_ui_create(FullscreenManager *self)
{
    g_return_val_if_fail(FULLSCREEN_IS_MANAGER(self), NULL);

    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file("/usr/share/icons/hicolor/scalable/hildon/fullscreen_overlay.png", NULL);
    GtkWidget *img = gtk_image_new_from_pixbuf(pixbuf);
    g_object_unref(pixbuf);

    g_signal_connect(G_OBJECT(img), "expose_event", G_CALLBACK(on_expose_event), pixbuf);

    HildonAnimationActor *actor = HILDON_ANIMATION_ACTOR(hildon_animation_actor_new());
    gtk_widget_set_size_request(GTK_WIDGET(actor), FULLSCREEN_UI_BUTTON_WIDTH, FULLSCREEN_UI_BUTTON_HEIGHT);
    gtk_container_add(GTK_CONTAINER(actor), img);
    realize (GTK_WIDGET(actor));
    gtk_widget_show_all (GTK_WIDGET(actor));

    /*hildon_animation_actor_set_opacity(actor, FULLSCREEN_UI_BUTTON_OPACITY);*/
    hildon_animation_actor_set_parent(actor, self->parent_window);

    return GTK_WIDGET(actor);
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

	return FALSE;
}


static void
fullscreen_manager_destroy (GtkWidget *parent_window, FullscreenManager *self)
{
	g_printerr("Destroy called()\n");

	g_return_if_fail (self != NULL);

    if (self->parent_window != NULL) {
        g_signal_handlers_disconnect_by_func (self->parent_window, fullscreen_manager_destroy, self);
        g_signal_handlers_disconnect_by_func (self->parent_window, fullscreen_on_toggled, self);
    }

    fullscreen_ui_disable (self);

    if (self->overlay != NULL) {
    	g_printerr("Destroying overlay\n");
		gtk_widget_destroy (GTK_WIDGET(self->overlay));
		self->overlay = NULL;
	}

}


/**
 * Called when the size allocation of the parent window changes.
 *
 * TODO: Connect to parent window again, if needed. Right now, I don't see a reason why the
 * size of the parent window should change.
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
}


/**
 * This function creates a new FullscreenManager, you have
 * to provide a parent window.
 */
FullscreenManager*
fullscreen_manager_new (GtkWindow *parent_window)
{
	g_return_val_if_fail (parent_window != NULL, NULL);
	g_return_val_if_fail (GTK_IS_WINDOW (parent_window), NULL);

    FullscreenManager *self = g_object_new (FULLSCREEN_MANAGER_TYPE, NULL);

    self->parent_window = parent_window;
    self->overlay = fullscreen_ui_create (self);

    g_signal_connect (parent_window, "destroy",
    		G_CALLBACK(fullscreen_manager_destroy), self);

    g_signal_connect (parent_window, "window-state-event",
    		G_CALLBACK(fullscreen_on_toggled), self);

    return self;
}


/*
 * GObject stuff
 */

/**
* FullscreenManager object class initialization.
*
* @param klass A FullscreenManager class object instance.
*/
static void
fullscreen_manager_class_init (FullscreenManagerClass *klass)
{
}


/**
* FullscreenManager object instance initialization.
*
* @param self A FullscreenManager instance.
*/
static void
fullscreen_manager_init (FullscreenManager *self)
{
    g_return_if_fail (self != NULL);

    self->parent_window = NULL;
    self->overlay = NULL;
    self->release_event = TRUE;
    self->last_event_time = 0;

    self->overlay_x = 0;
    self->overlay_y = 0;
    self->overlay_visible = FALSE;

    self->button_press_signal_id = 0;
    self->button_release_signal_id = 0;

    self->button_press_hook_id = 0;
	self->button_release_hook_id = 0;
}

#endif /* HILDON_HAS_APP_MENU */
