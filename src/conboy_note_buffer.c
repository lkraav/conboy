/* This file is part of Conboy.
 *
 * Copyright (C) 2009 Cornelius Hald
 *
 * Conboy is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Conboy is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Conboy. If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <glib/gprintf.h>
#include <libxml/encoding.h>
#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>

#include "app_data.h"
#include "conboy_xml.h"
#include "conboy_note_buffer.h"

#define ELEMENT_IS(name) (strcmp (element_name, (name)) == 0)

G_DEFINE_TYPE(ConboyNoteBuffer, conboy_note_buffer, GTK_TYPE_TEXT_BUFFER)

static void
conboy_note_buffer_init (ConboyNoteBuffer *self)
{
	self->active_tags = NULL;
}

static void
conboy_note_buffer_dispose(GObject *object)
{
	ConboyNoteBuffer *self = CONBOY_NOTE_BUFFER(object);

	GSList *list = self->active_tags;
	while (list != NULL) {
		g_free(list->data);
		list = list->next;
	}
	g_slist_free(list);
	self->active_tags = NULL;

	G_OBJECT_CLASS(conboy_note_buffer_parent_class)->dispose(object);
}

static void
conboy_note_buffer_class_init (ConboyNoteBufferClass *klass)
{
	G_OBJECT_CLASS(klass)->dispose = conboy_note_buffer_dispose;
}


/*
 * Implementation
 */

ConboyNoteBuffer*
conboy_note_buffer_new()
{
	return g_object_new(CONBOY_TYPE_NOTE_BUFFER, NULL);
}

void
conboy_note_buffer_add_active_tag (ConboyNoteBuffer *self, GtkTextTag *tag)
{
	g_return_if_fail(self != NULL);
	g_return_if_fail(tag != NULL);
	g_return_if_fail(CONBOY_IS_NOTE_BUFFER(self));
	g_return_if_fail(GTK_IS_TEXT_TAG(tag));

	if (strncmp(tag->name, "depth", 5) == 0) {
		g_printerr("WARNING: Trying to add depth tag. Not added\n", tag->name);
		return;
	}

	GSList *tags = tags = self->active_tags;
	while (tags != NULL) {
		if (strcmp(GTK_TEXT_TAG(tags->data)->name, tag->name) == 0) {
			return;
		}
		tags = tags->next;
	}
	self->active_tags = g_slist_prepend(self->active_tags, tag);
}

void
conboy_note_buffer_remove_active_tag (ConboyNoteBuffer *self, GtkTextTag *tag)
{
	g_return_if_fail(self != NULL);
	g_return_if_fail(tag != NULL);
	g_return_if_fail(CONBOY_IS_NOTE_BUFFER(self));
	g_return_if_fail(GTK_IS_TEXT_TAG(tag));

	GSList *tags = self->active_tags;
	while (tags != NULL) {
		if (strcmp(GTK_TEXT_TAG(tags->data)->name, tag->name) == 0) {
			self->active_tags = g_slist_remove(self->active_tags, tags->data);
			return;
		}
		tags = tags->next;
	}
}

void
conboy_note_buffer_add_active_tag_by_name (ConboyNoteBuffer *self, const gchar *tag_name)
{
	g_return_if_fail(self != NULL);
	g_return_if_fail(tag_name != NULL);
	g_return_if_fail(CONBOY_IS_NOTE_BUFFER(self));

	conboy_note_buffer_add_active_tag(self, gtk_text_tag_table_lookup(GTK_TEXT_BUFFER(self)->tag_table, tag_name));
}

void
conboy_note_buffer_remove_active_tag_by_name (ConboyNoteBuffer *self, const gchar *tag_name)
{
	g_return_if_fail(self != NULL);
	g_return_if_fail(tag_name != NULL);
	g_return_if_fail(CONBOY_IS_NOTE_BUFFER(self));

	conboy_note_buffer_remove_active_tag(self, gtk_text_tag_table_lookup(GTK_TEXT_BUFFER(self)->tag_table, tag_name));
}

GSList*
conboy_note_buffer_get_active_tags (ConboyNoteBuffer *self)
{
	g_return_val_if_fail(self != NULL, NULL);
	g_return_val_if_fail(CONBOY_IS_NOTE_BUFFER(self), NULL);

	return self->active_tags;
}

void
conboy_note_buffer_clear_active_tags (ConboyNoteBuffer *self)
{
	g_return_if_fail(self != NULL);
	g_return_if_fail(CONBOY_IS_NOTE_BUFFER(self));

	g_slist_free(self->active_tags);
	self->active_tags = NULL;
}

void
conboy_note_buffer_set_active_tags (ConboyNoteBuffer *self, GSList *tags)
{
	g_return_if_fail(self != NULL);
	g_return_if_fail(tags != NULL);
	g_return_if_fail(CONBOY_IS_NOTE_BUFFER(self));

	/* Free the list */
	g_slist_free(self->active_tags);
	self->active_tags = NULL;

	/* Copy elements of given list, but ignore depth tags */
	while (tags != NULL) {
		GtkTextTag *tag = GTK_TEXT_TAG(tags->data);
		if (strncmp(tag->name, "depth", 5 != 0)) {
			self->active_tags = g_slist_prepend(self->active_tags, tag);
		}
		tags = tags->next;
	}
}

void
conboy_note_buffer_update_active_tags (ConboyNoteBuffer *self)
{
	g_return_if_fail(self != NULL);
	g_return_if_fail(CONBOY_IS_NOTE_BUFFER(self));

	/* Clean the list of active tags */
	conboy_note_buffer_clear_active_tags(self);

	/* Add tags at this location */
	GtkTextIter location;
	gtk_text_buffer_get_iter_at_mark(GTK_TEXT_BUFFER(self), &location, gtk_text_buffer_get_insert(GTK_TEXT_BUFFER(self)));
	GSList *tags = gtk_text_iter_get_tags(&location);
	if (tags != NULL) {
		conboy_note_buffer_set_active_tags(self, tags);
	}

	/* Go the beginning of line and check if there is a bullet.
	 * If yes, add list-item and list tags */
	gtk_text_iter_set_line_offset(&location, 0);
	if (conboy_note_buffer_find_depth_tag(&location) != NULL) {
		/* Add tags */
		conboy_note_buffer_add_active_tag_by_name(self, "list-item");
		conboy_note_buffer_add_active_tag_by_name(self, "list");
	}
}

