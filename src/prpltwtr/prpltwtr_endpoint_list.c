#include "prpltwtr_endpoint_list.h"

static gpointer twitter_list_timeout_context_new(GHashTable * components)
{
    TwitterListTimeoutContext *ctx = g_slice_new0(TwitterListTimeoutContext);
    ctx->list_name = g_strdup(g_hash_table_lookup(components, "list_name"));
    ctx->list_id = g_strdup(g_hash_table_lookup(components, "list_id"));
    ctx->owner = g_strdup(g_hash_table_lookup(components, "owner"));
    return ctx;
}

static void twitter_list_timeout_context_free(gpointer _ctx)
{
    TwitterListTimeoutContext *ctx;
    g_return_if_fail(_ctx != NULL);
    ctx = _ctx;

    ctx->last_tweet_id = 0;

    purple_debug_info(GENERIC_PROTOCOL_ID, "%s %s\n", G_STRFUNC, ctx->list_name);
    g_free(ctx->list_name);
    ctx->list_name = NULL;

    purple_debug_info(GENERIC_PROTOCOL_ID, "%s %s\n", G_STRFUNC, ctx->list_id);
    g_free(ctx->list_id);
    ctx->list_id = NULL;

    g_free(ctx->owner);
    ctx->owner = NULL;
    g_slice_free(TwitterListTimeoutContext, ctx);
}

static char    *twitter_chat_name_from_list(const char *list_name)
{
#ifdef _HAZE_
    /* This is broken TODO */
    return g_strdup_printf("#%s", search);
#else
    char           *list_lower = g_utf8_strdown(list_name, -1);
    char           *chat_name = g_strdup_printf("List: %s", list_lower);
    g_free(list_lower);
    return chat_name;
#endif
}

static char    *twitter_list_chat_name_from_components(GHashTable * components)
{
    const char     *list = g_hash_table_lookup(components, "list_name");
    return twitter_chat_name_from_list(list);
}

static void twitter_get_list_parse_statuses(TwitterEndpointChat * endpoint_chat, GList * statuses)
{
    PurpleConnection *gc;
    GList          *l;
    TwitterUserTweet *user_tweet;

    purple_debug_info(purple_account_get_protocol_id(endpoint_chat->account), "%s\n", G_STRFUNC);

    g_return_if_fail(endpoint_chat != NULL);
    gc = purple_account_get_connection(endpoint_chat->account);

    if (!statuses) {
        /* At least update the topic with the new rate limit info */
        twitter_chat_update_rate_limit(endpoint_chat);
        return;
    }

    l = g_list_last(statuses);
    user_tweet = l->data;
    if (user_tweet && user_tweet->status)
        /* Tweets might not be sequential anymore. Take since_id from the last one, not the greatest */
#if 0
        &&user_tweet->status->id > twitter_connection_get_last_home_timeline_id(gc)
#endif                       /* 0 */
    {
        TwitterListTimeoutContext *ctx = endpoint_chat->endpoint_data;
        gchar          *key = g_strdup_printf("list_%s", ctx->list_name);
        ctx->last_tweet_id = user_tweet->status->id;
        purple_account_set_long_long(endpoint_chat->account, key, ctx->last_tweet_id);
        g_free(key);
    }
    twitter_chat_got_user_tweets(endpoint_chat, statuses);
}

static gboolean twitter_get_list_all_error_cb(TwitterRequestor * r, const TwitterRequestErrorData * error_data, gpointer user_data)
{
    TwitterEndpointChatId *chat_id = (TwitterEndpointChatId *) user_data;
    TwitterEndpointChat *endpoint_chat;

    purple_debug_warning(purple_account_get_protocol_id(r->account), "%s(): %s\n", G_STRFUNC, error_data->message);

    g_return_val_if_fail(chat_id != NULL, TRUE);
    endpoint_chat = twitter_endpoint_chat_find_by_id(chat_id);
    twitter_endpoint_chat_id_free(chat_id);

    if (endpoint_chat) {
        endpoint_chat->retrieval_in_progress = FALSE;
    }

    return TRUE;
}

static void twitter_get_list_error_cb(TwitterRequestor * r, const TwitterRequestErrorData * error_data, gpointer user_data)
{
    twitter_get_list_all_error_cb(r, error_data, user_data);
    return;
}

