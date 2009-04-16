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

/* Based on gtktextbufferserialize.c from gtk+
 */

/* FIXME: We should use other error codes for the
 * parts that deal with the format errors
 */

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>

#include "gdk-pixbuf/gdk-pixdata.h"
#include "hildon/hildon-window.h"

#include "deserializer.h"
#include "metadata.h"


typedef struct
{
  GString *tag_table_str;
  GString *text_str;
  GHashTable *tags;
  GtkTextIter start, end;

  gint n_pixbufs;
  GList *pixbufs;
  gint tag_id;
  GHashTable *tag_id_tags;
} SerializationContext;


/* Copied from glib-2.18 */
#define ERROR_OVERWRITTEN_WARNING "GError set over the top of a previous GError or uninitialized memory.\n" \
               "This indicates a bug in someone's code. You must ensure an error is NULL before it's set.\n" \
               "The overwriting error message was: %s"

/* Copied from glib-2.18 */
/**
 * g_set_error_literal:
 * @err: a return location for a #GError, or %NULL
 * @domain: error domain
 * @code: error code 
 * @message: error message
 *
 * Does nothing if @err is %NULL; if @err is non-%NULL, then *@err must
 * be %NULL. A new #GError is created and assigned to *@err.
 * Unlike g_set_error(), @message is not a printf()-style format string.
 * Use this function if @message contains text you don't have control over,
 * that could include printf() escape sequences.
 *
 * Since: 2.18
 **/
static void
g_set_error_literal (GError      **err,
                     GQuark        domain,
                     gint          code,
                     const gchar  *message)
{
  GError *new;

  if (err == NULL)
    return;

  new = g_error_new_literal (domain, code, message);
  if (*err == NULL)
    *err = new;
  else
    g_warning (ERROR_OVERWRITTEN_WARNING, new->message);
}





static gboolean
deserialize_value (const gchar *str,
                   GValue      *value)
{
  if (g_value_type_transformable (G_TYPE_STRING, value->g_type))
    {
      GValue text_value = { 0 };
      gboolean retval;

      g_value_init (&text_value, G_TYPE_STRING);
      g_value_set_static_string (&text_value, str);

      retval = g_value_transform (&text_value, value);
      g_value_unset (&text_value);

      return retval;
    }
  else if (value->g_type == G_TYPE_BOOLEAN)
    {
      gboolean v;

      v = strcmp (str, "TRUE") == 0;

      g_value_set_boolean (value, v);

      return TRUE;
    }
  else if (value->g_type == G_TYPE_INT)
    {
      gchar *tmp;
      int v;

      v = strtol (str, &tmp, 10);

      if (tmp == NULL || tmp == str)
	return FALSE;

      g_value_set_int (value, v);

      return TRUE;
    }
  else if (value->g_type == G_TYPE_DOUBLE)
    {
      gchar *tmp;
      gdouble v;

      v = g_ascii_strtod (str, &tmp);

      if (tmp == NULL || tmp == str)
	return FALSE;

      g_value_set_double (value, v);

      return TRUE;
    }
  else if (value->g_type == GDK_TYPE_COLOR)
    {
      GdkColor color;
      const gchar *old;
      gchar *tmp;

      old = str;
      color.red = strtol (old, &tmp, 16);

      if (tmp == NULL || tmp == old)
	return FALSE;

      old = tmp;
      if (*old++ != ':')
	return FALSE;

      color.green = strtol (old, &tmp, 16);
      if (tmp == NULL || tmp == old)
	return FALSE;

      old = tmp;
      if (*old++ != ':')
	return FALSE;

      color.blue = strtol (old, &tmp, 16);

      if (tmp == NULL || tmp == old || *tmp != '\0')
	return FALSE;

      g_value_set_boxed (value, &color);

      return TRUE;
    }
  else if (G_VALUE_HOLDS_ENUM (value))
    {
      GEnumClass *class = G_ENUM_CLASS (g_type_class_peek (value->g_type));
      GEnumValue *enum_value;

      enum_value = g_enum_get_value_by_name (class, str);

      if (enum_value)
	{
	  g_value_set_enum (value, enum_value->value);
	  return TRUE;
	}

      return FALSE;
    }
  else
    {
      g_warning ("Type %s can not be deserialized\n", g_type_name (value->g_type));
    }

  return FALSE;
}




#if 0
static void
dump_tag_list (const gchar *str,
               GList       *list)
{
  g_print ("%s: ", str);

  if (!list)
    g_print ("(empty)");
  else
    {
      while (list)
	{
	  g_print ("%s ", ((GtkTextTag *)list->data)->name);
	  list = list->next;
	}
    }

  g_print ("\n");
}
#endif




