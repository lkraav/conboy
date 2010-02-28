/*
 * This file is a part of hildon-extras
 *
 * Copyright (C) 2009 Cornelius Hald <hald@icandy.de>
 *
 * Almost completely based on code by:
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

/**
 * SECTION:he-check-button
 * @short_description: Two-label check buttons in Hildon-Extras
 *
 * #HeCheckButton is a clickable button for Hildon applications. It is
 * derived from the #GtkButton widget and provides additional
 * commodities specific to the Hildon framework.
 *
 * The height of a #HeCheckButton can be set to either "finger" height
 * or "thumb" height. It can also be configured to use halfscreen or
 * fullscreen width. Alternatively, either dimension can be set to
 * "auto" so it behaves like a standard #GtkButton.
 *
 * A #HeCheckButton can hold any valid child widget, but it usually
 * contains two labels, named title and value. The contents of the button
 * are packed together inside a #GtkAlignment and they do not expand by
 * default (they don't use the full space of the button).
 *
 * To change the alignment of both labels, use gtk_button_set_alignment()
 *
 * To make them expand and use the full space of the button, use
 * he_check_button_set_alignment().
 *
 * To change the relative alignment of each label, use
 * he_check_button_set_title_alignment() and
 * he_check_button_set_value_alignment().
 *
 * <example>
 * <title>Creating a HeCheckButton</title>
 * <programlisting>
 * void
 * button_clicked (HeCheckButton *button, gpointer user_data)
 * {
 *     const gchar *title, *value;
 * <!-- -->
 *     title = he_check_button_get_title (button);
 *     value = he_check_button_get_value (button);
 *     g_debug ("Button clicked with title '&percnt;s' and value '&percnt;s'", title, value);
 * }
 * <!-- -->
 * GtkWidget *
 * create_button (void)
 * {
 *     GtkWidget *button;
 *     GtkWidget *image;
 * <!-- -->
 *     button = he_check_button_new (HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT,
 *                                 HE_CHECK_BUTTON_ARRANGEMENT_VERTICAL);
 *     he_check_button_set_text (HE_CHECK_BUTTON (button), "Some title", "Some value");
 * <!-- -->
 *     image = gtk_image_new_from_stock (GTK_STOCK_INFO, GTK_ICON_SIZE_BUTTON);
 *     he_check_button_set_image (HE_CHECK_BUTTON (button), image);
 *     he_check_button_set_image_position (HE_CHECK_BUTTON (button), GTK_POS_RIGHT);
 * <!-- -->
 *     gtk_button_set_alignment (GTK_BUTTON (button), 0.0, 0.5);
 * <!-- -->
 *     g_signal_connect (button, "clicked", G_CALLBACK (button_clicked), NULL);
 * <!-- -->
 *     return button;
 * }
 * </programlisting>
 * </example>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef HILDON_HAS_APP_MENU

#include                                        <stdlib.h>
#include                                        <hildon/hildon-button.h>
#include                                        <hildon/hildon-check-button.h>
#include                                        <hildon/hildon-enum-types.h>
#include                                        <hildon/hildon-gtk.h>
#include                                        "he-check-button.h"

enum {
	TOGGLED,
	LAST_SIGNAL
};

static guint									signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE                                   (HeCheckButton, he_check_button, GTK_TYPE_BUTTON);

#define                                         HE_CHECK_BUTTON_GET_PRIVATE(obj) \
                                                (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
                                                HE_TYPE_CHECK_BUTTON, HeCheckButtonPrivate));

struct                                          _HeCheckButtonPrivate
{
    GtkLabel *title;
    GtkLabel *value;
    GtkBox *hbox;
    GtkWidget *label_box;
    GtkWidget *alignment;
    GtkCellRendererToggle *toggle_renderer;
    GtkWidget *cell_view;
    HeCheckButtonStyle style;
    guint setting_style : 1;
};

enum {
    PROP_TITLE = 1,
    PROP_VALUE,
    PROP_SIZE,
    PROP_ARRANGEMENT,
    PROP_STYLE,
};

/**
 * he_check_button_toggled:
 * @button: A #HeCheckButton
 *
 * Emits the #HeCheckButton::toggled signal on the #HeCheckButton.
 * There is no good reason for an application ever to call this function.
 *
 * Since: 2.2
 */
void
he_check_button_toggled                     (HeCheckButton *button)
{
    g_return_if_fail (HE_IS_CHECK_BUTTON (button));

    g_signal_emit (button, signals[TOGGLED], 0);
}


