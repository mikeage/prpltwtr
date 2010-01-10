#include "twitter_endpoint_search.h"

static gpointer twitter_search_timeout_context_new(GHashTable *components)
{
	TwitterSearchTimeoutContext *ctx = g_slice_new0(TwitterSearchTimeoutContext);
	ctx->search_text = g_strdup(g_hash_table_lookup(components, "search"));
	ctx->refresh_url = NULL;
	return ctx;
}


static void twitter_search_timeout_context_free(gpointer _ctx)
{
	purple_debug_info(TWITTER_PROTOCOL_ID, "%s\n", G_STRFUNC);
	g_return_if_fail(_ctx != NULL);
	TwitterSearchTimeoutContext *ctx = _ctx;

	ctx->last_tweet_id = 0;

	purple_debug_info(TWITTER_PROTOCOL_ID, "%s %s\n", G_STRFUNC, ctx->search_text);
	g_free (ctx->search_text);
	ctx->search_text = NULL;

	purple_debug_info(TWITTER_PROTOCOL_ID, "%s %s\n", G_STRFUNC, ctx->refresh_url);
	g_free (ctx->refresh_url);
	ctx->refresh_url = NULL;

	g_slice_free (TwitterSearchTimeoutContext, ctx);
} 

static int twitter_chat_search_send(TwitterEndpointChat *ctx_base, const gchar *message)
{
	PurpleAccount *account = ctx_base->account;
	PurpleConnection *gc = purple_account_get_connection(account);
	TwitterSearchTimeoutContext *ctx = (TwitterSearchTimeoutContext *) ctx_base->endpoint_data;
	PurpleConversation *conv = twitter_chat_context_find_conv(ctx_base);
	char *status;
	char *message_lower, *search_text_lower;

	if (conv == NULL) return -1; //TODO: error?

	//TODO: verify that we want utf8 case insensitive, and not ascii
	message_lower = g_utf8_strdown(message, -1);
	search_text_lower = g_utf8_strdown(ctx->search_text, -1);
	if (strstr(message_lower, search_text_lower))
	{
		status = g_strdup(message);
	} else {
		status = g_strdup_printf("%s %s", ctx->search_text, message);
	}
	g_free(message_lower);
	g_free(search_text_lower);

	if (strlen(status) > MAX_TWEET_LENGTH)
	{
		//TODO: SHOW ERROR
		g_free(status);
		return -1;
	}
	else
	{
		PurpleAccount *account = purple_connection_get_account(gc);
		twitter_api_set_status(purple_connection_get_account(gc),
				status, 0,
				NULL, NULL,//TODO: verify & error
				NULL);
#if _HAZE_
		//It's already in the message box in haze. Maybe we should edit it before hand?
		//twitter_chat_add_tweet(PURPLE_CONV_IM(conv), account->username, status, 0, time(NULL));//TODO: FIX TIME
#else
		twitter_chat_add_tweet(PURPLE_CONV_CHAT(conv), account->username, status, 0, time(NULL));//TODO: FIX TIME
#endif
		g_free(status);
		return 0;
	}
	return -1;
}

static char *twitter_chat_name_from_search(const char *search)
{
#if _HAZE_
	return g_strdup_printf("#%s", search);
#else
	char *search_lower = g_utf8_strdown(search, -1);
	char *chat_name = g_strdup_printf("Search: %s", search_lower);
	g_free(search_lower);
	return chat_name;
#endif
}


static char *twitter_search_chat_name_from_components(GHashTable *components)
{
	const char *search = g_hash_table_lookup(components, "search");
	return twitter_chat_name_from_search(search);
}

static gchar *twitter_search_verify_components(GHashTable *components)
{
	const char *search = g_hash_table_lookup(components, "search");
	if (search == NULL || search[0] == '\0')
	{
		return g_strdup("Search must be filled in when joining a search chat");
	}
	return NULL;
}

