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

typedef enum
{
	TWITTER_REQUEST_ERROR_NONE,
	TWITTER_REQUEST_ERROR_SERVER,
	TWITTER_REQUEST_ERROR_TWITTER_GENERAL,
	TWITTER_REQUEST_ERROR_INVALID_XML
} TwitterRequestErrorType;

typedef struct
{
	TwitterRequestErrorType type;
	/*const xmlnode *response_node;*/
	const gchar *message;
} TwitterRequestErrorData;

typedef void (*TwitterSendRequestSuccessFunc)(PurpleAccount *acct, xmlnode *node, gpointer user_data);
typedef void (*TwitterSendRequestErrorFunc)(PurpleAccount *acct, const TwitterRequestErrorData *error_data, gpointer user_data);

typedef gboolean (*TwitterSendRequestMultiPageSuccessFunc)(PurpleAccount *acct, xmlnode *node, gboolean last_page, gpointer user_data);
typedef gboolean (*TwitterSendRequestMultiPageErrorFunc)(PurpleAccount *acct, const TwitterRequestErrorData *error_data, gpointer user_data);

typedef void (*TwitterSendRequestMultiPageAllSuccessFunc)(PurpleAccount *acct, GList *nodes, gpointer user_data);
typedef gboolean (*TwitterSendRequestMultiPageAllErrorFunc)(PurpleAccount *acct, const TwitterRequestErrorData *error_data, gpointer user_data);

void twitter_send_request(PurpleAccount *account, gboolean post,
		const char *url, const char *query_string, 
		TwitterSendRequestSuccessFunc success_callback, TwitterSendRequestErrorFunc error_callback,
		gpointer data);

//don't include count in the query_string
void twitter_send_request_multipage(PurpleAccount *account, 
		const char *url, const char *query_string,
		TwitterSendRequestMultiPageSuccessFunc success_callback, TwitterSendRequestMultiPageErrorFunc error_callback,
		int expected_count, gpointer data);

void twitter_send_request_multipage_all(PurpleAccount *account,
		const char *url, const char *query_string,
		TwitterSendRequestMultiPageAllSuccessFunc success_callback,
		TwitterSendRequestMultiPageAllErrorFunc error_callback,
		int expected_count, gpointer data);
