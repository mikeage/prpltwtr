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

#include "twitter_prefs.h"
#include <string.h>
#include <version.h>

GList *twitter_get_protocol_options()
{
	GList *options = NULL;
	PurpleAccountOption *option;

	option = purple_account_option_bool_new(
			_("Enable HTTPS"),      /* text shown to user */
			TWITTER_PREF_USE_HTTPS,                         /* pref name */
			TWITTER_PREF_USE_HTTPS_DEFAULT);                        /* default value */
	options = g_list_append(NULL, option);

	option = purple_account_option_bool_new(
			_("Enable OAuth (more secure, higher rate limit)"),
			TWITTER_PREF_USE_OAUTH,
			TWITTER_PREF_USE_OAUTH_DEFAULT);
	options = g_list_append(options, option);

	/* Default sending im to buddy is to dm */
	option = purple_account_option_bool_new(
			_("Default IM to buddy is a DM"),
			TWITTER_PREF_DEFAULT_DM,
			TWITTER_PREF_DEFAULT_DM_DEFAULT);
	options = g_list_append (options, option);

	/* Retrieve tweets history after login */
	option = purple_account_option_bool_new (
			_("Retrieve tweets history after login"),
			TWITTER_PREF_RETRIEVE_HISTORY,
			TWITTER_PREF_RETRIEVE_HISTORY_DEFAULT);
	options = g_list_append (options, option);

	/* Sync presence update to twitter */
	option = purple_account_option_bool_new (
			_("Sync availability status message to Twitter"),
			TWITTER_PREF_SYNC_STATUS,
			TWITTER_PREF_SYNC_STATUS_DEFAULT);
	options = g_list_append (options, option);

	/* Automatically generate a buddylist based on followers */
	option = purple_account_option_bool_new (
			_("Add following as friends (NOT recommended for large follower list)"),
			TWITTER_PREF_GET_FRIENDS,
			TWITTER_PREF_GET_FRIENDS_DEFAULT);
	options = g_list_append (options, option);

	/* Add URL link to each tweet */
	option = purple_account_option_bool_new (
			_("Add URL link to each tweet"),
			TWITTER_PREF_ADD_URL_TO_TWEET,
			TWITTER_PREF_ADD_URL_TO_TWEET_DEFAULT);
	options = g_list_append (options, option);

	/* Idle cutoff time */
	option = purple_account_option_int_new(
			_("Buddy last tweet hours before set offline (0: Always online)"),      /* text shown to user */
			TWITTER_ONLINE_CUTOFF_TIME_HOURS,                         /* pref name */
			TWITTER_ONLINE_CUTOFF_TIME_HOURS_DEFAULT);                        /* default value */
	options = g_list_append(options, option);


	/* Max tweets to retrieve when retrieving timeline data */
	option = purple_account_option_int_new(
			_("Max historical timeline tweets to retrieve (0: infinite)"),      /* text shown to user */
			TWITTER_PREF_HOME_TIMELINE_MAX_TWEETS,                         /* pref name */
			TWITTER_PREF_HOME_TIMELINE_MAX_TWEETS_DEFAULT);                        /* default value */
	options = g_list_append(options, option);

	/* Timeline tweets refresh interval */
	option = purple_account_option_int_new(
			_("Refresh timeline every (min)"),      /* text shown to user */
			TWITTER_PREF_TIMELINE_TIMEOUT,                         /* pref name */
			TWITTER_PREF_TIMELINE_TIMEOUT_DEFAULT);                        /* default value */
	options = g_list_append(options, option);

	/* Mentions/replies tweets refresh interval */
	option = purple_account_option_int_new(
			_("Refresh replies every (min)"),      /* text shown to user */
			TWITTER_PREF_REPLIES_TIMEOUT,                         /* pref name */
			TWITTER_PREF_REPLIES_TIMEOUT_DEFAULT);                        /* default value */
	options = g_list_append(options, option);

	/* Lists tweets refresh interval */
	option = purple_account_option_int_new(
			_("Refresh lists every (min)"),      /* text shown to user */
			TWITTER_PREF_LIST_TIMEOUT,                         /* pref name */
			TWITTER_PREF_LIST_TIMEOUT_DEFAULT);                        /* default value */
	options = g_list_append(options, option);

	/* Dms refresh interval */
	option = purple_account_option_int_new(
			_("Refresh direct messages every (min)"),      /* text shown to user */
			TWITTER_PREF_DMS_TIMEOUT,                         /* pref name */
			TWITTER_PREF_DMS_TIMEOUT_DEFAULT);                        /* default value */
	options = g_list_append(options, option);

	/* Friendlist refresh interval */
	option = purple_account_option_int_new(
			_("Refresh friendlist every (min)"),      /* text shown to user */
			TWITTER_PREF_USER_STATUS_TIMEOUT,                         /* pref name */
			TWITTER_PREF_USER_STATUS_TIMEOUT_DEFAULT);                        /* default value */
	options = g_list_append(options, option);

	/* Search results refresh interval */
	option = purple_account_option_int_new(
			_("Default refresh search results every (min)"),      /* text shown to user */
			TWITTER_PREF_SEARCH_TIMEOUT,                         /* pref name */
			TWITTER_PREF_SEARCH_TIMEOUT_DEFAULT);                        /* default value */
	options = g_list_append(options, option);

	option = purple_account_option_string_new(
			_("API Base URL"),      /* text shown to user */
			TWITTER_PREF_API_BASE,                         /* pref name */
			TWITTER_PREF_API_BASE_DEFAULT);                        /* default value */
	options = g_list_append(options, option);

	option = purple_account_option_string_new(
			_("Search API Base URL"),      /* text shown to user */
			TWITTER_PREF_SEARCH_API_BASE,                         /* pref name */
			TWITTER_PREF_SEARCH_API_BASE_DEFAULT);                        /* default value */
	options = g_list_append(options, option);


	return options;
}

