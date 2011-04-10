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
#include "twitter_api.h"
#include "twitter_conn.h"

static const gchar *twitter_api_create_url(PurpleAccount *account,
		const gchar *endpoint)
{
	static char url[1024];
	const gchar *host = twitter_option_api_host(account);
	const gchar *subdir = twitter_option_api_subdir(account);
	g_return_val_if_fail(host != NULL && host[0] != '\0' && endpoint != NULL && endpoint[0] != '\0', NULL);
	
	if (subdir == NULL || subdir[0] == '\0')
		 subdir = "/";

	snprintf(url, 1023, "%s%s%s%s%s", 
			host,
			subdir[0] == '/' ? "" : "/",
			subdir,
			subdir[strlen(subdir) - 1] == '/' || endpoint[0] == '/' ? "" : "/",
			subdir[strlen(subdir) - 1] == '/' && endpoint[0] == '/' ? endpoint + 1 : endpoint);
	return url;
}
static const gchar *twitter_api_create_web_url(PurpleAccount *account,
		const gchar *endpoint)
{
	static char url[1024];
	const gchar *host = twitter_option_web_host(account);
	const gchar *subdir = twitter_option_web_subdir(account);
	gboolean use_https = twitter_option_use_https(account) && purple_ssl_is_supported();
	g_return_val_if_fail(host != NULL && host[0] != '\0' && endpoint != NULL && endpoint[0] != '\0', NULL);
	
	if (subdir == NULL || subdir[0] == '\0')
		 subdir = "/";

	snprintf(url, 1023, "%s://%s%s%s%s%s",
			use_https ? "https" : "http",
			host,
			subdir[0] == '/' ? "" : "/",
			subdir,
			subdir[strlen(subdir) - 1] == '/' || endpoint[0] == '/' ? "" : "/",
			subdir[strlen(subdir) - 1] == '/' && endpoint[0] == '/' ? endpoint + 1 : endpoint);

	return url;
}

static const gchar *twitter_option_url_get_rate_limit_status(PurpleAccount *account)
{
	return twitter_api_create_url(account,
			TWITTER_PREF_URL_GET_RATE_LIMIT_STATUS);
}
static const gchar *twitter_option_url_get_friends(PurpleAccount *account)
{
	return twitter_api_create_url(account,
			TWITTER_PREF_URL_GET_FRIENDS);
}
static const gchar *twitter_option_url_get_home_timeline(PurpleAccount *account)
{
	return twitter_api_create_url(account,
			TWITTER_PREF_URL_GET_HOME_TIMELINE);
}
static const gchar *twitter_option_url_get_list(PurpleAccount *account)
{
	gchar *url = g_strdup_printf("%s%s",
			account->username,
			TWITTER_PREF_URL_GET_LIST);
	const gchar *result = twitter_api_create_url(account,
			url);
	g_free(url);
	return result;
}
static const gchar *twitter_option_url_get_mentions(PurpleAccount *account)
{
	return twitter_api_create_url(account,
			TWITTER_PREF_URL_GET_MENTIONS);
}
static const gchar *twitter_option_url_get_dms(PurpleAccount *account)
{
	return twitter_api_create_url(account,
			TWITTER_PREF_URL_GET_DMS);
}
static const gchar *twitter_option_url_update_status(PurpleAccount *account)
{
	return twitter_api_create_url(account,
			TWITTER_PREF_URL_UPDATE_STATUS);
}
static const gchar *twitter_option_url_new_dm(PurpleAccount *account)
{
	return twitter_api_create_url(account,
			TWITTER_PREF_URL_NEW_DM);
}
static const gchar *twitter_option_url_get_saved_searches(PurpleAccount *account)
{
	return twitter_api_create_url(account,
			TWITTER_PREF_URL_GET_SAVED_SEARCHES);
}
static const gchar *twitter_option_url_get_lists(PurpleAccount *account)
{
	return twitter_api_create_url(account,
			TWITTER_PREF_URL_GET_LISTS);
}
static const gchar *twitter_option_url_verify_credentials(PurpleAccount *account)
{
	return twitter_api_create_url(account,
			TWITTER_PREF_URL_VERIFY_CREDENTIALS);
}
static const gchar *twitter_option_url_rt(PurpleAccount *account, long long id)
{
	gchar *url = g_strdup_printf("%s/%lld.xml",
			TWITTER_PREF_URL_RT,
			id);
	const gchar *result = twitter_api_create_url(account,
			url);
	g_free(url);
	return result;
}

