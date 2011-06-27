#include "prpltwtr_endpoint_chat.h"
#include "prpltwtr_buddy.h"

#include "prpltwtr_endpoint_search.h"
#include "prpltwtr_endpoint_timeline.h"
#include "prpltwtr_endpoint_list.h"

static TwitterEndpointChatSettings *TwitterEndpointChatSettingsLookup[TWITTER_CHAT_UNKNOWN];

static void twitter_init_endpoint_chat_settings(TwitterEndpointChatSettings * settings)
{
    TwitterEndpointChatSettingsLookup[settings->type] = settings;
}

TwitterEndpointChatSettings *twitter_get_endpoint_chat_settings(TwitterChatType type)
{
    if (type < TWITTER_CHAT_UNKNOWN)
        return TwitterEndpointChatSettingsLookup[type];
    return NULL;
}

static gint twitter_get_next_chat_id(PurpleAccount * account)
{
    PurpleConnection *gc = purple_account_get_connection(account);
    TwitterConnectionData *twitter = gc->proto_data;
    return ++twitter->chat_id;
}

void twitter_endpoint_chat_init(const char *protocol_id)
{
    twitter_init_endpoint_chat_settings(twitter_endpoint_search_get_settings());
    twitter_init_endpoint_chat_settings(twitter_endpoint_timeline_get_settings());
    if (!strcmp(protocol_id, TWITTER_PROTOCOL_ID)) {
        twitter_init_endpoint_chat_settings(twitter_endpoint_list_get_settings());
    }
}

void twitter_endpoint_chat_free(TwitterEndpointChat * ctx)
{
    PurpleConnection *gc;
    GList          *l;

    if (ctx->settings && ctx->settings->endpoint_data_free)
        ctx->settings->endpoint_data_free(ctx->endpoint_data);
    gc = purple_account_get_connection(ctx->account);

    if (ctx->timer_handle) {
        purple_timeout_remove(ctx->timer_handle);
        ctx->timer_handle = 0;
    }
    if (ctx->chat_name) {
        g_free(ctx->chat_name);
        ctx->chat_name = NULL;
    }

    for (l = ctx->sent_tweet_ids; l; l = l->next)
        g_free(l->data);
    g_list_free(ctx->sent_tweet_ids);
    g_slice_free(TwitterEndpointChat, ctx);
}

TwitterEndpointChat *twitter_endpoint_chat_new(TwitterEndpointChatSettings * settings, TwitterChatType type, PurpleAccount * account, const gchar * chat_name, GHashTable * components)
{
    TwitterEndpointChat *ctx = g_slice_new0(TwitterEndpointChat);
    ctx->settings = settings;
    ctx->type = type;
    ctx->account = account;
    ctx->chat_name = g_strdup(chat_name);
    ctx->endpoint_data = settings->create_endpoint_data ? settings->create_endpoint_data(components) : NULL;

    return ctx;
}

TwitterEndpointChatId *twitter_endpoint_chat_id_new(TwitterEndpointChat * chat)
{
    TwitterEndpointChatId *chat_id;
    g_return_val_if_fail(chat != NULL, NULL);

    chat_id = g_slice_new0(TwitterEndpointChatId);
    chat_id->account = chat->account;
    chat_id->chat_name = g_strdup(chat->chat_name);
    return chat_id;
}

void twitter_endpoint_chat_id_free(TwitterEndpointChatId * chat_id)
{
    if (chat_id == NULL)
        return;
    g_free(chat_id->chat_name);
    chat_id->chat_name = NULL;

    g_slice_free(TwitterEndpointChatId, chat_id);
}

TwitterEndpointChat *twitter_endpoint_chat_find_by_id(TwitterEndpointChatId * chat_id)
{
    g_return_val_if_fail(chat_id != NULL, NULL);
    return twitter_endpoint_chat_find(chat_id->account, chat_id->chat_name);
}

