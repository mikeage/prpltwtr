#ifndef _PT_PREFS_H_
#define _PT_PREFS_H_

#include "account.h"

#define PT_PREF_DEFAULT_BUDDY_GROUP _("twitter")
#define PT_PREF_DEFAULT_LIST_GROUP _("twitter lists")
#define PT_PREF_DEFAULT_SEARCH_GROUP _("twitter searches")
#define PT_PREF_DEFAULT_TIMELINE_GROUP _("twitter timelines")

#define PT_PREF_USE_HTTPS "use_https"
#define PT_PREF_USE_HTTPS_DEFAULT TRUE

#define PT_PREF_RETRIEVE_HISTORY "retrieve_tweets_history_after_login"
#define PT_PREF_RETRIEVE_HISTORY_DEFAULT TRUE

#if 0
#define PT_PREF_SYNC_STATUS "sync_availability_status_message_to_twitter"
#define PT_PREF_SYNC_STATUS_DEFAULT FALSE

#define PT_PREF_ALIAS_FORMAT "alias_format"
#define PT_PREF_ALIAS_FORMAT_ALL "all"
#define PT_PREF_ALIAS_FORMAT_NICK "nick"
#define PT_PREF_ALIAS_FORMAT_FULLNAME "fullname"
#define PT_PREF_ALIAS_FORMAT_DEFAULT PT_PREF_ALIAS_FORMAT_ALL

#define PT_PREF_ADD_URL_TO_TWEET "add_url_link_to_each_tweet"
#define PT_PREF_ADD_URL_TO_TWEET_DEFAULT TRUE

#define PT_PREF_HOME_TIMELINE_MAX_TWEETS "home_timeline_max_tweets"
#define PT_PREF_HOME_TIMELINE_MAX_TWEETS_DEFAULT 200

#endif
#define PT_PREF_TIMELINE_TIMEOUT "refresh_timeline_minutes"
#define PT_PREF_TIMELINE_TIMEOUT_DEFAULT 1
#define PT_PREF_LIST_TIMEOUT "refresh_list_minutes"
#define PT_PREF_LIST_TIMEOUT_DEFAULT 10
#if 0
#define PT_PREF_REPLIES_TIMEOUT "refresh_replies_minutes"
#define	PT_PREF_REPLIES_TIMEOUT_DEFAULT 30

#define PT_PREF_DMS_TIMEOUT "refresh_dms_minutes"
#define	PT_PREF_DMS_TIMEOUT_DEFAULT 30

#define PT_PREF_USER_STATUS_TIMEOUT "refresh_friendlist_minutes"
#define	PT_PREF_USER_STATUS_TIMEOUT_DEFAULT 60

#define PT_PREF_SEARCH_TIMEOUT "refresh_search_minutes"
#define PT_PREF_SEARCH_TIMEOUT_DEFAULT 5

#define PT_PREF_GET_FRIENDS "get_friends"
#define PT_PREF_GET_FRIENDS_DEFAULT TRUE

#endif
#define PT_PREF_DEFAULT_DM "default_message_is_dm"
#define PT_PREF_DEFAULT_DM_DEFAULT FALSE

#if 0

#define PT_PREF_API_BASE "twitter_api_base_url"
#define PT_PREF_API_BASE_DEFAULT "api.twitter.com/1"
#define STATUSNET_PREF_API_BASE_DEFAULT "identi.ca/api"

#define PT_PREF_CONSUMER_KEY "consumer_key"
#define PT_PREF_CONSUMER_SECRET "consumer_secret"

#define PT_PREF_WEB_BASE "twitter_web_base_url"
#define PT_PREF_WEB_BASE_DEFAULT "twitter.com"

#define PT_PREF_URL_OPEN_RETWEETED_OF_MINE "retweeted_of_mine"
#define PT_PREF_URL_OPEN_FAVORITES "favorites"
#define PT_PREF_URL_OPEN_REPLIES "replies"
#define PT_PREF_URL_OPEN_SUGGESTED_FRIENDS "invitations/twitter_suggests"

#define PT_PREF_SEARCH_API_BASE "twitter_search_api_base_url"
#define PT_PREF_SEARCH_API_BASE_DEFAULT "search.twitter.com"

