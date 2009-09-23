#ifndef __CONBOY_CHECK_BUTTON_H__
#define __CONBOY_CHECK_BUTTON_H__


#include <hildon/hildon-button.h>

#define CONBOY_TYPE_CHECK_BUTTON			(conboy_check_button_get_type())
#define CONBOY_CHECK_BUTTON(object)			(G_TYPE_CHECK_INSTANCE_CAST ((object),CONBOY_TYPE_CHECK_BUTTON, ConboyCheckButton))
#define CONBOY_CHECK_BUTTON_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), CONBOY_TYPE_CHECK_BUTTON, ConboyCheckButtonClass))
#define CONBOY_IS_CHECK_BUTTON(object)		(G_TYPE_CHECK_INSTANCE_TYPE ((object), CONBOY_TYPE_CHECK_BUTTON))
#define CONBOY_IS_CHECK_BUTTON_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), CONBOY_TYPE_CHECK_BUTTON))
#define CONBOY_CHECK_BUTTON_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), CONBOY_TYPE_CHECK_BUTTON, ConboyCheckButtonClass))


typedef struct _ConboyCheckButton			ConboyCheckButton;
typedef struct _ConboyCheckButtonClass		ConboyCheckButtonClass;
typedef struct _ConboyCheckButtonPrivate	ConboyCheckButtonPrivate;

struct _ConboyCheckButton {
	HildonButton parent;
	/* <private> */
	ConboyCheckButtonPrivate *priv;
};

struct _ConboyCheckButtonClass {
	HildonButtonClass parent;
	/* Signal handlers */
	void (*toggled)	(ConboyCheckButton *button);
};


GType	conboy_check_button_get_type	(void);

GtkWidget*	conboy_check_button_new			(HildonSizeType size, HildonButtonArrangement arrangement);
void		conboy_check_button_set_active	(ConboyCheckButton *button, gboolean is_active);
gboolean	conboy_check_button_get_active	(ConboyCheckButton *button);
void		conboy_check_button_toggled		(ConboyCheckButton *button);


#endif /* __CONBOY_CHECK_BUTTON_H__ */
