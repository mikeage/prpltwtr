#include "prpltwtr_endpoint_reply.h"
#include "prpltwtr_util.h"

static void twitter_send_reply_success_cb(PurpleAccount * account, gpointer node, gboolean last, gpointer _who)
{
    if (last && _who)
        g_free(_who);
}

static gboolean twitter_send_reply_error_cb(PurpleAccount * account, const TwitterRequestErrorData * error_data, gpointer _who)
{
    //TODO: this doesn't work yet
    gchar          *who = _who;
    if (who) {
        gchar          *conv_name = twitter_endpoint_im_buddy_name_to_conv_name(twitter_endpoint_im_find(account, TWITTER_IM_TYPE_AT_MSG), _who);
        gchar          *error = g_strdup_printf(_("Error sending reply: %s"), error_data->message ? error_data->message : _("unknown error"));
        purple_conv_present_error(conv_name, account, error);
        g_free(error);
        g_free(who);
        g_free(conv_name);
    }

    return FALSE;                                //give up trying
}

static int twitter_send_reply_do(PurpleAccount * account, const char *who, const char *message, PurpleMessageFlags flags)
{
    PurpleConnection *gc = purple_account_get_connection(account);
    TwitterConnectionData *twitter = gc->proto_data;
    gchar          *added_text = g_strdup_printf("@%s", who);
    GArray         *statuses = twitter_utf8_get_segments(message, MAX_TWEET_LENGTH, added_text, TRUE);
    gchar          *in_reply_to_status_id = NULL;
    gchar          *conv_name = twitter_endpoint_im_buddy_name_to_conv_name(twitter_endpoint_im_find(account, TWITTER_IM_TYPE_AT_MSG), who);
    PurpleConversation *conv = purple_find_conversation_with_account(PURPLE_CONV_TYPE_IM, conv_name, account);

    /* The in_reply_to tweet is set according to the following logic:
     * 1. If the user Locked the reply, twitter_conv_last_reply_id_locked will be set (the value doesn't matter, but it's "(gpointer) 1", for those who care). In that case, we use the value of twitter_conv_last_reply_id, which was set by the same GtkPrplTwtr function that set locked. We don't unset it, because the lock means we want to keep it
     * 2. If the user did NOT lock the reply, we only want the current tweet to be a reply, but subsequent tweets in the same chat window should continue the thread via replying to the _latest_ incoming message. As such, we'll clear the value after we use it (actually, we store it in a different variable and clear it immediately).
     * 3. If the user did not specify a reply, we'll check twitter->user_reply_id_table for the last message received. This is updated on each reply in _process_replies.
     * 4. If twitter->user_reply_id_table is not set, it's probably our first message, so there's no conversation
     */
    if (conv) {
        gchar          *in_reply_to_status_id_tmp = NULL;
        in_reply_to_status_id_tmp = purple_conversation_get_data(conv, "twitter_conv_last_reply_id");
        if (in_reply_to_status_id_tmp) {
            in_reply_to_status_id = g_strdup(in_reply_to_status_id_tmp);
            if (!purple_conversation_get_data(conv, "twitter_conv_last_reply_id_locked")) {
                g_free(in_reply_to_status_id_tmp);
                purple_conversation_set_data(conv, "twitter_conv_last_reply_id", NULL);
                purple_signal_emit(purple_conversations_get_handle(), "prpltwtr-set-reply", conv, NULL);
            }
        }
    }

    if (!in_reply_to_status_id) {
        in_reply_to_status_id = g_strdup((gchar *) g_hash_table_lookup(twitter->user_reply_id_table, who));
    }

    twitter_api_set_statuses(purple_account_get_requestor(account), statuses, in_reply_to_status_id, twitter_send_reply_success_cb, twitter_send_reply_error_cb, g_strdup(who));    //TODO

    g_free(in_reply_to_status_id);

    g_free(conv_name);
    g_free(added_text);

    return 1;
}

typedef struct {
    void            (*success_cb) (PurpleAccount * account, gchar * id, gpointer user_data);
    void            (*error_cb) (PurpleAccount * account, const TwitterRequestErrorData * error_data, gpointer user_data);
    gpointer        user_data;
} TwitterLastSinceIdRequest;

static void twitter_get_replies_timeout_error_cb(PurpleAccount * account, const TwitterRequestErrorData * error_data, gpointer user_data)
{
    if (error_data->type != TWITTER_REQUEST_ERROR_CANCELED) {
        PurpleConnection *gc = purple_account_get_connection(account);
        TwitterConnectionData *twitter = gc->proto_data;
        twitter->failed_get_replies_count++;

        if (twitter->failed_get_replies_count >= 3) {
            purple_connection_error_reason(gc, PURPLE_CONNECTION_ERROR_NETWORK_ERROR, _("Could not retrieve replies, giving up trying"));
        }
    }
}

static gboolean twitter_get_replies_all_timeout_error_cb(TwitterRequestor * r, const TwitterRequestErrorData * error_data, gpointer user_data)
{
    twitter_get_replies_timeout_error_cb(r->account, error_data, user_data);
    return error_data->type != TWITTER_REQUEST_ERROR_CANCELED;  //restart timer and try again
}