GtkTextTag*
conboy_note_buffer_get_depth_tag(ConboyNoteBuffer *buffer, gint depth)
{
	GtkTextBuffer *buf = GTK_TEXT_BUFFER(buffer);
	GtkTextTag *tag;
	gchar depth_str[5] = {0};
	gchar *tag_name;

	if (depth < 1) {
		g_printerr("ERROR: buffer_get_depth_tag(): depth must be at least 1. Not: %i \n", depth);
	}

	g_sprintf(depth_str, "%i", depth);
	tag_name = g_strconcat("depth", ":", depth_str, NULL);

	tag = gtk_text_tag_table_lookup(buf->tag_table, tag_name);

	if (tag == NULL) {
		tag = gtk_text_buffer_create_tag(buf, tag_name, "indent", -20, "left-margin", depth * 25, NULL);
	}

	g_free(tag_name);

	return tag;
}

static gboolean
tag_is_depth_tag(GtkTextTag *tag)
{
	if (tag == NULL) {
		return FALSE;
	}
	return (strncmp(tag->name, "depth", 5) == 0);
}

static gint
tag_get_depth(GtkTextTag *tag)
{
	if (tag_is_depth_tag(tag)) {
		char **strings = g_strsplit(tag->name, ":", 2);
		int depth = atoi(strings[1]);
		g_strfreev(strings);
		return depth;
	}
	return 0;
}

/* char *_bullets[] = {"\u2022 ", "\u2218 ", "\u2023 ", "\u2043 ", "\u204d ", "\u2219 ", "\u25e6 "}; */
/* These 3 bullets work with diablo and standard font */
char *_bullets[] = {"\u2022 ", "\u25e6 ", "\u2219 "};
static const gchar*
get_bullet_by_depth(gint depth) {
	if (depth <= 0) {
		g_printerr("ERROR: get_bullets_by_depth(): depth must be at least 1.\n");
		return "\u2022 ";
	}
	return _bullets[(depth - 1) % 3];
}

static const gchar*
get_bullet_by_depth_tag(GtkTextTag *tag) {
	return get_bullet_by_depth(tag_get_depth(tag));
}

static void
add_bullets(ConboyNoteBuffer *buffer, gint start_line, gint end_line, GtkTextTag *depth_tag)
{
	GtkTextBuffer *buf = GTK_TEXT_BUFFER(buffer);
	gint i = 0;
	GtkTextIter start_iter, end_iter;
	gint total_lines = gtk_text_buffer_get_line_count(buf);

	/* For each selected line */
	for (i = start_line; i <= end_line; i++) {

		gtk_text_buffer_get_iter_at_line(buf, &start_iter, i);
		/* If there is already a bullet, skip this line */
		if (conboy_note_buffer_find_depth_tag(&start_iter)) {
			continue;
		}

		/* Insert bullet character */
		//gtk_text_buffer_get_iter_at_line(buf, &start_iter, i);
		gtk_text_buffer_insert(buf, &start_iter, get_bullet_by_depth_tag(depth_tag), -1);

		/* Remove existing tags from the bullet and add the <depth> tag */
		gtk_text_buffer_get_iter_at_line_offset(buf, &start_iter, i, 0);
		gtk_text_buffer_get_iter_at_line_offset(buf, &end_iter, i, 2);
		gtk_text_buffer_remove_all_tags(buf, &start_iter, &end_iter);
		gtk_text_buffer_apply_tag(buf, depth_tag, &start_iter, &end_iter);

		/* Surround line (starting after BULLET) with "list-item" tags */
		gtk_text_buffer_get_iter_at_line_offset(buf, &start_iter, i, 2); /* Jump bullet */
		if (i == end_line) {
			gtk_text_buffer_get_iter_at_line(buf, &end_iter, i);
			gtk_text_iter_forward_to_line_end(&end_iter);
		} else {
			gtk_text_buffer_get_iter_at_line(buf, &end_iter, i + 1);
		}
		gtk_text_buffer_apply_tag_by_name(buf, "list-item", &start_iter, &end_iter);
	}

	/* Surround everything it with "list" tags */
	/* Check line above and below. If one or both are bullet lines, include the newline chars at the beginning and the end */
	/* This is done, so that there are no gaps in the <list> tag and that it really surrounds the whole list. */

	/* Set start iter */
	if (start_line > 0) {
		gtk_text_buffer_get_iter_at_line(buf, &start_iter, start_line - 1);
		if (conboy_note_buffer_find_depth_tag(&start_iter) != NULL) {
			gtk_text_iter_forward_to_line_end(&start_iter);
		} else {
			gtk_text_buffer_get_iter_at_line(buf, &start_iter, start_line);
		}
	} else {
		gtk_text_buffer_get_iter_at_line(buf, &start_iter, start_line);
	}

	/* Set end iter */
	if (end_line < total_lines - 1) {
		gtk_text_buffer_get_iter_at_line(buf, &end_iter, end_line + 1);
		if (conboy_note_buffer_find_depth_tag(&end_iter) == NULL) {
			gtk_text_buffer_get_iter_at_line(buf, &end_iter, end_line);
			gtk_text_iter_forward_to_line_end(&end_iter);
		}
	} else {
		gtk_text_buffer_get_iter_at_line(buf, &end_iter, end_line);
		gtk_text_iter_forward_to_line_end(&end_iter);
	}

	gtk_text_buffer_apply_tag_by_name(buf, "list", &start_iter, &end_iter);
}






static void
remove_bullets(ConboyNoteBuffer *buffer, GtkTextIter *start_iter, GtkTextIter *end_iter)
{
	GtkTextBuffer *buf = GTK_TEXT_BUFFER(buffer);
	gint i = 0;
	GtkTextIter start = *start_iter;
	GtkTextIter end = *end_iter;
	gint start_line = gtk_text_iter_get_line(&start);
	gint end_line = gtk_text_iter_get_line(&end);

	/* Remove tags */
	gtk_text_iter_set_line_offset(&start, 0);
	gtk_text_iter_set_line_offset(&end, 0);
	gtk_text_iter_forward_to_line_end(&end);

	/* Include the newline char before and after this line */
	gtk_text_iter_backward_char(&start);
	gtk_text_iter_forward_char(&end);

	gtk_text_buffer_remove_tag_by_name(buf, "list-item", &start, &end);
	gtk_text_buffer_remove_tag_by_name(buf, "list", &start, &end);

	conboy_note_buffer_remove_active_tag_by_name(buffer, "list");
	conboy_note_buffer_remove_active_tag_by_name(buffer, "list-item");

	/* Remove bullets */
	for (i = start_line; i <= end_line; i++) {
		gtk_text_buffer_get_iter_at_line(buf, &start, i);
		gtk_text_buffer_get_iter_at_line(buf, &end, i);
		gtk_text_iter_forward_chars(&end, 2);
		//gtk_text_buffer_remove_all_tags(buf, &start, &end); /* NEW */
		gtk_text_buffer_delete(buf, &start, &end);
	}
}

