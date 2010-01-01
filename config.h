#ifndef G_GNUC_NULL_TERMINATED
#  if __GNUC__ >= 4
#    define G_GNUC_NULL_TERMINATED __attribute__((__sentinel__))
#  else
#    define G_GNUC_NULL_TERMINATED
#  endif /* __GNUC__ >= 4 */
#endif /* G_GNUC_NULL_TERMINATED */

#ifdef _WIN32
#	include <win32dep.h>
#endif

#define TWITTER_PREF_DEFAULT_BUDDY_GROUP "twitter"
#define TWITTER_PREF_DEFAULT_SEARCH_GROUP "twitter searches"
#define TWITTER_PREF_DEFAULT_TIMELINE_GROUP "twitter timelines"

#define TWITTER_PREF_RETRIEVE_HISTORY "retrieve_tweets_history_after_login"
#define TWITTER_PREF_RETRIEVE_HISTORY_DEFAULT TRUE

#define TWITTER_PREF_SYNC_STATUS "sync_availability_status_message_to_twitter"
#define TWITTER_PREF_SYNC_STATUS_DEFAULT FALSE

#define TWITTER_PREF_ADD_URL_TO_TWEET "add_url_link_to_each_tweet"
#define TWITTER_PREF_ADD_URL_TO_TWEET_DEFAULT TRUE

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

/* The number of tweets to return per page, up to a max of 100 */
#define TWITTER_SEARCH_RPP_DEFAULT 20

#define TWITTER_INITIAL_REPLIES_COUNT 150
#define TWITTER_EVERY_REPLIES_COUNT 40

#define TWITTER_PROTOCOL_ID "prpl-twitter"