gboolean twitter_option_add_link_to_tweet(PurpleAccount *account)
{
	return purple_account_get_bool(account,
			TWITTER_PREF_ADD_URL_TO_TWEET,
			TWITTER_PREF_ADD_URL_TO_TWEET_DEFAULT);
}

gint twitter_option_search_timeout(PurpleAccount *account)
{
	return purple_account_get_int(account,
			TWITTER_PREF_SEARCH_TIMEOUT,
			TWITTER_PREF_SEARCH_TIMEOUT_DEFAULT);
}

gint twitter_option_timeline_timeout(PurpleAccount *account)
{
	return purple_account_get_int(account,
			TWITTER_PREF_TIMELINE_TIMEOUT,
			TWITTER_PREF_TIMELINE_TIMEOUT_DEFAULT);
}

gint twitter_option_list_timeout(PurpleAccount *account)
{
	return purple_account_get_int(account,
			TWITTER_PREF_LIST_TIMEOUT,
			TWITTER_PREF_LIST_TIMEOUT_DEFAULT);
}
const gchar *twitter_option_list_group(PurpleAccount *account)
{
	//TODO: create an option for this
	return TWITTER_PREF_DEFAULT_LIST_GROUP;
}
const gchar *twitter_option_search_group(PurpleAccount *account)
{
	//TODO: create an option for this
	return TWITTER_PREF_DEFAULT_SEARCH_GROUP;
}
const gchar *twitter_option_buddy_group(PurpleAccount *account)
{
	//TODO: create an option for this
	return TWITTER_PREF_DEFAULT_BUDDY_GROUP;
}

gint twitter_option_replies_timeout(PurpleAccount *account)
{
	return purple_account_get_int(account,
			TWITTER_PREF_REPLIES_TIMEOUT,
			TWITTER_PREF_REPLIES_TIMEOUT_DEFAULT);
}

gint twitter_option_dms_timeout(PurpleAccount *account)
{
	return purple_account_get_int(account,
			TWITTER_PREF_DMS_TIMEOUT,
			TWITTER_PREF_DMS_TIMEOUT_DEFAULT);
}