static const gchar *twitter_option_url_report_spam(PurpleAccount *account)
{
	return twitter_api_create_url(account,
			TWITTER_PREF_URL_REPORT_SPAMMER);
}

static const gchar *twitter_option_url_get_status(PurpleAccount *account, long long id)
{
	gchar *url = g_strdup_printf("%s/%lld.xml",
			TWITTER_PREF_URL_GET_STATUS,
			id);
	const gchar *result = twitter_api_create_url(account,
			url);
	g_free(url);
	return result;
}

static const gchar *twitter_option_url_delete_status(PurpleAccount *account, long long id)
{
	gchar *url = g_strdup_printf("%s/%lld.xml",
			TWITTER_PREF_URL_DELETE_STATUS,
			id);
	const gchar *result = twitter_api_create_url(account, url);
	g_free(url);
	return result;
}

static const gchar *twitter_option_url_add_favorite(PurpleAccount *account, long long id)
{
	gchar *url = g_strdup_printf("%s/%lld.xml",
			TWITTER_PREF_URL_ADD_FAVORITE,
			id);
	const gchar *result = twitter_api_create_url(account, url);
	g_free(url);
	return result;
}

static const gchar *twitter_option_url_delete_favorite(PurpleAccount *account, long long id)
{
	gchar *url = g_strdup_printf("%s/%lld.xml",
			TWITTER_PREF_URL_DELETE_FAVORITE,
			id);
	const gchar *result = twitter_api_create_url(account, url);
	g_free(url);
	return result;
}

void twitter_api_get_rate_limit_status(TwitterRequestor *r,
		TwitterSendXmlRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data)
{
	twitter_send_xml_request(r, FALSE,
			twitter_option_url_get_rate_limit_status(r->account), NULL,
			success_func, error_func, data);
}

void twitter_api_web_open_favorites(PurplePluginAction *action)
{
	PurpleConnection *gc = (PurpleConnection *)action->context;
	PurpleAccount *account = purple_connection_get_account(gc);
	const gchar *url = twitter_api_create_web_url(account, TWITTER_PREF_URL_OPEN_FAVORITES);
	purple_debug_info(TWITTER_PROTOCOL_ID, "MHM: Opening link %s\n", url);
	purple_notify_uri(NULL, url);
}

void twitter_api_web_open_replies(PurplePluginAction *action)
{
	PurpleConnection *gc = (PurpleConnection *)action->context;
	PurpleAccount *account = purple_connection_get_account(gc);
	const gchar *url = twitter_api_create_web_url(account, TWITTER_PREF_URL_OPEN_REPLIES);
	purple_notify_uri(NULL, url);
}

void twitter_api_web_open_suggested_friends(PurplePluginAction *action)
{
	PurpleConnection *gc = (PurpleConnection *)action->context;
	PurpleAccount *account = purple_connection_get_account(gc);
	const gchar *url = twitter_api_create_web_url(account, TWITTER_PREF_URL_OPEN_SUGGESTED_FRIENDS);
	purple_notify_uri(NULL, url);
}

void twitter_api_web_open_retweets_of_mine(PurplePluginAction *action)
{
	PurpleConnection *gc = (PurpleConnection *)action->context;
	PurpleAccount *account = purple_connection_get_account(gc);
	const gchar *url = twitter_api_create_web_url(account, TWITTER_PREF_URL_OPEN_RETWEETED_OF_MINE);
	purple_notify_uri(NULL, url);
}


void twitter_api_get_friends(TwitterRequestor *r,
		TwitterSendRequestMultiPageAllSuccessFunc success_func,
		TwitterSendRequestMultiPageAllErrorFunc error_func,
		gpointer data)
{
	purple_debug_info (TWITTER_PROTOCOL_ID, "%s\n", G_STRFUNC);

	twitter_send_xml_request_with_cursor(r,
			twitter_option_url_get_friends(r->account), NULL, -1,
			success_func, error_func, data);
}

static void twitter_api_send_request_single(TwitterRequestor *r,
	const gchar *url,
	long long since_id,
	int count,
	int page,
	TwitterSendXmlRequestSuccessFunc success_func,
	TwitterSendRequestErrorFunc error_func,
	gpointer data)
{
	TwitterRequestParams *params = twitter_request_params_new();
	twitter_request_params_add(params, twitter_request_param_new_int("count", count));
	/* Timelines use count. Lists use per_page. But twitter seems to accept both w/o complaining */
	twitter_request_params_add(params, twitter_request_param_new_int("per_page", count));
	twitter_request_params_add(params, twitter_request_param_new_int("page", page));
	if (since_id > 0)
		twitter_request_params_add(params, twitter_request_param_new_int("since_id", since_id));

	purple_debug_info (TWITTER_PROTOCOL_ID, "%s\n", G_STRFUNC);

	twitter_send_xml_request(r, FALSE,
			url, params,
			success_func, error_func, data);

	twitter_request_params_free(params);
}

