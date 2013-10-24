#include "prpltwtr_endpoint_timeline.h"

//TODO: Should these be here?
const gchar * twitter_account_get_last_home_timeline_id(PurpleAccount * account)
{
	// DREM Discard the const gchar *
    gchar * results = (gchar *)purple_account_get_string(account, "twitter_last_home_timeline_id", NULL);
	purple_debug_info("prpltwtr", "%s: Get last ID: %s\n", G_STRFUNC, results);
	return results;
}

void twitter_account_set_last_home_timeline_id(PurpleAccount * account, gchar * reply_id)
{
	purple_debug_info("prpltwtr", "%s: Setting last ID to %s\n", G_STRFUNC, reply_id);
    purple_account_set_string(account, "twitter_last_home_timeline_id", reply_id);
}

const gchar * twitter_connection_get_last_home_timeline_id(PurpleConnection * gc)
{
    gchar *       reply_id = "0";
    TwitterConnectionData *connection_data = gc->proto_data;
    reply_id = connection_data->last_home_timeline_id;
    return (reply_id ? reply_id : twitter_account_get_last_home_timeline_id(purple_connection_get_account(gc)));
}

void twitter_connection_set_last_home_timeline_id(PurpleConnection * gc, gchar * reply_id)
{
    TwitterConnectionData *connection_data = gc->proto_data;

    connection_data->last_home_timeline_id = reply_id;
    twitter_account_set_last_home_timeline_id(purple_connection_get_account(gc), reply_id);
}

static gpointer twitter_timeline_timeout_context_new(GHashTable * components)
{
    TwitterTimelineTimeoutContext *ctx = g_slice_new0(TwitterTimelineTimeoutContext);
    ctx->timeline_id = 0;
    return ctx;
}

static void twitter_timeline_timeout_context_free(gpointer _ctx)
{
    TwitterTimelineTimeoutContext *ctx;
    g_return_if_fail(_ctx != NULL);
    ctx = _ctx;

    g_slice_free(TwitterTimelineTimeoutContext, ctx);
}

static char    *twitter_chat_name_from_timeline_id(const gint timeline_id)
{
    return g_strdup("Timeline: Home");
}

static char    *twitter_timeline_chat_name_from_components(GHashTable * components)
{
    return twitter_chat_name_from_timeline_id(0);
}

static void twitter_get_home_timeline_parse_statuses(TwitterEndpointChat * endpoint_chat, GList * statuses)
{
    PurpleConnection *gc;
    GList          *l;
    TwitterUserTweet *user_tweet;

    purple_debug_info(purple_account_get_protocol_id(endpoint_chat->account), "%s: begin\n", G_STRFUNC);

    g_return_if_fail(endpoint_chat != NULL);
    gc = purple_account_get_connection(endpoint_chat->account);

    if (!statuses) {
        /* At least update the topic with the new rate limit info */
		purple_debug_info(purple_account_get_protocol_id(endpoint_chat->account), "%s: No statuses\n", G_STRFUNC);
        twitter_chat_update_rate_limit(endpoint_chat);
        return;
    }
 
    purple_debug_info(purple_account_get_protocol_id(endpoint_chat->account), "%s: has status\n", G_STRFUNC);

	l = g_list_last(statuses);
    user_tweet = l->data;
    if (user_tweet && user_tweet->status)
    {
		purple_debug_info(purple_account_get_protocol_id(endpoint_chat->account), "%s: has tweet: %s\n", G_STRFUNC, user_tweet->status->text);

        if (user_tweet->status->id < twitter_connection_get_last_home_timeline_id(gc)) {
            purple_debug_info(purple_account_get_protocol_id(endpoint_chat->account), "Setting last as %s, although it's less than the previous %s\n", user_tweet->status->id, twitter_connection_get_last_home_timeline_id(gc));
        }

		purple_debug_info(purple_account_get_protocol_id(endpoint_chat->account), "%s: set last: %s\n", G_STRFUNC, user_tweet->status->id);
        twitter_connection_set_last_home_timeline_id(gc, user_tweet->status->id);
    }

	purple_debug_info(purple_account_get_protocol_id(endpoint_chat->account), "%s: twitter_chat_got_user_tweets\n", G_STRFUNC);
    twitter_chat_got_user_tweets(endpoint_chat, statuses);
}

static gboolean twitter_get_home_timeline_all_error_cb(TwitterRequestor * r, const TwitterRequestErrorData * error_data, gpointer user_data)
{
    TwitterEndpointChatId *chat_id = (TwitterEndpointChatId *) user_data;
    TwitterEndpointChat *endpoint_chat;

    purple_debug_warning(purple_account_get_protocol_id(r->account), "%s(%p): %s\n", G_STRFUNC, user_data, error_data->message);

    g_return_val_if_fail(chat_id != NULL, TRUE);
    endpoint_chat = twitter_endpoint_chat_find_by_id(chat_id);
    twitter_endpoint_chat_id_free(chat_id);

    if (endpoint_chat) {
        endpoint_chat->retrieval_in_progress = FALSE;
        endpoint_chat->retrieval_in_progress_timeout = 0;
    }

    return FALSE;                                /* Do not retry. Too many edge cases */
}