typedef enum
{
  STATE_TEXT_VIEW_MARKUP, /* 0 */
  STATE_START,
  STATE_NOTE,			/* 2 */
  STATE_TITLE,
  STATE_METADATA,
  STATE_TAGS,
  STATE_TAG,
  STATE_ATTR,
  STATE_TEXT,
  STATE_FORMAT,			/* 9 */
  STATE_APPLY_TAG,
  STATE_PIXBUF,
  STATE_CONTENT, 		/* 12 */
  STATE_LIST_ITEM
} ParseState;

typedef struct
{
  gchar *text;
  GdkPixbuf *pixbuf;
  GSList *tags;
} TextSpan;

typedef struct
{
  GtkTextTag *tag;
  gint prio;
} TextTagPrio;

typedef struct
{
  GSList *states;

  GList *headers;

  GtkTextBuffer *buffer;

  /* Tags that are defined in <tag> elements */
  GHashTable *defined_tags;

  /* Tags that are anonymous */
  GHashTable *anonymous_tags;

  /* Tag name substitutions */
  GHashTable *substitutions;

  /* Current tag */
  GtkTextTag *current_tag;

  /* Priority of current tag */
  gint current_tag_prio;

  /* Id of current tag */
  gint current_tag_id;

  /* Tags and their priorities */
  GList *tag_priorities;

  GSList *tag_stack;

  GList *spans;

  gboolean create_tags;

  gboolean parsed_text;
  gboolean parsed_tags;
  
  Note *note;
  GSList *meta_data_tag_stack;
  
} ParseInfo;

static void
set_error (GError              **err,
           GMarkupParseContext  *context,
           int                   error_domain,
           int                   error_code,
           const char           *format,
           ...)
{
  int line, ch;
  va_list args;
  char *str;

  g_markup_parse_context_get_position (context, &line, &ch);

  va_start (args, format);
  str = g_strdup_vprintf (format, args);
  va_end (args);

  g_set_error (err, error_domain, error_code,
               ("Line %d character %d: %s"),
               line, ch, str);

  g_free (str);
}

static void
push_state (ParseInfo  *info,
            ParseState  state)
{
  info->states = g_slist_prepend (info->states, GINT_TO_POINTER (state));
}

static void
pop_state (ParseInfo *info)
{
  g_return_if_fail (info->states != NULL);

  info->states = g_slist_remove (info->states, info->states->data);
}

static ParseState
peek_state (ParseInfo *info)
{
  g_return_val_if_fail (info->states != NULL, STATE_START);

  return GPOINTER_TO_INT (info->states->data);
}

#define ELEMENT_IS(name) (strcmp (element_name, (name)) == 0)


static gboolean
check_id_or_name (GMarkupParseContext  *context,
		  const gchar          *element_name,
		  const gchar         **attribute_names,
		  const gchar         **attribute_values,
		  gint                 *id,
		  const gchar         **name,
		  GError              **error)
{
  gboolean has_id = FALSE;
  gboolean has_name = FALSE;
  int i;

  *id = 0;
  *name = NULL;

  for (i = 0; attribute_names[i] != NULL; i++)
    {
      if (strcmp (attribute_names[i], "name") == 0)
	{
	  *name = attribute_values[i];

	  if (has_id)
	    {
	      set_error (error, context,
			 G_MARKUP_ERROR,
			 G_MARKUP_ERROR_PARSE,
			 ("Both \"id\" and \"name\" were found on the <%s> element"),
			 element_name);
	      return FALSE;
	    }

	  if (has_name)
	    {
	      set_error (error, context,
			 G_MARKUP_ERROR,
			 G_MARKUP_ERROR_PARSE,
			 ("The attribute \"%s\" was found twice on the <%s> element"),
			 "name", element_name);
	      return FALSE;
	    }

	  has_name = TRUE;
	}
      else if (strcmp (attribute_names[i], "id") == 0)
	{
	  gchar *tmp;

	  if (has_name)
	    {
	      set_error (error, context,
			 G_MARKUP_ERROR,
			 G_MARKUP_ERROR_PARSE,
			 ("Both \"id\" and \"name\" were found on the <%s> element"),
			 element_name);
	      return FALSE;
	    }

	  if (has_id)
	    {
	      set_error (error, context,
			 G_MARKUP_ERROR,
			 G_MARKUP_ERROR_PARSE,
			 ("The attribute \"%s\" was found twice on the <%s> element"),
			 "id", element_name);
	      return FALSE;
	    }

	  has_id = TRUE;

	  /* Try parsing the integer */
	  *id = strtol (attribute_values[i], &tmp, 10);

	  if (tmp == NULL || tmp == attribute_values[i])
	    {
	      set_error (error, context,
			 G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
			 ("<%s> element has invalid id \"%s\""), attribute_values[i]);
	      return FALSE;
	    }
	}
    }

  if (!has_id && !has_name)
    {
      set_error (error, context,
		 G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
		 ("<%s> element has neither a \"name\" nor an \"id\" attribute"), element_name);
      return FALSE;
    }

  return TRUE;
}

