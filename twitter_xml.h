#ifndef _TWITTER_XML_H_
#define _TWITTER_XML_H_

#include "config.h"

#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <glib.h>

#include <account.h>
#include <debug.h>
#include <util.h>
//#include "twitter_util.h" //TODO fix me

typedef struct 
{
	PurpleAccount *account;
	long long id;
	char *name;
	char *screen_name;
	char *profile_image_url;
	char *description;
} TwitterUserData;

typedef struct 
{
	char *text;
	long long id;
	long long in_reply_to_status_id;
	char *in_reply_to_screen_name;
	time_t created_at;
} TwitterTweet;

typedef struct
{
	char *screen_name;
	TwitterTweet *status;
	TwitterUserData *user;
} TwitterUserTweet;

gchar *xmlnode_get_child_data(const xmlnode *node, const char *name);
TwitterUserData *twitter_user_node_parse(xmlnode *user_node);
TwitterTweet *twitter_status_node_parse(xmlnode *status_node);
GList *twitter_users_node_parse(xmlnode *users_node);
GList *twitter_users_nodes_parse(GList *nodes);
GList *twitter_statuses_node_parse(xmlnode *statuses_node);
GList *twitter_statuses_nodes_parse(GList *nodes);
TwitterTweet *twitter_dm_node_parse(xmlnode *dm_node);
GList *twitter_dms_node_parse(xmlnode *dms_node);
GList *twitter_dms_nodes_parse(GList *nodes);
void twitter_user_data_free(TwitterUserData *user_data);
void twitter_status_data_free(TwitterTweet *status);

TwitterUserTweet *twitter_user_tweet_new(const char *screen_name, TwitterUserData *user, TwitterTweet *tweet);
TwitterUserData *twitter_user_tweet_take_user_data(TwitterUserTweet *ut);
TwitterTweet *twitter_user_tweet_take_tweet(TwitterUserTweet *ut);
void twitter_user_tweet_free(TwitterUserTweet *ut);

#endif
