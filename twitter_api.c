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

#include <account.h>
#include <accountopt.h>
#include <blist.h>
#include <cmds.h>
#include <conversation.h>
#include <connection.h>
#include <debug.h>
#include <notify.h>
#include <privacy.h>
#include <prpl.h>
#include <roomlist.h>
#include <status.h>
#include <util.h>
#include <version.h>
#include <cipher.h>
#include <request.h>

#include "twitter_request.h"
#include "twitter_search.h"
#include "twitter_api.h"
static const char *twitter_api_get_pref_host_url(PurpleAccount *account)
{
	return purple_account_get_string(account,
			TWITTER_PREF_HOST_URL, TWITTER_PREF_HOST_URL_DEFAULT);
}

static const char *twitter_api_get_pref_host_api_url(PurpleAccount *account)
{
	return purple_account_get_string(account,
			TWITTER_PREF_HOST_API_URL, TWITTER_PREF_HOST_API_URL_DEFAULT);
}

void twitter_api_get_rate_limit_status(PurpleAccount *account,
		TwitterSendRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data)
{
	twitter_send_request(account, FALSE,
			twitter_api_get_pref_host_url(account),
			"/account/rate_limit_status.xml", NULL,
			success_func, error_func, data);
}
void twitter_api_get_friends(PurpleAccount *account,
		TwitterSendRequestMultiPageAllSuccessFunc success_func,
		TwitterSendRequestMultiPageAllErrorFunc error_func,
		gpointer data)
{
	purple_debug_info (TWITTER_PROTOCOL_ID, "%s\n", G_STRFUNC);

	twitter_send_request_with_cursor (account,
			twitter_api_get_pref_host_url(account),
			"/statuses/friends.xml", NULL, -1,
			success_func, error_func, data);
}

void twitter_api_get_home_timeline(PurpleAccount *account,
		long long since_id,
		int count,
		int page,
		TwitterSendRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data)
{
	char *query = since_id ?
		g_strdup_printf("count=%d&page=%d&since_id=%lld", count, page, since_id) :
		g_strdup_printf("count=%d&page=%d", count, page);

	purple_debug_info (TWITTER_PROTOCOL_ID, "%s\n", G_STRFUNC);

	twitter_send_request(account, FALSE,
			twitter_api_get_pref_host_api_url(account),
			"/1/statuses/home_timeline.xml", query,
			success_func, error_func, data);

	g_free(query);
}

void twitter_api_get_home_timeline_all(PurpleAccount *account,
		long long since_id,
		TwitterSendRequestMultiPageAllSuccessFunc success_func,
		TwitterSendRequestMultiPageAllErrorFunc error_func,
		gpointer data)
{
	int count = TWITTER_EVERY_REPLIES_COUNT; //TODO set this its own pref
	char *query = since_id ?
		g_strdup_printf ("since_id=%lld&count=%d", since_id, count) :
		g_strdup_printf ("count=%d", count);

	purple_debug_info (TWITTER_PROTOCOL_ID, "%s\n", G_STRFUNC);

	twitter_send_request_multipage_all(account,
			twitter_api_get_pref_host_api_url(account),
			"/1/statuses/home_timeline.xml", query,
			success_func, error_func,
			count, data);
	g_free(query);
}
void twitter_api_get_replies(PurpleAccount *account,
		long long since_id,
		int count,
		int page,
		TwitterSendRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data)
{
	char *query = since_id ?
		g_strdup_printf("count=%d&page=%d&since_id=%lld", count, page, since_id) :
		g_strdup_printf("count=%d&page=%d", count, page);

	purple_debug_info (TWITTER_PROTOCOL_ID, "%s\n", G_STRFUNC);

	twitter_send_request(account, FALSE,
			twitter_api_get_pref_host_url(account),
			"/statuses/mentions.xml", query,
			success_func, error_func, data);

	g_free(query);
}

void twitter_api_get_replies_all(PurpleAccount *account,
		long long since_id,
		TwitterSendRequestMultiPageAllSuccessFunc success_func,
		TwitterSendRequestMultiPageAllErrorFunc error_func,
		gpointer data)
{
	int count = TWITTER_EVERY_REPLIES_COUNT;
	char *query = since_id ?
		g_strdup_printf ("since_id=%lld&count=%d", since_id, count) :
		g_strdup_printf ("count=%d", count);

	purple_debug_info (TWITTER_PROTOCOL_ID, "%s\n", G_STRFUNC);

	twitter_send_request_multipage_all(account,
			twitter_api_get_pref_host_url(account),
			"/statuses/mentions.xml", query,
			success_func, error_func,
			count, data);
	g_free(query);
}

void twitter_api_set_status(PurpleAccount *account,
		const char *msg,
		long long in_reply_to_status_id,
		TwitterSendRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data)
{
	if (msg != NULL && strcmp("", msg))
	{
		char *query = in_reply_to_status_id ?
			g_strdup_printf ("status=%s&in_reply_to_status_id=%lld",
					purple_url_encode(msg), in_reply_to_status_id) :
			g_strdup_printf("status=%s", purple_url_encode(msg));
		twitter_send_request(account, TRUE,
				twitter_api_get_pref_host_url(account),
				"/statuses/update.xml", query,
				success_func, NULL, data);
		g_free(query);
	} else {
		//SEND error?
	}
}

void twitter_api_send_dm(PurpleAccount *account,
		const char *user,
		const char *msg,
		TwitterSendRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data)
{
	if (msg != NULL && strcmp("", msg) && user != NULL && strcmp("", user))
	{
		char *user_encoded = g_strdup(purple_url_encode(user));
		char *query = g_strdup_printf ("text=%s&user=%s",
				purple_url_encode(msg), user_encoded);
		twitter_send_request(account, TRUE,
				twitter_api_get_pref_host_url(account),
				"/direct_messages/new.xml", query,
				success_func, NULL, data);
		g_free(user_encoded);
		g_free(query);
	} else {
		//SEND error?
	}
}

void twitter_api_get_saved_searches (PurpleAccount *account,
		TwitterSendRequestSuccessFunc success_func,
		TwitterSendRequestErrorFunc error_func,
		gpointer data)
{
	purple_debug_info (TWITTER_PROTOCOL_ID, "%s\n", G_STRFUNC);

	twitter_send_request (account, FALSE,
			twitter_api_get_pref_host_url(account),
			"/saved_searches.xml", NULL,
			success_func, error_func, data);
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
	char *query = since_id ?
		g_strdup_printf ("?q=%s&rpp=%u&since_id=%lld", purple_url_encode(keyword), rpp, since_id) :
		g_strdup_printf ("?q=%s&rpp=%u", purple_url_encode(keyword), rpp);

	twitter_search (account, query, success_func, error_func, data);
	g_free (query);
}

void twitter_api_search_refresh (PurpleAccount *account,
		const char *refresh_url,
		TwitterSearchSuccessFunc success_func,
		TwitterSearchErrorFunc error_func,
		gpointer data)
{
	twitter_search (account, refresh_url, success_func, error_func, data);
}
