#ifndef __CONBOY_CHECK_MARK_H__
#define __CONBOY_CHECK_MARK_H__



#include <gtk/gtk.h>


#define CONBOY_TYPE_CHECK_MARK				(conboy_check_mark_get_type())
#define CONBOY_CHECK_MARK(object)			(G_TYPE_CHECK_INSTANCE_CAST ((object),CONBOY_TYPE_CHECK_MARK, ConboyCheckMark))
#define CONBOY_CHECK_MARK_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), CONBOY_TYPE_CHECK_MARK, ConboyCheckMarkClass))
#define CONBOY_IS_CHECK_MARK(object)		(G_TYPE_CHECK_INSTANCE_TYPE ((object), CONBOY_TYPE_CHECK_MARK))
#define CONBOY_IS_CHECK_MARK_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), CONBOY_TYPE_CHECK_MARK))
#define CONBOY_CHECK_MARK_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), CONBOY_TYPE_CHECK_MARK, ConboyCheckMarkClass))


typedef struct _ConboyCheckMark				ConboyCheckMark;
typedef struct _ConboyCheckMarkClass		ConboyCheckMarkClass;


struct _ConboyCheckMark {
	GtkMisc parent;
	/* <private> */
	GtkCellView *cell_view;
	GtkCellRendererToggle *renderer_toggle;
};

struct _ConboyCheckMarkClass {
	GtkMiscClass parent;
};

GType	conboy_check_mark_get_type	(void);


#endif /*__CONBOY_CHECK_MARK_H__*/
