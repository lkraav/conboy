#ifndef JSON_H
#define JSON_H

#include <json-glib/json-glib.h>
#include "note.h"

JsonNode* get_json_object_from_note(Note *note);
Note* get_note_from_json_object(JsonObject *json);
void print_note_as_json(Note *note);


#endif /* JSON_H */