typedef struct
{
  const char  *name;
  const char **retloc;
} LocateAttr;

static gboolean
locate_attributes (GMarkupParseContext  *context,
                   const char           *element_name,
                   const char          **attribute_names,
                   const char          **attribute_values,
		   gboolean              allow_unknown_attrs,
                   GError              **error,
                   const char           *first_attribute_name,
                   const char          **first_attribute_retloc,
                   ...)
{
  va_list args;
  const char *name;
  const char **retloc;
  int n_attrs;
#define MAX_ATTRS 24
  LocateAttr attrs[MAX_ATTRS];
  gboolean retval;
  int i;

  g_return_val_if_fail (first_attribute_name != NULL, FALSE);
  g_return_val_if_fail (first_attribute_retloc != NULL, FALSE);

  retval = TRUE;

  n_attrs = 1;
  attrs[0].name = first_attribute_name;
  attrs[0].retloc = first_attribute_retloc;
  *first_attribute_retloc = NULL;

  va_start (args, first_attribute_retloc);

  name = va_arg (args, const char*);
  retloc = va_arg (args, const char**);

  while (name != NULL)
    {
      g_return_val_if_fail (retloc != NULL, FALSE);

      g_assert (n_attrs < MAX_ATTRS);

      attrs[n_attrs].name = name;
      attrs[n_attrs].retloc = retloc;
      n_attrs += 1;
      *retloc = NULL;

      name = va_arg (args, const char*);
      retloc = va_arg (args, const char**);
    }

  va_end (args);

  if (!retval)
    return retval;

  i = 0;
  while (attribute_names[i])
    {
      int j;
      gboolean found;

      found = FALSE;
      j = 0;
      while (j < n_attrs)
        {
          if (strcmp (attrs[j].name, attribute_names[i]) == 0)
            {
              retloc = attrs[j].retloc;

              if (*retloc != NULL)
                {
                  set_error (error, context,
                             G_MARKUP_ERROR,
                             G_MARKUP_ERROR_PARSE,
                             ("Attribute \"%s\" repeated twice on the same <%s> element"),
                             attrs[j].name, element_name);
                  retval = FALSE;
                  goto out;
                }

              *retloc = attribute_values[i];
              found = TRUE;
            }

          ++j;
        }

      if (!found && !allow_unknown_attrs)
        {
          set_error (error, context,
                     G_MARKUP_ERROR,
                     G_MARKUP_ERROR_PARSE,
                     ("Attribute \"%s\" is invalid on <%s> element in this context"),
                     attribute_names[i], element_name);
          retval = FALSE;
          goto out;
        }

      ++i;
    }

 out:
  return retval;
}

/*
static gboolean
check_no_attributes (GMarkupParseContext  *context,
                     const char           *element_name,
                     const char          **attribute_names,
                     const char          **attribute_values,
                     GError              **error)
{
  if (attribute_names[0] != NULL)
    {
      set_error (error, context,
                 G_MARKUP_ERROR,
                 G_MARKUP_ERROR_PARSE,
                 ("Attribute \"%s\" is invalid on <%s> element in this context"),
                 attribute_names[0], element_name);
      return FALSE;
    }

  return TRUE;
}
*/

static GtkTextTag *
tag_exists (GMarkupParseContext *context,
	    const gchar         *name,
	    gint                 id,
	    ParseInfo           *info,
	    GError             **error)
{
  const gchar *real_name;

  if (info->create_tags)
    {
      /* If we have an anonymous tag, just return it directly */
      if (!name)
	return g_hash_table_lookup (info->anonymous_tags,
				    GINT_TO_POINTER (id));

      /* First, try the substitutions */
      real_name = g_hash_table_lookup (info->substitutions, name);

      if (real_name)
	return gtk_text_tag_table_lookup (info->buffer->tag_table, real_name);

      /* Next, try the list of defined tags */
      if (g_hash_table_lookup (info->defined_tags, name) != NULL)
	return gtk_text_tag_table_lookup (info->buffer->tag_table, name);

      set_error (error, context,
		 G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
		 ("Tag \"%s\" has not been defined."), name);

      return NULL;
    }
  else
    {
      GtkTextTag *tag;

      if (!name)
	{
	  set_error (error, context,
		     G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
		     ("Anonymous tag found and tags can not be created."));
	  return NULL;
	}

      tag = gtk_text_tag_table_lookup (info->buffer->tag_table, name);

      if (tag)
	return tag;

      set_error (error, context,
		 G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
		 ("Tag \"%s\" does not exist in buffer and tags can not be created."), name);

      return NULL;
    }
}

