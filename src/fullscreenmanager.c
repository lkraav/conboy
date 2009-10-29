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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HILDON_HAS_APP_MENU

#include <gtk/gtk.h>
#include "fullscreenmanager.h"

/* Full screen mode UI related stuff: */
#define FULLSCREEN_UI_BUTTON_WIDTH          80
#define FULLSCREEN_UI_BUTTON_HEIGHT         70
#define FULLSCREEN_UI_BUTTON_HIDE_DELAY		5000
#define OFFSET 0


G_DEFINE_TYPE(FullscreenManager, fullscreen_manager, G_TYPE_OBJECT)


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

    if (self->overlay != NULL && GTK_IS_WIDGET(self->overlay)) {
    	gtk_widget_hide(self->overlay);
    }
}


/**
 * Changes the position of the overlay, but only if it is visible.
 */
static void
fullscreen_set_overlay_position (FullscreenManager *self)
{
	GtkWidget *parent = GTK_WIDGET(self->parent_window);
	GtkWidget *overlay = GTK_WIDGET(self->overlay);

	/* For some reason I have to call hide/show to make it appear at the new position */
	gint x = parent->allocation.width - overlay->allocation.width;
	gint y = parent->allocation.height - overlay->allocation.height - OFFSET;

	gtk_widget_hide(overlay);
	gtk_window_move(GTK_WINDOW(overlay), x, y);
	gtk_widget_show(overlay);
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
    FullscreenManager *manager = FULLSCREEN_MANAGER(data);

    fullscreen_ui_hide (manager);

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
    	gtk_widget_show (self->overlay);

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
fullscreen_ui_input_activity_hook(GSignalInvocationHint *ihint,
                                  guint n_param_values,
                                  const GValue *param_values,
                                  gpointer data)
{
    FullscreenManager *self = FULLSCREEN_MANAGER (data);
    g_return_val_if_fail (self, FALSE);

    GdkEventAny *event = NULL;

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
    }

    fullscreen_ui_show(self);

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
    g_return_if_fail (FULLSCREEN_IS_MANAGER(self));

    fullscreen_ui_hide (self);

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
change_color_map (GtkWidget *widget)
{
    GdkScreen *screen = gtk_widget_get_screen (widget);
    gtk_widget_set_colormap (widget, gdk_screen_get_rgba_colormap (screen));
}

#define RADIUS 20

static void
create_rectangle (cairo_t *ctx, double x, double y, double w, double h, double r)
{
	cairo_move_to(ctx, x+r, y);
	cairo_line_to(ctx, x+w, y);
	cairo_line_to(ctx, x+w, y+h);
	cairo_line_to(ctx, x,   y+h);
	cairo_line_to(ctx, x,   y+r);

	/* Left upper corner is rounded */
	cairo_curve_to(ctx, x, y, x, y, x+r, y);
}

static gboolean
on_expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    cairo_t *ctx;
    GdkPixbuf *pixbuf = GDK_PIXBUF(data);

    /* Create context */
    ctx = gdk_cairo_create (widget->window);

    /* Clear surface */
    cairo_set_operator (ctx, CAIRO_OPERATOR_CLEAR);
    cairo_paint (ctx);

    /* Add rectangle */
    cairo_set_operator (ctx, CAIRO_OPERATOR_OVER);
    cairo_set_source_rgba (ctx, 0, 0, 0, 0.60);
    create_rectangle (ctx, 0, 0, FULLSCREEN_UI_BUTTON_WIDTH, FULLSCREEN_UI_BUTTON_HEIGHT, RADIUS);
    cairo_fill (ctx);

    /* Add icon */
    gdk_cairo_set_source_pixbuf (ctx, pixbuf, 15, 10);
    cairo_paint (ctx);

    /* Destroy context */
    cairo_destroy (ctx);
    return TRUE;
}


gboolean
on_overlay_clicked (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	FullscreenManager *self = FULLSCREEN_MANAGER(data);
	/*gtk_window_unfullscreen(self->parent_window);*/
	if (self->callback == NULL) {
		gtk_window_unfullscreen(self->parent_window);
	} else {
		self->callback(self->parent_window);
	}
}