static PurpleConversation *twitter_endpoint_chat_find_open_conv(TwitterEndpointChat * ctx)
{
#ifdef _HAZE_
    return purple_find_conversation_with_account(PURPLE_CONV_TYPE_IM, ctx->chat_name, ctx->account);
#else
    return purple_find_conversation_with_account(PURPLE_CONV_TYPE_CHAT, ctx->chat_name, ctx->account);
#endif
}

//Taken mostly from blist.c
//XXX
static PurpleChat *_twitter_blist_chat_find(PurpleAccount * account, TwitterChatType chat_type, const char *component_key, const char *component_value)
{
    const char     *node_chat_name;
    gint            node_chat_type = 0;
    const char     *node_chat_type_str;
    PurpleChat     *chat;
    PurpleBlistNode *node,
                   *group;
    char           *normname;
    PurpleBuddyList *purplebuddylist = purple_get_blist();
    GHashTable     *components;

    g_return_val_if_fail(purplebuddylist != NULL, NULL);
    g_return_val_if_fail((component_value != NULL) && (*component_value != '\0'), NULL);

    normname = g_strdup(purple_normalize(account, component_value));
    purple_debug_info(purple_account_get_protocol_id(account), "Account %s searching for chat %s type %d\n", account->username, component_value == NULL ? "NULL" : component_value, chat_type);

    if (normname == NULL) {
        return NULL;
    }
    for (group = purplebuddylist->root; group != NULL; group = group->next) {
        for (node = group->child; node != NULL; node = node->next) {
            if (PURPLE_BLIST_NODE_IS_CHAT(node)) {

                chat = (PurpleChat *) node;

                if (account != chat->account)
                    continue;

                components = purple_chat_get_components(chat);
                if (components != NULL) {
                    node_chat_type_str = g_hash_table_lookup(components, "chat_type");
                    node_chat_name = g_hash_table_lookup(components, component_key);
                    node_chat_type = node_chat_type_str == NULL ? 0 : strtol(node_chat_type_str, NULL, 10);

                    if (node_chat_name != NULL && node_chat_type == chat_type && !strcmp(purple_normalize(account, node_chat_name), normname)) {
                        g_free(normname);
                        return chat;
                    }
                }
            }
        }
    }

    g_free(normname);
    return NULL;
}

//XXX
PurpleChat     *twitter_blist_chat_find_search(PurpleAccount * account, const char *name)
{
    PurpleChat     *chat = _twitter_blist_chat_find(account, TWITTER_CHAT_SEARCH, "search", name);
    return chat;
}

//XXX
PurpleChat     *twitter_blist_chat_find_timeline(PurpleAccount * account, gint timeline_id)
{
    char           *tmp = g_strdup_printf("%d", timeline_id);
    PurpleChat     *chat = _twitter_blist_chat_find(account, TWITTER_CHAT_TIMELINE, "timeline_id", tmp);
    g_free(tmp);
    return chat;
}

PurpleChat     *twitter_blist_chat_find_list(PurpleAccount * account, const char *name)
{
    PurpleChat     *chat = _twitter_blist_chat_find(account, TWITTER_CHAT_LIST, "list_name", name);
    return chat;
}

//XXX TODO: fix me
PurpleChat     *twitter_blist_chat_find(PurpleAccount * account, const char *name)
{
    static char    *timeline = "Timeline: ";
    static char    *search = "Search: ";
    static char    *list = "List: ";
    PurpleChat     *c = NULL;
    if (strlen(name) > strlen(timeline) && !strncmp(timeline, name, strlen(timeline))) {
        c = twitter_blist_chat_find_timeline(account, 0);
    } else if (strlen(name) > strlen(search) && !strncmp(search, name, strlen(search))) {
        c = twitter_blist_chat_find_search(account, name + strlen(search));
    } else if (strlen(name) > strlen(list) && !strncmp(list, name, strlen(list))) {
        c = twitter_blist_chat_find_list(account, name + strlen(list));
    } else {
        purple_debug_error(purple_account_get_protocol_id(account), "Invalid call to %s; assuming \"search\" for %s\n", G_STRFUNC, name);
        c = twitter_blist_chat_find_search(account, name);
    }
    return c;
}

