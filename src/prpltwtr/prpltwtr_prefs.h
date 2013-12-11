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
#include "defaults.h"

#include <account.h>
#include <accountopt.h>

#define TWITTER_PREF_DEFAULT_BUDDY_GROUP _("twitter")
#define TWITTER_PREF_DEFAULT_LIST_GROUP _("twitter lists")
#define TWITTER_PREF_DEFAULT_SEARCH_GROUP _("twitter searches")
#define TWITTER_PREF_DEFAULT_TIMELINE_GROUP _("twitter timelines")

#define TWITTER_PREF_USE_HTTPS "use_https"
#define TWITTER_PREF_USE_HTTPS_DEFAULT TRUE

#define TWITTER_PREF_USE_OAUTH "use_oauth"
#define TWITTER_PREF_USE_OAUTH_DEFAULT FALSE

#define TWITTER_PREF_RETRIEVE_HISTORY "retrieve_tweets_history_after_login"
#define TWITTER_PREF_RETRIEVE_HISTORY_DEFAULT TRUE

#define TWITTER_PREF_SYNC_STATUS "sync_availability_status_message_to_twitter"
#define TWITTER_PREF_SYNC_STATUS_DEFAULT FALSE

#define TWITTER_PREF_ALIAS_FORMAT "alias_format"
#define TWITTER_PREF_ALIAS_FORMAT_ALL "all"
#define TWITTER_PREF_ALIAS_FORMAT_NICK "nick"
#define TWITTER_PREF_ALIAS_FORMAT_FULLNAME "fullname"
#define TWITTER_PREF_ALIAS_FORMAT_DEFAULT TWITTER_PREF_ALIAS_FORMAT_ALL

#define TWITTER_PREF_ADD_URL_TO_TWEET "add_url_link_to_each_tweet"
#define TWITTER_PREF_ADD_URL_TO_TWEET_DEFAULT TRUE

#define TWITTER_PREF_HOME_TIMELINE_MAX_TWEETS "home_timeline_max_tweets"
#define TWITTER_PREF_HOME_TIMELINE_MAX_TWEETS_DEFAULT 200

#define TWITTER_PREF_TIMELINE_TIMEOUT "refresh_timeline_minutes"
#define TWITTER_PREF_TIMELINE_TIMEOUT_DEFAULT 1
#define TWITTER_PREF_LIST_TIMEOUT "refresh_list_minutes"
#define TWITTER_PREF_LIST_TIMEOUT_DEFAULT 10

#define TWITTER_PREF_REPLIES_TIMEOUT "refresh_replies_minutes"
#define	TWITTER_PREF_REPLIES_TIMEOUT_DEFAULT 30

#define TWITTER_PREF_DMS_TIMEOUT "refresh_dms_minutes"
#define	TWITTER_PREF_DMS_TIMEOUT_DEFAULT 30

#define TWITTER_PREF_USER_STATUS_TIMEOUT "refresh_friendlist_minutes"
#define	TWITTER_PREF_USER_STATUS_TIMEOUT_DEFAULT 60

#define TWITTER_PREF_SEARCH_TIMEOUT "refresh_search_minutes"
#define TWITTER_PREF_SEARCH_TIMEOUT_DEFAULT 5

#define TWITTER_PREF_GET_FRIENDS "get_friends"
#define TWITTER_PREF_GET_FRIENDS_DEFAULT TRUE

#define TWITTER_PREF_DEFAULT_DM "default_message_is_dm"
#define TWITTER_PREF_DEFAULT_DM_DEFAULT FALSE

#define TWITTER_PREF_API_BASE "twitter_api_base_url"
#define TWITTER_PREF_API_BASE_DEFAULT "api.twitter.com/1.1"
#define STATUSNET_PREF_API_BASE_DEFAULT "identi.ca/api"

#define TWITTER_PREF_CONSUMER_KEY "consumer_key"
#define TWITTER_PREF_CONSUMER_SECRET "consumer_secret"

#define TWITTER_PREF_WEB_BASE "twitter_web_base_url"
#define TWITTER_PREF_WEB_BASE_DEFAULT "twitter.com"

#define TWITTER_PREF_URL_OPEN_RETWEETED_OF_MINE "retweeted_of_mine"
#define TWITTER_PREF_URL_OPEN_FAVORITES "favorites"
#define TWITTER_PREF_URL_OPEN_REPLIES "replies"
#define TWITTER_PREF_URL_OPEN_SUGGESTED_FRIENDS "invitations/twitter_suggests"

