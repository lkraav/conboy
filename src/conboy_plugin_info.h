/*
 * Heavily borrowed from gedit here
 */

#ifndef __CONBOY_PLUGIN_INFO_H__
#define __CONBOY_PLUGIN_INFO_H__

#include <glib-object.h>
#include "conboy_plugin.h"

G_BEGIN_DECLS

#define CONBOY_TYPE_PLUGIN_INFO				(conboy_plugin_info_get_type())
#define CONBOY_PLUGIN_INFO(object)			(G_TYPE_CHECK_INSTANCE_CAST ((object),CONBOY_TYPE_PLUGIN_INFO, ConboyPluginInfo))
#define CONBOY_PLUGIN_INFO_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), CONBOY_TYPE_PLUGIN_INFO, ConboyPluginInfoClass))
#define CONBOY_IS_PLUGIN_INFO(object)		(G_TYPE_CHECK_INSTANCE_TYPE ((object), CONBOY_TYPE_PLUGIN_INFO))
#define CONBOY_IS_PLUGIN_INFO_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), CONBOY_TYPE_PLUGIN_INFO))
#define CONBOY_PLUGIN_INFO_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), CONBOY_TYPE_PLUGIN_INFO, ConboyPluginInfoClass))

typedef struct _ConboyPluginInfo        ConboyPluginInfo;
typedef struct _ConboyPluginInfoClass	ConboyPluginInfoClass;

struct _ConboyPluginInfo {
	GObject parent;
	/* <private> */
	ConboyPlugin*	plugin;
	gchar*			file;
	gchar*			module_name;
	gchar*			name;
	gchar*			desc;
	gchar**			authors;
	gchar*			copyright;
	gchar*			version;
	gboolean		available;
};

struct _ConboyPluginInfoClass
{
	GObjectClass parent;
	
	void (*plugin_status_changed)	(ConboyPluginInfo *info, gboolean active);
};

ConboyPluginInfo  *conboy_plugin_info_new       (const gchar *file);

GType		 conboy_plugin_info_get_type		(void) G_GNUC_CONST;

const gchar	 *conboy_plugin_info_get_module_name   (ConboyPluginInfo *info);
const gchar	 *conboy_plugin_info_get_name          (ConboyPluginInfo *info);
const gchar	 *conboy_plugin_info_get_description   (ConboyPluginInfo *info);
const gchar **conboy_plugin_info_get_authors       (ConboyPluginInfo *info);
const gchar	 *conboy_plugin_info_get_copyright     (ConboyPluginInfo *info);
const gchar  *conboy_plugin_info_get_version       (ConboyPluginInfo *info);
gboolean      conboy_plugin_info_is_available      (ConboyPluginInfo *info);
gboolean      conboy_plugin_info_is_configurable   (ConboyPluginInfo *info);
gboolean      conboy_plugin_info_is_active         (ConboyPluginInfo *info);

gboolean	conboy_plugin_info_activate_plugin		(ConboyPluginInfo *info);
gboolean	conboy_plugin_info_deactivate_plugin	(ConboyPluginInfo *info);


G_END_DECLS

#endif /* __CONBOY_PLUGIN_INFO_H__ */