TWITTER_ATTACH_SEARCH_TEXT twitter_blist_chat_attach_search_text(PurpleChat * chat)
{
    GHashTable     *components;
    char           *attach_search_text_str;
    TWITTER_ATTACH_SEARCH_TEXT attach_search_text = TWITTER_ATTACH_SEARCH_TEXT_NONE;
    g_return_val_if_fail(chat != NULL, FALSE);
    components = purple_chat_get_components(chat);
    attach_search_text_str = g_hash_table_lookup(components, "attach_search_text");
    if (attach_search_text_str != NULL) {
        attach_search_text = (TWITTER_ATTACH_SEARCH_TEXT) strtol(attach_search_text_str, NULL, 10);
    }
    return attach_search_text;
}

//TODO should be static?
gboolean twitter_blist_chat_is_auto_open(PurpleChat * chat)
{
    GHashTable     *components;
    char           *auto_open;
    g_return_val_if_fail(chat != NULL, FALSE);
    components = purple_chat_get_components(chat);
    auto_open = g_hash_table_lookup(components, "auto_open");
    return (auto_open != NULL && auto_open[0] != '0');
}

static void twitter_chat_add_tweet(PurpleConversation * conv, const char *who, const char *message, long long id, time_t time, long long in_reply_to_status_id, gboolean favorited)
{
    gchar          *tweet;
#ifndef _HAZE_
    PurpleConvChat *chat;
#endif

    purple_debug_info(purple_account_get_protocol_id(purple_conversation_get_account(conv)), "%s\n", G_STRFUNC);

#ifndef _HAZE_
    chat = purple_conversation_get_chat_data(conv);
    g_return_if_fail(chat != NULL);
#endif
    g_return_if_fail(conv != NULL);
    g_return_if_fail(who != NULL);
    g_return_if_fail(message != NULL);

    tweet = twitter_format_tweet(purple_conversation_get_account(conv), who, message, id, PURPLE_CONV_TYPE_CHAT, purple_conversation_get_name(conv), TRUE, in_reply_to_status_id, favorited);
#ifdef _HAZE_
    //This isn't in twitter_Format_tweet because we can't distinguish between a im and a chat
    gchar          *tweet2 = g_strdup_printf("%s: %s", who, tweet);
    g_free(tweet);
    tweet = tweet2;
    serv_got_im(purple_conversation_get_gc(conv), conv->name, tweet, PURPLE_MESSAGE_RECV, time);
#else
    if (!purple_conv_chat_find_user(chat, who)) {
        purple_debug_info(purple_account_get_protocol_id(purple_conversation_get_account(conv)), "added %s to chat %s\n", who, purple_conversation_get_name(conv));
        purple_conv_chat_add_user(chat, who, NULL,  /* user-provided join message, IRC style */
                                  PURPLE_CBFLAGS_NONE, FALSE);  /* show a join message */
    }
    purple_debug_info(purple_account_get_protocol_id(purple_conversation_get_account(conv)), "message %s\n", message);
    serv_got_chat_in(purple_conversation_get_gc(conv), purple_conv_chat_get_id(chat), who, PURPLE_MESSAGE_RECV, tweet, time);
#endif
    g_free(tweet);
}

static PurpleConversation *twitter_endpoint_chat_get_conv(TwitterEndpointChat * endpoint_chat)
{
#ifdef _HAZE_
    return twitter_endpoint_chat_find_open_conv(endpoint_chat);
#else
    PurpleConversation *conv = twitter_endpoint_chat_find_open_conv(endpoint_chat);
    PurpleChat     *blist_chat;
    if (conv == NULL && (blist_chat = twitter_blist_chat_find(endpoint_chat->account, endpoint_chat->chat_name))) {
        if (twitter_blist_chat_is_auto_open(blist_chat)) {
            guint           chat_id;
            purple_debug_info(purple_account_get_protocol_id(endpoint_chat->account), "%s, recreated conv for auto open chat (%s)\n", G_STRFUNC, endpoint_chat->chat_name);
            chat_id = twitter_get_next_chat_id(endpoint_chat->account);
            conv = serv_got_joined_chat(purple_account_get_connection(endpoint_chat->account), chat_id, endpoint_chat->chat_name);
        }
    }
    return conv;
#endif
}

