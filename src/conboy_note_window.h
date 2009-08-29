#ifndef CONBOY_NOTE_WINDOW_H_
#define CONBOY_NOTE_WINDOW_H_

#include <gtk/gtk.h>

typedef struct {
	GtkWindow *window;
	GtkTextBuffer *buffer;
	GtkTextView *view;
	ConboyNote *note;
	
} ConboyNoteWindow;

ConboyNoteWindow* conboy_note_window_new(void);

void conboy_note_window_show_note(ConboyNoteWindow* window, ConboyNote *note);



#endif /*CONBOY_NOTE_WINDOW_H_*/
