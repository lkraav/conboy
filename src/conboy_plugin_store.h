#ifndef CONBOY_PLUGIN_STORE_H_
#define CONBOY_PLUGIN_STORE_H_

#include <glib-object.h>
#include "conboy_plugin_info.h"

#define CONBOY_TYPE_PLUGIN_STORE			(conboy_plugin_store_get_type())
#define CONBOY_PLUGIN_STORE(object)			(G_TYPE_CHECK_INSTANCE_CAST ((object),CONBOY_TYPE_PLUGIN_STORE, ConboyPluginStore))
#define CONBOY_PLUGIN_STORE_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), CONBOY_TYPE_PLUGIN_STORE, ConboyPluginStoreClass))
#define CONBOY_IS_PLUGIN_STORE(object)		(G_TYPE_CHECK_INSTANCE_TYPE ((object), CONBOY_TYPE_PLUGIN_STORE))
#define CONBOY_IS_PLUGIN_STORE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), CONBOY_TYPE_PLUGIN_STORE))
#define CONBOY_PLUGIN_STORE_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), CONBOY_TYPE_PLUGIN_STORE, ConboyPluginStoreClass))

typedef struct _ConboyPluginStore		ConboyPluginStore;
typedef struct _ConboyPluginStoreClass	ConboyPluginStoreClass;

struct _ConboyPluginStore {
	GObject parent;
	
	gchar *plugin_path;
	GList *plugins;
};

struct _ConboyPluginStoreClass {
	GObjectClass parent;
	
	void (*plugin_activate)		(ConboyPluginStore *store, ConboyPluginInfo *info);
	void (*plugin_activated)	(ConboyPluginStore *store, ConboyPluginInfo *info);
	void (*plugin_deactivate)	(ConboyPluginStore *store, ConboyPluginInfo *info);
	void (*plugin_deactivated)	(ConboyPluginStore *store, ConboyPluginInfo *info);
	
};

GType				conboy_plugin_store_get_type (void);

ConboyPluginStore*	conboy_plugin_store_new (void);

gboolean			conboy_plugin_store_activate_by_name (ConboyPluginStore *self, const gchar* name);

const gchar*		conboy_plugin_store_get_path (ConboyPluginStore *self);

GList*				conboy_plugin_store_get_plugin_infos (ConboyPluginStore *self);



#endif /*CONBOY_PLUGIN_STORE_H_*/