typedef struct
{
  const gchar *id;
  gint length;
  const gchar *start;
} Header;

static GdkPixbuf *
get_pixbuf_from_headers (GList   *headers,
                         int      id,
                         GError **error)
{
  Header *header;
  GdkPixdata pixdata;
  GdkPixbuf *pixbuf;

  header = g_list_nth_data (headers, id);

  if (!header)
    return NULL;

  if (!gdk_pixdata_deserialize (&pixdata, header->length,
                                (const guint8 *) header->start, error))
    return NULL;

  pixbuf = gdk_pixbuf_from_pixdata (&pixdata, TRUE, error);

  return pixbuf;
}

/*
static void
parse_apply_tag_element (GMarkupParseContext  *context,
			 const gchar          *element_name,
			 const gchar         **attribute_names,
			 const gchar         **attribute_values,
			 ParseInfo            *info,
			 GError              **error)
{
  const gchar *name, *priority;
  gint id;
  GtkTextTag *tag;

  g_assert (peek_state (info) == STATE_TEXT ||
	    peek_state (info) == STATE_APPLY_TAG);

  if (ELEMENT_IS ("apply_tag"))
    {
      if (!locate_attributes (context, element_name, attribute_names, attribute_values, TRUE, error,
			      "priority", &priority, NULL))
	return;

      if (!check_id_or_name (context, element_name, attribute_names, attribute_values,
			     &id, &name, error))
	return;


      tag = tag_exists (context, name, id, info, error);

      if (!tag)
	return;

      info->tag_stack = g_slist_prepend (info->tag_stack, tag);

      push_state (info, STATE_APPLY_TAG);
    }
  else if (ELEMENT_IS ("pixbuf"))
    {
      int int_id;
      GdkPixbuf *pixbuf;
      TextSpan *span;
      const gchar *pixbuf_id;

      if (!locate_attributes (context, element_name, attribute_names, attribute_values, FALSE, error,
			      "index", &pixbuf_id, NULL))
	return;

      int_id = atoi (pixbuf_id);
      pixbuf = get_pixbuf_from_headers (info->headers, int_id, error);

      span = g_new0 (TextSpan, 1);
      span->pixbuf = pixbuf;
      span->tags = NULL;

      info->spans = g_list_prepend (info->spans, span);

      if (!pixbuf)
	return;

      push_state (info, STATE_PIXBUF);
    }
  else
    set_error (error, context,
	       G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
	       ("Element <%s> is not allowed below <%s>"),
	       element_name, peek_state(info) == STATE_TEXT ? "text" : "apply_tag");
}
*/

static void
parse_attr_element (GMarkupParseContext  *context,
		    const gchar          *element_name,
		    const gchar         **attribute_names,
		    const gchar         **attribute_values,
		    ParseInfo            *info,
		    GError              **error)
{
  const gchar *name, *type, *value;
  GType gtype;
  GValue gvalue = { 0 };
  GParamSpec *pspec;

  g_assert (peek_state (info) == STATE_TAG);

  if (ELEMENT_IS ("attr"))
    {
      if (!locate_attributes (context, element_name, attribute_names, attribute_values, FALSE, error,
			      "name", &name, "type", &type, "value", &value, NULL))
	return;

      gtype = g_type_from_name (type);

      if (gtype == G_TYPE_INVALID)
	{
	  set_error (error, context,
		     G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
		     ("\"%s\" is not a valid attribute type"), type);
	  return;
	}

      if (!(pspec = g_object_class_find_property (G_OBJECT_GET_CLASS (info->current_tag), name)))
	{
	  set_error (error, context,
		     G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
		     ("\"%s\" is not a valid attribute name"), name);
	  return;
	}

      g_value_init (&gvalue, gtype);

      if (!deserialize_value (value, &gvalue))
	{
	  set_error (error, context,
		     G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
		     ("\"%s\" could not be converted to a value of type \"%s\" for attribute \"%s\""),
		     value, type, name);
	  return;
	}

      if (g_param_value_validate (pspec, &gvalue))
	{
	  set_error (error, context,
		     G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
		     ("\"%s\" is not a valid value for attribute \"%s\""),
		     value, name);
	  g_value_unset (&gvalue);
	  return;
	}

      g_object_set_property (G_OBJECT (info->current_tag),
			     name, &gvalue);

      g_value_unset (&gvalue);

      push_state (info, STATE_ATTR);
    }
  else
    {
      set_error (error, context,
                 G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                 ("Element <%s> is not allowed below <%s>"),
                 element_name, "tag");
    }
}