void twitter_api_get_home_timeline(TwitterRequestor *r,
		long long since_id,
		int count,
		int page,
		TwitterSendXmlRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data)
{
	twitter_api_send_request_single(r,
		twitter_option_url_get_home_timeline(r->account),
		since_id,
		count,
		page,
		success_func,
		error_func,
		data);
}

void twitter_api_get_list(TwitterRequestor *r,
		const gchar * list_id,
		long long since_id,
		int count,
		int page,
		TwitterSendXmlRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data)
{
	gchar *url;
	url = g_strdup_printf("%s%s%s",twitter_option_url_get_list(r->account), list_id, "/statuses.xml");

	twitter_api_send_request_single(r,
		url,
		since_id,
		count,
		page,
		success_func,
		error_func,
		data);

	g_free(url);
}



static void twitter_api_get_all_since(TwitterRequestor *r,
		const gchar *url,
		long long since_id,
		TwitterSendRequestMultiPageAllSuccessFunc success_func,
		TwitterSendRequestMultiPageAllErrorFunc error_func,
		gint count,
		gint max_count,
		gpointer data)
{
	TwitterRequestParams *params = twitter_request_params_new();
	if (since_id > 0)
		twitter_request_params_add(params, twitter_request_param_new_ll("since_id", since_id));

	purple_debug_info (TWITTER_PROTOCOL_ID, "%s\n", G_STRFUNC);

	twitter_send_xml_request_multipage_all(r,
			url, params,
			success_func, error_func,
			count, max_count, data);
	twitter_request_params_free(params);
}
void twitter_api_get_home_timeline_all(TwitterRequestor *r,
		long long since_id,
		TwitterSendRequestMultiPageAllSuccessFunc success_func,
		TwitterSendRequestMultiPageAllErrorFunc error_func,
		gint max_count,
		gpointer data)
{
	twitter_api_get_all_since(r,
			twitter_option_url_get_home_timeline(r->account),
			since_id,
			success_func,
			error_func,
			TWITTER_HOME_TIMELINE_PAGE_COUNT,
			max_count,
			data);
}

void twitter_api_get_list_all(TwitterRequestor *r,
		const gchar * list_id,
		long long since_id,
		TwitterSendRequestMultiPageAllSuccessFunc success_func,
		TwitterSendRequestMultiPageAllErrorFunc error_func,
		gint max_count,
		gpointer data)
{
	gchar *url;
	url = g_strdup_printf("%s%s%s",twitter_option_url_get_list(r->account), list_id, "/statuses.xml");

	twitter_api_get_all_since(r,
			url,
			since_id,
			success_func,
			error_func,
			TWITTER_LIST_PAGE_COUNT,
			max_count,
			data);

	g_free(url);
}
void twitter_api_get_replies(TwitterRequestor *r,
		long long since_id,
		int count,
		int page,
		TwitterSendXmlRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data)
{
	twitter_api_send_request_single(r,
		twitter_option_url_get_mentions(r->account),
		since_id,
		count,
		page,
		success_func,
		error_func,
		data);
}

void twitter_api_get_replies_all(TwitterRequestor *r,
		long long since_id,
		TwitterSendRequestMultiPageAllSuccessFunc success_func,
		TwitterSendRequestMultiPageAllErrorFunc error_func,
		gint max_count,
		gpointer data)
{
	twitter_api_get_all_since(r,
			twitter_option_url_get_mentions(r->account),
			since_id,
			success_func,
			error_func,
			TWITTER_EVERY_REPLIES_COUNT,
			max_count,
			data);
}

void twitter_api_get_dms(TwitterRequestor *r,
		long long since_id,
		int count,
		int page,
		TwitterSendXmlRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data)
{
	twitter_api_send_request_single(r,
			twitter_option_url_get_dms(r->account),
			since_id,
			count,
			page,
			success_func,
			error_func,
			data);
}

