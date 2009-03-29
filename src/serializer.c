/* Based on gtktextbufferserialize.c from gtk+
 */

#include <gtk/gtk.h>
#include <hildon/hildon-window.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>

#include "serializer.h"
#include "metadata.h"

typedef struct {
	GString *text_str;
	GHashTable *tags;
	GtkTextIter start, end;
	
} SerializationContext;

static void find_list_delta(GSList *old_list, GSList *new_list, GList **added,
		GList **removed) {

	GSList *tmp;
	GList *tmp_added, *tmp_removed;

	tmp_added = NULL;
	tmp_removed = NULL;

	/* Find added tags */
	tmp = new_list;
	while (tmp) {
		if (!g_slist_find(old_list, tmp->data)) {
			tmp_added = g_list_prepend(tmp_added, tmp->data);
		}

		tmp = tmp->next;
	}

	*added = tmp_added;

	/* Find removed tags */
	tmp = old_list;
	while (tmp) {
		if (!g_slist_find(new_list, tmp->data)) {
			tmp_removed = g_list_prepend(tmp_removed, tmp->data);
		}

		tmp = tmp->next;
	}

	/* We reverse the list here to match the xml semantics */
	*removed = g_list_reverse(tmp_removed);
}

/**
 * Serializes all GtkTextTags by using there name as xml tags. E.g <tag_name></tag_name>.
 * Only GtkTextTags which name starts with an underscore (_) are ignored and not serialized.
 */
