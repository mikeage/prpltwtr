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
#include "twitter_api.h"

void twitter_api_get_rate_limit_status(PurpleAccount *account,
		TwitterSendRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data)
{
	twitter_send_request(account, FALSE,
			twitter_option_url_get_rate_limit_status(account), NULL,
			success_func, error_func, data);
}
void twitter_api_get_friends(PurpleAccount *account,
		TwitterSendRequestMultiPageAllSuccessFunc success_func,
		TwitterSendRequestMultiPageAllErrorFunc error_func,
		gpointer data)
{
	purple_debug_info (TWITTER_PROTOCOL_ID, "%s\n", G_STRFUNC);

	twitter_send_request_with_cursor (account,
			twitter_option_url_get_friends(account), NULL, -1,
			success_func, error_func, data);
}

static void twitter_api_send_request_single(PurpleAccount *account,
	const gchar *url,
	long long since_id,
	int count,
	int page,
	TwitterSendRequestSuccessFunc success_func,
	TwitterSendRequestErrorFunc error_func,
	gpointer data)
{
	TwitterRequestParams *params = twitter_request_params_new();
	twitter_request_params_add(params, twitter_request_param_new_int("count", count));
	twitter_request_params_add(params, twitter_request_param_new_int("page", page));
	if (since_id > 0)
		twitter_request_params_add(params, twitter_request_param_new_int("since_id", since_id));

	purple_debug_info (TWITTER_PROTOCOL_ID, "%s\n", G_STRFUNC);

	twitter_send_request(account, FALSE,
			url, params,
			success_func, error_func, data);

	twitter_request_params_free(params);
}

void twitter_api_get_home_timeline(PurpleAccount *account,
		long long since_id,
		int count,
		int page,
		TwitterSendRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data)
{
	twitter_api_send_request_single(account,
		twitter_option_url_get_home_timeline(account),
		since_id,
		count,
		page,
		success_func,
		error_func,
		data);
}

static void twitter_api_get_all_since(PurpleAccount *account,
		const gchar *url,
		long long since_id,
		TwitterSendRequestMultiPageAllSuccessFunc success_func,
		TwitterSendRequestMultiPageAllErrorFunc error_func,
		gint count_per_page,
		gint max_count,
		gpointer data)
{
	TwitterRequestParams *params = twitter_request_params_new();
	if (since_id > 0)
		twitter_request_params_add(params, twitter_request_param_new_ll("since_id", since_id));

	purple_debug_info (TWITTER_PROTOCOL_ID, "%s\n", G_STRFUNC);

	twitter_send_request_multipage_all(account,
			url, params,
			success_func, error_func,
			count_per_page, max_count, data);
	twitter_request_params_free(params);
}

void twitter_api_get_home_timeline_all(PurpleAccount *account,
		long long since_id,
		TwitterSendRequestMultiPageAllSuccessFunc success_func,
		TwitterSendRequestMultiPageAllErrorFunc error_func,
		gint max_count,
		gpointer data)
{
	twitter_api_get_all_since(account,
			twitter_option_url_get_home_timeline(account),
			since_id,
			success_func,
			error_func,
			TWITTER_HOME_TIMELINE_PAGE_COUNT,
			max_count,
			data);
}
void twitter_api_get_replies(PurpleAccount *account,
		long long since_id,
		int count,
		int page,
		TwitterSendRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data)
{
	twitter_api_send_request_single(account,
		twitter_option_url_get_mentions(account),
		since_id,
		count,
		page,
		success_func,
		error_func,
		data);
}

void twitter_api_get_replies_all(PurpleAccount *account,
		long long since_id,
		TwitterSendRequestMultiPageAllSuccessFunc success_func,
		TwitterSendRequestMultiPageAllErrorFunc error_func,
		gint max_count,
		gpointer data)
{
	twitter_api_get_all_since(account,
			twitter_option_url_get_mentions(account),
			since_id,
			success_func,
			error_func,
			TWITTER_EVERY_REPLIES_COUNT,
			max_count,
			data);
}

void twitter_api_get_dms(PurpleAccount *account,
		long long since_id,
		int count,
		int page,
		TwitterSendRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data)
{
	twitter_api_send_request_single(account,
			twitter_option_url_get_dms(account),
			since_id,
			count,
			page,
			success_func,
			error_func,
			data);
}

void twitter_api_get_dms_all(PurpleAccount *account,
		long long since_id,
		TwitterSendRequestMultiPageAllSuccessFunc success_func,
		TwitterSendRequestMultiPageAllErrorFunc error_func,
		gint max_count,
		gpointer data)
{
	twitter_api_get_all_since(account,
			twitter_option_url_get_dms(account),
			since_id,
			success_func,
			error_func,
			TWITTER_EVERY_DMS_COUNT,
			max_count,
			data);
}