static void twitter_get_list_cb(TwitterRequestor * r, xmlnode * node, gpointer user_data)
{
    TwitterEndpointChatId *chat_id = (TwitterEndpointChatId *) user_data;
    TwitterEndpointChat *endpoint_chat;
    GList          *statuses;

    purple_debug_info(purple_account_get_protocol_id(r->account), "%s\n", G_STRFUNC);

    g_return_if_fail(chat_id != NULL);
    endpoint_chat = twitter_endpoint_chat_find_by_id(chat_id);
    twitter_endpoint_chat_id_free(chat_id);

    if (endpoint_chat == NULL)
        return;

    endpoint_chat->rate_limit_remaining = r->rate_limit_remaining;
    endpoint_chat->rate_limit_total = r->rate_limit_total;

    endpoint_chat->retrieval_in_progress = FALSE;

    statuses = twitter_statuses_node_parse(node);
    twitter_get_list_parse_statuses(endpoint_chat, statuses);

}

static void twitter_get_list_all_cb(TwitterRequestor * r, GList * nodes, gpointer user_data)
{
    TwitterEndpointChatId *chat_id = (TwitterEndpointChatId *) user_data;
    TwitterEndpointChat *endpoint_chat;
    GList          *statuses;

    purple_debug_info(purple_account_get_protocol_id(r->account), "%s\n", G_STRFUNC);

    g_return_if_fail(chat_id != NULL);
    endpoint_chat = twitter_endpoint_chat_find_by_id(chat_id);
    twitter_endpoint_chat_id_free(chat_id);

    if (endpoint_chat == NULL)
        return;

    endpoint_chat->rate_limit_remaining = r->rate_limit_remaining;
    endpoint_chat->rate_limit_total = r->rate_limit_total;

    endpoint_chat->retrieval_in_progress = FALSE;

    statuses = twitter_statuses_nodes_parse(nodes);
    twitter_get_list_parse_statuses(endpoint_chat, statuses);
}

static gboolean twitter_list_timeout(TwitterEndpointChat * endpoint_chat)
{
    PurpleAccount  *account = endpoint_chat->account;
    TwitterListTimeoutContext *ctx = endpoint_chat->endpoint_data;
    TwitterEndpointChatId *chat_id = twitter_endpoint_chat_id_new(endpoint_chat);
    gchar          *key = g_strdup_printf("list_%s", ctx->list_name);

    ctx->last_tweet_id = purple_account_get_long_long(endpoint_chat->account, key, -1);
    g_free(key);

    purple_debug_info(purple_account_get_protocol_id(account), "Resuming list for %s from %lld\n", ctx->list_name, ctx->last_tweet_id);

    if (endpoint_chat->retrieval_in_progress) {
        purple_debug_warning(purple_account_get_protocol_id(account), "Skipping retreival for %s because one is already in progress!\n", account->username);
        return TRUE;
    }

    endpoint_chat->retrieval_in_progress = TRUE;

    if (ctx->last_tweet_id == 0) {
        purple_debug_info(purple_account_get_protocol_id(account), "Retrieving %s statuses for first time\n", ctx->list_name);
        twitter_api_get_list(purple_account_get_requestor(account), ctx->list_id, ctx->owner, ctx->last_tweet_id, TWITTER_LIST_INITIAL_COUNT, 1, twitter_get_list_cb, twitter_get_list_error_cb, chat_id);
    } else {
        purple_debug_info(purple_account_get_protocol_id(account), "Retrieving %s statuses since %lld\n", ctx->list_name, ctx->last_tweet_id);
        twitter_api_get_list_all(purple_account_get_requestor(account), ctx->list_id, ctx->owner, ctx->last_tweet_id, twitter_get_list_all_cb, twitter_get_list_all_error_cb, twitter_option_list_max_tweets(account), chat_id);
    }

    return TRUE;
}

static gboolean twitter_endpoint_list_interval_start(TwitterEndpointChat * endpoint_chat)
{
    return twitter_list_timeout(endpoint_chat);
}

static TwitterEndpointChatSettings TwitterEndpointListSettings = {
    TWITTER_CHAT_LIST,
#ifdef _HAZE_
    '*',
#endif
    NULL, twitter_list_timeout_context_free,     //endpoint_data_free
    twitter_option_list_timeout,                 //get_default_interval
    twitter_list_chat_name_from_components,      //get_name
    NULL,                                        //verify_components TODO: only in search? 
    twitter_list_timeout, twitter_endpoint_list_interval_start, twitter_list_timeout_context_new,
};

TwitterEndpointChatSettings *twitter_endpoint_list_get_settings(void)
{
    return &TwitterEndpointListSettings;
}
