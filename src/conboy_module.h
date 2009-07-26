/* 
 * Copyright (C) 2009 Piotr Pokora <piotrek.pokora@gmail.com>
 * Copyright (C) 2009 Cornelius Hald <hald@icandy.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef CONBOY_MODULE_H
#define CONBOY_MODULE_H

#include <glib-object.h>
#include <gmodule.h>

/* convention macros */
#define CONBOY_TYPE_MODULE (conboy_module_get_type())
#define CONBOY_MODULE(object)  (G_TYPE_CHECK_INSTANCE_CAST ((object),CONBOY_TYPE_MODULE, ConboyModule))
#define CONBOY_MODULE_CLASS(klass)  (G_TYPE_CHECK_CLASS_CAST ((klass), CONBOY_TYPE_MODULE, ConboyModuleClass))
#define CONBOY_IS_MODULE(object)   (G_TYPE_CHECK_INSTANCE_TYPE ((object), CONBOY_TYPE_MODULE))
#define CONBOY_IS_MODULE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CONBOY_TYPE_MODULE))
#define CONBOY_MODULE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), CONBOY_TYPE_MODULE, ConboyModuleClass))

typedef struct _ConboyModule ConboyModule;
typedef struct _ConboyModuleClass ConboyModuleClass;

struct _ConboyModule{
	GObject parent;
	
	/* <private> */
	gchar *path;
	gchar *name;
	GModule *gmodule;
	gboolean open_on_startup;	
};

struct _ConboyModuleClass{
	GObjectClass parent;

	/* virtual methods */
	gboolean	(*initialize)	(ConboyModule *module);
	
	/* signals */
	
};

GType		conboy_module_get_type		(void);

ConboyModule	*conboy_module_new(const gchar *name);
void		conboy_module_initialize_modules(ConboyModule **modules, GError **error);
gboolean	conboy_module_initialize(ConboyModule *module);

#endif /* CONBOY_MODULE_H */
