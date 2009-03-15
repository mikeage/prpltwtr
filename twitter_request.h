/**
 * TODO: legal stuff
 *
 * purple
 *
 * Purple is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#include <glib.h>

typedef void (*TwitterSendRequestFunc)(PurpleAccount *acct, xmlnode *node, gpointer user_data);

void twitter_send_request(PurpleAccount *account, gboolean post,
		const char *url, const char *query_string, 
		TwitterSendRequestFunc success_callback, TwitterSendRequestFunc error_callback,
		gpointer data);

//don't include count in the query_string
void twitter_send_request_multipage(PurpleAccount *account, 
		const char *url, const char *query_string,
		TwitterSendRequestFunc success_callback, TwitterSendRequestFunc error_callback,
		int expected_count, gpointer data);

