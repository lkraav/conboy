#ifndef SERIALIZER_H
#define SERIALIZER_H

guint8 * serialize_to_tomboy  (GtkTextBuffer     *register_buffer,
                                                 GtkTextBuffer     *content_buffer,
                                                 const GtkTextIter *start,
                                                 const GtkTextIter *end,
                                                 gsize             *length,
                                                 gpointer           user_data);

#endif /* SERIALIZER_H */
