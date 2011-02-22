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
 * it with code to include your own defaults.h or similar.  If you're going to
 * provide for translation, you'll also need to setup the gettext macros. */
#include "defaults.h"

#include <debug.h>
#include <request.h>
#include "twitter_prefs.h"
#include "twitter_request.h"
#include "twitter_util.h"
#include "twitter_conn.h"

#define USER_AGENT "Mozilla/4.0 (compatible; MSIE 5.5)"

//TODO: clean this up to be a bit more robust. 

typedef struct {
	TwitterRequestor *requestor;
	TwitterSendRequestSuccessFunc success_func;
	TwitterSendRequestErrorFunc error_func;

	gpointer request_id;
	gpointer user_data;
} TwitterSendRequestData;

typedef struct {
	TwitterSendXmlRequestSuccessFunc success_func;
	TwitterSendRequestErrorFunc error_func;
	gpointer user_data;
} TwitterSendXmlRequestData;

typedef struct
{
	GList *nodes;
	TwitterSendRequestMultiPageAllSuccessFunc success_callback;
	TwitterSendRequestMultiPageAllErrorFunc error_callback;
	gint max_count;
	gint current_count;
	gpointer user_data;
} TwitterMultiPageAllRequestData;

typedef struct {
	GList *nodes;
	long long next_cursor;
	gchar *url;
	TwitterRequestParams *params;

	TwitterSendRequestMultiPageAllSuccessFunc success_callback;
	TwitterSendRequestMultiPageAllErrorFunc error_callback;
	gpointer user_data;
} TwitterRequestWithCursorData;

void twitter_send_xml_request_multipage_do(TwitterRequestor *r,
		TwitterMultiPageRequestData *request_data);

static void twitter_send_xml_request_with_cursor_cb(TwitterRequestor *r,
		xmlnode *node,
		gpointer user_data);


TwitterRequestParam *twitter_request_param_new(const gchar *name, const gchar *value)
{
	TwitterRequestParam *p = g_new(TwitterRequestParam, 1);
	p->name = g_strdup(name);
	p->value = g_strdup(value);
	return p;
}
TwitterRequestParam *twitter_request_param_new_int(const gchar *name, int value)
{
	TwitterRequestParam *p = g_new(TwitterRequestParam, 1);
	p->name = g_strdup(name);
	p->value = g_strdup_printf("%d", value);
	return p;
}
TwitterRequestParam *twitter_request_param_new_ll(const gchar *name, long long value)
{
	TwitterRequestParam *p = g_new(TwitterRequestParam, 1);
	p->name = g_strdup(name);
	p->value = g_strdup_printf("%lld", value);
	return p;
}
static TwitterRequestParam *twitter_request_param_clone(const TwitterRequestParam *p)
{
	if (p == NULL)
		return NULL;
	return twitter_request_param_new(p->name, p->value);
}

TwitterRequestParams *twitter_request_params_new()
{
	return g_array_new(FALSE, FALSE, sizeof(TwitterRequestParam *));
}
TwitterRequestParams *twitter_request_params_add(TwitterRequestParams *params, TwitterRequestParam *p)
{
	return g_array_append_val(params, p);
}
static TwitterRequestParams *twitter_request_params_clone(const TwitterRequestParams *params)
{
	int i;
	TwitterRequestParams *clone;
	if (params == NULL)
		return NULL;
	clone = twitter_request_params_new();
	for (i = 0; i < params->len; i++)
		twitter_request_params_add(clone,
				twitter_request_param_clone(g_array_index(params, TwitterRequestParam *, i)));
	return clone;
}
void twitter_request_params_free(TwitterRequestParams *params)
{
	int i;
	if (!params)
		return;
	for (i = 0; i < params->len; i++)
		twitter_request_param_free(g_array_index(params, TwitterRequestParam *, i));
	g_array_free(params, FALSE);
}

static void twitter_request_params_set_size(TwitterRequestParams *params, guint length)
{
	int i;
	for (i = length; i < params->len; i++)
	{
		twitter_request_param_free(g_array_index(params, TwitterRequestParam *, i));
	}
	g_array_set_size(params, length);
}
void twitter_request_param_free(TwitterRequestParam *p)
{
	g_free(p->name);
	g_free(p->value);
	g_free(p);
}

