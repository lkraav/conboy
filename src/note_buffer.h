#ifndef NOTE_BUFFER_H_
#define NOTE_BUFFER_H_

#include <gtk/gtk.h>

void note_buffer_set_xml(GtkTextBuffer *buffer, const gchar *xmlString);
gchar *note_buffer_get_xml(GtkTextBuffer *buffer); 


#endif /*NOTE_BUFFER_H_*/
