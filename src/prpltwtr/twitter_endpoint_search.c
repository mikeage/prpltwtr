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
	TwitterSearchTimeoutContext *ctx;
	purple_debug_info(TWITTER_PROTOCOL_ID, "%s\n", G_STRFUNC);
	g_return_if_fail(_ctx != NULL);
	ctx = _ctx;

	ctx->last_tweet_id = 0;

	purple_debug_info(TWITTER_PROTOCOL_ID, "%s %s\n", G_STRFUNC, ctx->search_text);
	g_free (ctx->search_text);
	ctx->search_text = NULL;

	purple_debug_info(TWITTER_PROTOCOL_ID, "%s %s\n", G_STRFUNC, ctx->refresh_url);
	g_free (ctx->refresh_url);
	ctx->refresh_url = NULL;

	g_slice_free (TwitterSearchTimeoutContext, ctx);
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
		GList *search_results,
		const gchar *refresh_url,
		long long max_id,
		gpointer user_data)
{
	TwitterEndpointChatId *id = (TwitterEndpointChatId *) user_data;
	TwitterEndpointChat *endpoint_chat;
	TwitterSearchTimeoutContext *ctx;

	g_return_if_fail(id != NULL);

	purple_debug_info(TWITTER_PROTOCOL_ID, "%s, chat_name %s\n", G_STRFUNC, id->chat_name);

	endpoint_chat = twitter_endpoint_chat_find_by_id(id);
	twitter_endpoint_chat_id_free(id);

	if (endpoint_chat == NULL)
	{
		purple_debug_info(TWITTER_PROTOCOL_ID, "%s, chat data went away\n", G_STRFUNC);
		return;
	}
	ctx = (TwitterSearchTimeoutContext *) endpoint_chat->endpoint_data;

	g_return_if_fail (ctx != NULL);

	if (search_results)
	{
		twitter_chat_got_user_tweets(endpoint_chat, search_results);
	}

	ctx->last_tweet_id = max_id;
	g_free (ctx->refresh_url);
	ctx->refresh_url = g_strdup (refresh_url);
}

static gboolean twitter_endpoint_search_interval_start(TwitterEndpointChat *endpoint)
{
	TwitterSearchTimeoutContext *ctx = endpoint->endpoint_data;
	TwitterEndpointChatId *id = twitter_endpoint_chat_id_new(endpoint);
	twitter_api_search(purple_account_get_requestor(endpoint->account),
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

		twitter_api_search_refresh(purple_account_get_requestor(endpoint_chat->account), ctx->refresh_url,
				twitter_search_cb, NULL, id);
	}
	else {
		gchar *refresh_url;

		refresh_url = g_strdup_printf ("?q=%s&since_id=%lld",
				purple_url_encode(ctx->search_text), ctx->last_tweet_id);

		purple_debug_info(TWITTER_PROTOCOL_ID, "%s, create refresh_url: %s\n",
				G_STRFUNC, refresh_url);

		twitter_api_search_refresh (purple_account_get_requestor(endpoint_chat->account), refresh_url,
				twitter_search_cb, NULL, id);

		g_free (refresh_url);
	}

	return TRUE;
}
static gchar *twitter_endpoint_search_get_status_added_text(TwitterEndpointChat *endpoint_chat)
{
	TwitterSearchTimeoutContext *ctx = (TwitterSearchTimeoutContext *) endpoint_chat->endpoint_data;
	return g_strdup(ctx->search_text);
}


static TwitterEndpointChatSettings TwitterEndpointSearchSettings =
{
	TWITTER_CHAT_SEARCH,
#if _HAZE_
	'#',
#endif
	twitter_endpoint_search_get_status_added_text,
	twitter_search_timeout_context_free, //endpoint_data_free
	twitter_option_search_timeout, //get_default_interval
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