void twitter_api_get_dms_all(TwitterRequestor *r,
		long long since_id,
		TwitterSendRequestMultiPageAllSuccessFunc success_func,
		TwitterSendRequestMultiPageAllErrorFunc error_func,
		gint max_count,
		gpointer data)
{
	twitter_api_get_all_since(r,
			twitter_option_url_get_dms(r->account),
			since_id,
			success_func,
			error_func,
			TWITTER_EVERY_DMS_COUNT,
			max_count,
			data);
}

void twitter_api_set_status(TwitterRequestor *r,
		const char *msg,
		long long in_reply_to_status_id,
		TwitterSendXmlRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data)
{
	TwitterRequestParams *params;
	g_return_if_fail(msg != NULL && msg[0] != '\0');

	params = twitter_request_params_new();
	twitter_request_params_add(params, twitter_request_param_new("status", msg));
	if (in_reply_to_status_id)
		twitter_request_params_add(params, twitter_request_param_new_ll("in_reply_to_status_id", in_reply_to_status_id));
	twitter_send_xml_request(r, TRUE,
			twitter_option_url_update_status(r->account), params,
			success_func, error_func, data);
	twitter_request_params_free(params);
}

typedef struct
{
	GArray *statuses;
	TwitterApiMultiStatusSuccessFunc success_func;
	TwitterApiMultiStatusErrorFunc error_func;
	gpointer user_data;
	int statuses_index;

	//set status only
	long long in_reply_to_status_id;

	//dm only
	gchar *dm_who;
} TwitterMultiMessageContext;

static void twitter_api_set_statuses_success_cb(TwitterRequestor *r, xmlnode *node, gpointer _ctx);
static void twitter_api_set_statuses_error_cb(TwitterRequestor *r, const TwitterRequestErrorData *error_data, gpointer _ctx)
{
	TwitterMultiMessageContext *ctx = _ctx;

	if (ctx->error_func && !ctx->error_func(r->account, error_data, ctx->user_data))
	{
		//TODO: verify this doesn't leak
		g_array_free(ctx->statuses, TRUE);
		g_free(ctx);
		return;
	}
	//Try again
	twitter_api_set_status(r,
			g_array_index(ctx->statuses, gchar*, ctx->statuses_index),
			ctx->in_reply_to_status_id,
			twitter_api_set_statuses_success_cb,
			twitter_api_set_statuses_error_cb,
			ctx);
}

static void twitter_api_set_statuses_success_cb(TwitterRequestor *r, xmlnode *node, gpointer _ctx)
{
	TwitterMultiMessageContext *ctx = _ctx;
	gboolean last = FALSE;

	if (++ctx->statuses_index >= ctx->statuses->len)
	{
		last = TRUE;
		//TODO: verify this doesn't leak
		g_array_free(ctx->statuses, TRUE);
		g_free(ctx);
		return;
	}

	if (ctx->success_func)
		ctx->success_func(r->account, node, last, ctx->user_data);

	if (last)
		return;

	twitter_api_set_status(r,
			g_array_index(ctx->statuses, gchar*, ctx->statuses_index),
			ctx->in_reply_to_status_id,
			twitter_api_set_statuses_success_cb,
			twitter_api_set_statuses_error_cb,
			ctx);
}

void twitter_api_set_statuses(TwitterRequestor *r,
		GArray *statuses,
		long long in_reply_to_status_id,
		TwitterApiMultiStatusSuccessFunc success_func,
		TwitterApiMultiStatusErrorFunc error_func,
		gpointer data)
{
	TwitterMultiMessageContext *ctx;
	g_return_if_fail(statuses && statuses->len);
	ctx = g_new0(TwitterMultiMessageContext, 1);
	ctx->statuses = statuses;
	ctx->in_reply_to_status_id = in_reply_to_status_id;
	ctx->success_func = success_func;
	ctx->error_func = error_func;
	ctx->user_data = data;
	ctx->statuses_index = 0;

	twitter_api_set_status(r,
			g_array_index(statuses, gchar*, 0),
			in_reply_to_status_id,
			twitter_api_set_statuses_success_cb,
			twitter_api_set_statuses_error_cb,
			ctx);
}

void twitter_api_send_dm(TwitterRequestor *r,
		const char *user,
		const char *msg,
		TwitterSendXmlRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data)
{
	TwitterRequestParams *params;
	g_return_if_fail(msg != NULL && user != NULL && msg[0] != '\0' && user[0] != '\0');

	params = twitter_request_params_new();
	twitter_request_params_add(params, twitter_request_param_new("text", msg));
	twitter_request_params_add(params, twitter_request_param_new("user", user));
	twitter_send_xml_request(r, TRUE,
			twitter_option_url_new_dm(r->account), params,
			success_func, error_func, data);
	twitter_request_params_free(params);

}