#define TWITTER_PREF_SEARCH_API_BASE "twitter_search_api_base_url"
#define TWITTER_PREF_SEARCH_API_BASE_DEFAULT "search.twitter.com"

#define TWITTER_ONLINE_CUTOFF_TIME_HOURS "online_cutoff_time_hours"
#define TWITTER_ONLINE_CUTOFF_TIME_HOURS_DEFAULT 24

/***** START URLS *****/
#define TWITTER_PREF_URL_GET_RATE_LIMIT_STATUS "/account/rate_limit_status"
#define TWITTER_PREF_URL_GET_FRIENDS "/friends/ids"
#define TWITTER_PREF_URL_GET_HOME_TIMELINE "/statuses/home_timeline"
#define TWITTER_PREF_URL_GET_LIST "/lists/"      /* We need to prepend the username and add the ID and then statuses.xml here */
#define TWITTER_PREF_URL_GET_MENTIONS "/statuses/mentions_timeline"
#define TWITTER_PREF_URL_GET_DMS "/direct_messages"
#define TWITTER_PREF_URL_UPDATE_STATUS "/statuses/update"
#define TWITTER_PREF_URL_NEW_DM "/direct_messages/new"
#define TWITTER_PREF_URL_GET_SAVED_SEARCHES "/saved_searches/list"
#define TWITTER_PREF_URL_GET_SUBSCRIBED_LISTS "/lists/subscriptions"
#define TWITTER_PREF_URL_GET_PERSONAL_LISTS "/lists/list"
#define TWITTER_PREF_URL_GET_SEARCH_RESULTS "/search"
#define TWITTER_PREF_URL_VERIFY_CREDENTIALS "/account/verify_credentials"
#define TWITTER_PREF_URL_RT "/statuses/retweet"  /* Yay for inconsistency */
#define TWITTER_PREF_URL_DELETE_STATUS "/statuses/destroy"  /* Yay for inconsistency */
#define TWITTER_PREF_URL_ADD_FAVORITE "/favorites/create"   /* Yay for inconsistency */
#define TWITTER_PREF_URL_DELETE_FAVORITE "/favorites/destroy"   /* Yay for inconsistency */
#define TWITTER_PREF_URL_GET_STATUS "/statuses/show"
#define TWITTER_PREF_URL_REPORT_SPAMMER "/report_spam"
#define TWITTER_PREF_URL_GET_USER_INFO "/users/show"

/***** END URLS *****/

GList          *prpltwtr_twitter_get_protocol_options(void);
GList          *prpltwtr_statusnet_get_protocol_options(void);

const gchar    *twitter_option_alias_format(PurpleAccount * account);
gboolean        twitter_option_add_link_to_tweet(PurpleAccount * account);
gint            twitter_option_search_timeout(PurpleAccount * account);
gint            twitter_option_timeline_timeout(PurpleAccount * account);
gint            twitter_option_list_timeout(PurpleAccount * account);
const gchar    *twitter_option_list_group(PurpleAccount * account);
const gchar    *twitter_option_search_group(PurpleAccount * account);
const gchar    *twitter_option_buddy_group(PurpleAccount * account);
gint            twitter_option_dms_timeout(PurpleAccount * account);
gint            twitter_option_replies_timeout(PurpleAccount * account);
gboolean        twitter_option_get_following(PurpleAccount * account);
gint            twitter_option_user_status_timeout(PurpleAccount * account);
gboolean        twitter_option_get_history(PurpleAccount * account);
gboolean        twitter_option_sync_status(PurpleAccount * account);
gboolean        twitter_option_use_https(PurpleAccount * account);
gboolean        twitter_option_use_oauth(PurpleAccount * account);
gint            twitter_option_home_timeline_max_tweets(PurpleAccount * account);
gint            twitter_option_list_max_tweets(PurpleAccount * account);
gboolean        twitter_option_default_dm(PurpleAccount * account);
gboolean        twitter_option_enable_conv_icon(PurpleAccount * account);

const gchar    *twitter_option_api_host(PurpleAccount * account);
const gchar    *twitter_option_api_subdir(PurpleAccount * account);
const gchar    *twitter_option_web_host(PurpleAccount * account);
const gchar    *twitter_option_web_subdir(PurpleAccount * account);
const gchar    *twitter_option_search_api_host(PurpleAccount * account);
const gchar    *twitter_option_search_api_subdir(PurpleAccount * account);
int             twitter_option_cutoff_time(PurpleAccount * account);
#endif