static void _process_replies(PurpleAccount * account, GList * statuses, TwitterConnectionData * twitter)
{
    GList          *l;
    TwitterEndpointIm *ctx = twitter_connection_get_endpoint_im(twitter, TWITTER_IM_TYPE_AT_MSG);

    for (l = statuses; l; l = l->next) {
        TwitterUserTweet *data = l->data;
        TwitterTweet   *status = twitter_user_tweet_take_tweet(data);
        TwitterUserData *user_data = twitter_user_tweet_take_user_data(data);

        if (!user_data) {
            twitter_status_data_free(status);
        } else {
            gchar          *reply_id;
            twitter_buddy_set_user_data(account, user_data, FALSE);
            twitter_status_data_update_conv(ctx, data->screen_name, status);

            /* update user_reply_id_table table */
            reply_id = g_strdup_printf("%s", status->id);
            g_hash_table_insert(twitter->user_reply_id_table, g_strdup(data->screen_name), reply_id);

            twitter_buddy_set_status_data(account, data->screen_name, status);
        }
        twitter_user_tweet_free(data);
    }

    twitter->failed_get_replies_count = 0;
}

static void twitter_get_replies_all_cb(TwitterRequestor * r, GList * nodes, gpointer user_data)
{
    PurpleConnection *gc = purple_account_get_connection(r->account);
    TwitterConnectionData *twitter = gc->proto_data;

    GList          *statuses = twitter_statuses_nodes_parse(r, nodes);
    _process_replies(r->account, statuses, twitter);

    g_list_free(statuses);
}

static void twitter_get_replies_get_last_since_id_success_cb(TwitterRequestor * r, gpointer node, gpointer user_data)
{
    TwitterLastSinceIdRequest *last = user_data;

    gchar          *id = 0;

    /* In XML, this was inside a "status" node. TODO */
    /* gpointer       *status_node = r->format->get_node(node, "status"); *//* XML only */
    gpointer       *status_node = node;

    purple_debug_info(purple_account_get_protocol_id(r->account), "BEGIN: %s\n", G_STRFUNC);

    if (status_node != NULL) {
        TwitterTweet   *status_data = twitter_status_node_parse(r, status_node);
        if (status_data != NULL) {
            id = status_data->id;

            twitter_status_data_free(status_data);
        }
    }
    last->success_cb(r->account, id, last->user_data);
    g_free(last);
}

static void twitter_get_last_since_id_error_cb(TwitterRequestor * r, const TwitterRequestErrorData * error_data, gpointer user_data)
{
    TwitterLastSinceIdRequest *last = user_data;
    last->error_cb(r->account, error_data, last->user_data);
    g_free(last);
}

static void twitter_get_replies_last_since_id(PurpleAccount * account, void (*success_cb) (PurpleAccount * account, gchar * id, gpointer user_data), void (*error_cb) (PurpleAccount * account, const TwitterRequestErrorData * error_data, gpointer user_data), gpointer user_data)
{
    TwitterLastSinceIdRequest *request = g_new0(TwitterLastSinceIdRequest, 1);
    request->success_cb = success_cb;
    request->error_cb = error_cb;
    request->user_data = user_data;
    /* Simply get the last reply */
    twitter_api_get_replies(purple_account_get_requestor(account), 0, 1, 1, twitter_get_replies_get_last_since_id_success_cb, twitter_get_last_since_id_error_cb, request);
}

static void twitter_endpoint_reply_convo_closed(PurpleConversation * conv)
{
    gchar         **id;
    PurpleConnection *gc = NULL;
    TwitterConnectionData *twitter = NULL;
    if (!conv)
        return;
    g_free(purple_conversation_get_data(conv, "twitter_conv_last_reply_id"));
    purple_conversation_set_data(conv, "twitter_conv_last_reply_id", NULL);

    /* Don't continue to send replies (if someone messaged us) the next time this window is opened */
    gc = purple_conversation_get_gc(conv);
    if (gc) {
        twitter = gc->proto_data;
        g_hash_table_remove(twitter->user_reply_id_table, conv->name);
    }
}

PurpleConversation *twitter_endpoint_reply_conversation_new(TwitterEndpointIm * im, const gchar * buddy_name, gchar * reply_id, gboolean force)
{
    if (im) {
        gchar          *conv_name = twitter_endpoint_im_buddy_name_to_conv_name(im, buddy_name);
        PurpleConversation *conv = purple_conversation_new(PURPLE_CONV_TYPE_IM, im->account, conv_name);

        purple_debug_info(purple_account_get_protocol_id(im->account), "%s() conv %p (%s) %s replies to %s\n", G_STRFUNC, conv, conv_name, force ? "force" : "suggest", reply_id);

        if (conv) {
            if (force || !purple_conversation_get_data(conv, "twitter_conv_last_reply_id_locked")) {
                g_free(purple_conversation_get_data(conv, "twitter_conv_last_reply_id"));
                purple_conversation_set_data(conv, "twitter_conv_last_reply_id", NULL);

                if (reply_id) {
                    purple_conversation_set_data(conv, "twitter_conv_last_reply_id", g_strdup(reply_id));
                }
            }
        }
        g_free(conv_name);
        return conv;
    }
    return NULL;

}

static TwitterEndpointImSettings TwitterEndpointReplySettings = {
    TWITTER_IM_TYPE_AT_MSG,
    "twitter_last_reply_id",
    "@",                                         //conv_id
    twitter_send_reply_do,
    twitter_option_replies_timeout,
    twitter_api_get_replies_all,
    twitter_get_replies_all_cb,
    twitter_get_replies_all_timeout_error_cb,
    twitter_get_replies_last_since_id,
    twitter_endpoint_reply_convo_closed,         //convo_closed
};

TwitterEndpointImSettings *twitter_endpoint_reply_get_settings(void)
{
    return &TwitterEndpointReplySettings;
}