static gchar *twitter_request_params_to_string(const TwitterRequestParams *params)
{
	TwitterRequestParam *p;
	GString *rv;
	int i;
	if (!params || !params->len)
		return NULL;
	p = g_array_index(params, TwitterRequestParam *, 0);
	rv = g_string_new(NULL);
	rv = g_string_append(rv, purple_url_encode(p->name));
	rv = g_string_append_c(rv, '=');
	rv = g_string_append(rv, purple_url_encode(p->value));
	for (i = 1; i < params->len; i++)
	{
		p = g_array_index(params, TwitterRequestParam *, i);
		rv = g_string_append_c(rv, '&');
		rv = g_string_append(rv, purple_url_encode(p->name));
		rv = g_string_append_c(rv, '=');
		rv = g_string_append(rv, purple_url_encode(p->value));
	}
	return g_string_free(rv, FALSE);

}

static void twitter_requestor_on_error(TwitterRequestor *r, const TwitterRequestErrorData *error_data, TwitterSendRequestErrorFunc called_error_cb, gpointer user_data)
{
	if (r->pre_failed)
		r->pre_failed(r, &error_data);
	if (called_error_cb)
		called_error_cb(r, error_data, user_data);
	if (r->post_failed)
		r->post_failed(r, &error_data);
}

gint twitter_response_text_status_code(const gchar *response_text)
{
	const gchar *ptr;
	const gchar *starts_with = "HTTP/1.";
	if (response_text == NULL || !g_str_has_prefix(response_text, starts_with))
	{
		return 0;
	}
	ptr = response_text + strlen(starts_with) + 2; //add the "0 "

	return atoi(ptr);
}

static gboolean twitter_response_text_rate_limit(const gchar *response_text, int * remaining, int *total)
{
	const gchar *ptr;
	const gchar *remaining_str = "X-RateLimit-Remaining: ";
	const gchar *total_str = "X-RateLimit-Limit: ";

	if (!response_text)
		return FALSE;

	ptr = g_strrstr(response_text, remaining_str);
	if (!ptr)
		return FALSE;

	*remaining = atoi(ptr+strlen(remaining_str));

	ptr = g_strrstr(response_text, total_str);
	if (!ptr)
		return FALSE;

	*total = atoi(ptr+strlen(total_str));

	return TRUE;
}

const gchar *twitter_response_text_data(const gchar *response_text, gsize len)
{
	const gchar *data = g_strstr_len(response_text, len, "\r\n\r\n");
	if (data)
	{
		return data + 4;
	}
	return NULL;
}

static gchar *twitter_xml_node_parse_error(const xmlnode *node)
{
	return xmlnode_get_child_data(node, "error");
}

static gchar *twitter_xml_text_parse_error(const gchar *response)
{
	xmlnode *response_node;
	if (response && (response_node = xmlnode_from_str(response, strlen(response))))
	{
		gchar *message = twitter_xml_node_parse_error(response_node);
		xmlnode_free(response_node);
		return message;
	}
	return NULL;
}


