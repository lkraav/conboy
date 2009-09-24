/*
 * This file is a part of hildon
 *
 * Copyright (C) 2008, 2009 Nokia Corporation, all rights reserved.
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

/**
 * SECTION:hildon-button
 * @short_description: Two-label buttons in the Hildon framework.
 *
 * #ConboyCheckButton is a clickable button for Hildon applications. It is
 * derived from the #GtkButton widget and provides additional
 * commodities specific to the Hildon framework.
 *
 * The height of a #ConboyCheckButton can be set to either "finger" height
 * or "thumb" height. It can also be configured to use halfscreen or
 * fullscreen width. Alternatively, either dimension can be set to
 * "auto" so it behaves like a standard #GtkButton.
 *
 * A #ConboyCheckButton can hold any valid child widget, but it usually
 * contains two labels, named title and value, and it can also contain
 * an image. The contents of the button are packed together inside a
 * #GtkAlignment and they do not expand by default (they don't use the
 * full space of the button).
 *
 * To change the alignment of both labels, use gtk_button_set_alignment()
 *
 * To make them expand and use the full space of the button, use
 * conboy_check_button_set_alignment().
 *
 * To change the relative alignment of each label, use
 * conboy_check_button_set_title_alignment() and
 * conboy_check_button_set_value_alignment().
 *
 * In hildon-button-example.c included in the Hildon distribution you
 * can see examples of how to create the most common button
 * layouts.
 *
 * If only one label is needed, #GtkButton can be used as well, see
 * also hildon_gtk_button_new().
 *
 * <example>
 * <title>Creating a ConboyCheckButton</title>
 * <programlisting>
 * void
 * button_clicked (ConboyCheckButton *button, gpointer user_data)
 * {
 *     const gchar *title, *value;
 * <!-- -->
 *     title = conboy_check_button_get_title (button);
 *     value = conboy_check_button_get_value (button);
 *     g_debug ("Button clicked with title '&percnt;s' and value '&percnt;s'", title, value);
 * }
 * <!-- -->
 * GtkWidget *
 * create_button (void)
 * {
 *     GtkWidget *button;
 *     GtkWidget *image;
 * <!-- -->
 *     button = conboy_check_button_new (HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT,
 *                                 CONBOY_CHECK_BUTTON_ARRANGEMENT_VERTICAL);
 *     conboy_check_button_set_text (CONBOY_CHECK_BUTTON (button), "Some title", "Some value");
 * <!-- -->
 *     image = gtk_image_new_from_stock (GTK_STOCK_INFO, GTK_ICON_SIZE_BUTTON);
 *     conboy_check_button_set_image (CONBOY_CHECK_BUTTON (button), image);
 *     conboy_check_button_set_image_position (CONBOY_CHECK_BUTTON (button), GTK_POS_RIGHT);
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

#include                                        <hildon/hildon-enum-types.h>
#include                                        <hildon/hildon-gtk.h>
#include                                        "conboy_check_button.h"

enum {
	TOGGLED,
	LAST_SIGNAL
};

static guint									signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE                                   (ConboyCheckButton, conboy_check_button, GTK_TYPE_BUTTON);

#define                                         CONBOY_CHECK_BUTTON_GET_PRIVATE(obj) \
                                                (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
                                                CONBOY_TYPE_CHECK_BUTTON, ConboyCheckButtonPrivate));

struct                                          _ConboyCheckButtonPrivate
{
    GtkLabel *title;
    GtkLabel *value;
    GtkBox *hbox;
    GtkWidget *label_box;
    GtkWidget *alignment;
    GtkCellRendererToggle *toggle_renderer;
    GtkWidget *cell_view;
    ConboyCheckButtonStyle style;
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
 * conboy_check_button_toggled:
 * @button: A #ConboyCheckButton
 *
 * Emits the #ConboyCheckButton::toggled signal on the #ConboyCheckButton.
 * There is no good reason for an application ever to call this function.
 *
 * Since: 2.2
 */
void
conboy_check_button_toggled                     (ConboyCheckButton *button)
{
    g_return_if_fail (CONBOY_IS_CHECK_BUTTON (button));

    g_signal_emit (button, signals[TOGGLED], 0);
}


