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

#ifndef _TWITTER_REQUEST_H_
#define _TWITTER_REQUEST_H_

#include <glib.h>

typedef struct
{
	gchar *name;
	gchar *value;
} TwitterRequestParam;

TwitterRequestParam *twitter_request_param_new(const gchar *name, const gchar *value);
TwitterRequestParam *twitter_request_param_new_int(const gchar *name, int value);
TwitterRequestParam *twitter_request_param_new_ll(const gchar *name, long long value);

typedef GArray TwitterRequestParams;

TwitterRequestParams *twitter_request_params_new();
TwitterRequestParams *twitter_request_params_add(GArray *params, TwitterRequestParam *p);
void twitter_request_params_free(TwitterRequestParams *params);
void twitter_request_param_free(TwitterRequestParam *p);

typedef enum
{
	TWITTER_REQUEST_ERROR_NONE,
	TWITTER_REQUEST_ERROR_SERVER,
	TWITTER_REQUEST_ERROR_TWITTER_GENERAL,
	TWITTER_REQUEST_ERROR_INVALID_XML,
	TWITTER_REQUEST_ERROR_NO_OAUTH,
	
	TWITTER_REQUEST_ERROR_UNAUTHORIZED
} TwitterRequestErrorType;

typedef struct
{
	TwitterRequestErrorType type;
	/*const xmlnode *response_node;*/
	const gchar *message;
} TwitterRequestErrorData;

typedef void (*TwitterSendRequestSuccessFunc)(PurpleAccount *account, const gchar *response, gpointer user_data);

typedef void (*TwitterSendXmlRequestSuccessFunc)(PurpleAccount *account, xmlnode *node, gpointer user_data);
typedef void (*TwitterSendRequestErrorFunc)(PurpleAccount *account, const TwitterRequestErrorData *error_data, gpointer user_data);

typedef struct _TwitterMultiPageRequestData TwitterMultiPageRequestData;

typedef gboolean (*TwitterSendRequestMultiPageSuccessFunc)(PurpleAccount *account, xmlnode *node, gboolean last_page, TwitterMultiPageRequestData *request, gpointer user_data);
typedef gboolean (*TwitterSendRequestMultiPageErrorFunc)(PurpleAccount *account, const TwitterRequestErrorData *error_data, gpointer user_data);

struct _TwitterMultiPageRequestData
{
	gpointer user_data;
	char *url;
	//char *query_string;
	TwitterRequestParams *params;
	TwitterSendRequestMultiPageSuccessFunc success_callback;
	TwitterSendRequestMultiPageErrorFunc error_callback;
	int page;
	int expected_count;
};


typedef void (*TwitterSendRequestMultiPageAllSuccessFunc)(PurpleAccount *account, GList *nodes, gpointer user_data);
typedef gboolean (*TwitterSendRequestMultiPageAllErrorFunc)(PurpleAccount *account, const TwitterRequestErrorData *error_data, gpointer user_data);

void twitter_send_request(PurpleAccount *account,
		gboolean post,
		const char *url,
		const TwitterRequestParams *params,
		gboolean auth_basic,
		TwitterSendRequestSuccessFunc success_callback,
		TwitterSendRequestErrorFunc error_callback,
		gpointer data);

void twitter_send_xml_request(PurpleAccount *account, gboolean post,
		const char *url, TwitterRequestParams *params,
		TwitterSendXmlRequestSuccessFunc success_callback, TwitterSendRequestErrorFunc error_callback,
		gpointer data);

//don't include count in the query_string
void twitter_send_xml_request_multipage_all(PurpleAccount *account,
		const char *url, TwitterRequestParams *params,
		TwitterSendRequestMultiPageAllSuccessFunc success_callback,
		TwitterSendRequestMultiPageAllErrorFunc error_callback,
		int expected_count, gint max_count, gpointer data);

/* statuses/friends API deprecated page based retrieval,
 * and use cursor based method instead */
void twitter_send_xml_request_with_cursor (PurpleAccount *account,
       const char *url, const TwitterRequestParams *params, long long cursor,
       TwitterSendRequestMultiPageAllSuccessFunc success_callback,
       TwitterSendRequestMultiPageAllErrorFunc error_callback,
       gpointer data);

#endif