static void twitter_send_request_cb(PurpleUtilFetchUrlData *url_data, gpointer user_data,
		const gchar *response_text, gsize len,
		const gchar *server_error_message)
{
	const gchar *url_text;
	TwitterSendRequestData *request_data = user_data;
	gchar *error_message = NULL;
	TwitterRequestErrorType error_type = TWITTER_REQUEST_ERROR_NONE;
	gint status_code;

	request_data->requestor->pending_requests =
		g_list_remove(request_data->requestor->pending_requests, request_data);

#ifdef _DEBUG_
	purple_debug_info(TWITTER_PROTOCOL_ID, "Received response: %s\n",
			response_text ? response_text : "NULL");
#endif
	
	status_code = twitter_response_text_status_code(response_text);

	url_text = twitter_response_text_data(response_text, len);

	if (server_error_message)
	{
		purple_debug_info(TWITTER_PROTOCOL_ID, "Response error: %s\n", server_error_message);
		error_type = TWITTER_REQUEST_ERROR_SERVER;
		error_message = g_strdup(server_error_message);
	} else {
		switch (status_code)
		{
			case 200: //OK
			break;
			case 304: //Not Modified
			break;
			case 401: //Unauthorized
				error_type = TWITTER_REQUEST_ERROR_UNAUTHORIZED;
			break;
			default:
				error_type = TWITTER_REQUEST_ERROR_TWITTER_GENERAL;
			break;
			/*
			case 400: //Bad Request
				//TODO
			break;
			case 403: //Forbidden
				//TODO
			break;
			case 404: //Not Found
				//TODO?
			break;
			case 406: //Not Acceptable (Search)
				//TODO
			break;
			case 420: //Search Rate Limiting
				//TODO
			break;
			case 500: //Internal Server Error
				//TODO
			break;
			case 502: //Bad Gateway
				//TODO
			break;
			case 504: //Service Unavailable
				//TODO
			break;
			*/
		}
		if (error_type != TWITTER_REQUEST_ERROR_NONE)
		{
			error_message = twitter_xml_text_parse_error(url_text);
			if (!error_message)
				error_message = g_strdup_printf("Status code: %d", status_code);
		}
	}

	if (error_type != TWITTER_REQUEST_ERROR_NONE)
	{
		TwitterRequestErrorData *error_data = g_new0(TwitterRequestErrorData, 1);
		error_data->type = error_type;
		error_data->message = error_message;
		twitter_requestor_on_error(request_data->requestor, error_data, request_data->error_func, request_data->user_data);
		g_free(error_data);
	} else {
		twitter_response_text_rate_limit(response_text, &(request_data->requestor->rate_limit_remaining), &(request_data->requestor->rate_limit_total));

		purple_debug_info(TWITTER_PROTOCOL_ID, "Valid response, calling success func\n");
		if (request_data->success_func)
			request_data->success_func(request_data->requestor, url_text, request_data->user_data);
	}

	if (error_message)
		g_free(error_message);
	g_free(request_data);
}

static gpointer twitter_send_request_querystring(TwitterRequestor *r,
		gboolean post,
		const char *url,
		const char *query_string,
		char **header_fields,
		TwitterSendRequestSuccessFunc success_callback,
		TwitterSendRequestErrorFunc error_callback,
		gpointer data)
{
	PurpleAccount *account = r->account;
	gchar *request;
	gboolean use_https = twitter_option_use_https(account) && purple_ssl_is_supported();
	char *slash = strchr(url, '/');
	TwitterSendRequestData *request_data = g_new0(TwitterSendRequestData, 1);
	char *host = slash ? g_strndup(url, slash - url) : g_strdup(url);
	char *full_url = g_strdup_printf("%s://%s",
			use_https ? "https" : "http",
			url);
	char *header_fields_text = (header_fields ? g_strjoinv("\r\n", header_fields) : NULL);

	purple_debug_info(TWITTER_PROTOCOL_ID, "Sending %s request to: %s ? %s\n", post ? "POST" : "GET",
			full_url,
			query_string ? query_string : "");

	request_data->requestor = r;
	request_data->user_data = data;
	request_data->success_func = success_callback;
	request_data->error_func = error_callback;

	request = g_strdup_printf(
			"%s %s%s%s HTTP/1.0\r\n"
			"User-Agent: " USER_AGENT "\r\n"
			"Host: %s\r\n"
			"%s" //Content-Type if post
			"%s%s" //extra header fields, if any
			"Content-Length: %lu\r\n\r\n"
			"%s",
			post ? "POST" : "GET",
			full_url,
			(!post && query_string ? "?" : ""), (!post && query_string ? query_string : ""),
			host,
			header_fields_text ? header_fields_text : "",
			header_fields_text ? "\r\n" : "",
			post ? "Content-Type: application/x-www-form-urlencoded\r\n" : "",
			query_string && post ? (unsigned long) strlen(query_string) : 0,
			query_string && post ? query_string : "");

#ifdef _DEBUG_
	purple_debug_info(TWITTER_PROTOCOL_ID, "Sending request: %s\n", request);
#endif
	
	request_data->request_id = purple_util_fetch_url_request(full_url, TRUE,
			USER_AGENT, TRUE, request, TRUE,
			twitter_send_request_cb, request_data);
	g_free(full_url);
	g_free(request);
	g_free(host);
	g_free(header_fields_text);

	return request_data;
}

