
#include <string.h>
#include <glib/gprintf.h>
#include <libxml/encoding.h>
#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>

#include "app_data.h"
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
	
	/* Copy elements of given list */
	while (tags != NULL) {
		self->active_tags = g_slist_prepend(self->active_tags, tags->data);
		tags = tags->next;
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
		/*g_printerr("new_depth > depth. DIFF: %i \n", new_depth - depth);*/
		
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
	const gchar *element_name = xmlTextReaderConstName(reader);
	
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
		push_tag(ctx, element_name);
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
	
	const gchar *element_name = xmlTextReaderConstName(reader);
	
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

static
void apply_tags(GSList *tags, GtkTextBuffer *buffer, GtkTextIter *start_iter, GtkTextIter *end_iter)
{
	GtkTextTag *tag;
	GSList *tag_list = tags;
	while (tag_list) {
		tag = gtk_text_tag_table_lookup(buffer->tag_table, tag_list->data);
		if (tag == NULL) {
			g_printerr("ERROR: TAG DOES NOT EXIST: %s", (gchar*)tag_list->data);
			/* TODO: Auto create unknown tags */
		}
		gtk_text_buffer_apply_tag(buffer, tag, start_iter, end_iter);
		tag_list = tag_list->next;
	}
}

static
void handle_text_element(ParseContext *ctx, xmlTextReader *reader, GtkTextBuffer *buffer)
{
	const gchar *text = xmlTextReaderConstValue(reader);
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
conboy_note_buffer_set_xml (ConboyNoteBuffer *self, const gchar *xmlString)
{
	g_return_if_fail(self != NULL);
	g_return_if_fail(xmlString != NULL);
	g_return_if_fail(CONBOY_IS_NOTE_BUFFER(self));
	
	ParseContext *ctx;
	int ret;
	GtkTextIter iter;
	AppData *app_data = app_data_get();
	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(self);
	
	/* We try to reuse the existing xml parser. If none exists yet, we create a new one. */
	if (app_data->reader == NULL) {
		app_data->reader = xmlReaderForMemory(xmlString, strlen(xmlString), "", "UTF-8", 0);
	}
	
	if (xmlReaderNewMemory(app_data->reader, xmlString, strlen(xmlString), "", "UTF-8", 0) != 0) {
		g_printerr("ERROR: Cannot reuse xml parser. \n");
		g_assert_not_reached();
	}
	
	if (app_data->reader == NULL) {
		g_printerr("ERROR: Couldn't init xml parser.\n");
		g_assert_not_reached();
	}
	
	/* Clear text buffer */
	gtk_text_buffer_set_text(buffer, "", -1);
	
	ctx = init_parse_context(buffer, &iter);
	
	ret = xmlTextReaderRead(app_data->reader);
	while (ret == 1) {
		process_note(ctx, app_data->reader, buffer);
		ret = xmlTextReaderRead(app_data->reader);
	}
	
	if (ret != 0) {
		g_printerr("ERROR: Failed to parse content.\n");
	}
	
	destroy_parse_context(ctx);
}





