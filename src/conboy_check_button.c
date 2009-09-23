
#ifdef HILDON_HAS_APP_MENU

#include "conboy_check_button.h"

enum {
	TOGGLED,
	LAST_SIGNAL
};

enum {
	PROP_SIZE = 1
};

static guint signals[LAST_SIGNAL] = {0};

G_DEFINE_TYPE(ConboyCheckButton, conboy_check_button, HILDON_TYPE_BUTTON);

#define CONBOY_CHECK_BUTTON_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CONBOY_TYPE_CHECK_BUTTON, ConboyCheckButtonPrivate));

struct _ConboyCheckButtonPrivate {
	GtkCellRendererToggle *toggle_renderer;
};




/**
 * conboy_check_button_toggled:
 * @button: A #ConboyCheckButton
 *
 * Emits the #HildonCheckButton::toggled signal on the #HildonCheckButton.
 * There is no good reason for an application ever to call this function.
 *
 * Since: 2.2
 */
void
conboy_check_button_toggled (ConboyCheckButton *button)
{
    g_return_if_fail (CONBOY_IS_CHECK_BUTTON (button));

    g_signal_emit (button, signals[TOGGLED], 0);
}

/**
 * hildon_check_button_set_active:
 * @button: A #HildonCheckButton
 * @is_active: new state for the button
 *
 * Sets the status of a #HildonCheckButton. Set to %TRUE if you want
 * @button to be 'pressed-in', and %FALSE to raise it. This action
 * causes the #HildonCheckButton::toggled signal to be emitted.
 *
 * Since: 2.2
 **/
void
conboy_check_button_set_active (ConboyCheckButton *button, gboolean is_active)
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
 * hildon_check_button_get_active:
 * @button: A #HildonCheckButton
 *
 * Gets the current state of @button.
 *
 * Return value: %TRUE if @button is active, %FALSE otherwise.
 *
 * Since: 2.2
 **/
gboolean
conboy_check_button_get_active (ConboyCheckButton *button)
{
    g_return_val_if_fail (CONBOY_IS_CHECK_BUTTON (button), FALSE);

    return gtk_cell_renderer_toggle_get_active (button->priv->toggle_renderer);
}

/**
 * hildon_check_button_new:
 * @size: Flags indicating the size of the new button
 *
 * Creates a new #HildonCheckButton.
 *
 * Return value: A newly created #HildonCheckButton
 *
 * Since: 2.2
 **/
GtkWidget *
conboy_check_button_new (HildonSizeType size, HildonButtonArrangement arrangement)
{
    return g_object_new (CONBOY_TYPE_CHECK_BUTTON, "xalign", 0.0, "size", size, "arrangement", arrangement, NULL);
}

static void
conboy_check_button_clicked (GtkButton *button)
{
    ConboyCheckButton *checkbutton = CONBOY_CHECK_BUTTON (button);
    gboolean current = conboy_check_button_get_active (checkbutton);

    gtk_cell_renderer_toggle_set_active (checkbutton->priv->toggle_renderer, !current);

    conboy_check_button_toggled (checkbutton);
}

static void
conboy_check_button_apply_style (GtkWidget *widget)
{
    guint checkbox_size;
    ConboyCheckButtonPrivate *priv = CONBOY_CHECK_BUTTON (widget)->priv;

    gtk_widget_style_get (widget, "checkbox-size", &checkbox_size, NULL);
    g_object_set (priv->toggle_renderer, "indicator-size", checkbox_size, NULL);
}

static void
conboy_check_button_style_set (GtkWidget *widget, GtkStyle  *previous_style)
{
    if (GTK_WIDGET_CLASS (conboy_check_button_parent_class)->style_set)
        GTK_WIDGET_CLASS (conboy_check_button_parent_class)->style_set (widget, previous_style);

    conboy_check_button_apply_style (widget);
}

static void
set_property                                    (GObject      *object,
                                                 guint         prop_id,
                                                 const GValue *value,
                                                 GParamSpec   *pspec)
{
    switch (prop_id)
    {
    case PROP_SIZE:
        hildon_gtk_widget_set_theme_size (GTK_WIDGET (object), g_value_get_flags (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}







static void
conboy_check_button_class_init (ConboyCheckButtonClass *klass)
{
	GObjectClass *gobject_class = (GObjectClass*) klass;
	GtkWidgetClass *widget_class = (GtkWidgetClass*) klass;
	GtkButtonClass *button_class = (GtkButtonClass*) klass;

	gobject_class->set_property = set_property;
	widget_class->style_set = conboy_check_button_style_set;
	button_class->clicked = conboy_check_button_clicked;

	klass->toggled = NULL;

	/**
	 * ConboyCheckButton::toggled
	 *
	 * Emitted when the #HildonCheckButton's state is changed.
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

	gtk_widget_class_install_style_property (
		widget_class,
		g_param_spec_uint (
			"checkbox-size",
			"Size of the check box",
			"Size of the check box",
			0, G_MAXUINT, 48,
			G_PARAM_READABLE));

	g_object_class_install_property (
		gobject_class,
		PROP_SIZE,
		g_param_spec_flags (
			"size",
			"Size",
			"Size request for the button",
			HILDON_TYPE_SIZE_TYPE,
			HILDON_SIZE_AUTO,
			G_PARAM_WRITABLE));

	g_type_class_add_private (klass, sizeof (ConboyCheckButtonPrivate));

}

static void
conboy_check_button_init (ConboyCheckButton *button)
{
    ConboyCheckButtonPrivate *priv = CONBOY_CHECK_BUTTON_GET_PRIVATE (button);
    GtkWidget *cell_view = gtk_cell_view_new ();
    gtk_widget_set_size_request(cell_view, 48, 48);

    /* Store private part */
    button->priv = priv;

    /* Make sure that the check box is always shown, no matter the value of gtk-button-images */
    g_signal_connect (cell_view, "notify::visible", G_CALLBACK (gtk_widget_show), NULL);

    /* Create toggle renderer and pack it into the cell view */
    priv->toggle_renderer = GTK_CELL_RENDERER_TOGGLE (gtk_cell_renderer_toggle_new ());
    g_object_set(priv->toggle_renderer, "indicator-size", 48, NULL);
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (cell_view),
                                GTK_CELL_RENDERER (priv->toggle_renderer), TRUE);

    /* Add cell view to the image */

    /*
    gtk_button_set_use_stock(GTK_BUTTON(button), FALSE);
    gtk_button_set_image (GTK_BUTTON(button), cell_view);
    gtk_button_set_image_position(GTK_BUTTON(button), GTK_POS_LEFT);

    hildon_button_set_image_position(button, GTK_POS_RIGHT);

    hildon_button_set_image_alignment(button, 1, -1);
    hildon_button_set_title_alignment(button, 1, -1);
    hildon_button_set_value_alignment(button, 1, -1);
    hildon_button_set_alignment(button, 1, -1, 1, 1);
	*/

    /*
    GtkImage *img = gtk_image_new();
    gtk_widget_set_size_request(img, 48, 48);
    hildon_button_set_image(button, img);
    */

    hildon_button_set_image(HILDON_BUTTON(button), cell_view);
    hildon_button_set_image_alignment(HILDON_BUTTON(button), 0, 0.5);


    gtk_button_set_focus_on_click (GTK_BUTTON (button), FALSE);

    conboy_check_button_apply_style (GTK_WIDGET (button));
}

#endif