gpointer twitter_requestor_send(TwitterRequestor *r,
		gboolean post,
		const char *url,
		TwitterRequestParams *params,
		char **header_fields,
		TwitterSendRequestSuccessFunc success_callback,
		TwitterSendRequestErrorFunc error_callback,
		gpointer data)
{
	gpointer request;
	gchar *querystring = twitter_request_params_to_string(params);
	request = twitter_send_request_querystring(r, post, url, querystring, header_fields, success_callback, error_callback, data);
	g_free(querystring);
	return request;
}

void twitter_send_request(TwitterRequestor *r,
		gboolean post,
		const char *url,
		TwitterRequestParams *params,
		TwitterSendRequestSuccessFunc success_callback,
		TwitterSendRequestErrorFunc error_callback,
		gpointer data)
{
	gpointer requestor_data = NULL;
	gpointer request = NULL;
	gchar **header_fields = NULL;

	if (r->pre_send)
		r->pre_send(r, &post, &url, &params, &header_fields, &requestor_data);

	if (r->do_send)
		request = r->do_send(r, post,
			url, params,
			header_fields,
			success_callback,
			error_callback,
			data);

	if (request)
		r->pending_requests = g_list_append(r->pending_requests, request);

	if (r->post_send)
		r->post_send(r, &post, &url, &params, &header_fields, &requestor_data);
}

static void twitter_xml_request_success_cb(TwitterRequestor *r, const gchar *response, gpointer user_data)
{
	TwitterSendXmlRequestData *request_data = user_data;
	const gchar *error_message = NULL;
	gchar *error_node_text = NULL;
	xmlnode *response_node = NULL;
	TwitterRequestErrorType error_type = TWITTER_REQUEST_ERROR_NONE;

	response_node = xmlnode_from_str(response, strlen(response));
	if (!response_node)
	{
		purple_debug_info(TWITTER_PROTOCOL_ID, "Response error: invalid xml\n");
		error_type = TWITTER_REQUEST_ERROR_INVALID_XML;
		error_message = response;
	} else {
		if ((error_message = twitter_xml_node_parse_error(response_node)))
		{
			error_type = TWITTER_REQUEST_ERROR_TWITTER_GENERAL;
			error_message = error_node_text;
			purple_debug_info(TWITTER_PROTOCOL_ID, "Response error: Twitter error %s\n", error_message);
		}
	}

	if (error_type != TWITTER_REQUEST_ERROR_NONE)
	{
		/* Turns out this wasn't really a success. We got a twitter error instead of an HTTP error
		 * So go through the error cycle 
		 */
		TwitterRequestErrorData *error_data = g_new0(TwitterRequestErrorData, 1);
		error_data->type = error_type;
		error_data->message = error_message;
		twitter_requestor_on_error(r, error_data, request_data->error_func, request_data->user_data);

		g_free(error_data);
	} else {
		purple_debug_info(TWITTER_PROTOCOL_ID, "Valid response, calling success func\n");
		if (request_data->success_func)
			request_data->success_func(r, response_node, request_data->user_data);
	}

	if (response_node != NULL)
		xmlnode_free(response_node);
	if (error_node_text != NULL)
		g_free(error_node_text);
	g_free(request_data);
}

static void twitter_xml_request_error_cb(TwitterRequestor *r, const TwitterRequestErrorData *error_data, gpointer user_data)
{
	/* This gets called after the pre_failed and before the post_failed.
	 * So we just pass the error along to our caller. No need to call the requestor_on_fail 
	 * In fact, if we do, we'll get an infinite loop
	 */
	TwitterSendXmlRequestData *request_data = user_data;
	if (request_data->error_func)
		request_data->error_func(r, error_data, request_data->user_data);
	g_free(request_data);
}

