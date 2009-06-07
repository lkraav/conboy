#ifndef STORAGE_H_
#define STORAGE_H_

#include "note.h"

/**
 * Initializes the storage backend. E.g. conntect to a database,
 * create file paths etc.
 * 
 * user_data: Stuff like hostname, username, password, etc...
 */
void storage_initialize(gpointer user_data);

/**
 * Close all open connections, files, etc. Then free all memory that
 * was allocated by the storage_initialize() function.
 */
void storage_destroy(void);

/**
 * Loads a complete note from the storage backend. The note must be freed
 * when you're done with it.
 */
Note* storage_load_note(const gchar *guid);

/**
 * Only load the notes metadata, but not the content.
 * Should at least load "title" and "last-change-date" that how
 * it works ATM.
 */
/*Note* storage_load_note_partial(const gchar *guid);*/

/**
 * Adds the content to a existing partial note. 
 * Returns TRUE if successfull.
 */
/*gboolean storage_load_note_content(Note *note);*/

/**
 * Saves a note into the storage backend. The containing GUID is used
 * as identifier.
 * Returns TRUE if successfull.
 */
gboolean storage_save_note(Note *note);

/**
 * Returns a list of gchar* GUIDs which can be used
 * to load a note.
 * Free the list and the containing strings if you're done.
 */
GList* storage_get_all_note_ids(void);

/**
 * Deletes a note with the identifier "guid" from the storage backend.
 * Returns: TRUE when successfull, FALSE otherwise.
 */
gboolean storage_delete_note(const gchar *guid);

/**
 * Returns the time in seconds that pass between a note modification and the
 * actual saving.
 * Note: Tomboy uses 4 seconds with its xml backend.
 */ 
gint storage_get_save_interval(void); 


#endif /*STORAGE_H_*/