/**
 * conboy_check_button_set_active:
 * @button: A #ConboyCheckButton
 * @is_active: new state for the button
 *
 * Sets the status of a #ConboyCheckButton. Set to %TRUE if you want
 * @button to be 'pressed-in', and %FALSE to raise it. This action
 * causes the #ConboyCheckButton::toggled signal to be emitted.
 *
 * Since: 2.2
 **/
void
conboy_check_button_set_active                  (ConboyCheckButton *button,
                                                 gboolean           is_active)
{
    gboolean prev_is_active;

    g_return_if_fail (CONBOY_IS_CHECK_BUTTON (button));

    prev_is_active = conboy_check_button_get_active (button);

    if (prev_is_active != is_active) {
        gtk_button_clicked (GTK_BUTTON (button));
        gtk_widget_queue_draw (GTK_WIDGET (button));
    }
}

/**
 * conboy_check_button_get_active:
 * @button: A #ConboyCheckButton
 *
 * Gets the current state of @button.
 *
 * Return value: %TRUE if @button is active, %FALSE otherwise.
 *
 * Since: 2.2
 **/
gboolean
conboy_check_button_get_active                  (ConboyCheckButton *button)
{
    g_return_val_if_fail (CONBOY_IS_CHECK_BUTTON (button), FALSE);

    return gtk_cell_renderer_toggle_get_active (button->priv->toggle_renderer);
}

static void
conboy_check_button_set_arrangement                   (ConboyCheckButton            *button,
                                                 ConboyCheckButtonArrangement  arrangement);

static void
conboy_check_button_construct_child                   (ConboyCheckButton *button);

