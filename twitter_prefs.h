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
#ifndef _TWITTER_PREFS_H_
#define _TWITTER_PREFS_H_
#include "config.h"

#include <account.h>
#include <accountopt.h>

#define TWITTER_PREF_DEFAULT_BUDDY_GROUP "twitter"
#define TWITTER_PREF_DEFAULT_SEARCH_GROUP "twitter searches"
#define TWITTER_PREF_DEFAULT_TIMELINE_GROUP "twitter timelines"

#define TWITTER_PREF_USE_HTTPS "use_https"
#define TWITTER_PREF_USE_HTTPS_DEFAULT FALSE

#define TWITTER_PREF_RETRIEVE_HISTORY "retrieve_tweets_history_after_login"
#define TWITTER_PREF_RETRIEVE_HISTORY_DEFAULT TRUE

#define TWITTER_PREF_SYNC_STATUS "sync_availability_status_message_to_twitter"
#define TWITTER_PREF_SYNC_STATUS_DEFAULT FALSE

#define TWITTER_PREF_ADD_URL_TO_TWEET "add_url_link_to_each_tweet"
#define TWITTER_PREF_ADD_URL_TO_TWEET_DEFAULT TRUE

#define TWITTER_PREF_HOME_TIMELINE_MAX_TWEETS "home_timeline_max_tweets"
#define TWITTER_PREF_HOME_TIMELINE_MAX_TWEETS_DEFAULT 100

#define TWITTER_PREF_HOST_URL "host_url"
#define TWITTER_PREF_HOST_URL_DEFAULT "twitter.com"

#define TWITTER_PREF_HOST_API_URL "api_host_url"
#define TWITTER_PREF_HOST_API_URL_DEFAULT "api.twitter.com"

#define TWITTER_PREF_SEARCH_HOST_URL "search_host_url"
#define TWITTER_PREF_SEARCH_HOST_URL_DEFAULT "search.twitter.com"

#define TWITTER_PREF_REPLIES_TIMEOUT "refresh_replies_minutes"
#define	TWITTER_PREF_REPLIES_TIMEOUT_DEFAULT 30

#define TWITTER_PREF_USER_STATUS_TIMEOUT "refresh_friendlist_minutes"
#define	TWITTER_PREF_USER_STATUS_TIMEOUT_DEFAULT 60

#define TWITTER_PREF_SEARCH_TIMEOUT "refresh_search_minutes"
#define TWITTER_PREF_SEARCH_TIMEOUT_DEFAULT 5

#define TWITTER_PREF_GET_FRIENDS "get_friends"
#define TWITTER_PREF_GET_FRIENDS_DEFAULT TRUE

GList *twitter_get_protocol_options();

gboolean twitter_option_add_link_to_tweet(PurpleAccount *account);
gint twitter_option_search_timeout(PurpleAccount *account);
gint twitter_option_timeline_timeout(PurpleAccount *account);
const gchar *twitter_option_search_group(PurpleAccount *account);
const gchar *twitter_option_buddy_group(PurpleAccount *account);
gint twitter_option_replies_timeout(PurpleAccount *account);
gboolean twitter_option_get_following(PurpleAccount *account);
gint twitter_option_user_status_timeout(PurpleAccount *account);
gboolean twitter_option_get_history(PurpleAccount *account);
gboolean twitter_option_sync_status(PurpleAccount *account);
gboolean twitter_option_use_https(PurpleAccount *account);
const gchar *twitter_option_host_url(PurpleAccount *account);
const gchar *twitter_option_host_api_url(PurpleAccount *account);
const gchar *twitter_option_host_search_url(PurpleAccount *account);
gint twitter_option_home_timeline_max_tweets(PurpleAccount *account);

#endif
