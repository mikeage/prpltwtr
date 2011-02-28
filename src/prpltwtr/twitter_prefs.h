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
#define TWITTER_PREF_USE_OAUTH_DEFAULT TRUE

#define TWITTER_PREF_RETRIEVE_HISTORY "retrieve_tweets_history_after_login"
#define TWITTER_PREF_RETRIEVE_HISTORY_DEFAULT TRUE

#define TWITTER_PREF_SYNC_STATUS "sync_availability_status_message_to_twitter"
#define TWITTER_PREF_SYNC_STATUS_DEFAULT FALSE

#define TWITTER_PREF_ADD_URL_TO_TWEET "add_url_link_to_each_tweet"
#define TWITTER_PREF_ADD_URL_TO_TWEET_DEFAULT TRUE

#define TWITTER_PREF_HOME_TIMELINE_MAX_TWEETS "home_timeline_max_tweets"
#define TWITTER_PREF_HOME_TIMELINE_MAX_TWEETS_DEFAULT 100

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
#define TWITTER_PREF_API_BASE_DEFAULT "api.twitter.com/1/"

#define TWITTER_PREF_SEARCH_API_BASE "twitter_search_api_base_url"
#define TWITTER_PREF_SEARCH_API_BASE_DEFAULT "search.twitter.com/"

#define TWITTER_ONLINE_CUTOFF_TIME_HOURS "online_cutoff_time_hours"
#define TWITTER_ONLINE_CUTOFF_TIME_HOURS_DEFAULT 24

/***** START URLS *****/
#define TWITTER_PREF_URL_GET_RATE_LIMIT_STATUS "/account/rate_limit_status.xml"
#define TWITTER_PREF_URL_GET_FRIENDS "/statuses/friends.xml"
#define TWITTER_PREF_URL_GET_HOME_TIMELINE "/statuses/home_timeline.xml"
#define TWITTER_PREF_URL_GET_MENTIONS "/statuses/mentions.xml"
#define TWITTER_PREF_URL_GET_DMS "/direct_messages.xml"
#define TWITTER_PREF_URL_UPDATE_STATUS "/statuses/update.xml"
#define TWITTER_PREF_URL_NEW_DM "/direct_messages/new.xml"
#define TWITTER_PREF_URL_GET_SAVED_SEARCHES "/saved_searches.xml"
#define TWITTER_PREF_URL_GET_LISTS "/lists.xml"
#define TWITTER_PREF_URL_GET_SEARCH_RESULTS "/search.atom"
#define TWITTER_PREF_URL_VERIFY_CREDENTIALS "/account/verify_credentials.xml"
#define TWITTER_PREF_URL_RT "/statuses/retweet" /* Yay for inconsistency */
#define TWITTER_PREF_URL_DELETE_STATUS "/statuses/destroy" /* Yay for inconsistency */
#define TWITTER_PREF_URL_GET_STATUS "/statuses/show"
#define TWITTER_PREF_URL_REPORT_SPAMMER "/report_spam.xml"

/***** END URLS *****/

GList *twitter_get_protocol_options(void);

gboolean twitter_option_add_link_to_tweet(PurpleAccount *account);
gint twitter_option_search_timeout(PurpleAccount *account);
gint twitter_option_timeline_timeout(PurpleAccount *account);
gint twitter_option_list_timeout(PurpleAccount *account);
const gchar *twitter_option_list_group(PurpleAccount *account);
const gchar *twitter_option_search_group(PurpleAccount *account);
const gchar *twitter_option_buddy_group(PurpleAccount *account);
gint twitter_option_dms_timeout(PurpleAccount *account);
gint twitter_option_replies_timeout(PurpleAccount *account);
gboolean twitter_option_get_following(PurpleAccount *account);
gint twitter_option_user_status_timeout(PurpleAccount *account);
gboolean twitter_option_get_history(PurpleAccount *account);
gboolean twitter_option_sync_status(PurpleAccount *account);
gboolean twitter_option_use_https(PurpleAccount *account);
gboolean twitter_option_use_oauth(PurpleAccount *account);
gint twitter_option_home_timeline_max_tweets(PurpleAccount *account);
gboolean twitter_option_default_dm(PurpleAccount *account);
gboolean twitter_option_enable_conv_icon(PurpleAccount *account);

const gchar *twitter_option_api_host(PurpleAccount *account);
const gchar *twitter_option_api_subdir(PurpleAccount *account);
const gchar *twitter_option_search_api_host(PurpleAccount *account);
const gchar *twitter_option_search_api_subdir(PurpleAccount *account);
int twitter_option_cutoff_time(PurpleAccount *account);
#endif
