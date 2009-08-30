
#include <glib.h>
#include <string.h>

#include "search.h"
#include "app_data.h"



/**
 * Returns a new gchar* which contains only the text, but no xml tags anymore.
 * Free the return value when not needed anymore.
 */
static gchar*
strip_tags(const gchar *xml_string)
{
	int ret;
	AppData *app_data = app_data_get();
	xmlTextReader *reader;
	GString *result = g_string_new("");
	
	/* TODO: Duplicated code. Almose the same code is in note_buffer.c */
	/* We try to reuse the existing xml parser. If none exists yet, we create a new one. */
	if (app_data->reader == NULL) {
		app_data->reader = xmlReaderForMemory(xml_string, strlen(xml_string), NULL, "UTF-8", 0);
	}
	
	if (xmlReaderNewMemory(app_data->reader, xml_string, strlen(xml_string), NULL, "UTF-8", 0) != 0) {
		g_printerr("ERROR: Cannot reuse xml parser. \n");
		g_assert_not_reached();
	}
	
	if (app_data->reader == NULL) {
		g_printerr("ERROR: Couldn't init xml parser.\n");
		g_assert_not_reached();
	}
	
	reader = app_data->reader;
	
	/*xmlReaderNewMemory(reader, xml_string, strlen(xml_string), NULL, NULL, 0);*/
	ret = xmlTextReaderRead(reader);
	while (ret == 1) {
		int type = xmlTextReaderNodeType(reader);
		const xmlChar *value = xmlTextReaderConstValue(reader);
		 
		if (type == XML_TEXT_NODE || type == XML_DTD_NODE) { 
			g_string_append(result, value);
		}
		
		ret = xmlTextReaderRead(reader);
	}
	
	if (ret != 0) {
		g_printerr("ERROR: Failed to strip tags from xml string.\n");
	}
	
	/* Returns the gchar array and frees the rest */
	return g_string_free(result, FALSE);
}

/* TODO: Clean up the g_free() mess */
static gint
find_match_count(const gchar *xml_string, gchar **words)
{
	gint matches = 0;
	gint i;
	gchar *note_content = strip_tags(xml_string);
	gchar *u_note_content = g_utf8_casefold(note_content, -1);
	g_free(note_content);
	
	for (i = 0; words[i] != NULL; i++) {
		
		gchar *u_word = g_utf8_casefold(words[i], -1);
		gchar *u_found = u_note_content;
		
		gboolean current_word_found = FALSE;	
		
		if (strcmp("", u_word) == 0) {
			g_free(u_word);
			continue;
		}
		
		/* Find all occurences of word */
		while((u_found = strstr(u_found, u_word)) != NULL) {
			matches++;
			current_word_found = TRUE;
			u_found = u_found + strlen(u_word);
		}
		
		/* If we didn't have at least one hit for all words, we return 0 hits, because we want "AND" search, not "OR". */
		if (current_word_found == FALSE) {
			g_free(u_note_content);
			g_free(u_word);
			return 0;
		}
		g_free(u_word);
	}
	
	g_free(u_note_content);
	
	return matches;
}

/**
 * Returns only TRUE if all words appear in the content.
 */
/*
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
*/

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
		ConboyNote *note;
		gtk_tree_model_get(model, &iter, NOTE_COLUMN, &note, -1);
		
		gchar *content;
		g_object_get(note, "content", &content, NULL);
		if (content != NULL) {
			match_count = find_match_count(content, words);
		}
		
			
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
	
	return;
}
