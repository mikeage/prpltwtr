#ifndef _TWITTER_XML_H_
#define _TWITTER_XML_H_

#include "defaults.h"

#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <glib.h>

#include <account.h>
#include <debug.h>
#include <util.h>
#include "prpltwtr_request.h"
#include "xmlnode_ext.h"

//#include "twitter_util.h" //TODO fix me
/* 
 * TODO: This needs some serious refactoring
 * A buddy should not store the TwitterUserData (should be a separate object)
 * And user_data should have a timestamp, so we can properly update it
 * And maybe have partial TwitterUserData values, so we can pass a user
 * where only the profile_image_url is set, and it won't remove our description
 * The real TODO is to think about this some more
 */

typedef struct {
    PurpleAccount  *account;
    gchar          *id;
    gchar          *name;
    gchar          *screen_name;
    gchar          *profile_image_url;
    gchar          *description;
    gchar          *statuses_count;
    gchar          *friends_count;
    gchar          *followers_count;
} TwitterUserData;

typedef struct {
    gchar          *text;
    gchar          *id;
    gchar          *in_reply_to_status_id;
    gchar          *in_reply_to_screen_name;
    time_t          created_at;
    gboolean        favorited;
} TwitterTweet;

typedef struct {
    char           *screen_name;
    char           *icon_url;   //I don't like this here
    TwitterTweet   *status;
    TwitterUserData *user;
} TwitterUserTweet;

typedef struct {
    char           *refresh_url;
    GList          *tweets;
    gchar          *max_id;
} TwitterSearchResults;

TwitterUserData *twitter_user_node_parse(TwitterRequestor * r, gpointer user_node);
TwitterTweet   *twitter_status_node_parse(TwitterRequestor * r, gpointer status_node);
GList          *twitter_users_node_parse(TwitterRequestor * r, gpointer users_node);
GList          *twitter_users_nodes_parse(TwitterRequestor * r, GList * nodes);
GList          *twitter_users_ids_nodes_parse(TwitterRequestor * r, GList * nodes);
GList          *twitter_statuses_node_parse(TwitterRequestor * r, gpointer statuses_node);
GList          *twitter_statuses_nodes_parse(TwitterRequestor * r, GList * nodes);
GList          *twitter_dms_node_parse(TwitterRequestor * r, gpointer dms_node);
GList          *twitter_dms_nodes_parse(TwitterRequestor * r, GList * nodes);
void            twitter_user_data_free(TwitterUserData * user_data);
void            twitter_status_data_free(TwitterTweet * status);
TwitterUserTweet *twitter_verify_credentials_parse(TwitterRequestor * r, gpointer node);
TwitterUserTweet *twitter_update_status_node_parse(TwitterRequestor * r, gpointer update_status_node);

TwitterSearchResults *twitter_search_results_node_parse(TwitterRequestor * r, gpointer response_node);
void            twitter_search_results_free(TwitterSearchResults * results);

TwitterUserTweet *twitter_user_tweet_new(const char *screen_name, const gchar * icon_url, TwitterUserData * user, TwitterTweet * tweet);
TwitterUserData *twitter_user_tweet_take_user_data(TwitterUserTweet * ut);
TwitterTweet   *twitter_user_tweet_take_tweet(TwitterUserTweet * ut);
void            twitter_user_tweet_free(TwitterUserTweet * ut);

#endif
