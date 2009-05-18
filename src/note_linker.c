#include <gtk/gtk.h>
#include <string.h>

#include "metadata.h"
#include "note_list_store.h"
#include "note_linker.h"

#ifndef max
	#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif

typedef struct {
	gint   start_offset;
	gint   end_offset;
	Note  *note;
} SearchHit;

/* TODO: Optimize this. We need a place where all titles are stored, info which is the longest, ... */
static gint
get_length_of_longest_title()
{
	gint max_length = 0;
	AppData *app_data = get_app_data();
	GtkTreeIter iter;
	GtkTreeModel *model = GTK_TREE_MODEL(app_data->note_store);
	gboolean valid = gtk_tree_model_get_iter_first(model, &iter);

	while (valid) {
		Note *note;
		gtk_tree_model_get(model, &iter, NOTE_COLUMN, &note, -1);
		max_length = max(g_utf8_strlen(note->title, -1), max_length);
		valid = gtk_tree_model_iter_next(model, &iter); /* TODO: Too much casting in this function. Maybe introduce note_list_store_iter_next() etc... */
	}

	return max_length;
}

static
GSList* find_titles(gchar *haystack) {
	/* TODO: The search algorithm needs to be optimized. Probably to use Aho-Corasick */
	/* ATM On the device (N810) searching takes around 1500 microseconds with ~70 notes. */

	GTimer *search_timer = g_timer_new();


	gchar *found;
	GSList *result = NULL;
	AppData *app_data = get_app_data();

	gchar *u_haystack = g_utf8_casefold(haystack, -1);
	gulong micro;

	GtkTreeIter iter;
	GtkTreeModel *model = GTK_TREE_MODEL(app_data->note_store);
	gboolean valid = gtk_tree_model_get_iter_first(model, &iter);
	while (valid) {
		gchar *u_needle;
		Note *note;
		gtk_tree_model_get(model, &iter, NOTE_COLUMN, &note, -1);

		u_needle = g_utf8_casefold(note->title, -1);

		found = u_haystack;

		while ( (found = strstr(found, u_needle)) != NULL ) {
			SearchHit *hit = g_new0(SearchHit, 1);
			hit->note = note;
			hit->start_offset = g_utf8_pointer_to_offset(u_haystack, found);
			hit->end_offset = hit->start_offset + g_utf8_strlen(u_needle, -1);
			result = g_slist_prepend(result, hit);
			found = found + strlen(u_needle);
		}
		g_free(u_needle);
		valid = gtk_tree_model_iter_next(model, &iter);
	}

	g_timer_stop(search_timer);

	g_timer_elapsed(search_timer, &micro);
	/*g_printerr("Search took %lu microseconds \n", micro);*/

	g_free(u_haystack);

	return result;
}

static void
extend_block(GtkTextIter *start_iter, GtkTextIter *end_iter, gint max_len, GtkTextTag *tag)
{
	/* Set start_iter max_len chars to the left or to the start of the line */
	if (gtk_text_iter_get_line_offset(start_iter) - max_len > 0) {
		gtk_text_iter_backward_chars(start_iter, max_len);
	} else {
		gtk_text_iter_set_line_offset(start_iter, 0);
	}

	/* Set end_iter max_len chars to the right or to the end of the line */
	if (gtk_text_iter_get_line_offset(end_iter) + max_len < gtk_text_iter_get_chars_in_line(end_iter) ) {
		gtk_text_iter_forward_chars(end_iter, max_len);
	} else {
		gtk_text_iter_forward_to_line_end(end_iter);
	}

	/* Expand selection to the left, if there is a link_tag inside */
	if (gtk_text_iter_has_tag(start_iter, tag)) {
		gtk_text_iter_backward_to_tag_toggle(start_iter, tag);
	}

	/* Expand selection to the right, if there is a link_tag inside */
	if (gtk_text_iter_has_tag(end_iter, tag)) {
		gtk_text_iter_forward_to_tag_toggle(end_iter, tag);
	}
}

static void
highlight_titles(Note *note, GtkTextBuffer *buffer, GtkTextIter *start_iter, GtkTextIter *end_iter)
{
	GtkTextTag *url_tag  = gtk_text_tag_table_lookup(buffer->tag_table, "link:url");
	/* For all titles look if they occure in haystack. For all matches apply tag */
	GSList *hits = find_titles(gtk_text_buffer_get_slice(buffer, start_iter, end_iter, FALSE));
	while (hits != NULL) {
		GtkTextIter *hit_start, *hit_end;
		SearchHit *hit = (SearchHit*)hits->data;

		hit_start = gtk_text_iter_copy(start_iter);
		gtk_text_iter_forward_chars(hit_start, hit->start_offset);

		hit_end = gtk_text_iter_copy(start_iter);
		gtk_text_iter_forward_chars(hit_end, hit->end_offset);

		/* Only link agains words or sentencens */
		if ( (!gtk_text_iter_starts_word(hit_start) && !gtk_text_iter_starts_sentence(hit_start)) ||
				(!gtk_text_iter_ends_word(hit_end) && !gtk_text_iter_ends_sentence(hit_end))) {
			hits = hits->next;
			continue;
		}

		/* Don´t link against the note itself */
		if (hit->note == note) {
			hits = hits->next;
			continue;
		}

		/* Don't create links inside external links */
		if (gtk_text_iter_has_tag(hit_start, url_tag)) {
			hits = hits->next;
			continue;
		}

		/* Apply the tag */
		gtk_text_buffer_apply_tag_by_name(buffer, "link:internal", hit_start, hit_end);

		hits = hits->next;
	}

	g_slist_free(hits);
}

void auto_highlight_links(Note *note, GtkTextIter *start_iter, GtkTextIter *end_iter)
{
	GtkTextBuffer *buffer = note->ui->buffer;
	GtkTextTag *link_tag = gtk_text_tag_table_lookup(buffer->tag_table, "link:internal");
	int max_len = get_length_of_longest_title();
	
	/* Grow the block which will be checked for links */
	extend_block(start_iter, end_iter, max_len, link_tag);
	
	/* Remove existing link tag */
	gtk_text_buffer_remove_tag(buffer, link_tag, start_iter, end_iter);

	/* Add links */
	highlight_titles(note, buffer, start_iter, end_iter);
}


