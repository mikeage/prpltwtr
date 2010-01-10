#include "twitter_endpoint_timeline.h"

//TODO: Should these be here?
long long twitter_account_get_last_home_timeline_id(PurpleAccount *account)
{
	return purple_account_get_long_long(account, "twitter_last_home_timeline_id", 0);
}

void twitter_account_set_last_home_timeline_id(PurpleAccount *account, long long reply_id)
{
	purple_account_set_long_long(account, "twitter_last_home_timeline_id", reply_id);
}

long long twitter_connection_get_last_home_timeline_id(PurpleConnection *gc)
{
	long long reply_id = 0;
	TwitterConnectionData *connection_data = gc->proto_data;
	reply_id = connection_data->last_home_timeline_id;
	return (reply_id ? reply_id : twitter_account_get_last_home_timeline_id(purple_connection_get_account(gc)));
}

void twitter_connection_set_last_home_timeline_id(PurpleConnection *gc, long long reply_id)
{
	TwitterConnectionData *connection_data = gc->proto_data;

	connection_data->last_home_timeline_id = reply_id;
	twitter_account_set_last_home_timeline_id(purple_connection_get_account(gc), reply_id);
}


static gpointer twitter_timeline_timeout_context_new(GHashTable *components)
{
	TwitterTimelineTimeoutContext *ctx = g_slice_new0(TwitterTimelineTimeoutContext);
	ctx->timeline_id = 0;
	return ctx;
}

static void twitter_timeline_timeout_context_free(gpointer _ctx)
{
	g_return_if_fail(_ctx != NULL);
	TwitterTimelineTimeoutContext *ctx = _ctx;

	g_slice_free (TwitterTimelineTimeoutContext, ctx);
} 

//TODO merge me
static int twitter_chat_timeline_send(TwitterEndpointChat *ctx_base, const gchar *message)
{
	PurpleAccount *account = ctx_base->account;
	PurpleConversation *conv = twitter_chat_context_find_conv(ctx_base);

	if (conv == NULL) return -1; //TODO: error?

	if (strlen(message) > MAX_TWEET_LENGTH)
	{
		//TODO: SHOW ERROR
		return -E2BIG;
	}
	else
	{
		twitter_api_set_status(account,
				message, 0,
				NULL, NULL,//TODO: verify & error
				NULL);
#if _HAZE_
		//It's already in the message box in haze. Maybe we should edit it before hand?
		//twitter_chat_add_tweet(PURPLE_CONV_IM(conv), account->username, message, 0, time(NULL));//TODO: FIX TIME
#else
		twitter_chat_add_tweet(PURPLE_CONV_CHAT(conv), account->username, message, 0, time(NULL));//TODO: FIX TIME
#endif
		return 0;
	}
}

static char *twitter_chat_name_from_timeline_id(const gint timeline_id)
{
	return g_strdup("Timeline: Home");
}
static char *twitter_timeline_chat_name_from_components(GHashTable *components)
{
	return twitter_chat_name_from_timeline_id(0);
}

//TODO: the proper way to do this would be to have this and twitter_search return the same data type
static void twitter_get_home_timeline_parse_statuses(PurpleAccount *account,
		TwitterEndpointChat *endpoint_chat, GList *statuses)
{
	PurpleConnection *gc = purple_account_get_connection(account);
#if _HAZE_
	PurpleConvIm *chat;
#else
	PurpleConvChat *chat;
#endif
	GList *l;

	purple_debug_info(TWITTER_PROTOCOL_ID, "%s\n", G_STRFUNC);

	g_return_if_fail(account != NULL);

	if (!statuses)
		return;
	chat = twitter_endpoint_chat_get_conv(endpoint_chat);
	g_return_if_fail(chat != NULL); //TODO: destroy context

	for (l = statuses; l; l = l->next)
	{
		TwitterBuddyData *data = l->data;
		TwitterStatusData *status = data->status;
		TwitterUserData *user_data = data->user;
		g_free(data);

		if (!user_data)
		{
			twitter_status_data_free(status);
		} else {
			const char *screen_name = user_data->screen_name;
			const char *text = status->text;
			twitter_chat_add_tweet(chat, screen_name, text, status->id, status->created_at);
			if (status->id && status->id > twitter_connection_get_last_home_timeline_id(gc))
			{
				twitter_connection_set_last_home_timeline_id(gc, status->id);
			}
			twitter_buddy_set_status_data(account, screen_name, status);
			twitter_buddy_set_user_data(account, user_data, FALSE);

			/* update user_reply_id_table table */
			//gchar *reply_id = g_strdup_printf ("%lld", status->id);
			//g_hash_table_insert (twitter->user_reply_id_table,
					//g_strdup (screen_name), reply_id);
		}
	}
}