void
conboy_note_buffer_enable_bullets(ConboyNoteBuffer *buffer)
{
	GtkTextBuffer *buf = GTK_TEXT_BUFFER(buffer);
	GtkTextIter start_iter, end_iter;

	if (gtk_text_buffer_get_has_selection(buf)) {
		/* Something is selected */
		gtk_text_buffer_get_selection_bounds(buf, &start_iter, &end_iter);
		add_bullets(buffer, gtk_text_iter_get_line(&start_iter), gtk_text_iter_get_line(&end_iter), conboy_note_buffer_get_depth_tag(buffer, 1));

	} else {
		/* Nothing is selected */
		int line;
		gtk_text_buffer_get_iter_at_mark(buf, &start_iter, gtk_text_buffer_get_insert(buf));
		line = gtk_text_iter_get_line(&start_iter);
		add_bullets(buffer, line, line, conboy_note_buffer_get_depth_tag(buffer, 1));

		conboy_note_buffer_add_active_tag_by_name(buffer, "list-item");
		conboy_note_buffer_add_active_tag_by_name(buffer, "list");
	}
}

void
conboy_note_buffer_disable_bullets(ConboyNoteBuffer *buffer)
{
	GtkTextBuffer *buf = GTK_TEXT_BUFFER(buffer);
	GtkTextIter start_iter, end_iter;

	if (gtk_text_buffer_get_has_selection(buf)) {
		/* Something is selected */
		gtk_text_buffer_get_selection_bounds(buf, &start_iter, &end_iter);
		remove_bullets(buffer, &start_iter, &end_iter);

		conboy_note_buffer_remove_active_tag_by_name(buffer, "list-item");
		conboy_note_buffer_remove_active_tag_by_name(buffer, "list");

	} else {
		/* Nothing is selected */
		gtk_text_buffer_get_iter_at_mark(buf, &start_iter, gtk_text_buffer_get_insert(buf));
		gtk_text_buffer_get_iter_at_mark(buf, &end_iter, gtk_text_buffer_get_insert(buf));
		remove_bullets(buffer, &start_iter, &end_iter);

		conboy_note_buffer_remove_active_tag_by_name(buffer, "list-item");
		conboy_note_buffer_remove_active_tag_by_name(buffer, "list");
	}
}


void
conboy_note_buffer_decrease_indent(ConboyNoteBuffer *buffer, gint start_line, gint end_line)
{
	GtkTextBuffer *buf = GTK_TEXT_BUFFER(buffer);
	GtkTextIter start_iter, end_iter;
	GtkTextTag *old_tag;
	GtkTextTag *new_tag;
	gint i;

	for (i = start_line; i <= end_line; i++) {

		gtk_text_buffer_get_iter_at_line(buf, &start_iter, i);
		end_iter = start_iter;

		old_tag = conboy_note_buffer_find_depth_tag(&start_iter);

		if (old_tag != NULL) {
			gint depth = tag_get_depth(old_tag);
			gtk_text_iter_set_line_offset(&end_iter, 2);

			if (depth == 1) {
				remove_bullets(buffer, &start_iter, &end_iter);
				continue;
			}

			/* Remove old tag */
			gtk_text_buffer_remove_tag(buf, old_tag, &start_iter, &end_iter);

			/* Delete old bullet */
			gtk_text_buffer_delete(buf, &start_iter, &end_iter);

			depth--;
			new_tag = conboy_note_buffer_get_depth_tag(buffer, depth);

			/* Add new bullet with new tag */
			add_bullets(buffer, i, i, new_tag);
		}
	}

	conboy_note_buffer_update_active_tags(buffer);
}

void
conboy_note_buffer_increase_indent(ConboyNoteBuffer *buffer, gint start_line, gint end_line)
{
	GtkTextBuffer *buf = GTK_TEXT_BUFFER(buffer);
	GtkTextIter start_iter, end_iter;
	GtkTextTag *old_tag;
	GtkTextTag *new_tag;
	gint i;

	for (i = start_line; i <= end_line; i++) {

		gtk_text_buffer_get_iter_at_line(buf, &start_iter, i);
		end_iter = start_iter;

		old_tag = conboy_note_buffer_find_depth_tag(&start_iter);

		gint depth = 0;
		/* If already bullet list, remove bullet and tags */
		if (old_tag != NULL) {
			depth = tag_get_depth(old_tag);
			gtk_text_iter_set_line_offset(&end_iter, 2);

			/* Remove old tag */
			gtk_text_buffer_remove_tag(buf, old_tag, &start_iter, &end_iter);

			/* Delete old bullet */
			gtk_text_buffer_delete(buf, &start_iter, &end_iter);
		}

		depth++;
		new_tag = conboy_note_buffer_get_depth_tag(buffer, depth);

		/* Add new bullet with new tag */
		add_bullets(buffer, i, i, new_tag);
	}

	conboy_note_buffer_update_active_tags(buffer);
}


static gboolean
line_needs_bullet(GtkTextBuffer *buffer, gint line_number)
{
	GtkTextIter iter;
	gtk_text_buffer_get_iter_at_line(buffer, &iter, line_number);

	while (!gtk_text_iter_ends_line(&iter)) {
		switch (gtk_text_iter_get_char(&iter))
		{
		case ' ':
			gtk_text_iter_forward_char(&iter);
			break;

		case '*':
		case '-':
			gtk_text_iter_forward_char(&iter);
			return (gtk_text_iter_get_char(&iter) == ' ');
			break;

		default:
			return FALSE;
		}
	}
	return FALSE;
}