static void twitter_search_cb(PurpleAccount *account,
		const GArray *search_results,
		const gchar *refresh_url,
		long long max_id,
		gpointer user_data)
{
	TwitterEndpointChatId *id = (TwitterEndpointChatId *) user_data;
	gint i, len = search_results->len;
#if _HAZE_
	PurpleConvIm *chat;
#else
	PurpleConvChat *chat;
#endif
	TwitterEndpointChat *endpoint_chat;
	TwitterSearchTimeoutContext *ctx;

	g_return_if_fail(id != NULL);

	purple_debug_info(TWITTER_PROTOCOL_ID, "%s, chat_name %s len: %d\n", G_STRFUNC, id->chat_name,  len);

	endpoint_chat = twitter_endpoint_chat_find_by_id(id);
	twitter_endpoint_chat_id_free(id);

	if (endpoint_chat == NULL)
	{
		purple_debug_info(TWITTER_PROTOCOL_ID, "%s, chat data went away\n", G_STRFUNC);
		return;
	}
	ctx = (TwitterSearchTimeoutContext *) endpoint_chat->endpoint_data;

	g_return_if_fail (ctx != NULL);
	//TODO add DEBUG stuff in case something breaks

	if (len)
	{
		chat = twitter_endpoint_chat_get_conv(endpoint_chat);
		if (chat)
		{
			purple_debug_info(TWITTER_PROTOCOL_ID, "%s found chat %s, adding tweets\n", G_STRFUNC, endpoint_chat->chat_name);
			for (i = len-1; i >= 0; i--) {
				TwitterSearchData *search_data;

				search_data = g_array_index (search_results,
						TwitterSearchData *, i);

				twitter_chat_add_tweet(chat, search_data->from_user, search_data->text, search_data->id, search_data->created_at);
			}
		} else {
			purple_debug_info(TWITTER_PROTOCOL_ID, "%s could not find chat %s", G_STRFUNC, endpoint_chat->chat_name);
			//destroy context
			return;
		}
	}

	ctx->last_tweet_id = max_id;
	g_free (ctx->refresh_url);
	ctx->refresh_url = g_strdup (refresh_url);
}

static gboolean twitter_endpoint_search_interval_start(TwitterEndpointChat *endpoint)
{
	TwitterSearchTimeoutContext *ctx = endpoint->endpoint_data;
	TwitterEndpointChatId *id = twitter_endpoint_chat_id_new(endpoint);
	twitter_api_search(endpoint->account,
			ctx->search_text, ctx->last_tweet_id,
			TWITTER_SEARCH_RPP_DEFAULT,
			twitter_search_cb, NULL, id);
	return TRUE;
}

static gboolean twitter_search_timeout(TwitterEndpointChat *endpoint_chat)
{
	TwitterSearchTimeoutContext *ctx = (TwitterSearchTimeoutContext *)endpoint_chat->endpoint_data;
	TwitterEndpointChatId *id = twitter_endpoint_chat_id_new(endpoint_chat);

	if (ctx->refresh_url) {
		purple_debug_info(TWITTER_PROTOCOL_ID, "%s, refresh_url exists: %s\n",
				G_STRFUNC, ctx->refresh_url);

		twitter_api_search_refresh(endpoint_chat->account, ctx->refresh_url,
				twitter_search_cb, NULL, id);
	}
	else {
		gchar *refresh_url;

		refresh_url = g_strdup_printf ("?q=%s&since_id=%lld",
				purple_url_encode(ctx->search_text), ctx->last_tweet_id);

		purple_debug_info(TWITTER_PROTOCOL_ID, "%s, create refresh_url: %s\n",
				G_STRFUNC, refresh_url);

		twitter_api_search_refresh (endpoint_chat->account, refresh_url,
				twitter_search_cb, NULL, id);

		g_free (refresh_url);
	}

	return TRUE;
}

static TwitterEndpointChatSettings TwitterEndpointSearchSettings =
{
	TWITTER_CHAT_SEARCH,
#if _HAZE_
	'#',
#endif
	twitter_chat_search_send, //send_message
	twitter_search_timeout_context_free, //endpoint_data_free
	twitter_option_timeline_timeout, //get_default_interval
	twitter_search_chat_name_from_components, //get_name
	twitter_search_verify_components, //verify_components
	twitter_search_timeout, //interval_timeout
	twitter_endpoint_search_interval_start,
	twitter_search_timeout_context_new,
};

TwitterEndpointChatSettings *twitter_endpoint_search_get_settings()
{
	return &TwitterEndpointSearchSettings;
}

