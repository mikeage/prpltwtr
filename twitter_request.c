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

#include <stdarg.h>
#include <string.h>
#include <time.h>

#include <glib.h>

/* If you're using this as the basis of a prpl that will be distributed
 * separately from libpurple, remove the internal.h include below and replace
 * it with code to include your own config.h or similar.  If you're going to
 * provide for translation, you'll also need to setup the gettext macros. */
#include "config.h"

#include "debug.h"
#include "request.h"
#include "twitter_request.h"

typedef struct {
	PurpleAccount *account;
	TwitterSendRequestSuccessFunc success_func;
	TwitterSendRequestErrorFunc error_func;
	gpointer user_data;
} TwitterSendRequestData;

typedef struct
{
	gpointer user_data;
	char *url;
	char *query_string;
	TwitterSendRequestMultiPageSuccessFunc success_callback;
	TwitterSendRequestMultiPageErrorFunc error_callback;
	int page;
	int expected_count;
} TwitterMultiPageRequestData;

typedef struct
{
	GList *nodes;
	TwitterSendRequestMultiPageAllSuccessFunc success_callback;
	TwitterSendRequestMultiPageAllErrorFunc error_callback;
	gpointer user_data;
} TwitterMultiPageAllRequestData;

void twitter_send_request_multipage_do(PurpleAccount *account,
		TwitterMultiPageRequestData *request_data);

void twitter_send_request_cb(PurpleUtilFetchUrlData *url_data, gpointer user_data,
		const gchar *url_text, gsize len,
		const gchar *server_error_message)
{
	TwitterSendRequestData *request_data = user_data;
	const gchar *error_message = NULL;
	gchar *error_node_text = NULL;
	xmlnode *response_node = NULL;
	TwitterRequestErrorType error_type = TWITTER_REQUEST_ERROR_NONE;

	purple_debug_info("twitter", "Response: %s\n", url_text);

	if (server_error_message)
	{
		error_type = TWITTER_REQUEST_ERROR_SERVER;
		error_message = server_error_message;
	} else {
		response_node = xmlnode_from_str(url_text, len);
		if (!response_node)
		{
			error_type = TWITTER_REQUEST_ERROR_INVALID_XML;
			error_message = url_text;
		} else {
			xmlnode *error_node;
			if ((error_node = xmlnode_get_child(response_node, "error")) != NULL)
			{
				error_type = TWITTER_REQUEST_ERROR_TWITTER_GENERAL;
				error_node_text = xmlnode_get_data(error_node);
				error_message = error_node_text;
			}
		}
	}

	if (error_type != TWITTER_REQUEST_ERROR_NONE)
	{
		TwitterRequestErrorData *error_data = g_new0(TwitterRequestErrorData, 1);
		error_data->type = error_type;
		error_data->message = error_message;
		//error_data->response_node = response_node;
		if (request_data->error_func)
			request_data->error_func(request_data->account, error_data, request_data->user_data);

		g_free(error_data);
	} else {
		if (request_data->success_func)
			request_data->success_func(request_data->account, response_node, request_data->user_data);
	}

	if (response_node != NULL)
		xmlnode_free(response_node);
	if (error_node_text != NULL)
		g_free(error_node_text);
	g_free(request_data);
}

void twitter_send_request(PurpleAccount *account, gboolean post,
		const char *url, const char *query_string, 
		TwitterSendRequestSuccessFunc success_callback, TwitterSendRequestErrorFunc error_callback,
		gpointer data)
{
	gchar *request;
	const char *pass = purple_connection_get_password(purple_account_get_connection(account));
	const char *sn = purple_account_get_username(account);
	char *auth_text = g_strdup_printf("%s:%s", sn, pass);
	char *auth_text_b64 = purple_base64_encode((guchar *) auth_text, strlen(auth_text));
	gboolean use_https = purple_account_get_bool(account, "use_https", FALSE) && purple_ssl_is_supported();
	char *host = "twitter.com";
	TwitterSendRequestData *request_data = g_new0(TwitterSendRequestData, 1);
	char *full_url = g_strdup_printf("%s://%s%s",
			use_https ? "https" : "http",
			host,
			url);
	request_data->account = account;
	request_data->user_data = data;
	request_data->success_func = success_callback;
	request_data->error_func = error_callback;

	g_free(auth_text);

	request = g_strdup_printf(
			"%s %s%s%s HTTP/1.0\r\n"
			"User-Agent: Mozilla/4.0 (compatible; MSIE 5.5)\r\n"
			"Host: %s\r\n"
			"Authorization: Basic %s\r\n"
			"Content-Length: %d\r\n\r\n"
			"%s",
			post ? "POST" : "GET",
			full_url,
			(!post && query_string ? "?" : ""), (!post && query_string ? query_string : ""),
			host,
			auth_text_b64,
			query_string  && post ? strlen(query_string) : 0,
			query_string && post ? query_string : "");

	g_free(auth_text_b64);
	purple_util_fetch_url_request(full_url, TRUE,
			"Mozilla/4.0 (compatible; MSIE 5.5)", TRUE, request, FALSE,
			twitter_send_request_cb, request_data);
	g_free(full_url);
	g_free(request);
}