static void serialize_text(GtkTextBuffer *buffer, SerializationContext *context, Note* metadata) {

	GtkTextIter iter, old_iter;
	GSList *tag_list, *new_tag_list;
	GSList *active_tags;

	/* TODO: Write correct <title> */
	g_string_append(context->text_str, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
	g_string_append(context->text_str, "<note version=\"0.3\" xmlns:link=\"http://beatniksoftware.com/tomboy/link\" xmlns:size=\"http://beatniksoftware.com/tomboy/size\" xmlns=\"http://beatniksoftware.com/tomboy\">\n");
	g_string_append(context->text_str, "  <title>");
	g_string_append(context->text_str, metadata->title);
	g_string_append(context->text_str, "</title>\n");
	g_string_append(context->text_str, "  <text xml:space=\"preserve\"><note-content version=\"0.1\">");

	iter = context->start;
	tag_list = NULL;
	active_tags = NULL;

	do {
		GList *added, *removed;
		GList *tmp;
		gchar *tmp_text, *escaped_text;

		new_tag_list = gtk_text_iter_get_tags (&iter);
		find_list_delta (tag_list, new_tag_list, &added, &removed);

		/* Handle removed tags */
		for (tmp = removed; tmp; tmp = tmp->next)
		{
			GtkTextTag *tag = tmp->data;
			gchar *tag_name = NULL;

			/* Only close the tag if we didn't close it before (by using
			 * the stack logic in the while() loop below)
			 */
			if (g_slist_find (active_tags, tag))
			{
				/* Append closing tag */
				tag_name = g_markup_escape_text (tag->name, -1);
				if (g_ascii_strncasecmp(tag_name, "_", 1) == 0) {
					/* Ignore tags which start with "_" */
					break;
				}
				g_string_append_printf (context->text_str, "</%s>", tag_name);

				/* Drop all tags that were opened after this one (which are
				 * above this on in the stack)
				 */
				while (active_tags->data != tag)
				{
					added = g_list_prepend (added, active_tags->data);
					active_tags = g_slist_remove (active_tags, active_tags->data);
					g_string_append_printf (context->text_str, "</%s>", tag_name);
				}

				active_tags = g_slist_remove (active_tags, active_tags->data);
			}
			g_free(tag_name);
		}

		/* Handle added tags */
		for (tmp = added; tmp; tmp = tmp->next)
		{
			GtkTextTag *tag = tmp->data;
			gchar *tag_name;

			/* Add it to the tag hash table */
			g_hash_table_insert (context->tags, tag, tag);

			if (tag->name)
			{
				tag_name = g_markup_escape_text (tag->name, -1);
				if (g_ascii_strncasecmp(tag_name, "_", 1) == 0) {
					/* Ignore tags which start with "_" */
					break;
				}

				g_string_append_printf (context->text_str, "<%s>", tag_name);
				g_free (tag_name);
			}
			else
			{
				/* TODO: Remove later */
				g_printerr("ERROR: Only named tags are allowed!!!");
			}

			active_tags = g_slist_prepend (active_tags, tag);
		}

		g_slist_free (tag_list);
		tag_list = new_tag_list;

		g_list_free (added);
		g_list_free (removed);

		old_iter = iter;

		/* Now try to go to either the next tag toggle, or if a pixbuf appears */
		while (TRUE)
		{
			gunichar ch = gtk_text_iter_get_char (&iter);

			if (ch == 0)
			{
				break;
			}
			else
			gtk_text_iter_forward_char (&iter);

			if (gtk_text_iter_toggles_tag (&iter, NULL))
			break;
		}

		/* We might have moved too far */
		if (gtk_text_iter_compare (&iter, &context->end)> 0)
		iter = context->end;

		/* Append the text */
		tmp_text = gtk_text_iter_get_slice (&old_iter, &iter);
		escaped_text = g_markup_escape_text (tmp_text, -1);
		g_free (tmp_text);

		g_string_append (context->text_str, escaped_text);
		g_free (escaped_text);
	}

	while (!gtk_text_iter_equal (&iter, &context->end));

	/* Close any open tags */
	for (tag_list = active_tags; tag_list; tag_list = tag_list->next) {
		/* TODO: Schoener ?! */
		GtkTextTag *taggg = tag_list->data;
		gchar *tag_name = taggg->name;
		g_string_append_printf (context->text_str, "</%s>", tag_name);
		g_free(tag_name);
	}

	g_slist_free (active_tags);
	g_string_append (context->text_str, "\n</note-content></text>\n");
}

static void serialize_metadata(GString *str, Note *metadata) {
	
	/* I'm not using printf for open_on_startup, because I want that
	 * True and False are written like that and not TRUE and FALSE.
	 * To stay as compatible with Tomboy as possible.
	 */
	const gchar *open_on_startup;
	if (metadata->open_on_startup) {
		open_on_startup = "True";
	} else {
		open_on_startup = "False";
	}
	
	g_string_append_printf(str, "  <last-change-date>%s</last-change-date>\n", metadata->last_change_date);
	g_string_append_printf(str, "  <last-metadata-change-date>%s</last-metadata-change-date>\n", metadata->last_metadata_change_date);
	g_string_append_printf(str, "  <create-date>%s</create-date>\n", metadata->create_date);
	g_string_append_printf(str, "  <cursor-position>%i</cursor-position>\n", metadata->cursor_position);
	g_string_append_printf(str, "  <width>%i</width>\n", metadata->width);
	g_string_append_printf(str, "  <height>%i</height>\n", metadata->height);
	g_string_append_printf(str, "  <x>%i</x>\n", metadata->x);
	g_string_append_printf(str, "  <y>%i</y>\n", metadata->y);
	g_string_append_printf(str, "  <open-on-startup>%s</open-on-startup>\n", open_on_startup);
	g_string_append_printf(str, "</note>\n");

}

guint8 * serialize_to_tomboy(GtkTextBuffer *register_buffer,
		GtkTextBuffer *content_buffer, const GtkTextIter *start,
		const GtkTextIter *end, gsize *length, gpointer user_data) {

	/* 
	 * register_buffer: the GtkTextBuffer for which the format is registered
	 * content_buffer:	the GtkTextsBuffer to serialize
	 * start: start of the block of text to serialize
	 * end : end of the block of text to serialize 
	 * length: Return location for the length of the serialized data
	 * user_data: user data that was specified when registering the format
	 */

	SerializationContext context;
	GString *text;
	GtkTextIter title_end;
	Note *metadata;
	
	metadata = (Note*)user_data;

	context.tags = g_hash_table_new(NULL, NULL);
	context.text_str = g_string_new(NULL);
	context.start = *start;
	context.end = *end;
	
	
	
	serialize_text(content_buffer, &context, metadata);
	
	

	text = g_string_new(NULL);
	
	/* TODO: Add the static xml stuff here */
	/*serialize_header(text);*/
	
	g_string_append_len(text, context.text_str->str, context.text_str->len);

	serialize_metadata(text, metadata);

	g_hash_table_destroy(context.tags);
	g_string_free(context.text_str, TRUE);

	*length = text->len;
	
	/* Mark the buffer as saved */
	gtk_text_buffer_set_modified(content_buffer, FALSE);

	/* Must return new allocated array of guint8 which contains the serialized data or NULL if error
	 * occured */
	return (guint8 *) g_string_free(text, FALSE);
}