static gchar *
get_tag_name (ParseInfo   *info,
	      const gchar *tag_name)
{
  gchar *name;
  gint i;

  name = g_strdup (tag_name);

  if (!info->create_tags)
    return name;

  i = 0;

  while (gtk_text_tag_table_lookup (info->buffer->tag_table, name) != NULL)
    {
      g_free (name);
      name = g_strdup_printf ("%s-%d", tag_name, ++i);
    }

  if (i != 0)
    {
      g_hash_table_insert (info->substitutions, g_strdup (tag_name), g_strdup (name));
    }

  return name;
}

/*
static void
parse_tag_element (GMarkupParseContext  *context,
		   const gchar          *element_name,
		   const gchar         **attribute_names,
		   const gchar         **attribute_values,
		   ParseInfo            *info,
		   GError              **error)
{
  const gchar *name, *priority;
  gchar *tag_name;
  gint id;
  gint prio;
  gchar *tmp;

  g_assert (peek_state (info) == STATE_TAGS);

  if (ELEMENT_IS ("tag"))
    {
      if (!locate_attributes (context, element_name, attribute_names, attribute_values, TRUE, error,
			      "priority", &priority, NULL))
	return;

      if (!check_id_or_name (context, element_name, attribute_names, attribute_values,
			     &id, &name, error))
	return;

      if (name)
	{
	  if (g_hash_table_lookup (info->defined_tags, name) != NULL)
	    {
	      set_error (error, context,
			 G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
			 ("Tag \"%s\" already defined"), name);
	      return;
	    }
	}

      prio = strtol (priority, &tmp, 10);

      if (tmp == NULL || tmp == priority)
	{
	  set_error (error, context,
		     G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
		     ("Tag \"%s\" has invalid priority \"%s\""), name, priority);
	  return;
	}

      if (name)
	{
	  tag_name = get_tag_name (info, name);
	  info->current_tag = gtk_text_tag_new (tag_name);
	  g_free (tag_name);
	}
      else
	{
	  info->current_tag = gtk_text_tag_new (NULL);
	  info->current_tag_id = id;
	}

      info->current_tag_prio = prio;

      push_state (info, STATE_TAG);
    }
  else
    {
      set_error (error, context,
                 G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                 ("Element <%s> is not allowed below <%s>"),
                 element_name, "tags");
    }
}
*/

static void
start_element_handler (GMarkupParseContext  *context,
		       const gchar          *element_name,
		       const gchar         **attribute_names,
		       const gchar         **attribute_values,
		       gpointer              user_data,
		       GError              **error)
{
  ParseInfo *info = user_data;
  GtkTextTag *tag = NULL;
  
	switch (peek_state(info)) {
	
	case STATE_START:
		if (ELEMENT_IS ("note")) {
			push_state(info, STATE_NOTE);
			break;
		} else {
			set_error(
					error,
					context,
					G_MARKUP_ERROR,
					G_MARKUP_ERROR_PARSE,
					("Outermost element in text must be <note> not <%s>"),
					element_name);
		}
		break;
		
	
	
	case STATE_NOTE:
		if (ELEMENT_IS ("title")) {
			push_state(info, STATE_TITLE);
		}
		else if (ELEMENT_IS("text")) {
			push_state(info, STATE_TEXT);
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
			info->meta_data_tag_stack = g_slist_prepend(info->meta_data_tag_stack, element_name);
			push_state(info, STATE_METADATA);
		}
		else {
			set_error(
					error,
					context,
					G_MARKUP_ERROR,
					G_MARKUP_ERROR_PARSE,
					("<note> must be followed by <title> or metadata, not <%s>"),
					element_name);
		}
		break;
		
	case STATE_TITLE:
		set_error(
				error,
				context,
				G_MARKUP_ERROR,
				G_MARKUP_ERROR_PARSE,
				("Inside <title> no other elements are allowed. Also not <%s>."),
				element_name);
		break;
	
	case STATE_TEXT:
		if (ELEMENT_IS("note-content")) {
			push_state(info, STATE_CONTENT);
		} else {
			set_error(
					error,
					context,
					G_MARKUP_ERROR,
					G_MARKUP_ERROR_PARSE,
					("<text> may only contain <note-content>, not <%s>."),
					element_name);
		}
		break;
	
	case STATE_FORMAT:
	case STATE_CONTENT:
		/* TODO: Make a definition what format-tags are. Then we can check if the tag is valid. */
		
		tag = gtk_text_tag_table_lookup(info->buffer->tag_table, element_name);
		if (tag == NULL) {
			g_printerr("ERROR: Can only be a <italic>, <bold>, etc... tag. But is <%s>.", element_name);
		}
		
		/* TODO: Make finer separation and add STATE_LIST not only STATE_LIST_ITEM */
		if (ELEMENT_IS("list-item")) {
			push_state(info, STATE_LIST_ITEM);
		} else {
			push_state(info, STATE_FORMAT);
		}
		
		info->tag_stack = g_slist_prepend(info->tag_stack, tag);

		break;
	
	default:
		g_assert_not_reached ();
		break;
	}
	
}