void twitter_send_request_multipage_cb(PurpleAccount *account, xmlnode *node, gpointer user_data)
{
	TwitterMultiPageRequestData *request_data = user_data;
	xmlnode *child = node->child;
	int count = 0;
	gboolean get_next_page;
	gboolean last_page;
	while ((child = child->next) != NULL)
		if (child->name)
			count++;

	last_page = count < request_data->expected_count;
	if (!request_data->success_callback)
		get_next_page = TRUE;
	else
		get_next_page = request_data->success_callback(account, node, last_page, request_data->user_data);

	if (last_page)
	{
		g_free(request_data->url);
		if (request_data->query_string)
			g_free(request_data->query_string);
	} else {
		request_data->page++;
		twitter_send_request_multipage_do(account, request_data);
	}
}
void twitter_send_request_multipage_error_cb(PurpleAccount *account, const TwitterRequestErrorData *error_data, gpointer user_data)
{
	TwitterMultiPageRequestData *request_data = user_data;
	gboolean try_again;

	if (!request_data->error_callback)
		try_again = FALSE;
	else
		try_again = request_data->error_callback(account, error_data, request_data->user_data);

	if (try_again)
		twitter_send_request_multipage_do(account, request_data);
}

void twitter_send_request_multipage_do(PurpleAccount *account,
		TwitterMultiPageRequestData *request_data)
{
	char *full_query_string = g_strdup_printf("%s%spage=%d",
			request_data->query_string ? request_data->query_string : "",
			request_data->query_string && strlen(request_data->query_string) > 0 ? "&" : "",
			request_data->page);

	twitter_send_request(account, FALSE,
			request_data->url, full_query_string,
			twitter_send_request_multipage_cb, twitter_send_request_multipage_error_cb,
			request_data);
	g_free(full_query_string);
}

//don't include count in the query_string
void twitter_send_request_multipage(PurpleAccount *account, 
		const char *url, const char *query_string,
		TwitterSendRequestMultiPageSuccessFunc success_callback,
		TwitterSendRequestMultiPageErrorFunc error_callback,
		int expected_count, gpointer data)
{
	TwitterMultiPageRequestData *request_data = g_new0(TwitterMultiPageRequestData, 1);
	request_data->user_data = data;
	request_data->url = g_strdup(url);
	request_data->query_string = query_string ? g_strdup(query_string) : NULL;
	request_data->success_callback = success_callback;
	request_data->error_callback = error_callback;
	request_data->page = 1;
	request_data->expected_count = expected_count;

	twitter_send_request_multipage_do(account, request_data);
}

static void twitter_multipage_all_request_data_free(TwitterMultiPageAllRequestData *request_data_all)
{
	GList *l = request_data_all->nodes;
	for (l = request_data_all->nodes; l; l = l->next)
	{
		xmlnode_free(l->data);
	}
	g_list_free(request_data_all->nodes);
	g_free(request_data_all);
}

static gboolean twitter_send_request_multipage_all_success_cb(PurpleAccount *acct, xmlnode *node, gboolean last_page, gpointer user_data)
{
	TwitterMultiPageAllRequestData *request_data_all = user_data;
	request_data_all->nodes = g_list_append(request_data_all->nodes, xmlnode_copy(node)); //TODO: update
	if (last_page)
	{
		request_data_all->success_callback(acct, request_data_all->nodes, request_data_all->user_data);
		twitter_multipage_all_request_data_free(request_data_all);
	}
	return TRUE;
}
static gboolean twitter_send_request_multipage_all_error_cb(PurpleAccount *acct, const TwitterRequestErrorData *error_data, gpointer user_data)
{
	TwitterMultiPageAllRequestData *request_data_all = user_data;
	if (request_data_all->error_callback && request_data_all->error_callback(acct, error_data, request_data_all->user_data))
		return TRUE;
	twitter_multipage_all_request_data_free(request_data_all);
	return FALSE;
}

void twitter_send_request_multipage_all(PurpleAccount *account,
		const char *url, const char *query_string,
		TwitterSendRequestMultiPageAllSuccessFunc success_callback,
		TwitterSendRequestMultiPageAllErrorFunc error_callback,
		int expected_count, gpointer data)
{
	TwitterMultiPageAllRequestData *request_data_all = g_new0(TwitterMultiPageAllRequestData, 1);
	request_data_all->success_callback = success_callback;
	request_data_all->error_callback = error_callback;
	request_data_all->nodes = NULL;
	request_data_all->user_data = data;

	twitter_send_request_multipage(account,
			url, query_string,
			twitter_send_request_multipage_all_success_cb,
			twitter_send_request_multipage_all_error_cb,
			expected_count, request_data_all);
}
