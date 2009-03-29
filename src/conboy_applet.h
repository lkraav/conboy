#ifndef HELLO_NAVIGATOR_PLUGIN_H
#define HELLO_NAVIGATOR_PLUGIN_H

#include <glib-object.h>

/* For Task Navigator plugins */
#include <libhildondesktop/tasknavigator-item.h>

G_BEGIN_DECLS

/* Common struct types declarations */
typedef struct _HelloNavigatorPlugin HelloNavigatorPlugin;
typedef struct _HelloNavigatorPluginClass HelloNavigatorPluginClass;
typedef struct _HelloNavigatorPluginPrivate HelloNavigatorPluginPrivate;

/* Common macros */
#define HELLO_TYPE_NAVIGATOR_PLUGIN (hello_navigator_plugin_get_type())
     
#define HELLO_NAVIGATOR_PLUGIN (obj) (G_TYPE_CHECK_INSTANCE_CAST (( obj), HELLO_TYPE_NAVIGATOR_PLUGIN , HelloNavigatorPlugin ))

#define HELLO_NAVIGATOR_PLUGIN_CLASS ( klass )    ( G_TYPE_CHECK_CLASS_CAST (( klass), HELLO_TYPE_NAVIGATOR_PLUGIN , HelloNavigatorPluginClass ))

#define HELLO_IS_NAVIGATOR_PLUGIN (obj) (G_TYPE_CHECK_INSTANCE_TYPE (( obj), HELLO_TYPE_NAVIGATOR_PLUGIN ))

#define HELLO_IS_NAVIGATOR_PLUGIN_CLASS ( klass ) ( G_TYPE_CHECK_CLASS_TYPE(( klass), HELLO_TYPE_NAVIGATOR_PLUGIN ))

#define HELLO_NAVIGATOR_PLUGIN_GET_CLASS (obj) (G_TYPE_INSTANCE_GET_CLASS (( obj), HELLO_TYPE_NAVIGATOR_PLUGIN , HelloNavigatorPluginClass ))

/* Instance struct */
struct _HelloNavigatorPlugin
{
     TaskNavigatorItem 				 tnitem;
     HelloNavigatorPluginPrivate 	*priv;
     GtkWidget              		*button;
     GtkWidget              		*menu;
};

/* Class struct */
struct _HelloNavigatorPluginClass
{
     TaskNavigatorItemClass parent_class ;
};

GType hello_navigator_plugin_get_type (void);
GtkWidget * hello_world_button_new (int padding );

G_END_DECLS

#endif /* HELLO_NAVIGATOR_PLUGIN_H */