/*
static gint
sort_tag_prio (TextTagPrio *a,
	       TextTagPrio *b)
{
  if (a->prio < b->prio)
    return -1;
  else if (a->prio > b->prio)
    return 1;
  else
    return 0;
}
*/


static void
end_element_handler (GMarkupParseContext  *context,
		     const gchar          *element_name,
		     gpointer              user_data,
		     GError              **error)
{
  ParseInfo *info = user_data;
  gchar *tmp;
  GList *list;

  switch (peek_state(info)) {
  
  case STATE_TITLE:
  	if (ELEMENT_IS("title")) {
  		pop_state(info);
  	} else {
  		g_printerr("ERRRRRROR: Expecting </title>");
  	}
  break;
  
  case STATE_CONTENT:
  	if (ELEMENT_IS("note-content")) {
  		pop_state(info);
  		/* Reverse the list of spans. Otherwise all text spans (text blocks) will be in 
  		 * reverse order. */
  		info->spans = g_list_reverse (info->spans);
  		/* TODO: Check if really needed and what it does. */
  		info->parsed_text = TRUE;
  		
  	} else {
  		g_printerr("ERRRRROR: Expecting </note-content>");
  	}
  break;
  
  case STATE_TEXT:
  	if (ELEMENT_IS("text")) {
  		pop_state(info);
  	} else {
  		g_printerr("ERROR: Expecting </text>");
  	}
  break;
  
  case STATE_FORMAT:
	  /* TODO: Add error handling: If not a valid formatting tag (bold, italic, etc.) then throw error */
	  pop_state(info);
	  g_assert(peek_state(info) == STATE_FORMAT ||
			   peek_state(info) == STATE_CONTENT);
	  info->tag_stack = g_slist_delete_link(info->tag_stack, info->tag_stack);
	  break;

  case STATE_LIST_ITEM:
	  pop_state(info);
	  g_assert(peek_state(info) == STATE_FORMAT ||
			   peek_state(info) == STATE_CONTENT); /* TODO: Should be STATE_LIST, but does not exist yet */
	  info->tag_stack = g_slist_delete_link(info->tag_stack, info->tag_stack);
	  break;
  
  case STATE_METADATA:
	pop_state(info);
	info->meta_data_tag_stack = g_slist_delete_link(info->meta_data_tag_stack, info->meta_data_tag_stack);
	break;
	
  case STATE_NOTE:
	  pop_state (info);
	  g_assert (peek_state (info) == STATE_START);
	  /* Means that we are at the end of the file */
	  break;
  
  default:
        g_assert_not_reached ();
        break;
  }
  
}

static gboolean
all_whitespace (const char *text,
                int         text_len)
{
  const char *p;
  const char *end;

  p = text;
  end = text + text_len;

  while (p != end)
    {
      if (!g_ascii_isspace (*p))
        return FALSE;

      p = g_utf8_next_char (p);
    }

  return TRUE;
}

