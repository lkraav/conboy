
#include <glib.h>
#include <string.h>

#include "search.h"
#include "app_data.h"

static gint
find_match_count(gchar *xml_string, gchar **words)
{
	gint matches = 0;
	/* TODO: Strip xml tags */
	gint i;
	
	for (i = 0; words[i] != NULL; i++) {
		
		gchar *word = words[i];
		gchar *found = xml_string;
		gboolean current_word_found = FALSE;	
		
		if (strcmp("", word) == 0) {
			continue;
		}
		
		/* Find all occurences of word */
		while((found = strstr(found, word)) != NULL) {
			matches++;
			current_word_found = TRUE;
			found = found + strlen(word);
		}
		
		/* If we didn't have at least one hit for all words, we return 0 hits, because we want "AND" search, not "OR". */
		if (current_word_found == FALSE) {
			return 0;
		}
	}
	
	return matches;
}

/**
 * Returns only TRUE if all words appear in the content.
 */
static gboolean
has_match(gchar *content, gchar **words)
{
	gint i;
	for (i = 0; words[i] != NULL; i++) {
		if (strcmp(content, words[i])) {
			continue;
		}
		return FALSE;
	}
	return TRUE;
}

/**
 * Returns a hashtable with pointers to notes as keys and an int as value.
 * The value is a number, how often the search string (query) was found in
 * the (key) note.
 * 
 * The query it cut into seperate words on whitespaces.
 * 
 * You have to free the hash table after using it.
 */
void
search(const gchar *query, GHashTable *result)
{	
	GTimer *timer = g_timer_new();
	g_timer_start(timer);
	AppData *app_data = app_data_get();
	GtkTreeIter iter;
	GtkTreeModel *model = GTK_TREE_MODEL(app_data->note_store);
	
	g_assert(result != NULL);
	g_hash_table_remove_all(result);
	
	gchar **words = g_strsplit_set(query, "' ''\t''\n'", -1);
	
	gboolean valid = gtk_tree_model_get_iter_first(model, &iter);

	while (valid) {
		gint match_count = 0;
		Note *note;
		gtk_tree_model_get(model, &iter, NOTE_COLUMN, &note, -1);
		
		/*if (has_match(note->content, words)) {*/
			match_count = find_match_count(note->content, words);
		/*}*/
			
		if (match_count > 0) {
			g_hash_table_insert(result, note, GINT_TO_POINTER(match_count));
		}
		
		valid = gtk_tree_model_iter_next(model, &iter);
	}
	
	g_strfreev(words);
	
	g_timer_stop(timer);
	gulong micro;
	g_timer_elapsed(timer, &micro);
	g_printerr("Search took %lu micro seconds \n", micro);
	g_timer_destroy(timer);
	
	return result;
}