void twitter_api_set_status(PurpleAccount *account,
		const char *msg,
		long long in_reply_to_status_id,
		TwitterSendRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data)
{
	TwitterRequestParams *params;
	g_return_if_fail(msg != NULL && msg[0] != '\0');

	params = twitter_request_params_new();
	twitter_request_params_add(params, twitter_request_param_new("status", msg));
	if (in_reply_to_status_id)
		twitter_request_params_add(params, twitter_request_param_new_ll("in_reply_to_status_id", in_reply_to_status_id));
	twitter_send_request(account, TRUE,
			twitter_option_url_update_status(account), params,
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

static void twitter_api_set_statuses_success_cb(PurpleAccount *account, xmlnode *node, gpointer _ctx);
static void twitter_api_set_statuses_error_cb(PurpleAccount *account, const TwitterRequestErrorData *error_data, gpointer _ctx)
{
	TwitterMultiMessageContext *ctx = _ctx;

	if (ctx->error_func && !ctx->error_func(account, error_data, ctx->user_data))
	{
		//TODO: verify this doesn't leak
		g_array_free(ctx->statuses, TRUE);
		g_free(ctx);
		return;
	}
	//Try again
	twitter_api_set_status(account,
			g_array_index(ctx->statuses, gchar*, ctx->statuses_index),
			ctx->in_reply_to_status_id,
			twitter_api_set_statuses_success_cb,
			twitter_api_set_statuses_error_cb,
			ctx);
}

static void twitter_api_set_statuses_success_cb(PurpleAccount *account, xmlnode *node, gpointer _ctx)
{
	TwitterMultiMessageContext *ctx = _ctx;
	gboolean last = FALSE;

	if (++ctx->statuses_index >= ctx->statuses->len)
	{
		last = TRUE;
		//TODO: verify this doesn't leak
		g_array_free(ctx->statuses, TRUE);
		g_free(ctx);
	}

	if (ctx->success_func)
		ctx->success_func(account, node, last, ctx->user_data);

	if (last)
		return;

	twitter_api_set_status(account,
			g_array_index(ctx->statuses, gchar*, ctx->statuses_index),
			ctx->in_reply_to_status_id,
			twitter_api_set_statuses_success_cb,
			twitter_api_set_statuses_error_cb,
			ctx);
}

void twitter_api_set_statuses(PurpleAccount *account,
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

	twitter_api_set_status(account,
			g_array_index(statuses, gchar*, 0),
			in_reply_to_status_id,
			twitter_api_set_statuses_success_cb,
			twitter_api_set_statuses_error_cb,
			ctx);
}

void twitter_api_send_dm(PurpleAccount *account,
		const char *user,
		const char *msg,
		TwitterSendRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data)
{
	TwitterRequestParams *params;
	g_return_if_fail(msg != NULL && user != NULL && msg[0] != '\0' && user[0] != '\0');

	params = twitter_request_params_new();
	twitter_request_params_add(params, twitter_request_param_new("text", msg));
	twitter_request_params_add(params, twitter_request_param_new("user", user));
	twitter_send_request(account, TRUE,
			twitter_option_url_new_dm(account), params,
			success_func, error_func, data);
	twitter_request_params_free(params);

}

static void twitter_api_send_dms_success_cb(PurpleAccount *account, xmlnode *node, gpointer _ctx);
static void twitter_api_send_dms_error_cb(PurpleAccount *account, const TwitterRequestErrorData *error_data, gpointer _ctx)
{
	TwitterMultiMessageContext *ctx = _ctx;

	purple_debug_info(TWITTER_PROTOCOL_ID, "%s\n", G_STRFUNC);

	if (ctx->error_func && !ctx->error_func(account, error_data, ctx->user_data))
	{
		//TODO: verify this doesn't leak
		g_array_free(ctx->statuses, TRUE);
		g_free(ctx->dm_who);
		g_free(ctx);
		return;
	}
	//Try again
	twitter_api_send_dm(account,
			ctx->dm_who,
			g_array_index(ctx->statuses, gchar*, ctx->statuses_index),
			twitter_api_send_dms_success_cb,
			twitter_api_send_dms_error_cb,
			ctx);
}

static void twitter_api_send_dms_success_cb(PurpleAccount *account, xmlnode *node, gpointer _ctx)
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
	}

	if (ctx->success_func)
		ctx->success_func(account, node, last, ctx->user_data);

	if (last)
		return;

	twitter_api_send_dm(account,
			ctx->dm_who,
			g_array_index(ctx->statuses, gchar*, ctx->statuses_index),
			twitter_api_send_dms_success_cb,
			twitter_api_send_dms_error_cb,
			ctx);
}

void twitter_api_send_dms(PurpleAccount *account,
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

	twitter_api_send_dm(account,
			ctx->dm_who,
			g_array_index(ctx->statuses, gchar*, ctx->statuses_index),
			twitter_api_send_dms_success_cb,
			twitter_api_send_dms_error_cb,
			ctx);
}

void twitter_api_get_saved_searches (PurpleAccount *account,
		TwitterSendRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data)
{
	const gchar *url = twitter_option_url_get_saved_searches(account);
	purple_debug_info (TWITTER_PROTOCOL_ID, "%s\n", G_STRFUNC);

	if (url && url[0] != '\0')
	{
		twitter_send_request(account, FALSE,
				url, NULL,
				success_func, error_func, data);
	}
}

void twitter_api_search (PurpleAccount *account,
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

	twitter_search(account, params, success_func, error_func, data);
	twitter_request_params_free(params);
}

void twitter_api_search_refresh(PurpleAccount *account,
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
	twitter_search(account, params, success_func, error_func, data);
	twitter_request_params_free(params);
}
