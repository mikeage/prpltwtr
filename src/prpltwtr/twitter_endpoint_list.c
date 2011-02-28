#include "twitter_endpoint_list.h"

//TODO: Should these be here?
#if 0
static long long twitter_account_get_last_home_timeline_id(PurpleAccount *account)
{
	return purple_account_get_long_long(account, "twitter_last_home_timeline_id", 0);
}

static void twitter_account_set_last_home_timeline_id(PurpleAccount *account, long long reply_id)
{
	purple_account_set_long_long(account, "twitter_last_home_timeline_id", reply_id);
}

static long long twitter_connection_get_last_home_timeline_id(PurpleConnection *gc)
{
	long long reply_id = 0;
	TwitterConnectionData *connection_data = gc->proto_data;
	reply_id = connection_data->last_home_timeline_id;
	return (reply_id ? reply_id : twitter_account_get_last_home_timeline_id(purple_connection_get_account(gc)));
}

static void twitter_connection_set_last_home_timeline_id(PurpleConnection *gc, long long reply_id)
{
	TwitterConnectionData *connection_data = gc->proto_data;

	connection_data->last_home_timeline_id = reply_id;
	twitter_account_set_last_home_timeline_id(purple_connection_get_account(gc), reply_id);
}
#endif 

static gpointer twitter_list_timeout_context_new(GHashTable *components)
{
	TwitterListTimeoutContext *ctx = g_slice_new0(TwitterListTimeoutContext);
//	ctx->timeline_id = 0;
	return ctx;
}

static void twitter_list_timeout_context_free(gpointer _ctx)
{
	TwitterListTimeoutContext *ctx;
	g_return_if_fail(_ctx != NULL);
	ctx = _ctx;

	g_slice_free (TwitterListTimeoutContext, ctx);
}
#if 0
static char *twitter_chat_name_from_timeline_id(const gint timeline_id)
{
	return g_strdup("Timeline: Home");
}
static char *twitter_timeline_chat_name_from_components(GHashTable *components)
{
	return twitter_chat_name_from_timeline_id(0);
}
#endif
static char *twitter_chat_name_from_list(const char *list_name)
{
#ifdef _HAZE_
	return g_strdup_printf("#%s", search);
#error
#else
	char *list_lower= g_utf8_strdown(list_name, -1);
	char *chat_name = g_strdup_printf("List: %s", list_lower);
	g_free(list_lower);
	return chat_name;
#endif
}


static char *twitter_list_chat_name_from_components(GHashTable *components)
{
	const char *list = g_hash_table_lookup(components, "name");
	return twitter_chat_name_from_list(list);
}


#if 0

static void twitter_get_home_timeline_parse_statuses(TwitterEndpointChat *endpoint_chat, GList *statuses)
{
	PurpleConnection *gc; 
	GList *l;
	TwitterUserTweet *user_tweet;

	purple_debug_info(TWITTER_PROTOCOL_ID, "%s\n", G_STRFUNC);

	g_return_if_fail(endpoint_chat != NULL);
	gc = purple_account_get_connection(endpoint_chat->account);

	if (!statuses)
	{
		/* At least update the topic with the new rate limit info */
		twitter_chat_update_rate_limit(endpoint_chat);
		return;
	}

	l = g_list_last(statuses);
	user_tweet = l->data;
	if (user_tweet && user_tweet->status)
	/* Tweets might not be sequential anymore. Take since_id from the last one, not the greatest */
#if 0
		&&
			user_tweet->status->id > twitter_connection_get_last_home_timeline_id(gc))
#endif /* 0 */
	{
		if (user_tweet->status->id < twitter_connection_get_last_home_timeline_id(gc)) {
			purple_debug_info(TWITTER_PROTOCOL_ID, "Setting last as %lld, although it's less than the previous %lld\n", user_tweet->status->id, twitter_connection_get_last_home_timeline_id(gc));
		}
		twitter_connection_set_last_home_timeline_id(gc, user_tweet->status->id);
	}
	twitter_chat_got_user_tweets(endpoint_chat, statuses);
}

static void twitter_get_home_timeline_cb(TwitterRequestor *r, xmlnode *node, gpointer user_data)
{
	TwitterEndpointChatId *chat_id = (TwitterEndpointChatId *)user_data;
	TwitterEndpointChat *endpoint_chat;
	GList *statuses;

	purple_debug_info(TWITTER_PROTOCOL_ID, "%s\n", G_STRFUNC);

	g_return_if_fail(chat_id != NULL);
	endpoint_chat = twitter_endpoint_chat_find_by_id(chat_id);
	twitter_endpoint_chat_id_free(chat_id);
	
	if (endpoint_chat == NULL)
		return;

	endpoint_chat->rate_limit_remaining = r->rate_limit_remaining;
	endpoint_chat->rate_limit_total = r->rate_limit_total;

	statuses = twitter_statuses_node_parse(node);
	twitter_get_home_timeline_parse_statuses(endpoint_chat, statuses);

}