/**
 * he_check_button_set_active:
 * @button: A #HeCheckButton
 * @is_active: new state for the button
 *
 * Sets the status of a #HeCheckButton. Set to %TRUE if you want
 * @button to be 'pressed-in', and %FALSE to raise it. This action
 * causes the #HeCheckButton::toggled signal to be emitted.
 *
 * Since: 2.2
 **/
void
he_check_button_set_active                  (HeCheckButton *button,
                                                 gboolean           is_active)
{
    gboolean prev_is_active;

    g_return_if_fail (HE_IS_CHECK_BUTTON (button));

    prev_is_active = he_check_button_get_active (button);

    if (prev_is_active != is_active) {
        gtk_button_clicked (GTK_BUTTON (button));
        gtk_widget_queue_draw (GTK_WIDGET (button));
    }
}

/**
 * he_check_button_get_active:
 * @button: A #HeCheckButton
 *
 * Gets the current state of @button.
 *
 * Return value: %TRUE if @button is active, %FALSE otherwise.
 *
 * Since: 2.2
 **/
gboolean
he_check_button_get_active                  (HeCheckButton *button)
{
    g_return_val_if_fail (HE_IS_CHECK_BUTTON (button), FALSE);

    return gtk_cell_renderer_toggle_get_active (button->priv->toggle_renderer);
}

static void
he_check_button_set_arrangement                   (HeCheckButton            *button,
                                                 HeCheckButtonArrangement  arrangement);

static void
he_check_button_construct_child                   (HeCheckButton *button);

