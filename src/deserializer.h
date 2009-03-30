#ifndef DESERIALIZER_H
#define DESERIALIZER_H

gboolean
deserialize_from_tomboy (GtkTextBuffer *register_buffer,
                                        GtkTextBuffer *content_buffer,
                                        GtkTextIter   *iter,
                                        const guint8  *text,
                                        gsize          length,
                                        gboolean       create_tags,
                                        gpointer       user_data,
                                        GError       **error);

#endif /* DESERIALIZER_H */