static void
conboy_check_button_set_property                      (GObject      *object,
                                                 guint         prop_id,
                                                 const GValue *value,
                                                 GParamSpec   *pspec)
{
    ConboyCheckButton *button = CONBOY_CHECK_BUTTON (object);

    switch (prop_id)
    {
    case PROP_TITLE:
        conboy_check_button_set_title (button, g_value_get_string (value));
        break;
    case PROP_VALUE:
        conboy_check_button_set_value (button, g_value_get_string (value));
        break;
    case PROP_SIZE:
        hildon_gtk_widget_set_theme_size (GTK_WIDGET (button), g_value_get_flags (value));
        break;
    case PROP_ARRANGEMENT:
        conboy_check_button_set_arrangement (button, g_value_get_enum (value));
        break;
    case PROP_STYLE:
        conboy_check_button_set_style (button, g_value_get_enum (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
conboy_check_button_get_property                      (GObject    *object,
                                                 guint       prop_id,
                                                 GValue     *value,
                                                 GParamSpec *pspec)
{
    ConboyCheckButton *button = CONBOY_CHECK_BUTTON (object);

    switch (prop_id)
    {
    case PROP_TITLE:
        g_value_set_string (value, conboy_check_button_get_title (button));
        break;
    case PROP_VALUE:
        g_value_set_string (value, conboy_check_button_get_value (button));
        break;
    case PROP_STYLE:
        g_value_set_enum (value, conboy_check_button_get_style (button));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
set_logical_font                                (GtkWidget *button)
{
    ConboyCheckButtonPrivate *priv = CONBOY_CHECK_BUTTON_GET_PRIVATE (button);

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
    ConboyCheckButtonPrivate *priv = CONBOY_CHECK_BUTTON_GET_PRIVATE (button);
    GtkWidget *label = GTK_WIDGET (priv->value);

    switch (priv->style) {
    case CONBOY_CHECK_BUTTON_STYLE_NORMAL:
        colorname = "SecondaryTextColor";
        break;
    case CONBOY_CHECK_BUTTON_STYLE_PICKER:
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

static void
conboy_check_button_style_set                         (GtkWidget *widget,
                                                 GtkStyle  *previous_style)
{
    guint horizontal_spacing, vertical_spacing, image_spacing;
    ConboyCheckButtonPrivate *priv = CONBOY_CHECK_BUTTON_GET_PRIVATE (widget);

    if (GTK_WIDGET_CLASS (conboy_check_button_parent_class)->style_set)
        GTK_WIDGET_CLASS (conboy_check_button_parent_class)->style_set (widget, previous_style);

    /* Prevent infinite recursion when calling set_logical_font() and
     * set_logical_color() */
    if (priv->setting_style)
        return;

    gtk_widget_style_get (widget,
                          "horizontal-spacing", &horizontal_spacing,
                          "vertical-spacing", &vertical_spacing,
                          "image-spacing", &image_spacing,
                          NULL);

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
conboy_check_button_finalize                          (GObject *object)
{
    ConboyCheckButtonPrivate *priv = CONBOY_CHECK_BUTTON_GET_PRIVATE (object);

    g_object_unref (priv->alignment);
    g_object_unref (priv->label_box);

    G_OBJECT_CLASS (conboy_check_button_parent_class)->finalize (object);
}

static void
conboy_check_button_clicked                     (GtkButton *button)
{
    ConboyCheckButton *checkbutton = CONBOY_CHECK_BUTTON (button);
    gboolean current = conboy_check_button_get_active (checkbutton);

    gtk_cell_renderer_toggle_set_active (checkbutton->priv->toggle_renderer, !current);

    conboy_check_button_toggled (checkbutton);
}

static void
conboy_check_button_class_init                        (ConboyCheckButtonClass *klass)
{
    GObjectClass *gobject_class = (GObjectClass *)klass;
    GtkWidgetClass *widget_class = (GtkWidgetClass *)klass;
    GtkButtonClass *button_class = (GtkButtonClass *)klass;

    gobject_class->set_property = conboy_check_button_set_property;
    gobject_class->get_property = conboy_check_button_get_property;
    gobject_class->finalize = conboy_check_button_finalize;
    widget_class->style_set = conboy_check_button_style_set;
    button_class->clicked = conboy_check_button_clicked;

    klass->toggled = NULL;

    /**
     * ConboyCheckButton::toggled
     *
     * Emitted when the #ConboyCheckButton's state is changed.
     *
     * Since: 2.2
     */
    signals[TOGGLED] =
        g_signal_new ("toggled",
                      G_OBJECT_CLASS_TYPE (gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (ConboyCheckButtonClass, toggled),
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
            CONBOY_CHECK_BUTTON_ARRANGEMENT_HORIZONTAL,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (
        gobject_class,
        PROP_STYLE,
        g_param_spec_enum (
            "style",
            "Style",
            "Visual style of the button",
            HILDON_TYPE_BUTTON_STYLE,
            CONBOY_CHECK_BUTTON_STYLE_NORMAL,
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

    g_type_class_add_private (klass, sizeof (ConboyCheckButtonPrivate));
}

static void
conboy_check_button_init                              (ConboyCheckButton *self)
{
    ConboyCheckButtonPrivate *priv = CONBOY_CHECK_BUTTON_GET_PRIVATE (self);

    /* Store private part */
    self->priv = priv;

    priv->title = GTK_LABEL (gtk_label_new (NULL));
    priv->value = GTK_LABEL (gtk_label_new (NULL));
    priv->alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
    priv->toggle_renderer = GTK_CELL_RENDERER_TOGGLE (gtk_cell_renderer_toggle_new ());
    priv->cell_view = gtk_cell_view_new ();
    priv->hbox = NULL;
    priv->label_box = NULL;
    priv->style = CONBOY_CHECK_BUTTON_STYLE_NORMAL;
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

	/* Set the indicator to the right size (the size of the pixmap) */
	g_object_set (priv->toggle_renderer, "indicator-size", 38, NULL);

	/* Setup the labels */
    gtk_widget_set_name (GTK_WIDGET (priv->title), "hildon-button-title");
    gtk_widget_set_name (GTK_WIDGET (priv->value), "hildon-button-value");

    conboy_check_button_set_style (self, CONBOY_CHECK_BUTTON_STYLE_NORMAL);

    gtk_misc_set_alignment (GTK_MISC (priv->title), 0, 0.5);
    gtk_misc_set_alignment (GTK_MISC (priv->value), 0, 0.5);

    g_object_ref_sink (priv->alignment);

    /* The labels are not shown automatically, see conboy_check_button_set_(title|value) */
    gtk_widget_set_no_show_all (GTK_WIDGET (priv->title), TRUE);
    gtk_widget_set_no_show_all (GTK_WIDGET (priv->value), TRUE);

    gtk_button_set_focus_on_click (GTK_BUTTON (self), FALSE);
}

/**
 * conboy_check_button_add_title_size_group:
 * @button: a #ConboyCheckButton
 * @size_group: A #GtkSizeGroup for the button title (main label)
 *
 * Adds the title label of @button to @size_group.
 *
 * Since: 2.2
 **/
void
conboy_check_button_add_title_size_group              (ConboyCheckButton *button,
                                                 GtkSizeGroup *size_group)
{
    ConboyCheckButtonPrivate *priv;

    g_return_if_fail (CONBOY_IS_CHECK_BUTTON (button));
    g_return_if_fail (GTK_IS_SIZE_GROUP (size_group));

    priv = CONBOY_CHECK_BUTTON_GET_PRIVATE (button);

    gtk_size_group_add_widget (size_group, GTK_WIDGET (priv->title));
}

/**
 * conboy_check_button_add_value_size_group:
 * @button: a #ConboyCheckButton
 * @size_group: A #GtkSizeGroup for the button value (secondary label)
 *
 * Adds the value label of @button to @size_group.
 *
 * Since: 2.2
 **/
void
conboy_check_button_add_value_size_group              (ConboyCheckButton *button,
                                                 GtkSizeGroup *size_group)
{
    ConboyCheckButtonPrivate *priv;

    g_return_if_fail (CONBOY_IS_CHECK_BUTTON (button));
    g_return_if_fail (GTK_IS_SIZE_GROUP (size_group));

    priv = CONBOY_CHECK_BUTTON_GET_PRIVATE (button);

    gtk_size_group_add_widget (size_group, GTK_WIDGET (priv->value));
}

/**
 * conboy_check_button_add_size_groups:
 * @button: a #ConboyCheckButton
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
conboy_check_button_add_size_groups                   (ConboyCheckButton *button,
                                                 GtkSizeGroup *title_size_group,
                                                 GtkSizeGroup *value_size_group,
                                                 GtkSizeGroup *image_size_group)
{
    if (title_size_group)
        conboy_check_button_add_title_size_group (button, title_size_group);

    if (value_size_group)
        conboy_check_button_add_value_size_group (button, value_size_group);

}

/**
 * conboy_check_button_new:
 * @size: Flags to set the size of the button.
 * @arrangement: How the labels must be arranged.
 *
 * Creates a new #ConboyCheckButton. To set text in the labels, use
 * conboy_check_button_set_title() and
 * conboy_check_button_set_value(). Alternatively, you can add a custom
 * child widget using gtk_container_add().
 *
 * Returns: a new #ConboyCheckButton
 *
 * Since: 2.2
 **/
GtkWidget *
conboy_check_button_new                               (HildonSizeType          size,
                                                 ConboyCheckButtonArrangement arrangement)
{
    return conboy_check_button_new_with_text (size, arrangement, NULL, NULL);
}

/**
 * conboy_check_button_new_with_text:
 * @size: Flags to set the size of the button.
 * @arrangement: How the labels must be arranged.
 * @title: Title of the button (main label), or %NULL
 * @value: Value of the button (secondary label), or %NULL
 *
 * Creates a new #ConboyCheckButton with two labels, @title and @value.
 *
 * If you just don't want to use one of the labels, set it to
 * %NULL. You can set it to a non-%NULL value at any time later using
 * conboy_check_button_set_title() or conboy_check_button_set_value() .
 *
 * Returns: a new #ConboyCheckButton
 *
 * Since: 2.2
 **/
GtkWidget *
conboy_check_button_new_with_text                     (HildonSizeType           size,
                                                 ConboyCheckButtonArrangement  arrangement,
                                                 const gchar             *title,
                                                 const gchar             *value)
{
    GtkWidget *button;

    /* Create widget */
    button = g_object_new (CONBOY_TYPE_CHECK_BUTTON,
                           "size", size,
                           "title", title,
                           "value", value,
                           "arrangement", arrangement,
                           NULL);

    return button;
}

static void
conboy_check_button_set_arrangement                   (ConboyCheckButton            *button,
                                                 ConboyCheckButtonArrangement  arrangement)
{
    ConboyCheckButtonPrivate *priv;

    priv = CONBOY_CHECK_BUTTON_GET_PRIVATE (button);

    /* Pack everything */
    if (arrangement == CONBOY_CHECK_BUTTON_ARRANGEMENT_VERTICAL) {
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

    conboy_check_button_construct_child (button);
}

/**
 * conboy_check_button_set_title:
 * @button: a #ConboyCheckButton
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
conboy_check_button_set_title                         (ConboyCheckButton *button,
                                                 const gchar  *title)
{
    ConboyCheckButtonPrivate *priv;

    g_return_if_fail (CONBOY_IS_CHECK_BUTTON (button));

    priv = CONBOY_CHECK_BUTTON_GET_PRIVATE (button);
    gtk_label_set_text (priv->title, title);

    /* If the button has no title, hide the label so the value is
     * properly aligned */
    if (title && title[0] != '\0') {
        conboy_check_button_construct_child (button);
        gtk_widget_show (GTK_WIDGET (priv->title));
    } else {
        gtk_widget_hide (GTK_WIDGET (priv->title));
    }

    g_object_notify (G_OBJECT (button), "title");
}

/**
 * conboy_check_button_set_value:
 * @button: a #ConboyCheckButton
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
conboy_check_button_set_value                         (ConboyCheckButton *button,
                                                 const gchar  *value)
{
    ConboyCheckButtonPrivate *priv;

    g_return_if_fail (CONBOY_IS_CHECK_BUTTON (button));

    priv = CONBOY_CHECK_BUTTON_GET_PRIVATE (button);
    gtk_label_set_text (priv->value, value);

    /* If the button has no value, hide the label so the title is
     * properly aligned */
    if (value && value[0] != '\0') {
        conboy_check_button_construct_child (button);
        gtk_widget_show (GTK_WIDGET (priv->value));
    } else {
        gtk_widget_hide (GTK_WIDGET (priv->value));
    }

    g_object_notify (G_OBJECT (button), "value");
}

/**
 * conboy_check_button_get_title:
 * @button: a #ConboyCheckButton
 *
 * Fetches the text from the main label (title) of @button,
 * as set by conboy_check_button_set_title() or conboy_check_button_set_text().
 * If the label text has not been set the return value will be %NULL.
 * This will be the case if you create an empty button with
 * conboy_check_button_new() to use as a container.
 *
 * Returns: The text of the title label. This string is owned by the
 * widget and must not be modified or freed.
 *
 * Since: 2.2
 **/
const gchar *
conboy_check_button_get_title                         (ConboyCheckButton *button)
{
    ConboyCheckButtonPrivate *priv;

    g_return_val_if_fail (CONBOY_IS_CHECK_BUTTON (button), NULL);

    priv = CONBOY_CHECK_BUTTON_GET_PRIVATE (button);

    return gtk_label_get_text (priv->title);
}

/**
 * conboy_check_button_get_value:
 * @button: a #ConboyCheckButton
 *
 * Fetches the text from the secondary label (value) of @button,
 * as set by conboy_check_button_set_value() or conboy_check_button_set_text().
 * If the label text has not been set the return value will be %NULL.
 * This will be the case if you create an empty button with conboy_check_button_new()
 * to use as a container.
 *
 * Returns: The text of the value label. This string is owned by the
 * widget and must not be modified or freed.
 *
 * Since: 2.2
 **/
const gchar *
conboy_check_button_get_value                         (ConboyCheckButton *button)
{
    ConboyCheckButtonPrivate *priv;

    g_return_val_if_fail (CONBOY_IS_CHECK_BUTTON (button), NULL);

    priv = CONBOY_CHECK_BUTTON_GET_PRIVATE (button);

    return gtk_label_get_text (priv->value);
}

/**
 * conboy_check_button_set_text:
 * @button: a #ConboyCheckButton
 * @title: new text for the button title (main label)
 * @value: new text for the button value (secondary label)
 *
 * Convenience function to change both labels of a #ConboyCheckButton
 *
 * Since: 2.2
 **/
void
conboy_check_button_set_text                          (ConboyCheckButton *button,
                                                 const gchar  *title,
                                                 const gchar  *value)
{
    conboy_check_button_set_title (button, title);
    conboy_check_button_set_value (button, value);
}

/**
 * conboy_check_button_set_alignment:
 * @button: a #ConboyCheckButton
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
 * @button must be a #GtkAlignment. That's what #ConboyCheckButton uses by
 * default, so this function will work unless you add a custom widget
 * to @button.
 *
 * Since: 2.2
 **/
void
conboy_check_button_set_alignment                     (ConboyCheckButton *button,
                                                 gfloat        xalign,
                                                 gfloat        yalign,
                                                 gfloat        xscale,
                                                 gfloat        yscale)
{
    ConboyCheckButtonPrivate *priv;
    GtkWidget *child;

    g_return_if_fail (CONBOY_IS_CHECK_BUTTON (button));

    priv = CONBOY_CHECK_BUTTON_GET_PRIVATE (button);

    child = gtk_bin_get_child (GTK_BIN (button));

    /* If the button has no child, use priv->alignment, which is the default one */
    if (child == NULL)
        child = priv->alignment;

    if (GTK_IS_ALIGNMENT (child)) {
        gtk_alignment_set (GTK_ALIGNMENT (priv->alignment), xalign, yalign, xscale, yscale);
    }
}

/**
 * conboy_check_button_set_title_alignment:
 * @button: a #ConboyCheckButton
 * @xalign: the horizontal alignment of the title label, from 0 (left) to 1 (right).
 * @yalign: the vertical alignment of the title label, from 0 (top) to 1 (bottom).
 *
 * Sets the alignment of the title label. See also
 * conboy_check_button_set_alignment() to set the alignment of the whole
 * contents of the button.
 *
 * Since: 2.2
 **/
void
conboy_check_button_set_title_alignment               (ConboyCheckButton *button,
                                                 gfloat        xalign,
                                                 gfloat        yalign)
{
    ConboyCheckButtonPrivate *priv;

    g_return_if_fail (CONBOY_IS_CHECK_BUTTON (button));

    priv = CONBOY_CHECK_BUTTON_GET_PRIVATE (button);

    gtk_misc_set_alignment (GTK_MISC (priv->title), xalign, yalign);
}

/**
 * conboy_check_button_set_value_alignment:
 * @button: a #ConboyCheckButton
 * @xalign: the horizontal alignment of the value label, from 0 (left) to 1 (right).
 * @yalign: the vertical alignment of the value label, from 0 (top) to 1 (bottom).
 *
 * Sets the alignment of the value label. See also
 * conboy_check_button_set_alignment() to set the alignment of the whole
 * contents of the button.
 *
 * Since: 2.2
 **/
void
conboy_check_button_set_value_alignment               (ConboyCheckButton *button,
                                                 gfloat        xalign,
                                                 gfloat        yalign)
{
    ConboyCheckButtonPrivate *priv;

    g_return_if_fail (CONBOY_IS_CHECK_BUTTON (button));

    priv = CONBOY_CHECK_BUTTON_GET_PRIVATE (button);

    gtk_misc_set_alignment (GTK_MISC (priv->value), xalign, yalign);
}

/**
 * conboy_check_button_set_style:
 * @button: A #ConboyCheckButton
 * @style: A #ConboyCheckButtonStyle for @button
 *
 * Sets the style of @button to @style. This changes the visual
 * appearance of the button (colors, font sizes) according to the
 * particular style chosen, but the general layout is not altered.
 *
 * Use %CONBOY_CHECK_BUTTON_STYLE_NORMAL to make it look like a normal
 * #ConboyCheckButton, or %CONBOY_CHECK_BUTTON_STYLE_PICKER to make it look like
 * a #HildonPickerButton.
 *
 * Since: 2.2
 */
void
conboy_check_button_set_style                         (ConboyCheckButton      *button,
                                                 ConboyCheckButtonStyle  style)
{
    ConboyCheckButtonPrivate *priv;

    g_return_if_fail (CONBOY_IS_CHECK_BUTTON (button));
    priv = CONBOY_CHECK_BUTTON_GET_PRIVATE (button);

    priv->style = style;
    set_logical_color (GTK_WIDGET (button));

    g_object_notify (G_OBJECT (button), "style");
}

/**
 * conboy_check_button_get_style:
 * @button: A #ConboyCheckButton
 *
 * Gets the visual style of the button.
 *
 * Returns: a #ConboyCheckButtonStyle
 *
 * Since: 2.2
 */
ConboyCheckButtonStyle
conboy_check_button_get_style                         (ConboyCheckButton *button)
{
    ConboyCheckButtonPrivate *priv;

    g_return_val_if_fail (CONBOY_IS_CHECK_BUTTON (button), CONBOY_CHECK_BUTTON_STYLE_NORMAL);

    priv = CONBOY_CHECK_BUTTON_GET_PRIVATE (button);

    return priv->style;
}

static void
conboy_check_button_construct_child                   (ConboyCheckButton *button)
{
    ConboyCheckButtonPrivate *priv = CONBOY_CHECK_BUTTON_GET_PRIVATE (button);
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