gboolean
conboy_note_buffer_add_new_line(UserInterface *ui)
{
	GtkTextBuffer *buf = ui->buffer;
	ConboyNoteBuffer *buffer = CONBOY_NOTE_BUFFER(buf);
	GtkTextView *view = ui->view;
	GtkTextIter iter;
	GtkTextTag *depth_tag;
	gint line;

	gtk_text_buffer_get_iter_at_mark(buf, &iter, gtk_text_buffer_get_insert(buf));
	gtk_text_iter_set_line_offset(&iter, 0);
	depth_tag = conboy_note_buffer_find_depth_tag(&iter);

	/* If line starts with a bullet */
	if (depth_tag != NULL) {

		/* If line is not empty, add new bullet. Else remove bullet. */
		gtk_text_iter_forward_to_line_end(&iter);
		if (gtk_text_iter_get_line_offset(&iter) > 2) {
			GSList *tmp;

			/* Remove all tags but <list> from active tags */
			tmp = g_slist_copy(conboy_note_buffer_get_active_tags(CONBOY_NOTE_BUFFER(buffer)));
			conboy_note_buffer_clear_active_tags(buffer);
			conboy_note_buffer_add_active_tag_by_name(buffer, "list");

			/* Insert newline and bullet */
			gtk_text_buffer_get_iter_at_mark(buf, &iter, gtk_text_buffer_get_insert(buf));
			if (gtk_text_iter_get_line_offset(&iter) < 2) {
				gtk_text_iter_set_line_offset(&iter, 2);
			}
			gtk_text_buffer_insert(buf, &iter, "\n", -1);
			gtk_text_view_scroll_mark_onscreen(view, gtk_text_buffer_get_insert(buf));
			line = gtk_text_iter_get_line(&iter);
			add_bullets(buffer, line, line, depth_tag);

			/* Add all tags back to active tags */
			conboy_note_buffer_set_active_tags(buffer, tmp);

			return TRUE;

		} else {
			/* Remove bullet and insert newline */
			GtkTextIter start = iter;
			GtkTextIter end = iter;

			/* Disable list and list-item tags */
			conboy_note_buffer_remove_active_tag_by_name(buffer, "list-item");
			conboy_note_buffer_remove_active_tag_by_name(buffer, "list");

			/* Delete the bullet and the last newline */
			gtk_text_iter_set_line_offset(&start, 0);
			gtk_text_iter_set_line_offset(&end, 0);
			gtk_text_iter_set_line_offset(&iter, 0);
			gtk_text_iter_forward_to_line_end(&end);
			gtk_text_iter_forward_to_line_end(&iter);
			gtk_text_iter_forward_char(&iter);
			gtk_text_buffer_remove_all_tags(buf, &start, &iter);
			gtk_text_buffer_delete(buf, &start, &end);

			gtk_text_buffer_insert(buf, &end, "\n", -1);

			gtk_text_view_scroll_mark_onscreen(ui->view, gtk_text_buffer_get_insert(buf));

			/* Disable the bullet button */
			conboy_note_buffer_update_active_tags(buffer);
			conboy_note_window_update_button_states(ui);

			return TRUE;
		}

	} else {

		/* If line start with a "- " or "- *" */
		if (line_needs_bullet(buf, gtk_text_iter_get_line(&iter))) {
			GSList *tmp;
			GtkTextIter end_iter;

			gtk_text_iter_set_line_offset(&iter, 0);
			end_iter = iter;
			line = gtk_text_iter_get_line(&iter);

			/* Skip trailing spaces */
			while (gtk_text_iter_get_char(&end_iter) == ' ') {
				gtk_text_iter_forward_char(&end_iter);
			}
			/* Skip "* " or "- " */
			gtk_text_iter_forward_chars(&end_iter, 2);

			/* Delete this part */
			gtk_text_buffer_delete(buf, &iter, &end_iter);

			/* Add bullet for this line */
			add_bullets(buffer, line, line, conboy_note_buffer_get_depth_tag(buffer, 1));


			/* TODO: Copied from above */

			/* Remove all tags but <list> from active tags */
			tmp = g_slist_copy(conboy_note_buffer_get_active_tags(buffer));
			conboy_note_buffer_clear_active_tags(buffer);
			conboy_note_buffer_add_active_tag_by_name(buffer, "list");

			/* Insert newline and bullet */
			gtk_text_buffer_get_iter_at_mark(buf, &iter, gtk_text_buffer_get_insert(buf));
			gtk_text_buffer_insert(buf, &iter, "\n", -1);
			line = gtk_text_iter_get_line(&iter);
			add_bullets(buffer, line, line, conboy_note_buffer_get_depth_tag(buffer, 1));
			gtk_text_view_scroll_mark_onscreen(view, gtk_text_buffer_get_insert(buf));

			/* Add all tags back to active tags */
			if (tmp != NULL) {
				conboy_note_buffer_set_active_tags(buffer, tmp);
			}

			/* Turn on the list-item tag from here on */
			conboy_note_buffer_add_active_tag_by_name(buffer, "list-item");

			/* Revaluate (turn on) the bullet button */
			conboy_note_window_update_button_states(ui);

			return TRUE;
		}

	}

	return FALSE;

}

/*
static
gboolean tag_is_depth_tag(GtkTextTag *tag)
{
	if (tag == NULL) {
		return FALSE;
	}
	return (strncmp(tag->name, "depth", 5) == 0);
}*/

GtkTextTag*
conboy_note_buffer_find_depth_tag(GtkTextIter* iter)
{
	GSList *tags = gtk_text_iter_get_tags(iter);
	while (tags != NULL) {
		if (tag_is_depth_tag(tags->data)) {
			return tags->data;
		}
		tags = tags->next;
	}
	return NULL;
}

/**
 * Change the selection on the buffer to not cut through any bullets.
 */