static void twitter_get_home_timeline_error_cb(TwitterRequestor * r, const TwitterRequestErrorData * error_data, gpointer user_data)
{
    twitter_get_home_timeline_all_error_cb(r, error_data, user_data);
    return;
}

static void twitter_get_home_timeline_cb(TwitterRequestor * r, gpointer node, gpointer user_data)
{
    TwitterEndpointChatId *chat_id = (TwitterEndpointChatId *) user_data;
    TwitterEndpointChat *endpoint_chat;
    GList          *statuses;

    purple_debug_info(purple_account_get_protocol_id(r->account), "BEGIN: %s\n", G_STRFUNC);

    g_return_if_fail(chat_id != NULL);
    endpoint_chat = twitter_endpoint_chat_find_by_id(chat_id);
    twitter_endpoint_chat_id_free(chat_id);

    if (endpoint_chat == NULL)
        return;

    endpoint_chat->rate_limit_remaining = r->rate_limit_remaining;
    endpoint_chat->rate_limit_total = r->rate_limit_total;

    endpoint_chat->retrieval_in_progress = FALSE;
    endpoint_chat->retrieval_in_progress_timeout = 0;

    statuses = twitter_statuses_node_parse(r, node);
    twitter_get_home_timeline_parse_statuses(endpoint_chat, statuses);

}

static void twitter_get_home_timeline_all_cb(TwitterRequestor * r, GList * nodes, gpointer user_data)
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
    endpoint_chat->retrieval_in_progress_timeout = 0;

    statuses = twitter_statuses_nodes_parse(r, nodes);
    twitter_get_home_timeline_parse_statuses(endpoint_chat, statuses);
}

static gboolean twitter_timeline_timeout(TwitterEndpointChat * endpoint_chat)
{
    PurpleAccount  *account = endpoint_chat->account;
    PurpleConnection *gc = purple_account_get_connection(account);
    TwitterEndpointChatId *chat_id = NULL;
    const gchar * since_id = twitter_connection_get_last_home_timeline_id(gc);

    purple_debug_info(purple_account_get_protocol_id(account), "BEGIN: %s %s\n", G_STRFUNC, account->username);

    if (endpoint_chat->retrieval_in_progress && endpoint_chat->retrieval_in_progress_timeout <= 0) {
        purple_debug_warning(purple_account_get_protocol_id(account), "There was a retrieval in progress, but it appears dead. Ignoring it\n");
        endpoint_chat->retrieval_in_progress = FALSE;
    }

    if (endpoint_chat->retrieval_in_progress) {
        purple_debug_warning(purple_account_get_protocol_id(account), "Skipping retreival for %s because one is already in progress!\n", account->username);
        endpoint_chat->retrieval_in_progress_timeout--;
        return TRUE;
    }

    chat_id = twitter_endpoint_chat_id_new(endpoint_chat);

    endpoint_chat->retrieval_in_progress = TRUE;
    endpoint_chat->retrieval_in_progress_timeout = 2;

	purple_debug_info("prpltwtr", "%s: preparing to send to twitter_send_format_request_multipage_cb: %s\n", G_STRFUNC, since_id);
	
    if (since_id == NULL || g_strcmp0("0", since_id) == 0) {
        purple_debug_info(purple_account_get_protocol_id(account), "%s: Retrieving %s statuses for first time\n", G_STRFUNC, gc->account->username);
		// DREM Discards const gchar *
        twitter_api_get_home_timeline(purple_account_get_requestor(account), (gchar *)since_id, TWITTER_HOME_TIMELINE_INITIAL_COUNT, 1, twitter_get_home_timeline_cb, twitter_get_home_timeline_error_cb, chat_id);
    } else {
        purple_debug_info(purple_account_get_protocol_id(account), "%s: Retrieving %s statuses since %s\n", G_STRFUNC, gc->account->username, since_id);
		// DREM Discard const gchar *
        twitter_api_get_home_timeline_all(purple_account_get_requestor(account), (gchar *)since_id, twitter_get_home_timeline_all_cb, twitter_get_home_timeline_all_error_cb, twitter_option_home_timeline_max_tweets(account), chat_id);
    }
    return TRUE;
}

static gboolean twitter_endpoint_timeline_interval_start(TwitterEndpointChat * endpoint_chat)
{
    return twitter_timeline_timeout(endpoint_chat);
}

static TwitterEndpointChatSettings TwitterEndpointTimelineSettings = {
    TWITTER_CHAT_TIMELINE,
#ifdef _HAZE_
    '!',
#endif
    NULL,                                        // append text; 
    twitter_timeline_timeout_context_free,       //endpoint_data_free
    twitter_option_timeline_timeout,             //get_default_interval
    twitter_timeline_chat_name_from_components,  //get_name
    NULL,                                        //verify_components
    twitter_timeline_timeout, twitter_endpoint_timeline_interval_start, twitter_timeline_timeout_context_new,
};

TwitterEndpointChatSettings *twitter_endpoint_timeline_get_settings(void)
{
    return &TwitterEndpointTimelineSettings;
}