#define PT_ONLINE_CUTOFF_TIME_HOURS "online_cutoff_time_hours"
#define PT_ONLINE_CUTOFF_TIME_HOURS_DEFAULT 24

/***** START URLS *****/
#define PT_PREF_URL_GET_RATE_LIMIT_STATUS "/account/rate_limit_status.xml"
#define PT_PREF_URL_GET_FRIENDS "/friends/ids.xml"
#define PT_PREF_URL_GET_HOME_TIMELINE "/statuses/home_timeline.xml"
#define PT_PREF_URL_GET_LIST "/lists/"      /* We need to prepend the username and add the ID and then statuses.xml here */
#define PT_PREF_URL_GET_MENTIONS "/statuses/mentions.xml"
#define PT_PREF_URL_GET_DMS "/direct_messages.xml"
#define PT_PREF_URL_UPDATE_STATUS "/statuses/update.xml"
#define PT_PREF_URL_NEW_DM "/direct_messages/new.xml"
#define PT_PREF_URL_GET_SAVED_SEARCHES "/saved_searches.xml"
#define PT_PREF_URL_GET_SUBSCRIBED_LISTS "/lists/subscriptions.xml"
#define PT_PREF_URL_GET_PERSONAL_LISTS "/lists.xml"
#define PT_PREF_URL_GET_SEARCH_RESULTS "/search.atom"
#define PT_PREF_URL_VERIFY_CREDENTIALS "/account/verify_credentials.xml"
#define PT_PREF_URL_RT "/statuses/retweet"  /* Yay for inconsistency */
#define PT_PREF_URL_DELETE_STATUS "/statuses/destroy"  /* Yay for inconsistency */
#define PT_PREF_URL_ADD_FAVORITE "/favorites/create"   /* Yay for inconsistency */
#define PT_PREF_URL_DELETE_FAVORITE "/favorites/destroy"   /* Yay for inconsistency */
#define PT_PREF_URL_GET_STATUS "/statuses/show"
#define PT_PREF_URL_REPORT_SPAMMER "/report_spam.xml"
#define PT_PREF_URL_GET_USER_INFO "/users/show.xml"

/***** END URLS *****/

GList          *prpltwtr_twitter_get_protocol_options(void);
GList          *prpltwtr_statusnet_get_protocol_options(void);

const gchar    *twitter_option_alias_format(PurpleAccount * account);
gboolean        twitter_option_add_link_to_tweet(PurpleAccount * account);
gint            twitter_option_search_timeout(PurpleAccount * account);
#endif 
gint            pt_option_timeline_timeout(PurpleAccount * account);
gint            pt_option_list_timeout(PurpleAccount * account);
#if 0
const gchar    *twitter_option_list_group(PurpleAccount * account);
const gchar    *twitter_option_search_group(PurpleAccount * account);
const gchar    *twitter_option_buddy_group(PurpleAccount * account);
gint            twitter_option_dms_timeout(PurpleAccount * account);
gint            twitter_option_replies_timeout(PurpleAccount * account);
gboolean        twitter_option_get_following(PurpleAccount * account);
gint            twitter_option_user_status_timeout(PurpleAccount * account);
#endif 
gboolean        pt_option_get_history(PurpleAccount * account);
#if 0
gboolean        twitter_option_sync_status(PurpleAccount * account);
gboolean        twitter_option_use_https(PurpleAccount * account);
gboolean        twitter_option_use_oauth(PurpleAccount * account);
gint            twitter_option_home_timeline_max_tweets(PurpleAccount * account);
gint            twitter_option_list_max_tweets(PurpleAccount * account);
#endif
gboolean        pt_option_default_dm(PurpleAccount * account);
#if 0
gboolean        twitter_option_enable_conv_icon(PurpleAccount * account);

const gchar    *twitter_option_api_host(PurpleAccount * account);
const gchar    *twitter_option_api_subdir(PurpleAccount * account);
const gchar    *twitter_option_web_host(PurpleAccount * account);
const gchar    *twitter_option_web_subdir(PurpleAccount * account);
const gchar    *twitter_option_search_api_host(PurpleAccount * account);
const gchar    *twitter_option_search_api_subdir(PurpleAccount * account);
int             twitter_option_cutoff_time(PurpleAccount * account);
#endif
gboolean        pt_prefs_get_use_https(PurpleAccount * account);
#endif