void twitter_send_xml_request(TwitterRequestor *r, gboolean post,
		const char *url, TwitterRequestParams *params,
		TwitterSendXmlRequestSuccessFunc success_callback, TwitterSendRequestErrorFunc error_callback,
		gpointer data)
{

	TwitterSendXmlRequestData *request_data = g_new0(TwitterSendXmlRequestData, 1);
	request_data->user_data = data;
	request_data->success_func = success_callback;
	request_data->error_func = error_callback;

	twitter_send_request(r,
			post,
			url,
			params,
			twitter_xml_request_success_cb,
			twitter_xml_request_error_cb,
			request_data);
}

static long long twitter_oauth_generate_nonce()
{
	static long long nonce = 0;
	return ++nonce;
}

static gint twitter_request_params_sort_do(TwitterRequestParam **a, TwitterRequestParam **b)
{
	gint val = strcmp((*a)->name, (*b)->name);
	if (val == 0)
		val = strcmp((*a)->value, (*b)->value);
	return val;
}

static gchar *twitter_oauth_get_text_to_sign(gboolean post, gboolean https, const gchar *url, const TwitterRequestParams *params)
{
	gchar *query_string = twitter_request_params_to_string(params);
	gchar *pieces[4];
	int i;
	gchar *sig_base;
	pieces[0] = g_strdup(post ? "POST" : "GET");
	pieces[1] = g_strdup_printf("http%s%%3A%%2F%%2F%s", https ? "s" : "", purple_url_encode(url));
	pieces[2] = g_strdup(purple_url_encode(query_string));
	pieces[3] = NULL;
	sig_base = g_strjoinv("&", pieces);
	for (i = 0; i < 3; i++)
		g_free(pieces[i]);
	g_free(query_string);
	return sig_base;
}

static gchar *twitter_oauth_sign(const gchar *txt, const gchar *key)
{
	PurpleCipher *cipher;
	PurpleCipherContext *ctx;
	static guchar output[20];
	size_t output_size;

	cipher = purple_ciphers_find_cipher("hmac");
	if (!cipher)
	{
		purple_debug_info(TWITTER_PROTOCOL_ID,
				"%s: Could not find cipher\n",
				G_STRFUNC);
		return NULL;
	}
	ctx = purple_cipher_context_new(cipher, NULL);
	if (!ctx)
	{
		purple_debug_info(TWITTER_PROTOCOL_ID,
				"%s: Could not create cipher context\n",
				G_STRFUNC);
		return NULL;
	}
	purple_cipher_context_set_option(ctx, "hash", "sha1");

	purple_cipher_context_set_key(ctx, (guchar *) key);
	purple_cipher_context_append(ctx, (guchar *) txt, strlen(txt));
	if (!purple_cipher_context_digest(ctx, 20, output, &output_size))
	{
		purple_debug_info(TWITTER_PROTOCOL_ID,
				"%s: Could not sign text\n",
				G_STRFUNC);
		purple_cipher_context_destroy(ctx);
		return NULL;
	}
	purple_cipher_context_destroy(ctx);
	return purple_base64_encode(output, output_size);

}

TwitterRequestParams *twitter_request_params_add_oauth_params(PurpleAccount *account,
		gboolean post, const gchar *url,
		const TwitterRequestParams *params,
		const gchar *token, const gchar *signing_key)
{
	gboolean use_https = twitter_option_use_https(account) && purple_ssl_is_supported();
	TwitterRequestParams *oauth_params = twitter_request_params_clone(params);
	gchar *signme;
	gchar *signature;
	if (oauth_params == NULL)
		oauth_params = twitter_request_params_new();

	twitter_request_params_add(oauth_params, twitter_request_param_new("oauth_consumer_key", TWITTER_OAUTH_KEY));
	twitter_request_params_add(oauth_params, twitter_request_param_new_ll("oauth_nonce", twitter_oauth_generate_nonce()));
	twitter_request_params_add(oauth_params, twitter_request_param_new("oauth_signature_method","HMAC-SHA1"));
	twitter_request_params_add(oauth_params, twitter_request_param_new_ll("oauth_timestamp", time(NULL)));
	if (token)
		twitter_request_params_add(oauth_params, twitter_request_param_new("oauth_token", token));

	g_array_sort(oauth_params, (GCompareFunc) twitter_request_params_sort_do);
	signme = twitter_oauth_get_text_to_sign(post, use_https, url, oauth_params);
	signature = twitter_oauth_sign(signme, signing_key);

	if (!signature)
	{
		twitter_request_params_free(oauth_params);
		return NULL;
	} else {
		twitter_request_params_add(oauth_params, twitter_request_param_new("oauth_signature", signature));
		return oauth_params;
	}
}