static void
he_check_button_set_property                      (GObject      *object,
                                                 guint         prop_id,
                                                 const GValue *value,
                                                 GParamSpec   *pspec)
{
    HeCheckButton *button = HE_CHECK_BUTTON (object);

    switch (prop_id)
    {
    case PROP_TITLE:
        he_check_button_set_title (button, g_value_get_string (value));
        break;
    case PROP_VALUE:
        he_check_button_set_value (button, g_value_get_string (value));
        break;
    case PROP_SIZE:
        hildon_gtk_widget_set_theme_size (GTK_WIDGET (button), g_value_get_flags (value));
        break;
    case PROP_ARRANGEMENT:
        he_check_button_set_arrangement (button, g_value_get_enum (value));
        break;
    case PROP_STYLE:
        he_check_button_set_style (button, g_value_get_enum (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
he_check_button_get_property                      (GObject    *object,
                                                 guint       prop_id,
                                                 GValue     *value,
                                                 GParamSpec *pspec)
{
    HeCheckButton *button = HE_CHECK_BUTTON (object);

    switch (prop_id)
    {
    case PROP_TITLE:
        g_value_set_string (value, he_check_button_get_title (button));
        break;
    case PROP_VALUE:
        g_value_set_string (value, he_check_button_get_value (button));
        break;
    case PROP_STYLE:
        g_value_set_enum (value, he_check_button_get_style (button));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
set_logical_font                                (GtkWidget *button)
{
    HeCheckButtonPrivate *priv = HE_CHECK_BUTTON_GET_PRIVATE (button);

    /* In buttons with vertical arrangement, the 'value' label uses a
     * different font */
    if (GTK_IS_VBOX (priv->label_box)) {
        GtkStyle *style = gtk_rc_get_style_by_paths (
            gtk_settings_get_default (), "SmallSystemFont", NULL, G_TYPE_NONE);
        if (style != NULL) {
            PangoFontDescription *font_desc = style->font_desc;
            if (font_desc != NULL) {
                priv->setting_style = TRUE;
                gtk_widget_modify_font (GTK_WIDGET (priv->value), font_desc);
                priv->setting_style = FALSE;
            }
        }
    }
}

static void
set_logical_color                               (GtkWidget *button)
{
    GdkColor color;
    const gchar *colorname;
    HeCheckButtonPrivate *priv = HE_CHECK_BUTTON_GET_PRIVATE (button);
    GtkWidget *label = GTK_WIDGET (priv->value);

    switch (priv->style) {
    case HE_CHECK_BUTTON_STYLE_NORMAL:
        colorname = "SecondaryTextColor";
        break;
    case HE_CHECK_BUTTON_STYLE_PICKER:
        colorname = "ActiveTextColor";
        break;
    default:
        g_return_if_reached ();
    }

    gtk_widget_ensure_style (label);
    if (gtk_style_lookup_color (label->style, colorname, &color) == TRUE) {
        priv->setting_style = TRUE;
        gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
        gtk_widget_modify_fg (label, GTK_STATE_PRELIGHT, &color);
        priv->setting_style = FALSE;
    }
}

static gint
gtk_rc_properties_cmp (gconstpointer bsearch_node1,
		       gconstpointer bsearch_node2)
{
  const GtkRcProperty *prop1 = bsearch_node1;
  const GtkRcProperty *prop2 = bsearch_node2;

  if (prop1->type_name == prop2->type_name)
    return prop1->property_name < prop2->property_name ? -1 : prop1->property_name == prop2->property_name ? 0 : 1;
  else
    return prop1->type_name < prop2->type_name ? -1 : 1;
}

/*
 * Returns a style property if it exists and if it is a long. If not, 0 is returned.
 * Its a dirty hack. The proper way would be using gtk_style_get() which is only
 * available with Gtk+ >= 2.16
 */
static glong
get_style_property_long                            (GtkStyle    *style,
                                                    GType        widget_type,
                                                    const gchar *property_name)
{
	GtkRcProperty *rcprop = NULL;
	GtkWidgetClass *klass = g_type_class_ref (widget_type);
	GParamSpec *pspec = gtk_widget_class_find_style_property (klass, property_name);

	/* Style must be rc style */
	g_return_val_if_fail(style->rc_style != NULL, 0);

	if (!pspec) {
		g_printerr("WARN: get_style_property_long: Property '%s' not found\n", property_name);
		return 0;
	}

	if (style->rc_style->rc_properties) {
		GtkRcProperty key;
		key.type_name = g_type_qname(widget_type);
		key.property_name = g_quark_from_string(pspec->name);
		/* Search all properties for given key */
		rcprop = bsearch (&key, style->rc_style->rc_properties->data, style->rc_style->rc_properties->len,
				sizeof(GtkRcProperty), gtk_rc_properties_cmp);
	}

	if (rcprop == NULL) {
		g_printerr("WARN: get_style_property_long: RcProperty '%s' not found\n", property_name);
		return 0;
	}

	if (G_VALUE_HOLDS_LONG(&(rcprop->value))) {
		return g_value_get_long(&(rcprop->value));
	} else {
		g_printerr("WARN: get_style_property_long: Property '%s' is not a long\n", property_name);
		return 0;
	}
}

static void
he_check_button_style_set                         (GtkWidget *widget,
                                                 GtkStyle  *previous_style)
{
    guint horizontal_spacing, vertical_spacing, image_spacing;
    HeCheckButtonPrivate *priv = HE_CHECK_BUTTON_GET_PRIVATE (widget);

    if (GTK_WIDGET_CLASS (he_check_button_parent_class)->style_set)
        GTK_WIDGET_CLASS (he_check_button_parent_class)->style_set (widget, previous_style);

    /* Prevent infinite recursion when calling set_logical_font() and
     * set_logical_color() */
    if (priv->setting_style)
        return;

    /* Get horizontal-spacing style property from ourself */
    gtk_widget_style_get (widget, "horizontal-spacing", &horizontal_spacing, NULL);

    /* Get vertical-spacing style property of HildonButton from theme */
    GtkStyle *style = gtk_rc_get_style_by_paths (gtk_widget_get_settings (widget),
    		"*.*Button-finger", NULL, HILDON_TYPE_BUTTON);
    vertical_spacing = get_style_property_long (style, HILDON_TYPE_BUTTON, "vertical-spacing");

    /* Get image-spacing style property of HildonCheckButton from theme */
    style = gtk_rc_get_style_by_paths (gtk_widget_get_settings (widget),
    		NULL, "*.HildonCheckButton", HILDON_TYPE_CHECK_BUTTON);
    image_spacing = get_style_property_long (style, GTK_TYPE_BUTTON, "image-spacing");

    /* Setting values we got from above */
    if (GTK_IS_HBOX (priv->label_box)) {
        gtk_box_set_spacing (GTK_BOX (priv->label_box), horizontal_spacing);
    } else {
        gtk_box_set_spacing (GTK_BOX (priv->label_box), vertical_spacing);
    }

    if (GTK_IS_BOX (priv->hbox)) {
        gtk_box_set_spacing (priv->hbox, image_spacing);
    }

    set_logical_font (widget);
    set_logical_color (widget);
}

static void
he_check_button_finalize                          (GObject *object)
{
    HeCheckButtonPrivate *priv = HE_CHECK_BUTTON_GET_PRIVATE (object);

    g_object_unref (priv->alignment);
    g_object_unref (priv->label_box);

    G_OBJECT_CLASS (he_check_button_parent_class)->finalize (object);
}

static void
he_check_button_clicked                     (GtkButton *button)
{
    HeCheckButton *checkbutton = HE_CHECK_BUTTON (button);
    gboolean current = he_check_button_get_active (checkbutton);

    gtk_cell_renderer_toggle_set_active (checkbutton->priv->toggle_renderer, !current);

    he_check_button_toggled (checkbutton);
}

static void
he_check_button_class_init                        (HeCheckButtonClass *klass)
{
    GObjectClass *gobject_class = (GObjectClass *)klass;
    GtkWidgetClass *widget_class = (GtkWidgetClass *)klass;
    GtkButtonClass *button_class = (GtkButtonClass *)klass;

    gobject_class->set_property = he_check_button_set_property;
    gobject_class->get_property = he_check_button_get_property;
    gobject_class->finalize = he_check_button_finalize;
    widget_class->style_set = he_check_button_style_set;
    button_class->clicked = he_check_button_clicked;

    klass->toggled = NULL;

    /**
     * HeCheckButton::toggled
     *
     * Emitted when the #HeCheckButton's state is changed.
     *
     * Since: 2.2
     */
    signals[TOGGLED] =
        g_signal_new ("toggled",
                      G_OBJECT_CLASS_TYPE (gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (HeCheckButtonClass, toggled),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    g_object_class_install_property (
        gobject_class,
        PROP_TITLE,
        g_param_spec_string (
            "title",
            "Title",
            "Text of the title label inside the button",
            NULL,
            G_PARAM_READWRITE));

    g_object_class_install_property (
        gobject_class,
        PROP_VALUE,
        g_param_spec_string (
            "value",
            "Value",
            "Text of the value label inside the button",
            NULL,
            G_PARAM_READWRITE));

    g_object_class_install_property (
        gobject_class,
        PROP_SIZE,
        g_param_spec_flags (
            "size",
            "Size",
            "Size request for the button",
            HILDON_TYPE_SIZE_TYPE,
            HILDON_SIZE_AUTO,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (
        gobject_class,
        PROP_ARRANGEMENT,
        g_param_spec_enum (
            "arrangement",
            "Arrangement",
            "How the button contents must be arranged",
            HILDON_TYPE_BUTTON_ARRANGEMENT,
            HE_CHECK_BUTTON_ARRANGEMENT_HORIZONTAL,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (
        gobject_class,
        PROP_STYLE,
        g_param_spec_enum (
            "style",
            "Style",
            "Visual style of the button",
            HILDON_TYPE_BUTTON_STYLE,
            HE_CHECK_BUTTON_STYLE_NORMAL,
            G_PARAM_READWRITE));

    gtk_widget_class_install_style_property (
        widget_class,
        g_param_spec_uint (
            "horizontal-spacing",
            "Horizontal spacing between labels",
            "Horizontal spacing between the title and value labels, when in horizontal mode",
            0, G_MAXUINT, 25,
            G_PARAM_READABLE));

    gtk_widget_class_install_style_property (
        widget_class,
        g_param_spec_uint (
            "vertical-spacing",
            "Vertical spacing between labels",
            "Vertical spacing between the title and value labels, when in vertical mode",
            0, G_MAXUINT, 5,
            G_PARAM_READABLE));

    g_type_class_add_private (klass, sizeof (HeCheckButtonPrivate));
}

static void
he_check_button_init                              (HeCheckButton *self)
{
    HeCheckButtonPrivate *priv = HE_CHECK_BUTTON_GET_PRIVATE (self);

    /* Store private part */
    self->priv = priv;

    priv->title = GTK_LABEL (gtk_label_new (NULL));
    priv->value = GTK_LABEL (gtk_label_new (NULL));
    priv->alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
    priv->toggle_renderer = GTK_CELL_RENDERER_TOGGLE (gtk_cell_renderer_toggle_new ());
    priv->cell_view = gtk_cell_view_new ();
    priv->hbox = NULL;
    priv->label_box = NULL;
    priv->style = HE_CHECK_BUTTON_STYLE_NORMAL;
    priv->setting_style = FALSE;

    /* Setup the cell renderer */
	/* We need to set the correct style from the gtkrc file. Otherwise the check box
	 * does not look like a check box on a HildonCheckButton. */
	GtkStyle *style = gtk_rc_get_style_by_paths(gtk_widget_get_settings(GTK_WIDGET(self)),
			NULL, "*.HildonCheckButton.GtkAlignment.GtkHBox.GtkCellView", G_TYPE_NONE);
	gtk_widget_set_style(priv->cell_view, style);

	/* Make sure that the check box is always shown, no matter the value of gtk-button-images */
	g_signal_connect (priv->cell_view, "notify::visible", G_CALLBACK (gtk_widget_show), NULL);

	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (priv->cell_view),
								GTK_CELL_RENDERER (priv->toggle_renderer), TRUE);

	/* Get checkbox-size style property of HildonCheckButton from theme */
	style = gtk_rc_get_style_by_paths (gtk_widget_get_settings (GTK_WIDGET(self)),
			NULL, "*.HildonCheckButton", HILDON_TYPE_CHECK_BUTTON);
	glong checkbox_size = get_style_property_long (style, HILDON_TYPE_CHECK_BUTTON, "checkbox-size");

	/* Set the indicator to the right size (the size of the pixmap) */
	g_object_set (priv->toggle_renderer, "indicator-size", checkbox_size, NULL);

	/* Setup the labels */
    gtk_widget_set_name (GTK_WIDGET (priv->title), "hildon-button-title");
    gtk_widget_set_name (GTK_WIDGET (priv->value), "hildon-button-value");

    he_check_button_set_style (self, HE_CHECK_BUTTON_STYLE_NORMAL);

    gtk_misc_set_alignment (GTK_MISC (priv->title), 0, 0.5);
    gtk_misc_set_alignment (GTK_MISC (priv->value), 0, 0.5);

    g_object_ref_sink (priv->alignment);

    /* The labels are not shown automatically, see he_check_button_set_(title|value) */
    gtk_widget_set_no_show_all (GTK_WIDGET (priv->title), TRUE);
    gtk_widget_set_no_show_all (GTK_WIDGET (priv->value), TRUE);

    gtk_button_set_focus_on_click (GTK_BUTTON (self), FALSE);
}

/**
 * he_check_button_add_title_size_group:
 * @button: a #HeCheckButton
 * @size_group: A #GtkSizeGroup for the button title (main label)
 *
 * Adds the title label of @button to @size_group.
 *
 * Since: 2.2
 **/
void
he_check_button_add_title_size_group              (HeCheckButton *button,
                                                 GtkSizeGroup *size_group)
{
    HeCheckButtonPrivate *priv;

    g_return_if_fail (HE_IS_CHECK_BUTTON (button));
    g_return_if_fail (GTK_IS_SIZE_GROUP (size_group));

    priv = HE_CHECK_BUTTON_GET_PRIVATE (button);

    gtk_size_group_add_widget (size_group, GTK_WIDGET (priv->title));
}

/**
 * he_check_button_add_value_size_group:
 * @button: a #HeCheckButton
 * @size_group: A #GtkSizeGroup for the button value (secondary label)
 *
 * Adds the value label of @button to @size_group.
 *
 * Since: 2.2
 **/
void
he_check_button_add_value_size_group              (HeCheckButton *button,
                                                 GtkSizeGroup *size_group)
{
    HeCheckButtonPrivate *priv;

    g_return_if_fail (HE_IS_CHECK_BUTTON (button));
    g_return_if_fail (GTK_IS_SIZE_GROUP (size_group));

    priv = HE_CHECK_BUTTON_GET_PRIVATE (button);

    gtk_size_group_add_widget (size_group, GTK_WIDGET (priv->value));
}

/**
 * he_check_button_add_size_groups:
 * @button: a #HeCheckButton
 * @title_size_group: A #GtkSizeGroup for the button title (main label), or %NULL
 * @value_size_group: A #GtkSizeGroup group for the button value (secondary label), or %NULL
 * @image_size_group: A #GtkSizeGroup group for the button image, or %NULL
 *
 * Convenience function to add title, value and image to size
 * groups. %NULL size groups will be ignored.
 *
 * Since: 2.2
 **/
void
he_check_button_add_size_groups                   (HeCheckButton *button,
                                                 GtkSizeGroup *title_size_group,
                                                 GtkSizeGroup *value_size_group,
                                                 GtkSizeGroup *image_size_group)
{
    if (title_size_group)
        he_check_button_add_title_size_group (button, title_size_group);

    if (value_size_group)
        he_check_button_add_value_size_group (button, value_size_group);

}

/**
 * he_check_button_new:
 * @size: Flags to set the size of the button.
 * @arrangement: How the labels must be arranged.
 *
 * Creates a new #HeCheckButton. To set text in the labels, use
 * he_check_button_set_title() and
 * he_check_button_set_value(). Alternatively, you can add a custom
 * child widget using gtk_container_add().
 *
 * Returns: a new #HeCheckButton
 *
 * Since: 2.2
 **/
GtkWidget *
he_check_button_new                               (HildonSizeType          size,
                                                 HeCheckButtonArrangement arrangement)
{
    return he_check_button_new_with_text (size, arrangement, NULL, NULL);
}

/**
 * he_check_button_new_with_text:
 * @size: Flags to set the size of the button.
 * @arrangement: How the labels must be arranged.
 * @title: Title of the button (main label), or %NULL
 * @value: Value of the button (secondary label), or %NULL
 *
 * Creates a new #HeCheckButton with two labels, @title and @value.
 *
 * If you just don't want to use one of the labels, set it to
 * %NULL. You can set it to a non-%NULL value at any time later using
 * he_check_button_set_title() or he_check_button_set_value() .
 *
 * Returns: a new #HeCheckButton
 *
 * Since: 2.2
 **/
GtkWidget *
he_check_button_new_with_text                     (HildonSizeType           size,
                                                 HeCheckButtonArrangement  arrangement,
                                                 const gchar             *title,
                                                 const gchar             *value)
{
    GtkWidget *button;

    /* Create widget */
    button = g_object_new (HE_TYPE_CHECK_BUTTON,
                           "size", size,
                           "title", title,
                           "value", value,
                           "arrangement", arrangement,
                           NULL);

    return button;
}

static void
he_check_button_set_arrangement                   (HeCheckButton            *button,
                                                 HeCheckButtonArrangement  arrangement)
{
    HeCheckButtonPrivate *priv;

    priv = HE_CHECK_BUTTON_GET_PRIVATE (button);

    /* Pack everything */
    if (arrangement == HE_CHECK_BUTTON_ARRANGEMENT_VERTICAL) {
        priv->label_box = gtk_vbox_new (FALSE, 0);
        set_logical_font (GTK_WIDGET (button));
    } else {
        priv->label_box = gtk_hbox_new (FALSE, 0);
    }

    g_object_ref_sink (priv->label_box);

    /* If we pack both labels with (TRUE, TRUE) or (FALSE, FALSE) they
     * can be painted outside of the button in some situations, see
     * NB#88126 and NB#110689 */
    gtk_box_pack_start (GTK_BOX (priv->label_box), GTK_WIDGET (priv->title), FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (priv->label_box), GTK_WIDGET (priv->value), TRUE, TRUE, 0);

    he_check_button_construct_child (button);
}

/**
 * he_check_button_set_title:
 * @button: a #HeCheckButton
 * @title: a new title (main label) for the button, or %NULL
 *
 * Sets the title (main label) of @button to @title.
 *
 * This will clear any previously set title.
 *
 * If @title is set to %NULL or an empty string, the title label will
 * be hidden and the value label will be realigned.
 *
 * Since: 2.2
 **/
void
he_check_button_set_title                         (HeCheckButton *button,
                                                 const gchar  *title)
{
    HeCheckButtonPrivate *priv;

    g_return_if_fail (HE_IS_CHECK_BUTTON (button));

    priv = HE_CHECK_BUTTON_GET_PRIVATE (button);
    gtk_label_set_text (priv->title, title);

    /* If the button has no title, hide the label so the value is
     * properly aligned */
    if (title && title[0] != '\0') {
        he_check_button_construct_child (button);
        gtk_widget_show (GTK_WIDGET (priv->title));
    } else {
        gtk_widget_hide (GTK_WIDGET (priv->title));
    }

    g_object_notify (G_OBJECT (button), "title");
}

/**
 * he_check_button_set_value:
 * @button: a #HeCheckButton
 * @value: a new value (secondary label) for the button, or %NULL
 *
 * Sets the value (secondary label) of @button to @value.
 *
 * This will clear any previously set value.
 *
 * If @value is set to %NULL or an empty string, the value label will
 * be hidden and the title label will be realigned.
 *
 * Since: 2.2
 **/
void
he_check_button_set_value                         (HeCheckButton *button,
                                                 const gchar  *value)
{
    HeCheckButtonPrivate *priv;

    g_return_if_fail (HE_IS_CHECK_BUTTON (button));

    priv = HE_CHECK_BUTTON_GET_PRIVATE (button);
    gtk_label_set_text (priv->value, value);

    /* If the button has no value, hide the label so the title is
     * properly aligned */
    if (value && value[0] != '\0') {
        he_check_button_construct_child (button);
        gtk_widget_show (GTK_WIDGET (priv->value));
    } else {
        gtk_widget_hide (GTK_WIDGET (priv->value));
    }

    g_object_notify (G_OBJECT (button), "value");
}

/**
 * he_check_button_get_title:
 * @button: a #HeCheckButton
 *
 * Fetches the text from the main label (title) of @button,
 * as set by he_check_button_set_title() or he_check_button_set_text().
 * If the label text has not been set the return value will be %NULL.
 * This will be the case if you create an empty button with
 * he_check_button_new() to use as a container.
 *
 * Returns: The text of the title label. This string is owned by the
 * widget and must not be modified or freed.
 *
 * Since: 2.2
 **/
const gchar *
he_check_button_get_title                         (HeCheckButton *button)
{
    HeCheckButtonPrivate *priv;

    g_return_val_if_fail (HE_IS_CHECK_BUTTON (button), NULL);

    priv = HE_CHECK_BUTTON_GET_PRIVATE (button);

    return gtk_label_get_text (priv->title);
}

/**
 * he_check_button_get_value:
 * @button: a #HeCheckButton
 *
 * Fetches the text from the secondary label (value) of @button,
 * as set by he_check_button_set_value() or he_check_button_set_text().
 * If the label text has not been set the return value will be %NULL.
 * This will be the case if you create an empty button with he_check_button_new()
 * to use as a container.
 *
 * Returns: The text of the value label. This string is owned by the
 * widget and must not be modified or freed.
 *
 * Since: 2.2
 **/
const gchar *
he_check_button_get_value                         (HeCheckButton *button)
{
    HeCheckButtonPrivate *priv;

    g_return_val_if_fail (HE_IS_CHECK_BUTTON (button), NULL);

    priv = HE_CHECK_BUTTON_GET_PRIVATE (button);

    return gtk_label_get_text (priv->value);
}

/**
 * he_check_button_set_text:
 * @button: a #HeCheckButton
 * @title: new text for the button title (main label)
 * @value: new text for the button value (secondary label)
 *
 * Convenience function to change both labels of a #HeCheckButton
 *
 * Since: 2.2
 **/
void
he_check_button_set_text                          (HeCheckButton *button,
                                                 const gchar  *title,
                                                 const gchar  *value)
{
    he_check_button_set_title (button, title);
    he_check_button_set_value (button, value);
}

/**
 * he_check_button_set_alignment:
 * @button: a #HeCheckButton
 * @xalign: the horizontal alignment of the contents, from 0 (left) to 1 (right).
 * @yalign: the vertical alignment of the contents, from 0 (top) to 1 (bottom).
 * @xscale: the amount that the child widget expands horizontally to fill up unused space, from 0 to 1
 * @yscale: the amount that the child widget expands vertically to fill up unused space, from 0 to 1
 *
 * Sets the alignment of the contents of the widget. If you don't need
 * to change @xscale or @yscale you can just use
 * gtk_button_set_alignment() instead.
 *
 * Note that for this method to work properly, the child widget of
 * @button must be a #GtkAlignment. That's what #HeCheckButton uses by
 * default, so this function will work unless you add a custom widget
 * to @button.
 *
 * Since: 2.2
 **/
void
he_check_button_set_alignment                     (HeCheckButton *button,
                                                 gfloat        xalign,
                                                 gfloat        yalign,
                                                 gfloat        xscale,
                                                 gfloat        yscale)
{
    HeCheckButtonPrivate *priv;
    GtkWidget *child;

    g_return_if_fail (HE_IS_CHECK_BUTTON (button));

    priv = HE_CHECK_BUTTON_GET_PRIVATE (button);

    child = gtk_bin_get_child (GTK_BIN (button));

    /* If the button has no child, use priv->alignment, which is the default one */
    if (child == NULL)
        child = priv->alignment;

    if (GTK_IS_ALIGNMENT (child)) {
        gtk_alignment_set (GTK_ALIGNMENT (priv->alignment), xalign, yalign, xscale, yscale);
    }
}

/**
 * he_check_button_set_title_alignment:
 * @button: a #HeCheckButton
 * @xalign: the horizontal alignment of the title label, from 0 (left) to 1 (right).
 * @yalign: the vertical alignment of the title label, from 0 (top) to 1 (bottom).
 *
 * Sets the alignment of the title label. See also
 * he_check_button_set_alignment() to set the alignment of the whole
 * contents of the button.
 *
 * Since: 2.2
 **/
void
he_check_button_set_title_alignment               (HeCheckButton *button,
                                                 gfloat        xalign,
                                                 gfloat        yalign)
{
    HeCheckButtonPrivate *priv;

    g_return_if_fail (HE_IS_CHECK_BUTTON (button));

    priv = HE_CHECK_BUTTON_GET_PRIVATE (button);

    gtk_misc_set_alignment (GTK_MISC (priv->title), xalign, yalign);
}

/**
 * he_check_button_set_value_alignment:
 * @button: a #HeCheckButton
 * @xalign: the horizontal alignment of the value label, from 0 (left) to 1 (right).
 * @yalign: the vertical alignment of the value label, from 0 (top) to 1 (bottom).
 *
 * Sets the alignment of the value label. See also
 * he_check_button_set_alignment() to set the alignment of the whole
 * contents of the button.
 *
 * Since: 2.2
 **/
void
he_check_button_set_value_alignment               (HeCheckButton *button,
                                                 gfloat        xalign,
                                                 gfloat        yalign)
{
    HeCheckButtonPrivate *priv;

    g_return_if_fail (HE_IS_CHECK_BUTTON (button));

    priv = HE_CHECK_BUTTON_GET_PRIVATE (button);

    gtk_misc_set_alignment (GTK_MISC (priv->value), xalign, yalign);
}

/**
 * he_check_button_set_style:
 * @button: A #HeCheckButton
 * @style: A #HeCheckButtonStyle for @button
 *
 * Sets the style of @button to @style. This changes the visual
 * appearance of the button (colors, font sizes) according to the
 * particular style chosen, but the general layout is not altered.
 *
 * Use %HE_CHECK_BUTTON_STYLE_NORMAL to make it look like a normal
 * #HeCheckButton, or %HE_CHECK_BUTTON_STYLE_PICKER to make it look like
 * a #HildonPickerButton.
 *
 * Since: 2.2
 */
void
he_check_button_set_style                         (HeCheckButton      *button,
                                                 HeCheckButtonStyle  style)
{
    HeCheckButtonPrivate *priv;

    g_return_if_fail (HE_IS_CHECK_BUTTON (button));
    priv = HE_CHECK_BUTTON_GET_PRIVATE (button);

    priv->style = style;
    set_logical_color (GTK_WIDGET (button));

    g_object_notify (G_OBJECT (button), "style");
}

/**
 * he_check_button_get_style:
 * @button: A #HeCheckButton
 *
 * Gets the visual style of the button.
 *
 * Returns: a #HeCheckButtonStyle
 *
 * Since: 2.2
 */
HeCheckButtonStyle
he_check_button_get_style                         (HeCheckButton *button)
{
    HeCheckButtonPrivate *priv;

    g_return_val_if_fail (HE_IS_CHECK_BUTTON (button), HE_CHECK_BUTTON_STYLE_NORMAL);

    priv = HE_CHECK_BUTTON_GET_PRIVATE (button);

    return priv->style;
}

static void
he_check_button_construct_child                   (HeCheckButton *button)
{
    HeCheckButtonPrivate *priv = HE_CHECK_BUTTON_GET_PRIVATE (button);
    GtkWidget *child;
    gint image_spacing;
    const gchar *title, *value;

    /* Don't do anything if the button is not constructed yet */
    if (G_UNLIKELY (priv->label_box == NULL))
        return;

    /* Don't do anything if the button has no contents */
    title = gtk_label_get_text (priv->title);
    value = gtk_label_get_text (priv->value);

    if (!title[0] && !value[0])
        return;

    /* Remove the parent if there is one */
    if (priv->cell_view) {
		g_object_ref (priv->cell_view);
		if (priv->cell_view->parent != NULL)
			gtk_container_remove (GTK_CONTAINER (priv->cell_view->parent), priv->cell_view);
	}

    if (priv->label_box->parent != NULL) {
        gtk_container_remove (GTK_CONTAINER (priv->label_box->parent), priv->label_box);
    }

    /* Remove the child from the container and add priv->alignment */
    child = gtk_bin_get_child (GTK_BIN (button));
    if (child != NULL && child != priv->alignment) {
        gtk_container_remove (GTK_CONTAINER (button), child);
        child = NULL;
    }

    if (child == NULL) {
        gtk_container_add (GTK_CONTAINER (button), GTK_WIDGET (priv->alignment));
    }

    /* Create a new hbox */
    if (priv->hbox) {
        gtk_container_remove (GTK_CONTAINER (priv->alignment), GTK_WIDGET (priv->hbox));
    }
    gtk_widget_style_get (GTK_WIDGET (button), "image-spacing", &image_spacing, NULL);
    priv->hbox = GTK_BOX (gtk_hbox_new (FALSE, image_spacing));
    gtk_container_add (GTK_CONTAINER (priv->alignment), GTK_WIDGET (priv->hbox));

    /* All left align */
    gtk_alignment_set(GTK_ALIGNMENT(priv->alignment), 0, 0.5, -1, -1);

    /* Add check box and box with labels */
    gtk_box_pack_start (priv->hbox, priv->cell_view, FALSE, FALSE, 0);
    gtk_box_pack_start (priv->hbox, priv->label_box, TRUE, TRUE, 0);

    /* Remove the ref */
    g_object_unref (priv->cell_view);

    /* Show everything */
    gtk_widget_show_all (GTK_WIDGET (priv->alignment));
}

#endif /* HILDON_HAS_APP_MENU */