void twitter_chat_got_tweet(TwitterEndpointChat * endpoint_chat, TwitterUserTweet * tweet)
{
    PurpleConversation *conv = twitter_endpoint_chat_get_conv(endpoint_chat);
    g_return_if_fail(conv != NULL);
    g_return_if_fail(tweet != NULL);
    g_return_if_fail(tweet->screen_name != NULL);
    g_return_if_fail(tweet->status != NULL);

    purple_signal_emit(purple_buddy_icons_get_handle(), "prpltwtr-update-iconurl", purple_conversation_get_account(conv), tweet->screen_name, tweet->icon_url, tweet->status->created_at);

    twitter_chat_add_tweet(conv, tweet->screen_name, tweet->status->text, tweet->status->id, tweet->status->created_at, tweet->status->in_reply_to_status_id, tweet->status->favorited);
}

static gboolean twitter_sent_tweets_contains_id(TwitterEndpointChat * ctx, long long id)
{
    GList          *l;
    for (l = ctx->sent_tweet_ids; l; l = l->next) {
        long long      *el = l->data;
        if (*el == id)
            return TRUE;
        else if (*el > id)
            return FALSE;
    }
    return FALSE;
}

//Removes all tweet id before id
static void twitter_sent_tweets_ids_remove_before(TwitterEndpointChat * ctx, long long id)
{
    while (ctx->sent_tweet_ids && *((long long *) ctx->sent_tweet_ids->data) <= id) {
        g_free(ctx->sent_tweet_ids->data);
        ctx->sent_tweet_ids = g_list_delete_link(ctx->sent_tweet_ids, ctx->sent_tweet_ids);
    }
}

void twitter_chat_update_rate_limit(TwitterEndpointChat * endpoint_chat)
{
    PurpleConversation *conv = twitter_endpoint_chat_find_open_conv(endpoint_chat);
//  PurpleConversation *conv = twitter_endpoint_chat_get_conv(endpoint_chat);

    if (!endpoint_chat)
        return;

    if (conv) {
        if (endpoint_chat->rate_limit_total) {
            gchar          *status;
            gchar           rate_limit_graph[] = "--------------------";
            int             num;
            num = (sizeof (rate_limit_graph) - 1) * 100 * endpoint_chat->rate_limit_remaining / endpoint_chat->rate_limit_total / 100;
            memset(rate_limit_graph, '>', num);

            status = g_strdup_printf("Rate limit: %d/%d [%s]", endpoint_chat->rate_limit_remaining, endpoint_chat->rate_limit_total, rate_limit_graph);
            purple_conv_chat_set_topic(PURPLE_CONV_CHAT(conv), "system", status);
            purple_debug_info(purple_account_get_protocol_id(purple_conversation_get_account(conv)), "Setting title to %s for conv=%p\n", status, conv);

            g_free(status);
        }
    }
}

void twitter_chat_got_user_tweets(TwitterEndpointChat * endpoint_chat, GList * user_tweets)
{
    PurpleAccount  *account;
    GList          *l;
    long long       max_id = 0;

    g_return_if_fail(endpoint_chat != NULL);

    account = endpoint_chat->account;

    if (user_tweets) {

        l = g_list_last(user_tweets);
        max_id = ((TwitterUserTweet *) l->data)->status->id;
        for (l = user_tweets; l; l = l->next) {
            TwitterUserTweet *user_tweet = l->data;
            TwitterUserData *user = twitter_user_tweet_take_user_data(user_tweet);
            TwitterTweet   *status;

            if (user)
                twitter_buddy_set_user_data(account, user, FALSE);

            /* This could be more efficient */
            if (!twitter_sent_tweets_contains_id(endpoint_chat, user_tweet->status->id))
                twitter_chat_got_tweet(endpoint_chat, user_tweet);

            status = twitter_user_tweet_take_tweet(user_tweet);
            twitter_buddy_set_status_data(account, user_tweet->screen_name, status);

            twitter_user_tweet_free(user_tweet);
        }
        twitter_sent_tweets_ids_remove_before(endpoint_chat, max_id);
        g_list_free(user_tweets);
    }

    twitter_chat_update_rate_limit(endpoint_chat);
}