static void
save_meta_data(gchar* tag_name, gchar* tag_content, Note *note)
{	
	if (strcmp(tag_name, "last-change-date") == 0) {
		note->last_change_date = tag_content;
		return;
	}
	if (strcmp(tag_name, "last-metadata-change-date") == 0) {
		note->last_metadata_change_date = tag_content;
		return;
	}
	if (strcmp(tag_name, "create-date") == 0) {
		note->create_date = tag_content;
		return;
	}
	if (strcmp(tag_name, "cursor-position") == 0) {
		note->cursor_position = atoi(tag_content); /* TODO Convert to gint */
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


static void
text_handler (GMarkupParseContext  *context,
	      const gchar          *text,
	      gsize                 text_len,
	      gpointer              user_data,
	      GError              **error)
{
  ParseInfo *info = user_data;
  TextSpan *span;
  gchar *tag_name;

  if (all_whitespace (text, text_len) &&
      peek_state (info) != STATE_CONTENT &&
      peek_state (info) != STATE_APPLY_TAG)
    return;
  
  
  switch (peek_state (info))
    {
    case STATE_START:
      g_assert_not_reached (); 
      break;
    
    case STATE_TITLE:
    	break;
      
    case STATE_CONTENT:
    case STATE_FORMAT:
      if (text_len == 0) {
    	  return;
      }

      span = g_new0(TextSpan, 1);
      span->text = g_strndup(text, text_len);
      span->tags = g_slist_copy(info->tag_stack);

      info->spans = g_list_prepend(info->spans, span);
      break;
    
    case STATE_LIST_ITEM:
    	if (text_len == 0) {
    		return;
    	}
    	
    	span = g_new0(TextSpan, 1);
    	/* Insert bullet character */
    	span->text = g_strconcat(BULLET, g_strndup(text, text_len), "\n", NULL); /*TODO: concat already copies, so we dont need strndup */
    	span->tags = g_slist_copy(info->tag_stack);
    	
    	info->spans = g_list_prepend(info->spans, span);
    	break;
      
    case STATE_METADATA:
    	tag_name = (gchar*)(info->meta_data_tag_stack->data);
    	save_meta_data(tag_name, g_strndup(text, text_len), info->note);
    	break;
    
    default:
      g_assert_not_reached ();
      break;
    }
	
}

static void
parse_info_init (ParseInfo     *info,
		 GtkTextBuffer *buffer,
		 gboolean       create_tags,
		 GList         *headers,
		 Note		   *note)
{
  info->states = g_slist_prepend (NULL, GINT_TO_POINTER (STATE_START));

  info->create_tags = create_tags;
  info->headers = headers;
  info->defined_tags = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
  info->substitutions = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
  info->anonymous_tags = g_hash_table_new_full (NULL, NULL, NULL, NULL);
  info->tag_stack = NULL;
  info->spans = NULL;
  info->parsed_text = FALSE;
  info->parsed_tags = FALSE;
  info->current_tag = NULL;
  info->current_tag_prio = -1;
  info->tag_priorities = NULL;

  info->buffer = buffer;
  info->meta_data_tag_stack = NULL;
  
  info->note = note;
}

static void
text_span_free (TextSpan *span)
{
  g_free (span->text);
  g_slist_free (span->tags);
  g_free (span);
}

static void
parse_info_free (ParseInfo *info)
{
  GList *list;

  g_slist_free (info->tag_stack);
  g_slist_free (info->states);

  g_hash_table_destroy (info->substitutions);
  g_hash_table_destroy (info->defined_tags);

  if (info->current_tag)
    g_object_unref (info->current_tag);

  list = info->spans;
  while (list)
    {
      text_span_free (list->data);

      list = list->next;
    }
  g_list_free (info->spans);

  list = info->tag_priorities;
  while (list)
    {
      TextTagPrio *prio = list->data;

      if (prio->tag)
	g_object_unref (prio->tag);
      g_free (prio);

      list = list->next;
    }
  g_list_free (info->tag_priorities);

}

static void
insert_text (ParseInfo   *info, GtkTextIter *iter)
{
  GtkTextIter start_iter;
  GtkTextMark *mark;
  GList *tmp;
  GSList *tags;

  start_iter = *iter;

  mark = gtk_text_buffer_create_mark (info->buffer, "deserialize_insert_point", &start_iter, TRUE);

  tmp = info->spans;
  while (tmp) {
    TextSpan *span = tmp->data;

    /* TEST */
    /*gtk_text_buffer_get_iter_at_offset(info->buffer, iter, 0);*/
    
    /* insert text */
	gtk_text_buffer_insert(info->buffer, iter, span->text, -1);
	
	/* get mark from iter */
	gtk_text_buffer_get_iter_at_mark(info->buffer, &start_iter, mark);

	/* Apply tags */
	tags = span->tags;
	while (tags) {
		GtkTextTag *tag = tags->data;
		gtk_text_buffer_apply_tag(info->buffer, tag, &start_iter, iter);
		tags = tags->next;
	}
	gtk_text_buffer_move_mark(info->buffer, mark, iter);
	tmp = tmp->next;
  }

  gtk_text_buffer_delete_mark (info->buffer, mark);
}



static int
read_int (const guchar *start)
{
  int result;

  result =
    start[0] << 24 |
    start[1] << 16 |
    start[2] << 8 |
    start[3];

  return result;
}

static gboolean
header_is (Header      *header,
           const gchar *id)
{
  return (strncmp (header->id, id, strlen (id)) == 0);
}

static GList *
read_headers (const gchar *start,
	      gint         len,
	      GError     **error)
{
  int i = 0;
  int section_len;
  Header *header;
  GList *headers = NULL;

  while (i < len)
    {
      if (i + 30 >= len)
	goto error;

      if (strncmp (start + i, "GTKTEXTBUFFERCONTENTS-0001", 26) == 0 ||
	  strncmp (start + i, "GTKTEXTBUFFERPIXBDATA-0001", 26) == 0)
	{
	  section_len = read_int ((const guchar *) start + i + 26);

	  if (i + 30 + section_len > len)
	    goto error;

	  header = g_new0 (Header, 1);
	  header->id = start + i;
	  header->length = section_len;
	  header->start = start + i + 30;

	  i += 30 + section_len;

	  headers = g_list_prepend (headers, header);
	}
      else
	break;
    }

  return g_list_reverse (headers);

 error:
  g_list_foreach (headers, (GFunc) g_free, NULL);
  g_list_free (headers);

  g_set_error_literal (error,
                       G_MARKUP_ERROR,
                       G_MARKUP_ERROR_PARSE,
                       ("Serialized data is malformed"));

  return NULL;
}

static gboolean
deserialize_text (GtkTextBuffer *buffer,
		  GtkTextIter   *iter,
		  const gchar   *text,
		  gint           len,
		  gboolean       create_tags,
		  GError       **error,
		  GList         *headers,
		  Note			*note)
{
  GMarkupParseContext *context;
  ParseInfo info;
  gboolean retval = FALSE;

  static const GMarkupParser rich_text_parser = {
    start_element_handler,
    end_element_handler,
    text_handler,
    NULL,
    NULL
  };

  parse_info_init (&info, buffer, create_tags, headers, note);

  context = g_markup_parse_context_new (&rich_text_parser,
                                        0, &info, NULL);

  if (!g_markup_parse_context_parse (context,
                                     text,
                                     len,
                                     error)) {
    goto out;
  }

  if (!g_markup_parse_context_end_parse (context, error)) {
    goto out;
  }

  retval = TRUE;
  
  /* Now insert the text */
  insert_text (&info, iter);

 out:
  /* Free some memory */
  parse_info_free (&info);
  g_markup_parse_context_free (context);

  return retval;
}


/*
 * 
 * register_buffer: the GtkTextBuffer the format is registered with
 * content_buffer:  the GtkTextBuffer to deserialize into
 * iter:            insertion point for the deserialized text
 * data:            data to deserialize
 * length:          length of data
 * create_tags:     TRUE if deserializing may create tags
 * user_data:       user data that was specified when registering the format
 * error:           return location for a GError
 *
 * Returns:         TRUE on success, FALSE otherwise 
 */

gboolean
deserialize_from_tomboy                (GtkTextBuffer *register_buffer,
                                        GtkTextBuffer *content_buffer,
                                        GtkTextIter   *iter,
                                        const guint8  *text,
                                        gsize          length,
                                        gboolean       create_tags,
                                        gpointer       user_data,
                                        GError       **error)
{
  GList *headers;
  Header *header;
  gboolean retval;
  GtkTextIter start, end;
  
  Note *note = (Note*)user_data;
  
  /* Check headers for error and fill in note version and title */
  /*read_headers((gchar*)text, note, length, error);*/
  
  /*
  headers = read_headers ((gchar *) text, length, error);

  if (!headers)
    return FALSE;

  header = headers->data;
  if (!header_is (header, "GTKTEXTBUFFERCONTENTS-0001"))
    {
      g_set_error_literal (error,
                           G_MARKUP_ERROR,
                           G_MARKUP_ERROR_PARSE,
                           ("Serialized data is malformed. First section isn't GTKTEXTBUFFERCONTENTS-0001"));

      retval = FALSE;
      goto out;
    }
  */
 
  
  
  retval = deserialize_text(content_buffer, iter, text, length, TRUE, error, NULL, note);

  /*deserialize_metadata(text, length, note);*/
  
  
/* out: */
  /*
  g_list_foreach (headers, (GFunc)g_free, NULL);
  g_list_free (headers);
  */

  return retval;
}