static void twitter_get_home_timeline_cb(PurpleAccount *account, xmlnode *node, gpointer user_data)
{
	TwitterEndpointChatId *chat_id = (TwitterEndpointChatId *)user_data;
	TwitterEndpointChat *endpoint_chat;

	purple_debug_info(TWITTER_PROTOCOL_ID, "%s\n", G_STRFUNC);

	g_return_if_fail(chat_id != NULL);
	endpoint_chat = twitter_endpoint_chat_find_by_id(chat_id);
	twitter_endpoint_chat_id_free(chat_id);
	
	if (endpoint_chat == NULL)
		return;

	GList *statuses = twitter_statuses_node_parse(node);
	twitter_get_home_timeline_parse_statuses(account, endpoint_chat, statuses);
	g_list_free(statuses);

}

static void twitter_get_home_timeline_all_cb(PurpleAccount *account,
		GList *nodes,
		gpointer user_data)
{
	TwitterEndpointChatId *chat_id = (TwitterEndpointChatId *)user_data;
	TwitterEndpointChat *endpoint_chat;

	purple_debug_info(TWITTER_PROTOCOL_ID, "%s\n", G_STRFUNC);

	g_return_if_fail(chat_id != NULL);
	endpoint_chat = twitter_endpoint_chat_find_by_id(chat_id);
	twitter_endpoint_chat_id_free(chat_id);
	
	if (endpoint_chat == NULL)
		return;

	GList *statuses = twitter_statuses_nodes_parse(nodes);
	twitter_get_home_timeline_parse_statuses(account, endpoint_chat, statuses);
	g_list_free(statuses);
}
static gboolean twitter_endpoint_timeline_interval_start(TwitterEndpointChat *endpoint)
{
	PurpleAccount *account = endpoint->account;
	PurpleConnection *gc = purple_account_get_connection(account);
	TwitterEndpointChatId *chat_id = twitter_endpoint_chat_id_new(endpoint);
	long long since_id = twitter_connection_get_last_home_timeline_id(gc);

	purple_debug_info(TWITTER_PROTOCOL_ID, "%s creating new timeline context\n", account->username);

	if (since_id == 0)
	{
		purple_debug_info(TWITTER_PROTOCOL_ID, "Retrieving %s statuses for first time\n", gc->account->username);
		twitter_api_get_home_timeline(account,
				since_id,
				20,
				1,
				twitter_get_home_timeline_cb,
				NULL,
				chat_id);
	} else {
		purple_debug_info(TWITTER_PROTOCOL_ID, "Retrieving %s statuses since %lld\n", gc->account->username, since_id);
		twitter_api_get_home_timeline_all(account,
				since_id,
				twitter_get_home_timeline_all_cb,
				NULL,
				chat_id);
	}
	return TRUE;
}
static gboolean twitter_timeline_timeout(TwitterEndpointChat *endpoint_chat)
{
	PurpleAccount *account = endpoint_chat->account;
	PurpleConnection *gc = purple_account_get_connection(account);
	TwitterEndpointChatId *chat_id = twitter_endpoint_chat_id_new(endpoint_chat);
	long long since_id = twitter_connection_get_last_home_timeline_id(gc);
	if (since_id == 0)
	{
		purple_debug_info(TWITTER_PROTOCOL_ID, "Retrieving %s statuses for first time\n", account->username);
		twitter_api_get_home_timeline(account,
				since_id,
				20,
				1,
				twitter_get_home_timeline_cb,
				NULL,
				chat_id);
	} else {
		purple_debug_info(TWITTER_PROTOCOL_ID, "Retrieving %s statuses since %lld\n", account->username, since_id);
		twitter_api_get_home_timeline_all(account,
				since_id,
				twitter_get_home_timeline_all_cb,
				NULL,
				chat_id);
	}

	return TRUE;
}

static TwitterEndpointChatSettings TwitterEndpointTimelineSettings =
{
	TWITTER_CHAT_TIMELINE,
#if _HAZE_
	'!',
#endif
	twitter_chat_timeline_send, //send_message
	twitter_timeline_timeout_context_free, //endpoint_data_free
	twitter_option_search_timeout, //get_default_interval
	twitter_timeline_chat_name_from_components, //get_name
	NULL, //verify_components
	twitter_timeline_timeout,
	twitter_endpoint_timeline_interval_start,
	twitter_timeline_timeout_context_new,
};

TwitterEndpointChatSettings *twitter_endpoint_timeline_get_settings()
{
	return &TwitterEndpointTimelineSettings;
}