void twitter_api_send_rt(TwitterRequestor *r,
		long long id,
		TwitterSendXmlRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data)
{
	g_return_if_fail(id > 0);

	twitter_send_xml_request(r, TRUE,
			twitter_option_url_rt(r->account, id), NULL,
			success_func, error_func, data);

}

void twitter_api_add_favorite(TwitterRequestor *r,
		long long id,
		TwitterSendXmlRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data)
{
	g_return_if_fail(id > 0);

	twitter_send_xml_request(r, TRUE,
			twitter_option_url_add_favorite(r->account, id), NULL,
			success_func, error_func, data);

}

void twitter_api_delete_favorite(TwitterRequestor *r,
		long long id,
		TwitterSendXmlRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data)
{
	g_return_if_fail(id > 0);

	twitter_send_xml_request(r, TRUE,
			twitter_option_url_delete_favorite(r->account, id), NULL,
			success_func, error_func, data);

}

void twitter_api_report_spammer(TwitterRequestor *r,
		const gchar *user,
		TwitterSendXmlRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data)
{
	TwitterRequestParams *params;

	g_return_if_fail(user != NULL && user[0] != '\0');

	params = twitter_request_params_new();
	twitter_request_params_add(params, twitter_request_param_new("screen_name", user));
	twitter_send_xml_request(r, TRUE,
			twitter_option_url_report_spam(r->account), params,
			success_func, error_func, data);
	twitter_request_params_free(params);
}

void twitter_api_get_status(TwitterRequestor *r,
		long long id,
		TwitterSendXmlRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data)
{
	g_return_if_fail(id > 0);

	twitter_send_xml_request(r, FALSE,
			twitter_option_url_get_status(r->account, id), NULL,
			success_func, error_func, data);
}

void twitter_api_delete_status(TwitterRequestor *r,
		long long id,
		TwitterSendXmlRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data)
{
	g_return_if_fail(id > 0);

	twitter_send_xml_request(r, TRUE,
			twitter_option_url_delete_status(r->account, id), NULL,
			success_func, error_func, data);

}

static void twitter_api_send_dms_success_cb(TwitterRequestor *r, xmlnode *node, gpointer _ctx);
static void twitter_api_send_dms_error_cb(TwitterRequestor *r, const TwitterRequestErrorData *error_data, gpointer _ctx)
{
	TwitterMultiMessageContext *ctx = _ctx;

	purple_debug_info(TWITTER_PROTOCOL_ID, "%s\n", G_STRFUNC);

	if (ctx->error_func && !ctx->error_func(r->account, error_data, ctx->user_data))
	{
		//TODO: verify this doesn't leak
		g_array_free(ctx->statuses, TRUE);
		g_free(ctx->dm_who);
		g_free(ctx);
		return;
	}
	//Try again
	twitter_api_send_dm(r,
			ctx->dm_who,
			g_array_index(ctx->statuses, gchar*, ctx->statuses_index),
			twitter_api_send_dms_success_cb,
			twitter_api_send_dms_error_cb,
			ctx);
}

static void twitter_api_send_dms_success_cb(TwitterRequestor *r, xmlnode *node, gpointer _ctx)
{
	TwitterMultiMessageContext *ctx = _ctx;
	gboolean last = FALSE;

	purple_debug_info(TWITTER_PROTOCOL_ID, "%s\n", G_STRFUNC);

	if (++ctx->statuses_index >= ctx->statuses->len)
	{
		//TODO: verify this doesn't leak
		g_array_free(ctx->statuses, TRUE);
		g_free(ctx->dm_who);
		g_free(ctx);
		last = TRUE;
		return;
	}

	if (ctx->success_func)
		ctx->success_func(r->account, node, last, ctx->user_data);

	if (last)
		return;

	twitter_api_send_dm(r,
			ctx->dm_who,
			g_array_index(ctx->statuses, gchar*, ctx->statuses_index),
			twitter_api_send_dms_success_cb,
			twitter_api_send_dms_error_cb,
			ctx);
}