static int xmlnode_child_count(xmlnode *parent)
{
	int count = 0;
	xmlnode *child;
	if (parent == NULL)
		return 0;
	for (child = parent->child; child; child = child->next)
		if (child->name && child->type == XMLNODE_TYPE_TAG)
			count++;
	return count;
}

static void twitter_send_xml_request_multipage_cb(TwitterRequestor *r, xmlnode *node, gpointer user_data)
{
	TwitterMultiPageRequestData *request_data = user_data;
	int count = 0;
	gboolean get_next_page;
	gboolean last_page = FALSE;

	count = xmlnode_child_count(node);

	if (count < request_data->expected_count)
		last_page = TRUE;

	purple_debug_info(TWITTER_PROTOCOL_ID,
			"%s: last_page: %s, count: %d, expected_count: %d\n",
			G_STRFUNC, last_page?"yes":"no",
			count, request_data->expected_count);

	if (!request_data->success_callback) {
		get_next_page = TRUE;

		purple_debug_info(TWITTER_PROTOCOL_ID,
				"%s no request_data->success_callback, get_next_page: yes\n",
				G_STRFUNC);
	}
	else {
		get_next_page = request_data->success_callback(r,
				node,
				last_page,
				request_data,
				request_data->user_data);

		purple_debug_info(TWITTER_PROTOCOL_ID,
				"%s get_next_page: %s\n",
				G_STRFUNC, get_next_page?"yes":"no");
	}

	if (last_page || !get_next_page)
	{
		g_free(request_data->url);
		twitter_request_params_free(request_data->params);
		g_free(request_data);
	} else {
		request_data->page++;
		twitter_send_xml_request_multipage_do(r, request_data);
	}
}
static void twitter_send_xml_request_multipage_error_cb(TwitterRequestor *r, const TwitterRequestErrorData *error_data, gpointer user_data)
{
	TwitterMultiPageRequestData *request_data = user_data;
	gboolean try_again;

	if (!request_data->error_callback)
		try_again = FALSE;
	else
		try_again = request_data->error_callback(r, error_data, request_data->user_data);

	if (try_again)
		twitter_send_xml_request_multipage_do(r, request_data);
}

void twitter_send_xml_request_multipage_do(TwitterRequestor *r,
		TwitterMultiPageRequestData *request_data)
{
	int len = request_data->params->len;
	twitter_request_params_add(request_data->params, twitter_request_param_new_int("page", request_data->page));
	twitter_request_params_add(request_data->params, twitter_request_param_new_int("count", request_data->expected_count));

	purple_debug_info(TWITTER_PROTOCOL_ID, "%s: page: %d\n", G_STRFUNC, request_data->page);

	twitter_send_xml_request(r, FALSE,
			request_data->url, request_data->params,
			twitter_send_xml_request_multipage_cb, twitter_send_xml_request_multipage_error_cb,
			request_data);
	twitter_request_params_set_size(request_data->params, len);
}