static int _tweet_id_compare(long long *a, long long *b)
{
    if (*a < *b)
        return -1;
    else if (*a == *b)
        return 0;
    else
        return 1;
}

static void twitter_add_sent_tweet_id(TwitterEndpointChat * endpoint_chat, long long tweet_id)
{
    long long      *p = g_new(long long, 1);
    *p = tweet_id;
    endpoint_chat->sent_tweet_ids = g_list_insert_sorted(endpoint_chat->sent_tweet_ids, p, (GCompareFunc) _tweet_id_compare);
}

static gboolean twitter_endpoint_chat_interval_timeout(gpointer data)
{
    TwitterEndpointChat *endpoint = data;
    if (endpoint->settings->interval_timeout)
        return endpoint->settings->interval_timeout(endpoint);
    return FALSE;
}

void twitter_endpoint_chat_start(PurpleConnection * gc, TwitterEndpointChatSettings * settings, GHashTable * components, gboolean open_conv)
{
    const char     *interval_str = g_hash_table_lookup(components, "interval");
    int             interval = 0;
    PurpleAccount  *account;
    int             default_interval;
    gchar          *error;
    char           *chat_name;

    g_return_if_fail(settings != NULL);

    interval = interval_str == NULL ? 0 : strtol(interval_str, NULL, 10);

    account = purple_connection_get_account(gc);
    default_interval = settings->get_default_interval(account);
    error = NULL;

    if (settings->verify_components && (error = settings->verify_components(components))) {
        purple_notify_info(gc,                   /* plugin handle or PurpleConnection */
                           _("Chat Join"), _("Invalid Chat"), (error));
        g_free(error);
        return;
    }

    if (interval < 1)
        interval = default_interval;

    chat_name = settings->get_name(components);

#ifdef _HAZE_
    //HAZE only works when the conv has already been created
    //as opposed to right before the conversation has been created
    if (!purple_find_conversation_with_account(PURPLE_CONV_TYPE_IM, chat_name, account)) {
        purple_debug_warning(purple_account_get_protocol_id(account), "Could not find chat %s\n", chat_name);
        g_free(chat_name);
        return;
    }
#endif
    if (!twitter_endpoint_chat_find(account, chat_name)) {
        TwitterConnectionData *twitter = gc->proto_data;
        TwitterEndpointChat *endpoint_chat = twitter_endpoint_chat_new(settings, settings->type, account, chat_name, components);
        g_hash_table_insert(twitter->chat_contexts, g_strdup(purple_normalize(account, chat_name)), endpoint_chat);
        settings->on_start(endpoint_chat);

        endpoint_chat->timer_handle = purple_timeout_add_seconds(60 * interval, twitter_endpoint_chat_interval_timeout, endpoint_chat);

        if (purple_find_conversation_with_account(PURPLE_CONV_TYPE_CHAT, chat_name, account)) {
            //We'd only get here if the chat was open already before the connection was established
            //eg, search was open, but user disconnected and reconnected

            //we need to tell the client that the server accepted the chat
            guint           chat_id = twitter_get_next_chat_id(account);
            serv_got_joined_chat(gc, chat_id, chat_name);
        }
    }
#ifndef _HAZE_
    if (!purple_find_conversation_with_account(PURPLE_CONV_TYPE_CHAT, chat_name, account)) {
        if (open_conv) {
            guint           chat_id = twitter_get_next_chat_id(account);
            serv_got_joined_chat(gc, chat_id, chat_name);
        }
    } else {
        purple_debug_warning(purple_account_get_protocol_id(account), "Chat %s is already open.\n", chat_name);
    }
#endif

    g_free(chat_name);
}