gint twitter_option_user_status_timeout(PurpleAccount *account)
{
	return purple_account_get_int(account,
			TWITTER_PREF_USER_STATUS_TIMEOUT,
			TWITTER_PREF_USER_STATUS_TIMEOUT_DEFAULT);
}

gboolean twitter_option_get_following(PurpleAccount *account)
{
	return purple_account_get_bool(account,
			TWITTER_PREF_GET_FRIENDS,
			TWITTER_PREF_GET_FRIENDS_DEFAULT);
}

gboolean twitter_option_get_history(PurpleAccount *account)
{
	return purple_account_get_bool(
			account,
			TWITTER_PREF_RETRIEVE_HISTORY,
			TWITTER_PREF_RETRIEVE_HISTORY_DEFAULT);
}
gboolean twitter_option_sync_status(PurpleAccount *account)
{
	return purple_account_get_bool(
			account,
			TWITTER_PREF_SYNC_STATUS,
			TWITTER_PREF_SYNC_STATUS_DEFAULT);
}

gboolean twitter_option_use_https(PurpleAccount *account)
{
	return purple_account_get_bool(
			account,
			TWITTER_PREF_USE_HTTPS,
			TWITTER_PREF_USE_HTTPS_DEFAULT);
}

gboolean twitter_option_use_oauth(PurpleAccount *account)
{
	return purple_account_get_bool(
			account,
			TWITTER_PREF_USE_OAUTH,
			TWITTER_PREF_USE_OAUTH_DEFAULT);
}

gint twitter_option_home_timeline_max_tweets(PurpleAccount *account)
{
	return purple_account_get_int(account,
			TWITTER_PREF_HOME_TIMELINE_MAX_TWEETS,
			TWITTER_PREF_HOME_TIMELINE_MAX_TWEETS_DEFAULT);
}

gint twitter_option_list_max_tweets(PurpleAccount *account)
{
	return purple_account_get_int(account,
			TWITTER_PREF_LIST_MAX_TWEETS,
			TWITTER_PREF_LIST_MAX_TWEETS_DEFAULT);
}

gboolean twitter_option_default_dm(PurpleAccount *account)
{
	return purple_account_get_bool(account,
			TWITTER_PREF_DEFAULT_DM,
			TWITTER_PREF_DEFAULT_DM_DEFAULT);
}

static const gchar *twitter_get_host_from_base(const gchar *base)
{
	static gchar host[256];
	gchar *slash = strchr(base, '/');
	int len = slash ? slash - base : strlen(base);
	if (len > 255)
		return NULL;
	strncpy(host, base, len);
	host[len] = '\0';
	return host;
}
static const gchar *twitter_get_subdir_from_base(const gchar *base)
{
	if (!base)
		return NULL;
	return strchr(base, '/');
}

const gchar *twitter_option_api_host(PurpleAccount *account)
{
	return twitter_get_host_from_base(
			purple_account_get_string(account,
				TWITTER_PREF_API_BASE,
				TWITTER_PREF_API_BASE_DEFAULT));
}
const gchar *twitter_option_api_subdir(PurpleAccount *account)
{
	return twitter_get_subdir_from_base(
			purple_account_get_string(account,
				TWITTER_PREF_API_BASE,
				TWITTER_PREF_API_BASE_DEFAULT));
}

const gchar *twitter_option_search_api_host(PurpleAccount *account)
{
	return twitter_get_host_from_base(
			purple_account_get_string(account,
				TWITTER_PREF_SEARCH_API_BASE,
				TWITTER_PREF_SEARCH_API_BASE_DEFAULT));
}
const gchar *twitter_option_search_api_subdir(PurpleAccount *account)
{
	return twitter_get_subdir_from_base(
			purple_account_get_string(account,
				TWITTER_PREF_SEARCH_API_BASE,
				TWITTER_PREF_SEARCH_API_BASE_DEFAULT));
}
int twitter_option_cutoff_time(PurpleAccount *account)
{
	return purple_account_get_int(account,
			TWITTER_ONLINE_CUTOFF_TIME_HOURS,
			TWITTER_ONLINE_CUTOFF_TIME_HOURS_DEFAULT);
}
