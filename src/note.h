#ifndef NOTE_H_
#define NOTE_H_

void note_open(Note* metadata);

/*void note_open_in_new_window(Note* metadata);*/

void note_load_to_buffer(const gchar* filename, GtkTextBuffer *buffer, Note *note);

/*void note_save_from_buffer(Note *metadata);*/

void note_save_from_buffer2(const gchar* filename, GtkTextBuffer *buffer, Note *metadata);

void note_format_title(GtkTextBuffer *buffer);

void note_set_window_title_from_buffer(GtkWindow *win, GtkTextBuffer *buffer);

void note_open_by_title(const char* title);

void note_open_new_with_title(const gchar* title);

void note_save(Note *note);

void note_close_window(Note *note);

const gchar* note_extract_title_from_buffer(GtkTextBuffer *buffer);

#endif /*NOTE_H_*/