TwitterEndpointChat *twitter_endpoint_chat_find(PurpleAccount * account, const char *chat_name)
{
    PurpleConnection *gc = purple_account_get_connection(account);
    TwitterConnectionData *twitter = gc->proto_data;
    return (TwitterEndpointChat *) g_hash_table_lookup(twitter->chat_contexts, purple_normalize(account, chat_name));
}

static void twitter_endpoint_chat_send_success_cb(PurpleAccount * account, xmlnode * node, gboolean last, gpointer _ctx_id)
{
    TwitterEndpointChatId *id = _ctx_id;
    TwitterUserTweet *user_tweet = twitter_update_status_node_parse(node);
    TwitterTweet   *tweet = user_tweet ? user_tweet->status : NULL;
    TwitterEndpointChat *ctx = twitter_endpoint_chat_find_by_id(id);

#ifndef _HAZE_
    PurpleConversation *conv;

    if (ctx && tweet && tweet->text && (conv = twitter_endpoint_chat_find_open_conv(ctx))) {
        char          **userparts = g_strsplit(purple_account_get_username(account), "@", 2);
        const char     *sn = userparts[0];
        purple_signal_emit(purple_buddy_icons_get_handle(), "prpltwtr-update-iconurl", account, user_tweet->screen_name, user_tweet->icon_url, user_tweet->status->created_at);
        twitter_chat_add_tweet(conv, sn, tweet->text, tweet->id, tweet->created_at, tweet->in_reply_to_status_id, tweet->favorited);
        g_strfreev(userparts);
    }
#endif
    if (tweet && tweet->id)
        twitter_add_sent_tweet_id(ctx, tweet->id);

    if (user_tweet)
        twitter_user_tweet_free(user_tweet);

    if (last)
        twitter_endpoint_chat_id_free(id);
}

static gboolean twitter_endpoint_chat_send_error_cb(PurpleAccount * account, const TwitterRequestErrorData * error_data, gpointer _ctx_id)
{
    TwitterEndpointChatId *id = _ctx_id;
    TwitterEndpointChat *ctx = twitter_endpoint_chat_find_by_id(id);
    PurpleConversation *conv;
    if (ctx) {
        conv = twitter_endpoint_chat_find_open_conv(ctx);

        if (conv) {
            gchar          *error = g_strdup_printf(_("Error sending tweet: %s"), error_data->message ? error_data->message : _("unknown error"));
            purple_conversation_write(conv, NULL, error, PURPLE_MESSAGE_ERROR, time(NULL));
            g_free(error);
        }
    }

    twitter_endpoint_chat_id_free(id);

    return FALSE;                                //give up trying
}

int twitter_endpoint_chat_send(TwitterEndpointChat * ctx, const gchar * message)
{
    PurpleAccount  *account = ctx->account;
    TwitterEndpointChatId *id;
    gchar          *added_text = NULL;
    GArray         *statuses;
    TWITTER_ATTACH_SEARCH_TEXT attach_search_text;

    PurpleChat     *chat = twitter_blist_chat_find(account, ctx->chat_name);
    attach_search_text = twitter_blist_chat_attach_search_text(chat);

    if (TWITTER_ATTACH_SEARCH_TEXT_NONE != attach_search_text && ctx->settings->get_status_added_text) {
        added_text = ctx->settings->get_status_added_text(ctx);
    }

    statuses = twitter_utf8_get_segments(message, MAX_TWEET_LENGTH, added_text, attach_search_text == TWITTER_ATTACH_SEARCH_TEXT_PREPEND);
    id = twitter_endpoint_chat_id_new(ctx);
    twitter_api_set_statuses(purple_account_get_requestor(account), statuses, 0, twitter_endpoint_chat_send_success_cb, twitter_endpoint_chat_send_error_cb, id);

    if (added_text)
        g_free(added_text);
    return 0;
}