static void twitter_get_home_timeline_all_cb(TwitterRequestor *r,
		GList *nodes,
		gpointer user_data)
{
	TwitterEndpointChatId *chat_id = (TwitterEndpointChatId *)user_data;
	TwitterEndpointChat *endpoint_chat;
	GList *statuses;

	purple_debug_info(TWITTER_PROTOCOL_ID, "%s\n", G_STRFUNC);

	g_return_if_fail(chat_id != NULL);
	endpoint_chat = twitter_endpoint_chat_find_by_id(chat_id);
	twitter_endpoint_chat_id_free(chat_id);
	
	if (endpoint_chat == NULL)
		return;

	endpoint_chat->rate_limit_remaining = r->rate_limit_remaining;
	endpoint_chat->rate_limit_total = r->rate_limit_total;

	statuses = twitter_statuses_nodes_parse(nodes);
	twitter_get_home_timeline_parse_statuses(endpoint_chat, statuses);
}
#endif 
static gboolean twitter_endpoint_list_interval_start(TwitterEndpointChat *endpoint)
{
#if 0
	PurpleAccount *account = endpoint->account;
	PurpleConnection *gc = purple_account_get_connection(account);
	TwitterEndpointChatId *chat_id = twitter_endpoint_chat_id_new(endpoint);
	long long since_id = twitter_connection_get_last_home_timeline_id(gc);

	purple_debug_info(TWITTER_PROTOCOL_ID, "%s creating new timeline context\n", account->username);

	if (since_id == 0)
	{
		purple_debug_info(TWITTER_PROTOCOL_ID, "Retrieving %s statuses for first time\n", gc->account->username);
		twitter_api_get_home_timeline(purple_account_get_requestor(account),
				since_id,
				TWITTER_HOME_TIMELINE_INITIAL_COUNT,
				1,
				twitter_get_home_timeline_cb,
				NULL,
				chat_id);
	} else {
		purple_debug_info(TWITTER_PROTOCOL_ID, "Retrieving %s statuses since %lld\n", gc->account->username, since_id);
		twitter_api_get_home_timeline_all(purple_account_get_requestor(account),
				since_id,
				twitter_get_home_timeline_all_cb,
				NULL,
				twitter_option_home_timeline_max_tweets(account),
				chat_id);
	}
#endif 
	return TRUE;
}
static gboolean twitter_list_timeout(TwitterEndpointChat *endpoint_chat)
{
#if 0
	PurpleAccount *account = endpoint_chat->account;
	PurpleConnection *gc = purple_account_get_connection(account);
	TwitterEndpointChatId *chat_id = twitter_endpoint_chat_id_new(endpoint_chat);
	long long since_id = twitter_connection_get_last_home_timeline_id(gc);
	if (since_id == 0)
	{
		purple_debug_info(TWITTER_PROTOCOL_ID, "Retrieving %s statuses for first time\n", account->username);
		twitter_api_get_home_timeline(purple_account_get_requestor(account),
				since_id,
				20,
				1,
				twitter_get_home_timeline_cb,
				NULL,
				chat_id);
	} else {
		purple_debug_info(TWITTER_PROTOCOL_ID, "Retrieving %s statuses since %lld\n", account->username, since_id);
		twitter_api_get_home_timeline_all(purple_account_get_requestor(account),
				since_id,
				twitter_get_home_timeline_all_cb,
				NULL,
				twitter_option_home_timeline_max_tweets(account),
				chat_id);
	}
#endif 
	return TRUE;
}
static TwitterEndpointChatSettings TwitterEndpointListSettings=
{
	TWITTER_CHAT_LIST,
#ifdef _HAZE_
	'*',
#endif
	NULL,
	twitter_list_timeout_context_free, //endpoint_data_free
	twitter_option_list_timeout, //get_default_interval
	twitter_list_chat_name_from_components, //get_name
	NULL, //verify_components TODO: only in search? MHM
	twitter_list_timeout,
	twitter_endpoint_list_interval_start,
	twitter_list_timeout_context_new,
};

TwitterEndpointChatSettings *twitter_endpoint_list_get_settings(void)
{
	return &TwitterEndpointListSettings;
}