static void
augment_selection(ConboyNoteBuffer *buffer, GtkTextIter *start, GtkTextIter *end)
{
	GtkTextBuffer *buf = GTK_TEXT_BUFFER(buffer);

	GtkTextTag *start_depth = conboy_note_buffer_find_depth_tag(start);
	GtkTextTag *end_depth = conboy_note_buffer_find_depth_tag(end);

	GtkTextIter inside_end = *end;
	gtk_text_iter_backward_char(&inside_end);

	GtkTextTag *inside_end_depth = conboy_note_buffer_find_depth_tag(&inside_end);

	/* Start inside bullet region */
	if (start_depth != NULL) {
		gtk_text_iter_set_line_offset(start, 2);
		gtk_text_buffer_select_range(buf, start, end);
	}

	/* End inside another bullet */
	if (inside_end_depth != NULL) {
		gtk_text_iter_set_line_offset(end, 2);
		gtk_text_buffer_select_range(buf, start, end);
	}

	/* Check if the end is right before the start of the bullet */
	if (end_depth != NULL){
		gtk_text_iter_set_line_offset(end, 2);
		gtk_text_buffer_select_range(buf, start, end);
	}

}

void
conboy_note_buffer_check_selection(ConboyNoteBuffer *buffer)
{
	GtkTextBuffer *buf = GTK_TEXT_BUFFER(buffer);
	GtkTextIter start;
	GtkTextIter end;

	if (gtk_text_buffer_get_selection_bounds(buf, &start, &end)) {
		augment_selection(buffer, &start, &end);
	} else {
		if (gtk_text_iter_get_line_offset(&start) < 2 && conboy_note_buffer_find_depth_tag(&start) != NULL) {
			gtk_text_iter_set_line_offset(&start, 2);
			gtk_text_buffer_select_range(buf, &start, &start); // Needed?
		}
	}
}

static gboolean
line_is_bullet_line(GtkTextIter *line_iter)
{
	GtkTextIter iter = *line_iter;
	gtk_text_iter_set_line(&iter, gtk_text_iter_get_line(line_iter));
	if (conboy_note_buffer_find_depth_tag(&iter)) {
		return TRUE;
	} else {
		return FALSE;
	}
}

gboolean
conboy_note_buffer_backspace_handler(ConboyNoteBuffer *buffer)
{
	GtkTextBuffer *buf = GTK_TEXT_BUFFER(buffer);
	GtkTextIter start;
	GtkTextIter end;
	gboolean selection = gtk_text_buffer_get_selection_bounds(buf, &start, &end);
	GtkTextTag *depth = conboy_note_buffer_find_depth_tag(&start);

	if (selection) {
		augment_selection(buffer, &start, &end);
		gtk_text_buffer_delete(buf, &start, &end);
		return TRUE;

	} else {
		GtkTextIter prev = start;
		if (gtk_text_iter_get_line_offset(&prev) != 0) {
			gtk_text_iter_backward_char(&prev);
		}
		GtkTextTag *prev_depth = conboy_note_buffer_find_depth_tag(&prev);
		if (depth != NULL || prev_depth != NULL) {
			int lineNr = gtk_text_iter_get_line(&start);
			conboy_note_buffer_decrease_indent(buffer, lineNr, lineNr);
			return TRUE;
		} else {
			/* Check if cursor is on the right side of a soft line break. If yes, remove it */
			prev = start;
			gtk_text_iter_backward_chars(&prev, 2);
			if (gtk_text_iter_get_char(&prev) == "\u2028") {
				GtkTextIter end_break = prev;
				gtk_text_iter_forward_char(&end_break);
				gtk_text_buffer_delete(buf, &prev, &end_break);
			}
		}
	}

	return FALSE;
}

