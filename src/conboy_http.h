/* This file is part of Conboy.
 *
 * Copyright (C) 2010 Cornelius Hald
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

#ifndef CONBOY_HTTP_H_
#define CONBOY_HTTP_H_


#include <glib/gtypes.h>

gchar* conboy_http_get(const gchar *url, gboolean auth);

gchar* conboy_http_post (const gchar *url, gchar *postdata, gboolean auth);

gchar* conboy_http_put(const gchar *url, const gchar *putdata, gboolean auth);

#endif /*CONBOY_HTTP_H_*/
