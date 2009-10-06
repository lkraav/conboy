#ifndef NOTE_LINKER_H_
#define NOTE_LINKER_H_

#include <gtk/gtk.h>
#include "note.h"

void auto_highlight_links(UserInterface *ui, GtkTextIter *start_iter, GtkTextIter *end_iter);

void auto_highlight_urls(UserInterface *ui, GtkTextIter *start_iter, GtkTextIter *end_iter);

#endif /*NOTE_LINKER_H_*/
