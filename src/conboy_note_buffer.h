#ifndef CONBOY_NOTE_BUFFER_H_
#define CONBOY_NOTE_BUFFER_H_


#include <glib-object.h>
#include <gtk/gtk.h>


/* convention macros */
#define CONBOY_TYPE_NOTE_BUFFER				(conboy_note_buffer_get_type())
#define CONBOY_NOTE_BUFFER(object)			(G_TYPE_CHECK_INSTANCE_CAST ((object),CONBOY_TYPE_NOTE_BUFFER, ConboyNoteBuffer))
#define CONBOY_NOTE_BUFFER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), CONBOY_TYPE_NOTE_BUFFER, ConboyNoteBufferClass))
#define CONBOY_IS_NOTE_BUFFER(object)		(G_TYPE_CHECK_INSTANCE_TYPE ((object), CONBOY_TYPE_NOTE_BUFFER))
#define CONBOY_IS_NOTE_BUFFER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), CONBOY_TYPE_NOTE_BUFFER))
#define CONBOY_NOTE_BUFFER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), CONBOY_TYPE_NOTE_BUFFER, ConboyNoteBufferClass))

typedef struct _ConboyNoteBuffer			ConboyNoteBuffer;
typedef struct _ConboyNoteBufferClass		ConboyNoteBufferClass;

struct _ConboyNoteBuffer {
	GtkTextBuffer parent;
	/* <private> */
	GSList *active_tags;
};

struct _ConboyNoteBufferClass {
	GtkTextBufferClass parent_class;
};


GType				conboy_note_buffer_get_type (void);

ConboyNoteBuffer*	conboy_note_buffer_new(void);

void	conboy_note_buffer_add_tag				(ConboyNoteBuffer *self, GtkTextTag *tag);
void	conboy_note_buffer_remove_tag			(ConboyNoteBuffer *self, GtkTextTag *tag);
void	conboy_note_buffer_add_tag_by_name		(ConboyNoteBuffer *self, const gchar *tag_name);
void	conboy_note_buffer_remove_tag_by_name	(ConboyNoteBuffer *self, const gchar *tag_name);

/**
 * The list belongs to the ConboyNoteBuffer, don't free or change it.
 */
GSList*	conboy_note_buffer_get_tags				(ConboyNoteBuffer *self);
void	conboy_note_buffer_set_tags				(ConboyNoteBuffer *self, GSList *tags);
void	conboy_note_buffer_clear_tags			(ConboyNoteBuffer *self);

gchar*	conboy_note_buffer_get_xml				(ConboyNoteBuffer *self);
void	conboy_note_buffer_set_xml				(ConboyNoteBuffer *self, const gchar *xmlString);

#endif /*CONBOY_NOTE_BUFFER_H_*/
