/*
 * Stolen from gedit
 */

#ifndef CONBOY_PLUGIN_MANAGER_H_
#define CONBOY_PLUGIN_MANAGER_H_

#include <gtk/gtk.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define CONBOY_TYPE_PLUGIN_MANAGER              (conboy_plugin_manager_get_type())
#define CONBOY_PLUGIN_MANAGER(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), CONBOY_TYPE_PLUGIN_MANAGER, ConboyPluginManager))
#define CONBOY_PLUGIN_MANAGER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), CONBOY_TYPE_PLUGIN_MANAGER, ConboyPluginManagerClass))
#define CONBOY_IS_PLUGIN_MANAGER(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), CONBOY_TYPE_PLUGIN_MANAGER))
#define CONBOY_IS_PLUGIN_MANAGER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), CONBOY_TYPE_PLUGIN_MANAGER))
#define CONBOY_PLUGIN_MANAGER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), CONBOY_TYPE_PLUGIN_MANAGER, ConboyPluginManagerClass))

/* Private structure type */
typedef struct _ConboyPluginManagerPrivate ConboyPluginManagerPrivate;

/*
 * Main object structure
 */
typedef struct _ConboyPluginManager ConboyPluginManager;

struct _ConboyPluginManager 
{
	GtkVBox vbox;

	/*< private > */
	ConboyPluginManagerPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _ConboyPluginManagerClass ConboyPluginManagerClass;

struct _ConboyPluginManagerClass 
{
	GtkVBoxClass parent_class;
};

/*
 * Public methods
 */
GType		 conboy_plugin_manager_get_type		(void) G_GNUC_CONST;

GtkWidget	*conboy_plugin_manager_new		(void);
   
G_END_DECLS


#endif /*CONBOY_PLUGIN_MANAGER_H_*/
