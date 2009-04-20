#include <glib.h>
#include <glib/gprintf.h>

#include <string.h>

#include <libxml/encoding.h>
#include <libxml/xmlreader.h>

#include "note.h"
#include "metadata.h"
#include "deserializer.h"

#define ELEMENT_IS(name) (strcmp (element_name, (name)) == 0)

typedef enum
{
  STATE_START,
  STATE_NOTE,
  STATE_TITLE,
  STATE_TEXT,
  STATE_CONTENT,
  STATE_LIST,
  STATE_LIST_ITEM,
  STATE_METADATA
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

static void
handle_meta_data(gchar* tag_name, gchar* tag_content, Note *note)
{	
	if (strcmp(tag_name, "last-change-date") == 0) {
		note->last_change_date = get_iso8601_time_in_seconds(tag_content);
		return;
	}
	if (strcmp(tag_name, "last-metadata-change-date") == 0) {
		note->last_metadata_change_date = get_iso8601_time_in_seconds(tag_content);
		return;
	}
	if (strcmp(tag_name, "create-date") == 0) {
		note->create_date = get_iso8601_time_in_seconds(tag_content);
		return;
	}
	if (strcmp(tag_name, "cursor-position") == 0) {
		note->cursor_position = atoi(tag_content);
		return;
	}
	if (strcmp(tag_name, "width") == 0) {
		note->width = atoi(tag_content);
		return;
	}
	if (strcmp(tag_name, "height") == 0) {
		note->height = atoi(tag_content);
		return;
	}
	if (strcmp(tag_name, "x") == 0) {
		note->x = atoi(tag_content);
		return;
	}
	if (strcmp(tag_name, "y") == 0) {
		note->y = atoi(tag_content);
		return;
	}
	if (strcmp(tag_name, "open-on-startup") == 0) {
		if (g_str_equal(tag_content, "False")) {
			note->open_on_startup = FALSE;
		} else if(g_str_equal(tag_content, "True")) {
			note->open_on_startup = TRUE;
		} else {
			g_printerr("ERROR: <open-on-startup> must be True or False. Not %s\n", tag_content);
		}
		return;
	}
}


void handle_start_element(ParseContext *ctx, xmlTextReader *reader)
{
	gchar *element_name = xmlTextReaderConstName(reader);
	
	switch (peek_state(ctx)) {
	case STATE_START:
		if (ELEMENT_IS("note")) {
			push_state(ctx, STATE_NOTE);
		} else {
			g_printerr("ERROR: The first element must be the <note> element.\n");
		}
		break;
		
	case STATE_NOTE:
		if (ELEMENT_IS ("title")) {
			push_state(ctx, STATE_TITLE);
		}
		else if (ELEMENT_IS("text")) {
			push_state(ctx, STATE_TEXT);
		}
		else if (ELEMENT_IS("last-change-date") ||
				 ELEMENT_IS("last-metadata-change-date") ||
				 ELEMENT_IS("create-date") ||
				 ELEMENT_IS("cursor-position") ||
				 ELEMENT_IS("width") ||
				 ELEMENT_IS("height") ||
				 ELEMENT_IS("x") ||
				 ELEMENT_IS("y") ||
				 ELEMENT_IS("open-on-startup")
				 )
			{
			push_tag(ctx, element_name);
			push_state(ctx, STATE_METADATA);
		}
		else {
			g_printerr("ERROR: Inside <note> only <title>, <text> or metadata is allowed.\n");
		}
		break;
	
	case STATE_TITLE:
		g_printerr("ERROR: Inside <title> no other elements are allowed. Also not <%s>.", element_name);
		break;
	
	case STATE_TEXT:
		if (ELEMENT_IS("note-content")) {
			push_state(ctx, STATE_CONTENT);
		} else {
			g_printerr("ERROR: <text> may only contain <note-content>, not <%s>.", element_name);
		}
		break;
	
	case STATE_CONTENT:
		if (ELEMENT_IS("list")) {
			push_state(ctx, STATE_LIST);
			ctx->depth++;
			break;
		}
		push_tag(ctx, element_name);
		break;
	
	case STATE_LIST:
		if (ELEMENT_IS("list-item")) {
			push_state(ctx, STATE_LIST_ITEM);
		} else {
			g_printerr("ERROR: <list> may only contain <list-item>, not <%s>.", element_name);
		}
		break;
		
	/* TODO: ATM This is the same as STATE_CONTENT. Maybe this should be unified */
	case STATE_LIST_ITEM:
		if (ELEMENT_IS("list")) {
			push_state(ctx, STATE_LIST);
			ctx->depth++;
			break;
		}
		push_tag(ctx, element_name);
		break;
	
	default:
		g_assert_not_reached();
		break;
	}
}

static
void handle_end_element(ParseContext *ctx, xmlTextReader *reader) {
	
	gchar *element_name = xmlTextReaderConstName(reader);
	
	switch (peek_state(ctx)) {
	
	case STATE_TITLE:
	  	if (ELEMENT_IS("title")) {
	  		pop_state(ctx);
	  	} else {
	  		g_printerr("ERROR: Expecting </title> not </%s>.\n", element_name);
	  	}
	  	break;
	  	
	case STATE_CONTENT:
		if (ELEMENT_IS("note-content")) {
			pop_state(ctx);
		} else {
			pop_tag(ctx);
		}
		break;
		
	case STATE_TEXT:
		if (ELEMENT_IS("text")) {
			pop_state(ctx);
		} else {
			g_printerr("ERROR: Expecting </text>");
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
	
	case STATE_METADATA:
		pop_state(ctx);
		pop_tag(ctx);
		break;
		
	case STATE_NOTE:
		if (ELEMENT_IS("note")) {
			pop_state(ctx);
			g_assert(peek_state(ctx) == STATE_START);
		} else {
			g_printerr("Error: Expecting </note>, not </%s>.", element_name);
		}
		break;
	  
	default:
		g_assert_not_reached ();
		break;
	}

}

GtkTextTag* get_depth_tag(ParseContext *ctx, GtkTextBuffer *buffer, gint line_num) {
	
	GtkTextTag *tag;
	gchar *tmp;
	gchar depth[5] = {0};
	gchar *tag_name;
	gchar *color;
	
	if ((line_num % 2) == 1 ) {
		tmp = g_strconcat("list-item-", "A", NULL);
		color = "red";
	} else {
		tmp = g_strconcat("list-item-", "B", NULL);
		color = "orange";
	}
	
	g_sprintf(depth, "%i", ctx->depth);
	tag_name = g_strconcat(tmp, ":", depth, NULL);
	
	tag = gtk_text_tag_table_lookup(buffer->tag_table, tag_name); 
	if (tag == NULL) {
		/* For debugging */
		/*tag = gtk_text_buffer_create_tag(buffer, tag_name, "indent", -20, "left-margin", ctx->depth * 25, "foreground", color, NULL);*/
		tag = gtk_text_buffer_create_tag(buffer, tag_name, "indent", -20, "left-margin", ctx->depth * 25, NULL);
		/* Set the priority of the "list" tag to the maximum. It needs a higher priority then the newly created "list-item" tag */
		gtk_text_tag_set_priority(gtk_text_tag_table_lookup(buffer->tag_table, "list"), gtk_text_tag_table_get_size(buffer->tag_table) - 1);
	}
	
	g_free(tmp);
	g_free(tag_name);
	
	return tag;
}


void handle_text_element(ParseContext *ctx, xmlTextReader *reader, Note *note)
{
	gchar *text = xmlTextReaderConstValue(reader);
	GtkTextBuffer *buffer = note->ui->buffer;
	GtkTextIter *iter = ctx->iter;
	GtkTextIter start_iter;
	GtkTextMark *mark;
	GSList *tag_stack;
	GtkTextTag *tag;
	gint line_num;
	gboolean bullet_was_inserted;
	
	switch (peek_state(ctx)) {
	
	case STATE_START:
		g_assert_not_reached (); 
		break;
		
	case STATE_TITLE:
		note->title = g_strdup(text);
		break;
	
	case STATE_CONTENT:
		mark = gtk_text_buffer_create_mark(buffer, "insert_point", iter, TRUE);
		gtk_text_buffer_insert(buffer, iter, text, -1);
		
		gtk_text_buffer_get_iter_at_mark(buffer, &start_iter, mark);
		
		tag_stack = ctx->tag_stack;
		while (tag_stack) {
			tag = gtk_text_tag_table_lookup(buffer->tag_table, tag_stack->data);
			if (tag == NULL) {
				g_printerr("ERROR: TAG DOES NOT EXIST: %s", (gchar*)tag_stack->data);
				/* TODO: Auto create unknown tags */
			}
			gtk_text_buffer_apply_tag(buffer, tag, &start_iter, iter);
			tag_stack = g_slist_next(tag_stack);
		}
		
		gtk_text_buffer_delete_mark(buffer, mark);
	
		break;
	
	case STATE_LIST:
		/* Ignore text in <list> elements - do nothing */
		break;
	
	case STATE_LIST_ITEM:
		/* TODO: Copy&Paste: Same as above, but with BULLET. Make function for common parts */

		bullet_was_inserted = FALSE;
		
		/* Save the line number where we start inserting */
		line_num = gtk_text_iter_get_line(iter);
		
		mark = gtk_text_buffer_create_mark(buffer, "insert_point", iter, TRUE);
		
		/* Insert bullet only if we are at the very beginning of a line */
		if (gtk_text_iter_get_line_offset(iter) == 0) {
			gtk_text_buffer_insert(buffer, iter, BULLET, -1);
			bullet_was_inserted = TRUE;
		}
		
		/* Insert the text into the buffer */
		gtk_text_buffer_insert(buffer, iter, text, -1);
		
		gtk_text_buffer_get_iter_at_mark(buffer, &start_iter, mark);
		
		/* Create depth tag and apply */
		tag = get_depth_tag(ctx, buffer, line_num);
		gtk_text_buffer_apply_tag(buffer, tag, &start_iter, iter);
		gtk_text_buffer_apply_tag_by_name(buffer, "list", &start_iter, iter);
		
		/* Now move start_iter behind BULLET character, because we don't want to format the bullet */
		if (bullet_was_inserted) {
			gtk_text_iter_forward_chars(&start_iter, 2);
		}
		
		/* Apply the rest of the tags */
		tag_stack = ctx->tag_stack;
		while (tag_stack) {
			tag = gtk_text_tag_table_lookup(buffer->tag_table, tag_stack->data);
			if (tag == NULL) {
				g_printerr("ERROR: TAG DOES NOT EXIST: %s\n", (gchar*)tag_stack->data);
			}
			gtk_text_buffer_apply_tag(buffer, tag, &start_iter, iter);
			tag_stack = g_slist_next(tag_stack);
		}
		
		gtk_text_buffer_delete_mark(buffer, mark);
		break;
	
	
		
		
		
	case STATE_METADATA:
		handle_meta_data(peek_tag(ctx), g_strdup(text), note);
		break;
		
	case STATE_NOTE:
		/* Ignore text - Do nothing */
		break;
		
	case STATE_TEXT:
		/* Ignore text - Do nothting */
		break;
	    
	default:
		g_printerr("ERROR: Wrong state: %i  Wrong tag: %s\n", peek_state(ctx), peek_tag(ctx));
		g_assert_not_reached();
		break;

	}
	
}


static
void process_note(ParseContext *ctx, xmlTextReader *reader, Note *note)
{
	const xmlChar *name, *value;
	int type;
	
	name = xmlTextReaderConstName(reader);
	if (name == NULL) {
		name = BAD_CAST "(NULL)";
	}
	
	value = xmlTextReaderConstValue(reader);
	
	type = xmlTextReaderNodeType(reader);
	
	
	switch(type) {
	case XML_ELEMENT_NODE:
		handle_start_element(ctx, reader);
		break;
	
	case XML_ELEMENT_DECL:
		handle_end_element(ctx, reader);
		break;
		
	case XML_TEXT_NODE:
		handle_text_element(ctx, reader, note);
		break;
	
	case XML_DTD_NODE:
		handle_text_element(ctx, reader, note);
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

void deserialize_note(Note *note)
{
	ParseContext *ctx;
	xmlTextReader *reader;
	int ret;
	GtkTextIter iter;
	
	if (note->filename == NULL) {
		g_printerr("ERROR: Cannot load note, the filename is not set.\n");
	}
	
	reader = xmlNewTextReaderFilename(note->filename);
	if (reader == NULL) {
		g_printerr("ERROR: Cannot open file: %s\n", note->filename);
		return;
	}
	
	gtk_text_buffer_set_text(note->ui->buffer, "", -1);
	
	ctx = init_parse_context(note->ui->buffer, &iter);
	
	ret = xmlTextReaderRead(reader);
	while (ret == 1) {
		process_note(ctx, reader, note);
		ret = xmlTextReaderRead(reader);
	}
	
	xmlFreeTextReader(reader);
	if (ret != 0) {
		g_printerr("ERROR: Failed to parse file.\n");
	}
	
	xmlCleanupParser();
	
	destroy_parse_context(ctx);
}
