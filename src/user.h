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

#ifndef USER_H_
#define USER_H_

typedef struct {
	/* Taken from Tomboy UserInfo.cs
	 * ResourceReference holds "href" and "api-ref" properties
	 */
	gchar *first_name;
	gchar *last_name;
	gint latest_sync_revision;
	gint current_sync_guid;
	ResourceReference *notes;
	ResourceReference *friends;
	
} User;

/* Those methods should call the service via REST. So they wrap the REST API */

/**
 * Returns a list of Note objects
 */
GList *user_get_notes(User *user, bool include_content, int since_revision, int *latest_sync_revision);

/**
 * Sends provided notes to server to update them.
 * Returns: The new sync revision provided by the server.
 */
int user_update_notes(User *user, GList *notes, gint expected_new_revision);

/**
 * Get a user object from the server. In Tomboy code it is not password,
 * but IAuthProvider, which is a interface for different auth methods - we
 * will see which are really required.
 */
User *user_get(gchar *server_url, gchar *user_name, gchar *password);



#endif /*USER_H_*/