void twitter_api_send_dms(TwitterRequestor *r,
		const gchar *who,
		GArray *statuses,
		TwitterApiMultiStatusSuccessFunc success_func,
		TwitterApiMultiStatusErrorFunc error_func,
		gpointer data)
{
	TwitterMultiMessageContext *ctx;

	purple_debug_info(TWITTER_PROTOCOL_ID, "%s\n", G_STRFUNC);
	g_return_if_fail(statuses && statuses->len);

	ctx = g_new0(TwitterMultiMessageContext, 1);
	ctx->statuses = statuses;
	ctx->success_func = success_func;
	ctx->error_func = error_func;
	ctx->user_data = data;
	ctx->statuses_index = 0;
	ctx->dm_who = g_strdup(who);

	twitter_api_send_dm(r,
			ctx->dm_who,
			g_array_index(ctx->statuses, gchar*, ctx->statuses_index),
			twitter_api_send_dms_success_cb,
			twitter_api_send_dms_error_cb,
			ctx);
}
void twitter_api_get_lists(TwitterRequestor *r,
		TwitterSendXmlRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data)
{
	const gchar *url = twitter_option_url_get_lists(r->account);
	purple_debug_info (TWITTER_PROTOCOL_ID, "%s\n", G_STRFUNC);

	if (url && url[0] != '\0')
	{
		twitter_send_xml_request(r, FALSE,
				url, NULL,
				success_func, error_func, data);
	}
}

void twitter_api_get_saved_searches(TwitterRequestor *r,
		TwitterSendXmlRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data)
{
	const gchar *url = twitter_option_url_get_saved_searches(r->account);
	purple_debug_info (TWITTER_PROTOCOL_ID, "%s\n", G_STRFUNC);

	if (url && url[0] != '\0')
	{
		twitter_send_xml_request(r, FALSE,
				url, NULL,
				success_func, error_func, data);
	}
}

void twitter_api_search(TwitterRequestor *r,
		const char *keyword,
		long long since_id,
		guint rpp,
		TwitterSearchSuccessFunc success_func,
		TwitterSearchErrorFunc error_func,
		gpointer data)
{
	/* http://search.twitter.com/search.atom + query (e.g. ?q=n900) */
	TwitterRequestParams *params = twitter_request_params_new();
	twitter_request_params_add(params, twitter_request_param_new("q", keyword));
	twitter_request_params_add(params, twitter_request_param_new_int("rpp", rpp));
	if (since_id > 0)
		twitter_request_params_add(params, twitter_request_param_new_ll("since_id", since_id));

	twitter_search(r, params, success_func, error_func, data);
	twitter_request_params_free(params);
}

void twitter_api_search_refresh(TwitterRequestor *r,
		const char *refresh_url, //this is already encoded
		TwitterSearchSuccessFunc success_func,
		TwitterSearchErrorFunc error_func,
		gpointer data)
{
	//Convert the refresh url to a params object
	TwitterRequestParams *params = twitter_request_params_new();
	gchar **pieces = g_strsplit(refresh_url+1, "&", 0);
	gchar **p;
	for (p = pieces; *p; p++)
	{
		gchar *equal = strchr(*p, '=');
		if (equal)
		{
			equal[0] = '\0';
			twitter_request_params_add(params, twitter_request_param_new(*p, purple_url_decode(equal+1)));
		}
	}
	g_strfreev(pieces);
	twitter_search(r, params, success_func, error_func, data);
	twitter_request_params_free(params);
}

void twitter_api_oauth_request_token(TwitterRequestor *r,
		TwitterSendRequestSuccessFunc success_cb,
		TwitterSendRequestErrorFunc error_cb,
		gpointer user_data)
{
	twitter_send_request(r,
			FALSE,
			"twitter.com/oauth/request_token",
			NULL,
			success_cb,
			error_cb,
			user_data);
}

void twitter_api_oauth_access_token(TwitterRequestor *r,
		const gchar *oauth_verifier,
		TwitterSendRequestSuccessFunc success_cb,
		TwitterSendRequestErrorFunc error_cb,
		gpointer user_data)
{
	TwitterRequestParams *params = twitter_request_params_new();
	twitter_request_params_add(params, twitter_request_param_new("oauth_verifier", oauth_verifier));
	twitter_send_request(r,
			FALSE,
			"twitter.com/oauth/access_token",
			params,
			success_cb,
			error_cb,
			user_data);
	twitter_request_params_free(params);
}

void twitter_api_verify_credentials(TwitterRequestor *r,
		TwitterSendXmlRequestSuccessFunc success_cb,
		TwitterSendRequestErrorFunc error_cb,
		gpointer user_data)
{
	twitter_send_xml_request(r,
			FALSE,
			twitter_option_url_verify_credentials(r->account),
			NULL,
			success_cb,
			error_cb,
			user_data);
}
