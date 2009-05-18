#ifndef NOTE_LINKER_H_
#define NOTE_LINKER_H_

#include <gtk/gtk.h>
#include "note.h"

void auto_highlight_links(Note *note, GtkTextIter *start_iter, GtkTextIter *end_iter);

#endif /*NOTE_LINKER_H_*/