gboolean
conboy_note_buffer_delete_handler(ConboyNoteBuffer *buffer)
{
	GtkTextBuffer *buf = GTK_TEXT_BUFFER(buffer);
	GtkTextIter start;
	GtkTextIter end;

	if (gtk_text_buffer_get_selection_bounds(buf, &start, &end)) {
		augment_selection(buffer, &start, &end);
		gtk_text_buffer_delete(buf, &start, &end);
		return TRUE;

	} else if (gtk_text_iter_ends_line(&start) && gtk_text_iter_get_line(&start) < gtk_text_buffer_get_line_count(buf)) {
		GtkTextIter next;
		gtk_text_buffer_get_iter_at_line(buf, &next, gtk_text_iter_get_line(&start) + 1);
		end = start;
		gtk_text_iter_forward_chars(&end, 3);

		GtkTextTag *depth = conboy_note_buffer_find_depth_tag(&next);

		if (depth != NULL) {
			gtk_text_buffer_delete(buf, &start, &end);
			return TRUE;
		}

	} else {
		GtkTextIter next = start;

		if (gtk_text_iter_get_line_offset(&next) != 0) {
			gtk_text_iter_forward_char(&next);
		}

		GtkTextTag *depth = conboy_note_buffer_find_depth_tag(&start);
		GtkTextTag *nextDepth = conboy_note_buffer_find_depth_tag(&next);
		if (depth != NULL || nextDepth != NULL) {
			gint lineNr = gtk_text_iter_get_line(&start);
			conboy_note_buffer_decrease_indent(buffer, lineNr, lineNr);
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * Add or remove list and list-item tags, depending whether or not the
 * cursor is in a list.
 */
void
conboy_note_buffer_fix_list_tags(ConboyNoteBuffer *buffer, GtkTextIter *start_iter, GtkTextIter *end_iter)
{
	GtkTextBuffer *buf = GTK_TEXT_BUFFER(buffer);
	GtkTextIter start = *start_iter;
	GtkTextIter end   = *end_iter;

	if (line_is_bullet_line(&start)) {
		/* Add list and list-item tags to complete line */
		gtk_text_iter_set_line(&start, gtk_text_iter_get_line(&start));
		gtk_text_iter_set_line(&end, gtk_text_iter_get_line(&end));
		gtk_text_iter_forward_to_line_end(&end);
		gtk_text_buffer_apply_tag_by_name(buf, "list", &start, &end);
		gtk_text_iter_forward_chars(&start, 2);
		gtk_text_buffer_apply_tag_by_name(buf, "list-item", &start, &end);
		conboy_note_buffer_add_active_tag_by_name(buffer, "list");
		conboy_note_buffer_add_active_tag_by_name(buffer, "list-item");
	} else {
		/* Remove list and list-item tags from complete line */
		gtk_text_iter_set_line(&start, gtk_text_iter_get_line(&start));
		gtk_text_iter_set_line(&end, gtk_text_iter_get_line(&end));
		/* Remove tags, only if the line is not empty. Otherwise the next line would be taken. */
		gunichar c = gtk_text_iter_get_char(&end);
		if (g_unichar_break_type(c) != G_UNICODE_BREAK_LINE_FEED) {
			gtk_text_iter_forward_to_line_end(&end);
			gtk_text_iter_forward_char(&end);
			gtk_text_buffer_remove_tag_by_name(buf, "list", &start, &end);
			gtk_text_buffer_remove_tag_by_name(buf, "list-item", &start, &end);
			conboy_note_buffer_remove_active_tag_by_name(buffer, "list");
			conboy_note_buffer_remove_active_tag_by_name(buffer, "list-item");
		}
	}
}


/*
 * XML handling
 */



/* TODO: Get rid of the global variables */
gint _depth = 0;
gint _new_depth = 0;
gboolean _list_active = FALSE;
gboolean _is_bullet = FALSE;

/*
typedef struct {
	gint depth;
	gint new_depth;
	gboolean list_active;
	gboolean is_bullet;
} ReadContext;

*/

typedef struct {
	gint depth;
	gint new_depth;
	gboolean list_active;
	gboolean is_bullet;
} Info;

/**
 * Writes a start element.
 */
static void write_start_element(GtkTextTag *tag, xmlTextWriter *writer)
{
	gchar** strings;
	gchar *tag_name;

	tag_name = tag->name;

	/* Ignore tags that start with "_". They are considered internal. */
	if (strncmp(tag_name, "_", 1) == 0) {
		return;
	}

	/* Ignore <list> tags. */
	if (strncmp(tag_name, "list", -1) == 0) {
		_list_active = TRUE;
		return;
	}

	/* Use tag_get_depth() here. Currently in callbacks.c */
	/* If a <depth> tag, ignore */
	if (strncmp(tag_name, "depth", 5) == 0) {
		_is_bullet = TRUE;
		/* NEW */
		strings = g_strsplit(tag_name, ":", 2);
		_new_depth = atoi(strings[1]);
		g_strfreev(strings);
		/* /NEW */
		return;
	}

	/* If not a <list-item> tag, write it and return */
	if (strncmp(tag_name, "list-item", 9) != 0) {
		xmlTextWriterStartElement(writer, BAD_CAST tag_name);
		return;
	}

	/* It is a <list-item:*> tag */
	if (_new_depth < _depth) {
		gint diff = _depth - _new_depth;

		/* </list-item> */
		xmlTextWriterEndElement(writer);

		while (diff > 0) { /* For each depth we need to close a <list-item> and a <list> tag. */
			/* </list> */
			xmlTextWriterEndElement(writer);
			/* </list-item> */
			xmlTextWriterEndElement(writer);
			diff--;
		}

		/* <list-item dir=ltr> */
		xmlTextWriterStartElement(writer, BAD_CAST "list-item");
		xmlTextWriterWriteAttribute(writer, BAD_CAST "dir", BAD_CAST "ltr");
	}


	/* If there was an increase in depth, we need to add a <list> tag */
	if (_new_depth > _depth) {
		gint diff = _new_depth - _depth;

		while (diff > 0) {
			xmlTextWriterStartElement(writer, BAD_CAST "list");
			xmlTextWriterStartElement(writer, BAD_CAST "list-item");
			xmlTextWriterWriteAttribute(writer, BAD_CAST "dir", BAD_CAST "ltr");
			diff--;
		}

	} else if (_new_depth == _depth) {
		xmlTextWriterEndElement(writer); /* </list-item> */
		xmlTextWriterStartElement(writer, BAD_CAST "list-item");
		xmlTextWriterWriteAttribute(writer, BAD_CAST "dir", BAD_CAST "ltr");
	}

	_depth = _new_depth;
}

/**
 * Writes an end element.
 */
static void write_end_element(GtkTextTag *tag, xmlTextWriter *writer)
{
	gchar *tag_name = tag->name;

	/* Ignore tags that start with "_". They are considered internal. */
	if (strncmp(tag_name, "_", 1) == 0) {
		return;
	}

	/* Ignore depth tags */
	if (strncmp(tag_name, "depth", 5) == 0) {
		_is_bullet = FALSE;
		return;
	}

	/* If the list completely ends, reset the depth */
	if (strncmp(tag_name, "list", -1) == 0) {

		/* Close all open <list-item> and <list> tags */
		while (_depth > 0) {
			/* </list-item> */
			xmlTextWriterEndElement(writer);
			/* </list> */
			xmlTextWriterEndElement(writer);
			_depth--;
		}

		_list_active = FALSE;
		return;
	}

	/* If it is not a <list-item> tag, just close it and return */
	if (strncmp(tag_name, "list-item", 9) != 0) {
		xmlTextWriterEndElement(writer);
		return;
	}
}

/**
 * Writes plain text.
 */
static void write_text(const gchar *text, xmlTextWriter *writer)
{
	/* Don't write bullets to the output */
	if (_is_bullet) {
		return;
	}

	xmlTextWriterWriteString(writer, BAD_CAST text);
}

/**
 * Sort two GtkTextTags by their priority. Higher priority (bigger number)
 * will be sorted to the left of the list
 */
static gint sort_by_prio(GtkTextTag *tag1, GtkTextTag *tag2)
{
	if (tag1->priority == tag2->priority) {
		g_printerr("ERROR: Two tags cannot have the same priority.\n");
		return 0;
	}

	return (tag2->priority - tag1->priority);
}

static void
write_content(xmlTextWriter *writer, GtkTextBuffer *buffer, gdouble version)
{
	int rc = 0;
	GtkTextIter start, end;
	GtkTextIter old_iter, iter;
	gchar *text;
	GSList *start_tags = NULL, *end_tags = NULL;
	gchar version_str[10] = {0};

	/* Disable indentation */
	rc = xmlTextWriterSetIndent(writer, FALSE);

	/* Start note-content element */
	rc = xmlTextWriterStartElement(writer, BAD_CAST "note-content");

	/* Add namespace information so that we are able to use the <content>
	 * sub tree separately. When saving the merged document, the redundant
	 * namespace information is removed again.
	 */
	rc = xmlTextWriterWriteAttributeNS(writer, BAD_CAST "xmlns", BAD_CAST "link", NULL, BAD_CAST "http://beatniksoftware.com/tomboy/link");
	rc = xmlTextWriterWriteAttributeNS(writer, BAD_CAST "xmlns", BAD_CAST "size", NULL, BAD_CAST "http://beatniksoftware.com/tomboy/size");
	rc = xmlTextWriterWriteAttributeNS(writer, NULL, BAD_CAST "xmlns", NULL, BAD_CAST "http://beatniksoftware.com/tomboy");

	/* Add version attribute */
	g_ascii_formatd(version_str, 10, "%.1f", version);
	rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "version", BAD_CAST &version_str);

	/**************************************************/

	gtk_text_buffer_get_bounds(buffer, &start, &end);
	iter = start;
	old_iter = start;

	while (gtk_text_iter_compare(&iter, &end) <= 0) {

		start_tags = gtk_text_iter_get_toggled_tags(&iter, TRUE);
		end_tags   = gtk_text_iter_get_toggled_tags(&iter, FALSE);

		/* Write end tags */
		g_slist_foreach(end_tags, (GFunc)write_end_element, writer);

		/* Sort start tags by priority */
		start_tags = g_slist_sort(start_tags, (GCompareFunc)sort_by_prio);

		/* Write start tags */
		g_slist_foreach(start_tags, (GFunc)write_start_element, writer);

		/* Move iter */
		/* Remember position and set iter to next toggle */
		old_iter = iter;

		if (gtk_text_iter_compare(&iter, &end) >= 0) {
			break;
		}
		gtk_text_iter_forward_to_tag_toggle(&iter, NULL);

		/* Write text */
		text = gtk_text_iter_get_text(&old_iter, &iter);
		write_text(text, writer);
	}

	/**************************************************/

	/* Close note-content */
	rc = xmlTextWriterEndElement(writer);

	/* Insert linebreak like in Tomboy format */
	rc = xmlTextWriterWriteRaw(writer, BAD_CAST "\n");
}


/*
 * Serialization
 */

gchar*
conboy_note_buffer_get_xml (ConboyNoteBuffer *self)
{
	g_return_val_if_fail(self != NULL, NULL);
	g_return_val_if_fail(CONBOY_IS_NOTE_BUFFER(self), NULL);

	xmlBuffer *buf;
	xmlTextWriter *writer;
	gchar *result;

	buf = xmlBufferCreate();
	writer = xmlNewTextWriterMemory(buf, 0);

	/* TODO: Implement version handling */
	write_content(writer, GTK_TEXT_BUFFER(self), 0.1);

	/* Flushes and frees */
	xmlFreeTextWriter(writer);

	result = g_strdup((gchar*)buf->content);

	xmlBufferFree(buf);

	return result;
}




/*
 * Deserialization
 */

typedef enum
{
	STATE_START,
	STATE_CONTENT,
	STATE_LIST,
	STATE_LIST_ITEM
} ParseState;

typedef struct
{
	GSList *tag_stack;
	GSList *state_stack;
	GtkTextIter *iter;
	gint depth;
} ParseContext;

static
void push_state(ParseContext  *ctx, ParseState  state)
{
  ctx->state_stack = g_slist_prepend(ctx->state_stack, GINT_TO_POINTER(state));
}

static
void pop_state(ParseContext *ctx)
{
  g_return_if_fail(ctx->state_stack != NULL);
  ctx->state_stack = g_slist_remove(ctx->state_stack, ctx->state_stack->data);
}

static
ParseState peek_state(ParseContext *ctx)
{
  g_return_val_if_fail(ctx->state_stack != NULL, STATE_START);
  return GPOINTER_TO_INT(ctx->state_stack->data);
}

static
void push_tag(ParseContext *ctx, gchar *tag_name)
{
	ctx->tag_stack = g_slist_prepend(ctx->tag_stack, tag_name);
}

static
void pop_tag(ParseContext *ctx)
{
	g_return_if_fail(ctx->tag_stack != NULL);
	ctx->tag_stack = g_slist_remove(ctx->tag_stack, ctx->tag_stack->data);
}

static
gchar* peek_tag(ParseContext *ctx)
{
	g_return_val_if_fail(ctx->tag_stack != NULL, NULL);
	return ctx->tag_stack->data;
}


static
void handle_start_element(ParseContext *ctx, xmlTextReader *reader)
{
	const gchar *element_name = (const gchar *) xmlTextReaderConstName(reader);

	switch (peek_state(ctx)) {
	case STATE_START:
		if (ELEMENT_IS("note-content")) {
			push_state(ctx, STATE_CONTENT);
		} else {
			g_printerr("ERROR: First element must be <note-content>, not <%s> \n", element_name);
		}
		break;
	case STATE_CONTENT:
	case STATE_LIST_ITEM:
		if (ELEMENT_IS("list")) {
			push_state(ctx, STATE_LIST);
			ctx->depth++;
			break;
		}
		push_tag(ctx, (gchar *)element_name);
		break;

	case STATE_LIST:
		if (ELEMENT_IS("list-item")) {
			push_state(ctx, STATE_LIST_ITEM);
		} else {
			g_printerr("ERROR: <list> may only contain <list-item>, not <%s>.", element_name);
		}
		break;

	default:
		g_assert_not_reached();
		break;
	}
}

static
void handle_end_element(ParseContext *ctx, xmlTextReader *reader) {

	const gchar *element_name = (const gchar *) xmlTextReaderConstName(reader);

	switch (peek_state(ctx)) {

	case STATE_CONTENT:
		if (ELEMENT_IS("note-content")) {
			pop_state(ctx);
		} else {
			pop_tag(ctx);
		}
		break;

	case STATE_LIST:
		if (ELEMENT_IS("list")) {
			pop_state(ctx);
			ctx->depth--;
		}
		break;

	case STATE_LIST_ITEM:
		if (ELEMENT_IS("list-item")) {
			pop_state(ctx);
			g_assert(peek_state(ctx) == STATE_LIST);
		} else {
			pop_tag(ctx);
		}
		break;

	default:
		g_assert_not_reached ();
		break;
	}
}

static
GtkTextTag* get_depth_tag(ParseContext *ctx, GtkTextBuffer *buffer, gchar* name) {

	GtkTextTag *tag;
	gchar depth[5] = {0};
	gchar *tag_name;

	g_sprintf(depth, "%i", ctx->depth);
	tag_name = g_strconcat(name, ":", depth, NULL);

	tag = gtk_text_tag_table_lookup(buffer->tag_table, tag_name);
	if (tag == NULL) {
		tag = gtk_text_buffer_create_tag(buffer, tag_name, "indent", -20, "left-margin", ctx->depth * 25, NULL);
		gtk_text_tag_set_priority(gtk_text_tag_table_lookup(buffer->tag_table, "list"), gtk_text_tag_table_get_size(buffer->tag_table) - 1);
	}

	g_free(tag_name);

	return tag;
}

static void
apply_tags(GSList *tags, GtkTextBuffer *buffer, GtkTextIter *start_iter, GtkTextIter *end_iter)
{
	GtkTextTag *tag;
	GSList *tag_list = tags;
	while (tag_list) {
		tag = gtk_text_tag_table_lookup(buffer->tag_table, tag_list->data);
		if (tag == NULL) {
			gchar *tag_name = (gchar*)tag_list->data;
			if (tag_name != NULL && strcmp(tag_name, "") != 0) {
				g_printerr("INFO: XML tag <%s> does not exist yet. Creating it.\n", tag_name);
				gtk_text_buffer_create_tag(buffer, tag_name, NULL);
				tag = gtk_text_tag_table_lookup(buffer->tag_table, tag_name);
			}
		}
		gtk_text_buffer_apply_tag(buffer, tag, start_iter, end_iter);
		tag_list = tag_list->next;
	}
}

static void
handle_text_element(ParseContext *ctx, xmlTextReader *reader, GtkTextBuffer *buffer)
{
	const gchar *text = (const gchar *) xmlTextReaderConstValue(reader);
	GtkTextIter *iter = ctx->iter;
	GtkTextIter start_iter;
	GtkTextMark *mark;
	GtkTextTag *depth_tag;
	gint line_num;
	gboolean bullet_was_inserted;

	switch (peek_state(ctx)) {

	case STATE_CONTENT:
		mark = gtk_text_buffer_create_mark(buffer, "insert_point", iter, TRUE);
		gtk_text_buffer_insert(buffer, iter, text, -1);

		gtk_text_buffer_get_iter_at_mark(buffer, &start_iter, mark);
		apply_tags(ctx->tag_stack, buffer, &start_iter, iter);
		gtk_text_buffer_delete_mark(buffer, mark);

		break;

	case STATE_LIST:
		/* Ignore text in <list> elements - do nothing */
		break;

	case STATE_LIST_ITEM:
		bullet_was_inserted = FALSE;

		/* Save the line number where we start inserting */
		line_num = gtk_text_iter_get_line(iter);

		mark = gtk_text_buffer_create_mark(buffer, "insert_point", iter, TRUE);

		/* Insert bullet only if we are at the very beginning of a line */
		depth_tag = get_depth_tag(ctx, buffer, "depth");
		if (gtk_text_iter_get_line_offset(iter) == 0) {
			gtk_text_buffer_insert_with_tags(buffer, iter, get_bullet_by_depth(ctx->depth), -1, depth_tag, NULL);
			bullet_was_inserted = TRUE;
		}

		/* Insert the text into the buffer with list-item tags */
		gtk_text_buffer_insert_with_tags_by_name(buffer, iter, text, -1, "list-item", NULL);

		/* Apply <list> tag to the complete line, incuding the bullet */
		gtk_text_buffer_get_iter_at_mark(buffer, &start_iter, mark);
		gtk_text_buffer_apply_tag_by_name(buffer, "list", &start_iter, iter);

		/* Now move start_iter behind BULLET character, because we don't want to format the bullet */
		if (bullet_was_inserted) {
			gtk_text_iter_forward_chars(&start_iter, 2);
		}

		/* Apply the rest of the tags */
		apply_tags(ctx->tag_stack, buffer, &start_iter, iter);

		gtk_text_buffer_delete_mark(buffer, mark);
		break;

	default:
		g_printerr("ERROR: Wrong state: %i  Wrong tag: %s\n", peek_state(ctx), peek_tag(ctx));
		g_assert_not_reached();
		break;
	}
}


static
void process_note(ParseContext *ctx, xmlTextReader *reader, GtkTextBuffer *buffer)
{
	int type;
	const xmlChar *name, *value;

	value = xmlTextReaderConstValue(reader);
	type  = xmlTextReaderNodeType(reader);
	name  = xmlTextReaderConstName(reader);
	if (name == NULL) {
		name = BAD_CAST "(NULL)";
	}

	switch(type) {
	case XML_ELEMENT_NODE:
		handle_start_element(ctx, reader);
		break;

	case XML_ELEMENT_DECL:
		handle_end_element(ctx, reader);
		break;

	case XML_TEXT_NODE:
		handle_text_element(ctx, reader, buffer);
		break;

	case XML_DTD_NODE:
		handle_text_element(ctx, reader, buffer);
		break;
	}
}

static
ParseContext* init_parse_context(GtkTextBuffer *buffer, GtkTextIter *iter)
{
	ParseContext *ctx = g_new0(ParseContext, 1);
	ctx->state_stack = g_slist_prepend(NULL, GINT_TO_POINTER(STATE_START));
	ctx->tag_stack = NULL;
	gtk_text_buffer_get_iter_at_offset(buffer, iter, 0);
	ctx->iter = iter;
	ctx->depth = 0;
	return ctx;
}

static
void destroy_parse_context(ParseContext *ctx) {
	ctx->iter = NULL;
	g_slist_free(ctx->state_stack);
	g_slist_free(ctx->tag_stack);
	g_free(ctx);
}

void
conboy_note_buffer_set_xml (ConboyNoteBuffer *self, const gchar *xml_string)
{
	g_return_if_fail(self != NULL);
	g_return_if_fail(xml_string != NULL);
	g_return_if_fail(CONBOY_IS_NOTE_BUFFER(self));

	int ret;
	ParseContext *ctx;
	GtkTextIter iter;
	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(self);
	xmlTextReader *reader = conboy_xml_get_reader_for_memory(xml_string);


	/* Clear text buffer */
	gtk_text_buffer_set_text(buffer, "", -1);

	ctx = init_parse_context(buffer, &iter);

	ret = xmlTextReaderRead(reader);
	while (ret == 1) {
		process_note(ctx, reader, buffer);
		ret = xmlTextReaderRead(reader);
	}

	if (ret != 0) {
		g_printerr("ERROR: Failed to parse content.\n");
	}

	destroy_parse_context(ctx);
}





