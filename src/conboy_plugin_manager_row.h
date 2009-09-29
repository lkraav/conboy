#ifndef _CONBOY_PLUGIN_MANAGER_ROW__
#define _CONBOY_PLUGIN_MANAGER_ROW__

#include <glib-object.h>
#include <gtk/gtk.h>

#include "conboy_plugin_info.h"

#define CONBOY_TYPE_PLUGIN_MANAGER_ROW				(conboy_plugin_manager_row_get_type())
#define CONBOY_PLUGIN_MANAGER_ROW(object)			(G_TYPE_CHECK_INSTANCE_CAST ((object),CONBOY_TYPE_PLUGIN_MANAGER_ROW, ConboyPluginManagerRow))
#define CONBOY_PLUGIN_MANAGER_ROW_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), CONBOY_TYPE_PLUGIN_MANAGER_ROW, ConboyPluginManagerRowClass))
#define CONBOY_IS_PLUGIN_MANAGER_ROW(object)		(G_TYPE_CHECK_INSTANCE_TYPE ((object), CONBOY_TYPE_PLUGIN_MANAGER_ROW))
#define CONBOY_IS_PLUGIN_MANAGER_ROW_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), CONBOY_TYPE_PLUGIN_MANAGER_ROW))
#define CONBOY_PLUGIN_MANAGER_ROW_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), CONBOY_TYPE_PLUGIN_MANAGER_ROW, ConboyPluginManagerRowClass))

typedef struct _ConboyPluginManagerRow		ConboyPluginManagerRow;
typedef struct _ConboyPluginManagerRowClass	ConboyPluginManagerRowClass;

struct _ConboyPluginManagerRow {
	GtkHBox 			parent;

	/* <private> */
	GtkWidget*			main_but;
	GtkWidget*			info_but;
	GtkWidget*			conf_but;
	ConboyPluginInfo*	plugin_info;
};

struct _ConboyPluginManagerRowClass {
	GtkHBoxClass 		parent;

	/* <private */
	GdkPixbuf*			info_pix;
	GdkPixbuf*			conf_pix;

	/* signals */
	void (*main_button_toggled)	(ConboyPluginManagerRow *row);
	void (*conf_button_clicked)	(ConboyPluginManagerRow *row);
	void (*info_button_clicked)	(ConboyPluginManagerRow *row);


};

GType			conboy_plugin_manager_row_get_type		(void);

GtkWidget*		conboy_plugin_manager_row_new			(ConboyPluginInfo *info);












#endif /* _CONBOY_PLUGIN_MANAGER_ROW_MANAGER_ROW_ */