/**
* Creates the full screen mode UI.
*
* @param manager A FullscreenManager instance.
* @return New full screen mode UI as GtkWidget pointer.
*/
static GtkWidget*
fullscreen_ui_create(FullscreenManager *self)
{
    g_return_val_if_fail(FULLSCREEN_IS_MANAGER(self), NULL);

    /*GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file("/usr/share/icons/hicolor/scalable/hildon/fullscreen_overlay.png", NULL);*/
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file("/usr/share/icons/hicolor/48x48/hildon/general_fullsize.png", NULL);
    GtkWidget *img = gtk_image_new_from_pixbuf(pixbuf);
    gtk_widget_show(img);
    g_object_unref(pixbuf);
    g_signal_connect(img, "expose_event", G_CALLBACK(on_expose_event), pixbuf);

    GtkWidget *box = gtk_event_box_new();
    gtk_event_box_set_visible_window(GTK_EVENT_BOX(box), FALSE);
    gtk_widget_show(box);
    gtk_container_add(GTK_CONTAINER(box), img);
    g_signal_connect (box, "button-release-event", G_CALLBACK(on_overlay_clicked), self);

    GtkWidget *overlay = gtk_window_new(GTK_WINDOW_POPUP);
    gtk_window_set_decorated(GTK_WINDOW(overlay), FALSE);
    gtk_widget_set_size_request(overlay, FULLSCREEN_UI_BUTTON_WIDTH, FULLSCREEN_UI_BUTTON_HEIGHT);
    gtk_window_set_resizable(GTK_WINDOW(overlay), FALSE);
    gtk_window_set_transient_for(GTK_WINDOW(overlay), self->parent_window);
    gtk_window_set_destroy_with_parent(GTK_WINDOW(overlay), TRUE);
    gtk_container_add(GTK_CONTAINER(overlay), box);
    change_color_map (overlay);
    gtk_widget_realize(overlay);

    return overlay;
}


static gboolean
fullscreen_on_toggled (GtkWidget *widget, GdkEventWindowState *event, gpointer data)
{
	FullscreenManager *self = FULLSCREEN_MANAGER(data);

	if (event->changed_mask & GDK_WINDOW_STATE_FULLSCREEN) {

		if (event->new_window_state == GDK_WINDOW_STATE_FULLSCREEN) {
			fullscreen_ui_enable(self);
		} else {
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

    if (self->overlay != NULL && GTK_IS_WIDGET(self->overlay)) {
    	g_printerr("Destroying overlay\n");
		gtk_widget_destroy (GTK_WIDGET(self->overlay));
		self->overlay = NULL;
	}

}


/**
 * Called when the size allocation of the parent window changes.
 *
 * TODO: Why is is called so often? Looks like that only happens inside the SDK
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

    if (gdk_window_is_visible(self->overlay->window)) {
    	fullscreen_set_overlay_position(self);
    }
}


/**
 * This function creates a new FullscreenManager, you have
 * to provide a parent window.
 */
FullscreenManager*
fullscreen_manager_new (GtkWindow *parent_window, Callback callback)
{
	g_return_val_if_fail (parent_window != NULL, NULL);
	g_return_val_if_fail (GTK_IS_WINDOW (parent_window), NULL);

    FullscreenManager *self = g_object_new (FULLSCREEN_MANAGER_TYPE, NULL);

    self->parent_window = parent_window;
    self->callback = callback;
    self->overlay = fullscreen_ui_create (self);

    g_signal_connect (parent_window, "destroy",
    		G_CALLBACK(fullscreen_manager_destroy), self);

    g_signal_connect (parent_window, "window-state-event",
    		G_CALLBACK(fullscreen_on_toggled), self);

    g_signal_connect_after (parent_window, "size-allocate",
    		G_CALLBACK(ui_parent_size_allocate_cb), self);

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

    self->button_press_signal_id = 0;
    self->button_release_signal_id = 0;

    self->button_press_hook_id = 0;
	self->button_release_hook_id = 0;
}

#endif /* HILDON_HAS_APP_MENU */