static void twitter_send_xml_request_multipage(TwitterRequestor *r,
		const char *url, TwitterRequestParams *params,
		TwitterSendRequestMultiPageSuccessFunc success_callback,
		TwitterSendRequestMultiPageErrorFunc error_callback,
		int expected_count, gpointer data)
{
	TwitterMultiPageRequestData *request_data = g_new0(TwitterMultiPageRequestData, 1);
	request_data->user_data = data;
	request_data->url = g_strdup(url);
	request_data->params = twitter_request_params_clone(params);
	request_data->success_callback = success_callback;
	request_data->error_callback = error_callback;
	request_data->page = 1;
	request_data->expected_count = expected_count;

	twitter_send_xml_request_multipage_do(r, request_data);
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

static gboolean twitter_send_xml_request_multipage_all_success_cb(TwitterRequestor *r,
		xmlnode *node,
		gboolean last_page,
		TwitterMultiPageRequestData *request_multi,
		gpointer user_data)
{
	TwitterMultiPageAllRequestData *request_data_all = user_data;

	purple_debug_info (TWITTER_PROTOCOL_ID, "%s\n", G_STRFUNC);

	request_data_all->nodes = g_list_prepend(request_data_all->nodes, xmlnode_copy(node)); //TODO: update
	request_data_all->current_count += xmlnode_child_count(node);

	purple_debug_info (TWITTER_PROTOCOL_ID, "%s last_page: %d current_count: %d max_count: %d per page: %d\n", G_STRFUNC, last_page ? 1 : 0, request_data_all->current_count, request_data_all->max_count, request_multi->expected_count);
	if (last_page || (request_data_all->max_count > 0 && request_data_all->current_count >= request_data_all->max_count))
	{
		request_data_all->success_callback(r, request_data_all->nodes, request_data_all->user_data);
		twitter_multipage_all_request_data_free(request_data_all);
		return FALSE;
	} else if (request_data_all->max_count > 0 && (request_data_all->current_count + request_multi->expected_count > request_data_all->max_count)) {
		request_multi->expected_count = request_data_all->max_count - request_data_all->current_count;
	}
	return TRUE;
}

static gboolean twitter_send_xml_request_multipage_all_error_cb(TwitterRequestor *r, const TwitterRequestErrorData *error_data, gpointer user_data)
{
	TwitterMultiPageAllRequestData *request_data_all = user_data;
	if (request_data_all->error_callback && request_data_all->error_callback(r, error_data, request_data_all->user_data))
		return TRUE;
	twitter_multipage_all_request_data_free(request_data_all);
	return FALSE;
}

void twitter_send_xml_request_multipage_all(TwitterRequestor *r,
		const char *url, TwitterRequestParams *params,
		TwitterSendRequestMultiPageAllSuccessFunc success_callback,
		TwitterSendRequestMultiPageAllErrorFunc error_callback,
		int expected_count, gint max_count, gpointer data)
{
	TwitterMultiPageAllRequestData *request_data_all = g_new0(TwitterMultiPageAllRequestData, 1);
	request_data_all->success_callback = success_callback;
	request_data_all->error_callback = error_callback;
	request_data_all->nodes = NULL;
	request_data_all->user_data = data;
	request_data_all->max_count = max_count;

	if (max_count > 0 && expected_count > max_count)
		expected_count = max_count;

	twitter_send_xml_request_multipage(r,
			url,
			params,
			twitter_send_xml_request_multipage_all_success_cb,
			twitter_send_xml_request_multipage_all_error_cb,
			expected_count,
			request_data_all);
}


/******************************************************
 *  Request with cursor
 ******************************************************/

static void twitter_request_with_cursor_data_free (
		TwitterRequestWithCursorData *request_data)
{
	GList *l;

	for (l = request_data->nodes; l; l = l->next)
		xmlnode_free (l->data);
	g_list_free (request_data->nodes);
	g_free (request_data->url);
	twitter_request_params_free(request_data->params);
	g_slice_free (TwitterRequestWithCursorData, request_data);
}
static void twitter_send_xml_request_with_cursor_error_cb(TwitterRequestor *r,
		const TwitterRequestErrorData *error_data,
		gpointer user_data);

static void twitter_send_xml_request_with_cursor_cb(TwitterRequestor *r,
		xmlnode *node,
		gpointer user_data)
{
	TwitterRequestWithCursorData *request_data = user_data;
	xmlnode *users;
	gchar *next_cursor_str;

	next_cursor_str = xmlnode_get_child_data (node, "next_cursor");
	if (next_cursor_str)
	{
		request_data->next_cursor = strtoll (next_cursor_str, NULL, 10);
		g_free (next_cursor_str);
	} else {
		request_data->next_cursor = 0;
	}

	purple_debug_info (TWITTER_PROTOCOL_ID, "%s next_cursor: %lld\n",
			G_STRFUNC, request_data->next_cursor);

	users = xmlnode_get_child (node, "users");
	if (!users && node->name && !strcmp(node->name, "users"))
		users = node;

	if (users) 
	{
		request_data->nodes = g_list_prepend (request_data->nodes,
				xmlnode_copy (users));
	}

	if (request_data->next_cursor) {
		int len = request_data->params->len;
		twitter_request_params_add(request_data->params,
				twitter_request_param_new_ll("cursor", request_data->next_cursor));

		twitter_send_xml_request(r, FALSE,
				request_data->url, request_data->params,
				twitter_send_xml_request_with_cursor_cb,
				twitter_send_xml_request_with_cursor_error_cb,
				request_data);

		twitter_request_params_set_size(request_data->params, len);
	}
	else {
		request_data->success_callback(r,
				request_data->nodes,
				request_data->user_data);
		twitter_request_with_cursor_data_free (request_data);
	}
}

static void twitter_send_xml_request_with_cursor_error_cb(TwitterRequestor *r,
		const TwitterRequestErrorData *error_data,
		gpointer user_data)
{
	TwitterRequestWithCursorData *request_data = user_data;
	if (request_data->error_callback && request_data->error_callback(r, error_data, request_data->user_data))
	{
		twitter_send_xml_request(r, FALSE,
				request_data->url, request_data->params,
				twitter_send_xml_request_with_cursor_cb,
				twitter_send_xml_request_with_cursor_error_cb,
				request_data);
		return;
	}
	twitter_request_with_cursor_data_free(request_data);
}

void twitter_send_xml_request_with_cursor(TwitterRequestor *r,
		const char *url, TwitterRequestParams *params, long long cursor,
		TwitterSendRequestMultiPageAllSuccessFunc success_callback,
		TwitterSendRequestMultiPageAllErrorFunc error_callback,
		gpointer data)
{
	int len;

	TwitterRequestWithCursorData *request_data = g_slice_new0 (TwitterRequestWithCursorData);
	request_data->url = g_strdup (url);
	request_data->params = twitter_request_params_clone(params);
	if (!request_data->params)
		request_data->params = twitter_request_params_new();
	request_data->success_callback = success_callback;
	request_data->error_callback = error_callback;
	request_data->user_data = data;

	len = request_data->params->len;
	twitter_request_params_add(request_data->params,
			twitter_request_param_new_ll("cursor", cursor));

	twitter_send_xml_request(r, FALSE,
			url, request_data->params,
			twitter_send_xml_request_with_cursor_cb,
			twitter_send_xml_request_with_cursor_error_cb,
			request_data);

	twitter_request_params_set_size(request_data->params, len);
}

void twitter_requestor_free(TwitterRequestor *requestor)
{
	GList *l;
	purple_debug_info(TWITTER_PROTOCOL_ID, "Freeing requestor\n");
	if (requestor->pending_requests)
	{
		TwitterRequestErrorData *error_data;
		error_data = g_new0(TwitterRequestErrorData, 1);
		error_data->type = TWITTER_REQUEST_ERROR_CANCELED;
		error_data->message = NULL;
		for (l = requestor->pending_requests; l; l = l->next)
		{
			TwitterSendRequestData *request_data = l->data;
			//TODO: move this to a ->cancel function
			purple_util_fetch_url_cancel(request_data->request_id);
			//TODO: created a ->free callback
			twitter_requestor_on_error(request_data->requestor,
					error_data,
					request_data->error_func,
					request_data->user_data);
			g_free(request_data);
		}
		g_list_free(requestor->pending_requests);
		g_free(error_data);
	}
	g_free(requestor);
}
