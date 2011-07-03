/**
 * TODO: legal stuff
 *
 * purple
 *
 * Purple is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#include <glib/gstdio.h>

#include "prpltwtr.h"

#include "prpltwtr_mbprefs.h"

#if !PURPLE_VERSION_CHECK(2, 6, 0)
#define PURPLE_CHAT(obj) ((PurpleChat *)(obj))
#define PURPLE_BUDDY(obj) ((PurpleBuddy *)(obj))
#endif

/******************************************************
 *  Chat
 ******************************************************/
GList          *twitter_chat_info(PurpleConnection * gc)
{
    struct proto_chat_entry *pce;   /* defined in prpl.h */
    GList          *chat_info = NULL;

    pce = g_new0(struct proto_chat_entry, 1);
    pce->label = ("Search");
    pce->identifier = "search";
    pce->required = FALSE;

    chat_info = g_list_append(chat_info, pce);

    pce = g_new0(struct proto_chat_entry, 1);
    pce->label = ("Update Interval");
    pce->identifier = "interval";
    pce->required = TRUE;
    pce->is_int = TRUE;
    pce->min = 1;
    pce->max = 60;

    chat_info = g_list_append(chat_info, pce);

    return chat_info;
}

GHashTable     *twitter_chat_info_defaults(PurpleConnection * gc, const char *chat_name)
{
    GHashTable     *defaults;

    defaults = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);

    g_hash_table_insert(defaults, "search", g_strdup(chat_name));

    //bug in pidgin prevents this from working
    g_hash_table_insert(defaults, "interval", g_strdup_printf("%d", twitter_option_search_timeout(purple_connection_get_account(gc))));
    return defaults;
}

#ifdef _HAZE_
static PurpleBuddy *twitter_blist_chat_timeline_new(PurpleAccount * account, gint timeline_id)
{
    return twitter_buddy_new(account, "Timeline: Home", NULL);
}
#else
static PurpleChat *twitter_blist_chat_timeline_new(PurpleAccount * account, gint timeline_id)
{
    PurpleGroup    *g;
    PurpleChat     *c = twitter_blist_chat_find_timeline(account, timeline_id);
    GHashTable     *components;
    if (c != NULL) {
        return c;
    }
    /* No point in making this a preference (yet?)
     * the idea is that this will only be done once, and the user can move the
     * chat to wherever they want afterwards */
    g = purple_find_group(TWITTER_PREF_DEFAULT_TIMELINE_GROUP);
    if (g == NULL)
        g = purple_group_new(TWITTER_PREF_DEFAULT_TIMELINE_GROUP);

    components = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);

    //TODO: fix all of this
    //1) FIXED: search shouldn't be set, but is currently a hack to fix purple_blist_find_chat (persistent chat, etc)
    //2) need this to work with multiple timelines.
    //3) this should be an option. Some people may not want the home timeline
    g_hash_table_insert(components, "interval", g_strdup_printf("%d", twitter_option_timeline_timeout(account)));
    g_hash_table_insert(components, "chat_type", g_strdup_printf("%d", TWITTER_CHAT_TIMELINE));
    g_hash_table_insert(components, "timeline_id", g_strdup_printf("%d", timeline_id));

    c = purple_chat_new(account, "Home Timeline", components);
    purple_blist_add_chat(c, g, NULL);
    return c;
}

static PurpleChat *twitter_blist_chat_list_new(PurpleAccount * account, const char *list_name, const char *owner, long long list_id)
{
    PurpleGroup    *g;
    PurpleChat     *c = twitter_blist_chat_find_list(account, list_name);
    GHashTable     *components;
    if (c != NULL) {
        return c;
    }
    /* No point in making this a preference (yet?)
     * the idea is that this will only be done once, and the user can move the
     * chat to wherever they want afterwards */
    g = purple_find_group(TWITTER_PREF_DEFAULT_LIST_GROUP);
    if (g == NULL)
        g = purple_group_new(TWITTER_PREF_DEFAULT_LIST_GROUP);

    components = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);

    //TODO: fix all of this
    //1) FIXED: search shouldn't be set, but is currently a hack to fix purple_blist_find_chat (persistent chat, etc)
    //2) need this to work with multiple timelines.
    //3) this should be an option. Some people may not want the home timeline
    g_hash_table_insert(components, "interval", g_strdup_printf("%d", twitter_option_list_timeout(account)));
    g_hash_table_insert(components, "chat_type", g_strdup_printf("%d", TWITTER_CHAT_LIST));
    g_hash_table_insert(components, "list_name", g_strdup(list_name));
    g_hash_table_insert(components, "owner", g_strdup(owner));
    g_hash_table_insert(components, "list_id", g_strdup_printf("%lld", list_id));

    c = purple_chat_new(account, list_name, components);
    purple_blist_add_chat(c, g, NULL);
    return c;
}
#endif

static PurpleChat *twitter_blist_chat_search_new(PurpleAccount * account, const char *searchtext)
{
    PurpleGroup    *g;
    PurpleChat     *c = twitter_blist_chat_find_search(account, searchtext);
    const char     *group_name;
    GHashTable     *components;
    if (c != NULL) {
        return c;
    }
    /* This is an option for when we sync our searches, the user
     * doesn't have to continuously move the chats */
    group_name = twitter_option_search_group(account);
    g = purple_find_group(group_name);
    if (g == NULL)
        g = purple_group_new(group_name);

    components = twitter_chat_info_defaults(purple_account_get_connection(account), searchtext);

    c = purple_chat_new(account, searchtext, components);
    purple_blist_add_chat(c, g, NULL);

    return c;
}

static void verify_connection_error_handler(PurpleAccount * account, const TwitterRequestErrorData * error_data)
{
    const gchar    *error_message;
    PurpleConnectionError reason;
    //purple_connection_error_reason(gc, PURPLE_CONNECTION_ERROR_NETWORK_ERROR, error_data->message);
    switch (error_data->type) {
    case TWITTER_REQUEST_ERROR_SERVER:
        reason = PURPLE_CONNECTION_ERROR_NETWORK_ERROR;
        error_message = error_data->message;
        break;
    case TWITTER_REQUEST_ERROR_INVALID_XML:
        reason = PURPLE_CONNECTION_ERROR_NETWORK_ERROR;
        error_message = _("Received Invalid XML");
        break;
    case TWITTER_REQUEST_ERROR_UNAUTHORIZED:
        reason = PURPLE_CONNECTION_ERROR_AUTHENTICATION_FAILED;
        error_message = _("Unauthorized");
        break;
    case TWITTER_REQUEST_ERROR_TWITTER_GENERAL:
        reason = PURPLE_CONNECTION_ERROR_NETWORK_ERROR;
        error_message = error_data->message;
        break;
    default:
        reason = PURPLE_CONNECTION_ERROR_OTHER_ERROR;
        error_message = _("Unknown error");
        break;
    }
    purple_connection_error_reason(purple_account_get_connection(account), reason, error_message);
}

static gboolean twitter_get_friends_verify_error_cb(TwitterRequestor * r, const TwitterRequestErrorData * error_data, gpointer user_data)
{
    verify_connection_error_handler(r->account, error_data);
    return FALSE;
}

/******************************************************
 *  Twitter friends
 ******************************************************/
static gboolean twitter_update_presence_timeout(gpointer _account)
{
    /* If someone is bored and wants to do this the right way, 
     * they would have a list of buddies sorted by time
     * so we don't have to go through all buddies. 
     * Of course, then you'd have to have each PurpleBuddy point to 
     * the position in the list, so when a status gets updated
     * we push the buddy back to the end of the list
     *
     * Additionally, we would want the timer to be reset to run
     * at the time when the next buddy should go offline 
     * (min(last_tweet_time) - current_time)
     *
     * However, this should be good enough. If we find that this
     * drains a lot of batteries on mobile phones (doubt it), then we can
     * look back into it.
     */
    PurpleAccount  *account = _account;
    twitter_buddy_touch_state_all(account);
    return TRUE;
}

static void twitter_buddy_datas_set_all(PurpleAccount * account, GList * buddy_datas)
{
    GList          *l;
    for (l = buddy_datas; l; l = l->next) {
        TwitterUserTweet *data = l->data;
        TwitterUserData *user = twitter_user_tweet_take_user_data(data);
        TwitterTweet   *status = twitter_user_tweet_take_tweet(data);

        if (user)
            twitter_buddy_set_user_data(account, user, twitter_option_get_following(account));
        if (status)
            twitter_buddy_set_status_data(account, data->screen_name, status);

        twitter_user_tweet_free(data);
    }
    g_list_free(buddy_datas);
}

static void twitter_get_friends_cb(TwitterRequestor * r, GList * nodes, gpointer user_data)
{
    GList          *buddy_datas = twitter_users_nodes_parse(nodes);
    twitter_buddy_datas_set_all(r->account, buddy_datas);
}

static gboolean twitter_get_friends_timeout(gpointer data)
{
    PurpleAccount  *account = data;
    //TODO handle errors
    twitter_api_get_friends(purple_account_get_requestor(account), twitter_get_friends_cb, NULL, NULL);
    return TRUE;
}

/******************************************************
 *  Twitter relies/mentions
 ******************************************************/

/******************************************************
 *  Twitter search
 ******************************************************/

char           *twitter_chat_get_name(GHashTable * components)
{
    const char     *chat_type_str = g_hash_table_lookup(components, "chat_type");
    TwitterChatType chat_type = chat_type_str == NULL ? 0 : strtol(chat_type_str, NULL, 10);

    TwitterEndpointChatSettings *settings = twitter_get_endpoint_chat_settings(chat_type);
    if (settings && settings->get_name)
        return settings->get_name(components);
    return NULL;
}

static void get_lists_cb(TwitterRequestor * r, xmlnode * node, gpointer user_data)
{
    xmlnode        *list;

    purple_debug_info(purple_account_get_protocol_id(r->account), "%s\n", G_STRFUNC);
    if (!node)
        return;

    list = xmlnode_get_child(node, "lists");

    if (!list) {
        return;
    }

    for (list = list->child; list; list = list->next) {
        if (list->name && !g_strcmp0(list->name, "list")) {
            long long       id;
            gchar          *owner = NULL;
            gchar          *id_str = xmlnode_get_child_data(list, "id");
            gchar          *name = xmlnode_get_child_data(list, "full_name");
            xmlnode        *user = xmlnode_get_child(list, "user");
            if (user) {
                owner = xmlnode_get_child_data(user, "screen_name");
            }

            if (id_str) {
                id = strtoll(id_str, NULL, 10);
            } else {
                id = 0;
                purple_debug_warning(purple_account_get_protocol_id(r->account), "error with xmlnode. name = 0x%p, id_str=0x%p\n", name, id_str);
            }
#ifdef _HAZE_
            /* TODO */
#else
            purple_debug_info(purple_account_get_protocol_id(r->account), "List found: name %s, id %lld\n", name, id);
            twitter_blist_chat_list_new(r->account, name, owner, id);
#endif

            g_free(id_str);
            g_free(name);
            g_free(owner);
        }
    }
}

static void get_saved_searches_cb(TwitterRequestor * r, xmlnode * node, gpointer user_data)
{
    xmlnode        *search;

    purple_debug_info(purple_account_get_protocol_id(r->account), "%s\n", G_STRFUNC);

    for (search = node->child; search; search = search->next) {
        if (search->name && !g_strcmp0(search->name, "saved_search")) {
            gchar          *query = xmlnode_get_child_data(search, "query");
#ifdef _HAZE_
            char           *buddy_name = g_strdup_printf("#%s", query);

            twitter_buddy_new(r->account, buddy_name, NULL);
            purple_prpl_got_user_status(r->account, buddy_name, TWITTER_STATUS_ONLINE, NULL);
            g_free(buddy_name);
#else
            twitter_blist_chat_search_new(r->account, query);
#endif
            g_free(query);
        }
    }
}

void twitter_chat_leave(PurpleConnection * gc, int id)
{
    PurpleConversation *conv = purple_find_chat(gc, id);
    TwitterConnectionData *twitter = gc->proto_data;
    PurpleAccount  *account = purple_connection_get_account(gc);
    TwitterEndpointChat *ctx = twitter_endpoint_chat_find(account, purple_conversation_get_name(conv));
    PurpleChat     *blist_chat;

    g_return_if_fail(ctx != NULL);
    //TODO move me to twitter_endpoint_chat

    blist_chat = twitter_blist_chat_find(account, ctx->chat_name);
    if (blist_chat != NULL && twitter_blist_chat_is_auto_open(blist_chat)) {
        return;
    }

    g_hash_table_remove(twitter->chat_contexts, purple_normalize(account, ctx->chat_name));
}

int twitter_chat_send(PurpleConnection * gc, int id, const char *message, PurpleMessageFlags flags)
{
    PurpleConversation *conv = purple_find_chat(gc, id);
    PurpleAccount  *account = purple_connection_get_account(gc);
    TwitterEndpointChat *ctx = twitter_endpoint_chat_find(account, purple_conversation_get_name(conv));
    char           *stripped_message;
    int             rv;

    g_return_val_if_fail(ctx != NULL, -1);

    stripped_message = purple_markup_strip_html(message);

    rv = twitter_endpoint_chat_send(ctx, stripped_message);
    g_free(stripped_message);
    return rv;
}

static void twitter_chat_join_do(PurpleConnection * gc, GHashTable * components, gboolean open_conv)
{
    const char     *conv_type_str = g_hash_table_lookup(components, "chat_type");
    gint            conv_type = conv_type_str == NULL ? 0 : strtol(conv_type_str, NULL, 10);
    twitter_endpoint_chat_start(gc, twitter_get_endpoint_chat_settings(conv_type), components, open_conv);
}

void twitter_chat_join(PurpleConnection * gc, GHashTable * components)
{
    twitter_chat_join_do(gc, components, TRUE);
}

static void twitter_set_all_buddies_online(PurpleAccount * account)
{
    GSList         *buddies = purple_find_buddies(account, NULL);
    GSList         *l;
    for (l = buddies; l; l = l->next) {
        purple_prpl_got_user_status(account, ((PurpleBuddy *) l->data)->name, TWITTER_STATUS_ONLINE, "message", NULL, NULL);
    }
    g_slist_free(buddies);
}

static void twitter_init_auto_open_contexts(PurpleAccount * account)
{
    PurpleChat     *chat;
    PurpleBlistNode *node,
                   *group;
    PurpleBuddyList *purplebuddylist = purple_get_blist();
    PurpleConnection *gc = purple_account_get_connection(account);
    GHashTable     *components;

    g_return_if_fail(purplebuddylist != NULL);

    for (group = purplebuddylist->root; group != NULL; group = group->next) {
        for (node = group->child; node != NULL; node = node->next) {
            if (PURPLE_BLIST_NODE_IS_CHAT(node)) {

                chat = (PurpleChat *) node;

                if (account != chat->account)
                    continue;

                if (twitter_blist_chat_is_auto_open(chat)) {
                    components = purple_chat_get_components(chat);
                    twitter_chat_join_do(gc, components, FALSE);
                }
            }
        }
    }

    return;
}

#ifdef _HAZE_
static void conversation_created_cb(PurpleConversation * conv, gpointer * unused)
{
    const char     *name = purple_conversation_get_name(conv);
    PurpleAccount  *account = purple_conversation_get_account(conv);
    g_return_if_fail(name != NULL && name[0] != '\0');

    if (name[0] == '#') {
        GHashTable     *components = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);
        g_hash_table_insert(components, "search", g_strdup(name + 1));
        g_hash_table_insert(components, "chat_type", g_strdup_printf("%d", TWITTER_CHAT_SEARCH));
        twitter_endpoint_chat_start(purple_account_get_connection(account), twitter_get_endpoint_chat_settings(TWITTER_CHAT_SEARCH), components, TRUE);
    } else if (twitter_usernames_match(account, name, "Timeline: Home")) {
        GHashTable     *components = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);
        g_hash_table_insert(components, "timeline_id", g_strdup("0"));
        g_hash_table_insert(components, "chat_type", g_strdup_printf("%d", TWITTER_CHAT_TIMELINE)); //i don't think this is needed
        twitter_endpoint_chat_start(purple_account_get_connection(account), twitter_get_endpoint_chat_settings(TWITTER_CHAT_TIMELINE), components, TRUE);
    }
}

static void deleting_conversation_cb(PurpleConversation * conv, gpointer * unused)
{
    const char     *name = purple_conversation_get_name(conv);
    PurpleAccount  *account = purple_conversation_get_account(conv);

    g_return_if_fail(name != NULL && name[0] != '\0');

    if (name[0] == '#' || twitter_usernames_match(account, name, "Timeline: Home")) {
        PurpleConnection *gc = purple_conversation_get_gc(conv);
        TwitterConnectionData *twitter = gc->proto_data;
        TwitterEndpointChat *ctx = twitter_endpoint_chat_find(account, purple_conversation_get_name(conv));
        if (ctx) {
            //TODO: move to endpoint_chat.c
            purple_debug_info(purple_account_get_protocol_id(account), "destroying haze im chat %s\n", ctx->chat_name);
            g_hash_table_remove(twitter->chat_contexts, purple_normalize(account, ctx->chat_name));
        }
    }
}
#endif

static void twitter_endpoint_im_start_foreach(TwitterConnectionData * twitter, TwitterEndpointIm * im, gpointer data)
{
    twitter_endpoint_im_start(im);
}

static gboolean TWITTER_SIGNALS_CONNECTED = FALSE;

static void twitter_connected(PurpleAccount * account)
{
    PurpleConnection *gc = purple_account_get_connection(account);
    TwitterConnectionData *twitter = gc->proto_data;
    int             get_friends_timer_timeout;

    purple_debug_info(purple_account_get_protocol_id(account), "%s\n", G_STRFUNC);

    twitter->mb_prefs = twitter_mb_prefs_new(account);

    twitter_connection_set_endpoint_im(twitter, TWITTER_IM_TYPE_AT_MSG, twitter_endpoint_im_new(account, twitter_endpoint_reply_get_settings(), twitter_option_get_history(account), TWITTER_INITIAL_REPLIES_COUNT));
    twitter_connection_set_endpoint_im(twitter, TWITTER_IM_TYPE_DM, twitter_endpoint_im_new(account, twitter_endpoint_dm_get_settings(), twitter_option_get_history(account), TWITTER_INITIAL_DMS_COUNT));

    purple_connection_update_progress(gc, _("Connected"), 2,    /* which connection step this is */
                                      3);        /* total number of steps */
    purple_connection_set_state(gc, PURPLE_CONNECTED);

    twitter_blist_chat_timeline_new(account, 0);
#ifdef _HAZE_
    //Set home timeline online
    purple_prpl_got_user_status(account, "Timeline: Home", TWITTER_STATUS_ONLINE, NULL);
#endif

    /* Status.net doesn't support lists or saved searches */
    if (!strcmp(purple_account_get_protocol_id(account), TWITTER_PROTOCOL_ID)) {
        /* Retrieve user's saved search queries */
        twitter_api_get_saved_searches(purple_account_get_requestor(account), get_saved_searches_cb, NULL, NULL);

        twitter_api_get_personal_lists(purple_account_get_requestor(account), get_lists_cb, NULL, NULL);
        twitter_api_get_subscribed_lists(purple_account_get_requestor(account), get_lists_cb, NULL, NULL);
    }

    /* Install periodic timers to retrieve replies and dms */
    twitter_connection_foreach_endpoint_im(twitter, twitter_endpoint_im_start_foreach, NULL);

    /* Immediately retrieve replies */

    get_friends_timer_timeout = twitter_option_user_status_timeout(account);

    //We will try to get all our friends' statuses, whether they're in the buddylist or not
    if (get_friends_timer_timeout > 0) {
        twitter->get_friends_timer = purple_timeout_add_seconds(60 * get_friends_timer_timeout, twitter_get_friends_timeout, account);
        if (!twitter_option_get_following(account) && twitter_option_cutoff_time(account) > 0)
            twitter_get_friends_timeout(account);
    } else {
        twitter->get_friends_timer = 0;
    }
    if (twitter_option_cutoff_time(account) > 0)
        twitter->update_presence_timer = purple_timeout_add_seconds(TWITTER_UPDATE_PRESENCE_TIMEOUT * 60, twitter_update_presence_timeout, account);
    twitter_init_auto_open_contexts(account);
}

static void twitter_get_friends_verify_connection_cb(TwitterRequestor * r, GList * nodes, gpointer user_data)
{
    PurpleConnection *gc = purple_account_get_connection(r->account);
    GList          *l_users_data = NULL;

    if (purple_connection_get_state(gc) == PURPLE_CONNECTING) {
        twitter_connected(r->account);

        l_users_data = twitter_users_nodes_parse(nodes);

        /* setup buddy list */
        twitter_buddy_datas_set_all(r->account, l_users_data);

    }
}

static void twitter_get_rate_limit_status_cb(TwitterRequestor * r, xmlnode * node, gpointer user_data)
{
    /*
     * <hash>
     * <reset-time-in-seconds type="integer">1236529763</reset-time-in-seconds>
     * <remaining-hits type="integer">100</remaining-hits>
     * <hourly-limit type="integer">100</hourly-limit>
     * <reset-time type="datetime">2009-03-08T16:29:23+00:00</reset-time>
     * </hash>
     */

    xmlnode        *child;
    int             remaining_hits = 0;
    int             hourly_limit = 0;
    char           *message;
    for (child = node->child; child; child = child->next) {
        if (child->name) {
            if (!strcmp(child->name, "remaining-hits")) {
                char           *data = xmlnode_get_data_unescaped(child);
                remaining_hits = atoi(data);
                g_free(data);
            } else if (!strcmp(child->name, "hourly-limit")) {
                char           *data = xmlnode_get_data_unescaped(child);
                hourly_limit = atoi(data);
                g_free(data);
            }
        }
    }
    message = g_strdup_printf("%d/%d %s", remaining_hits, hourly_limit, _("Remaining"));
    purple_notify_info(NULL,                     /* plugin handle or PurpleConnection */
                       _("Rate Limit Status"), _("Rate Limit Status"), (message));
    g_free(message);
}

/*
 * prpl functions
 */
const char     *twitter_list_icon(PurpleAccount * account, PurpleBuddy * buddy)
{
    return "prpltwtr";
}

char           *twitter_status_text(PurpleBuddy * buddy)
{
    purple_debug_info(purple_account_get_protocol_id(buddy->account), "getting %s's status text for %s\n", buddy->name, buddy->account->username);

    if (purple_find_buddy(buddy->account, buddy->name)) {
        PurplePresence *presence = purple_buddy_get_presence(buddy);
        PurpleStatus   *status = purple_presence_get_active_status(presence);
        const char     *message = status ? purple_status_get_attr_string(status, "message") : NULL;

        if (message && strlen(message) > 0)
            return g_markup_escape_text(message, -1);

    }
    return NULL;
}

void twitter_tooltip_text(PurpleBuddy * buddy, PurpleNotifyUserInfo * info, gboolean full)
{

    PurplePresence *presence = purple_buddy_get_presence(buddy);
    PurpleStatus   *status = purple_presence_get_active_status(presence);
    char           *msg;

    purple_debug_info(purple_account_get_protocol_id(buddy->account), "showing %s tooltip for %s\n", (full) ? "full" : "short", buddy->name);

    if ((msg = twitter_status_text(buddy))) {
        purple_notify_user_info_add_pair(info, purple_status_get_name(status), msg);
        g_free(msg);
    }

    if (full) {
        /*const char *user_info = purple_account_get_user_info(gc->account);
           if (user_info)
           purple_notify_user_info_add_pair(info, _("User info"), user_info); */
    }
}

/* everyone will be online
 * Future: possibly have offline mode for people who haven't updated in a while
 */
GList          *twitter_status_types(PurpleAccount * account)
{
    GList          *types = NULL;
    PurpleStatusType *type;
    PurpleStatusPrimitive status_primitives[] = {
        PURPLE_STATUS_UNAVAILABLE,
        PURPLE_STATUS_INVISIBLE,
        PURPLE_STATUS_AWAY,
        PURPLE_STATUS_EXTENDED_AWAY
    };
    int             status_primitives_count = sizeof (status_primitives) / sizeof (status_primitives[0]);
    int             i;

    type = purple_status_type_new(PURPLE_STATUS_AVAILABLE, TWITTER_STATUS_ONLINE, TWITTER_STATUS_ONLINE, TRUE);
    purple_status_type_add_attr(type, "message", ("Online"), purple_value_new(PURPLE_TYPE_STRING));
    types = g_list_prepend(types, type);

    //This is a hack to get notified when another protocol goes into a different status.
    //Eg aim goes "away", we still want to get notified
    for (i = 0; i < status_primitives_count; i++) {
        type = purple_status_type_new(status_primitives[i], TWITTER_STATUS_ONLINE, TWITTER_STATUS_ONLINE, FALSE);
        purple_status_type_add_attr(type, "message", ("Online"), purple_value_new(PURPLE_TYPE_STRING));
        types = g_list_prepend(types, type);
    }

    type = purple_status_type_new(PURPLE_STATUS_OFFLINE, TWITTER_STATUS_OFFLINE, TWITTER_STATUS_OFFLINE, TRUE);
    purple_status_type_add_attr(type, "message", ("Offline"), purple_value_new(PURPLE_TYPE_STRING));
    types = g_list_prepend(types, type);

    return g_list_reverse(types);
}

/*
 * UI callbacks
 */
#if 0
static void twitter_action_get_user_info(PurplePluginAction * action)
{
    PurpleConnection *gc = (PurpleConnection *) action->context;
    PurpleAccount  *account = purple_connection_get_account(gc);
    twitter_api_get_friends(purple_account_get_requestor(account), twitter_get_friends_cb, NULL, NULL);
}
#endif

static void twitter_action_set_status_ok(PurpleConnection * gc, PurpleRequestFields * fields)
{
    PurpleAccount  *account = purple_connection_get_account(gc);
    const char     *status = purple_request_fields_get_string(fields, "status");
    purple_account_set_status(account, TWITTER_STATUS_ONLINE, TRUE, "message", status, NULL);
}

static void twitter_action_set_status(PurplePluginAction * action)
{
    PurpleRequestFields *request;
    PurpleRequestFieldGroup *group;
    PurpleRequestField *field;
    PurpleConnection *gc = (PurpleConnection *) action->context;

    group = purple_request_field_group_new(NULL);

    field = purple_request_field_string_new("status", ("Status"), "", FALSE);
    purple_request_field_group_add_field(group, field);

    request = purple_request_fields_new();
    purple_request_fields_add_group(request, group);

    purple_request_fields(action->plugin, _("Status"), _("Set Account Status"), NULL, request, _("_Set"), G_CALLBACK(twitter_action_set_status_ok), _("_Cancel"), NULL, purple_connection_get_account(gc), NULL, NULL, gc);
}

static void twitter_action_get_rate_limit_status(PurplePluginAction * action)
{
    PurpleConnection *gc = (PurpleConnection *) action->context;
    TwitterConnectionData *twitter = gc->proto_data;
    twitter_api_get_rate_limit_status(twitter->requestor, twitter_get_rate_limit_status_cb, NULL, NULL);
}

/* this is set to the actions member of the PurplePluginInfo struct at the
 * bottom.
 */
GList          *twitter_actions(PurplePlugin * plugin, gpointer context)
{
    GList          *l = NULL;
    PurplePluginAction *action;

    action = purple_plugin_action_new(_("Set status"), twitter_action_set_status);
    l = g_list_append(l, action);

    action = purple_plugin_action_new(_("Rate Limit Status"), twitter_action_get_rate_limit_status);
    l = g_list_append(l, action);

#if 0
    action = purple_plugin_action_new(_("Debug - Retrieve users"), twitter_action_get_user_info);
    l = g_list_append(l, action);
#endif
    if (!strcmp(plugin->info->id, TWITTER_PROTOCOL_ID)) {
        l = g_list_append(l, NULL);

        action = purple_plugin_action_new(_("Open Favorites URL"), twitter_api_web_open_favorites);
        l = g_list_append(l, action);
        action = purple_plugin_action_new(_("Open Retweets of Mine"), twitter_api_web_open_retweets_of_mine);
        l = g_list_append(l, action);
        action = purple_plugin_action_new(_("Open Replies"), twitter_api_web_open_replies);
        l = g_list_append(l, action);
        action = purple_plugin_action_new(_("Open Suggested Friends"), twitter_api_web_open_suggested_friends);
        l = g_list_append(l, action);
    }

    return l;
}

typedef struct {
    void            (*success_cb) (PurpleAccount * account, long long id, gpointer user_data);
    void            (*error_cb) (PurpleAccount * account, const TwitterRequestErrorData * error_data, gpointer user_data);
    gpointer        user_data;
} TwitterLastSinceIdRequest;

//TODO: rename
void prpltwtr_verify_connection(PurpleAccount * account)
{
    gboolean        retrieve_history;
    PurpleConnection *gc;

    //To verify the connection, we get the user's friends.
    //With that we'll update the buddy list and set the last known reply id

    /* If history retrieval enabled, read last reply id from config file.
     * There's no config file, just set last reply id to 0 */
    retrieve_history = twitter_option_get_history(account);

    //If we don't have a stored last reply id, we don't want to get the entire history (EVERY reply)
    gc = purple_account_get_connection(account);

    if (purple_connection_get_state(gc) == PURPLE_CONNECTING) {

        purple_connection_update_progress(gc, _("Connecting..."), 1,    /* which connection step this is */
                                          3);    /* total number of steps */
    }

    if (twitter_option_get_following(account)) {
        twitter_api_get_friends(purple_account_get_requestor(account), twitter_get_friends_verify_connection_cb, twitter_get_friends_verify_error_cb, NULL);
    } else {
        twitter_connected(account);
        if (twitter_option_cutoff_time(account) <= 0)
            twitter_set_all_buddies_online(account);
    }
}

void prpltwtr_recoverable_disconnect(PurpleAccount * account, const char *message)
{
    PurpleConnection *gc = purple_account_get_connection(account);
    purple_connection_error_reason(gc, PURPLE_CONNECTION_ERROR_NETWORK_ERROR, (message));
}

void prpltwtr_disconnect(PurpleAccount * account, const char *message)
{
    PurpleConnection *gc = purple_account_get_connection(account);
    purple_connection_error_reason(gc, PURPLE_CONNECTION_ERROR_AUTHENTICATION_FAILED, (message));
}

static void requestor_post_failed(TwitterRequestor * r, const TwitterRequestErrorData ** error_data)
{
    purple_debug_error(purple_account_get_protocol_id(r->account), "post_failed called for account %s, error %d, message %s\n", r->account->username, (*error_data)->type, (*error_data)->message ? (*error_data)->message : "");
    switch ((*error_data)->type) {
    case TWITTER_REQUEST_ERROR_UNAUTHORIZED:
        prpltwtr_auth_invalidate_token(r->account);
        prpltwtr_disconnect(r->account, _("Unauthorized"));
        break;
    default:
        break;
    }
}

void prpltwtr_login(PurpleAccount * account)
{
    char          **userparts;
    PurpleConnection *gc = purple_account_get_connection(account);
    TwitterConnectionData *twitter = g_new0(TwitterConnectionData, 1);
    gc->proto_data = twitter;

    /* Since protocol plugins are loaded before conversations_init is called
     * we cannot connect these signals in plugin->load.
     * So we have this here, with a global var that tells us to only run this
     * once, regardless of number of accounts connecting 
     * There HAS to be a better way to do this. Need to do some research
     * this is an awful hack (tm)
     */
    if (!TWITTER_SIGNALS_CONNECTED) {
        TWITTER_SIGNALS_CONNECTED = TRUE;

#ifdef _HAZE_
        if (!strcmp(purple_account_get_protocol_id(account), TWITTER_PROTOCOL_ID)) {
            purple_debug_info(purple_account_get_protocol_id(account), "Connecting conv signals for first time\n");
            purple_signal_connect(purple_conversations_get_handle(), "conversation-created", twitter, PURPLE_CALLBACK(conversation_created_cb), NULL);
            purple_signal_connect(purple_conversations_get_handle(), "deleting-conversation", twitter, PURPLE_CALLBACK(deleting_conversation_cb), NULL);
        }
#endif

        purple_prefs_add_none("/prpltwtr");
        purple_prefs_add_bool("/prpltwtr/first-load-complete", FALSE);
        if (!purple_prefs_get_bool("/prpltwtr/first-load-complete")) {
            PurplePlugin   *plugin = purple_plugins_find_with_id("gtkprpltwtr");
            if (plugin) {
                purple_debug_info(purple_account_get_protocol_id(account), "Loading gtk plugin\n");
                purple_plugin_load(plugin);
            }
            purple_prefs_set_bool("/prpltwtr/first-load-complete", TRUE);
        }

    }

    purple_debug_info(purple_account_get_protocol_id(account), "logging in %s\n", account->username);

    userparts = g_strsplit(account->username, "@", 2);
    purple_connection_set_display_name(gc, userparts[0]);
    g_strfreev(userparts);

    twitter->requestor = g_new0(TwitterRequestor, 1);
    twitter->requestor->account = account;
    twitter->requestor->post_failed = requestor_post_failed;
    twitter->requestor->do_send = twitter_requestor_send;

    if (!twitter_option_use_oauth(account)) {
        twitter->requestor->pre_send = prpltwtr_auth_pre_send_auth_basic;
        twitter->requestor->post_send = prpltwtr_auth_post_send_auth_basic;
    } else {
        twitter->requestor->pre_send = prpltwtr_auth_pre_send_oauth;
        twitter->requestor->post_send = prpltwtr_auth_post_send_oauth;
    }

    /* key: gchar *, value: TwitterEndpointChat */
    twitter->chat_contexts = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify) twitter_endpoint_chat_free);

    /* key: gchar *, value: gchar * (of a long long) */
    twitter->user_reply_id_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

    purple_signal_emit(purple_accounts_get_handle(), "prpltwtr-connecting", account);

    /* purple wants a minimum of 2 steps */
    purple_connection_update_progress(gc, ("Connecting"), 0,    /* which connection step this is */
                                      2);        /* total number of steps */

    if (twitter_option_use_oauth(account)) {
        prpltwtr_auth_oauth_login(account, twitter);
    } else {
        //No oauth, just do the verification step, skipping previous oauth steps
        prpltwtr_verify_connection(account);
    }
}

static void twitter_endpoint_im_free_foreach(TwitterConnectionData * conn, TwitterEndpointIm * im, gpointer data)
{
    twitter_endpoint_im_free(im);
}

void twitter_close(PurpleConnection * gc)
{
    /* notify other twitter accounts */
    PurpleAccount  *account = purple_connection_get_account(gc);
    TwitterConnectionData *twitter = gc->proto_data;

    if (twitter->requestor)
        twitter_requestor_free(twitter->requestor);

    twitter_connection_foreach_endpoint_im(twitter, twitter_endpoint_im_free_foreach, NULL);

    if (twitter->get_friends_timer)
        purple_timeout_remove(twitter->get_friends_timer);

    if (twitter->chat_contexts)
        g_hash_table_destroy(twitter->chat_contexts);
    twitter->chat_contexts = NULL;

    if (twitter->update_presence_timer)
        purple_timeout_remove(twitter->update_presence_timer);

    if (twitter->user_reply_id_table)
        g_hash_table_destroy(twitter->user_reply_id_table);
    twitter->user_reply_id_table = NULL;

    purple_signal_emit(purple_accounts_get_handle(), "prpltwtr-disconnected", account);

    if (twitter->mb_prefs)
        twitter_mb_prefs_free(twitter->mb_prefs);

    if (twitter->oauth_token)
        g_free(twitter->oauth_token);

    if (twitter->oauth_token_secret)
        g_free(twitter->oauth_token_secret);

    g_free(twitter);
}

static void twitter_set_status_error_cb(TwitterRequestor * r, const TwitterRequestErrorData * error_data, gpointer user_data)
{
    const char     *message;
    if (error_data->type == TWITTER_REQUEST_ERROR_SERVER || error_data->type == TWITTER_REQUEST_ERROR_TWITTER_GENERAL) {
        message = error_data->message;
    } else if (error_data->type == TWITTER_REQUEST_ERROR_INVALID_XML) {
        message = _("Unknown reply by twitter server");
    } else {
        message = _("Unknown error");
    }
    purple_notify_error(NULL,                    /* plugin handle or PurpleConnection */
                        _("Twitter Set Status"), _("Error setting Twitter Status"), (message));
}

/* A few options here
 * Send message to "d buddy" will send a direct message to buddy
 * Send message to "@buddy" will send a @buddy message
 * Send message to "buddy" will send a @buddy message (TODO: will change in future, make it an option)
 * _HAZE_ Send message to "#text" will set status message with appended "text" (eg hello text)
 * _HAZE_ Send message to "Timeline: Home" will set status
 */
int twitter_send_im(PurpleConnection * gc, const char *conv_name, const char *message, PurpleMessageFlags flags)
{
    TwitterEndpointIm *im;
    const char     *buddy_name;
    PurpleAccount  *account = purple_connection_get_account(gc);
    char           *stripped_message;
    int             rv = 0;

    g_return_val_if_fail(message != NULL && message[0] != '\0' && conv_name != NULL && conv_name[0] != '\0', 0);

    stripped_message = purple_markup_strip_html(message);

#ifdef _HAZE_
    if (conv_name[0] == '#') {
        purple_debug_info(purple_account_get_protocol_id(account), "%s of search %s\n", G_STRFUNC, conv_name);
        TwitterEndpointChat *endpoint = twitter_endpoint_chat_find(account, conv_name);
        rv = twitter_endpoint_chat_send(endpoint, stripped_message);
        g_free(stripped_message);
        return rv;
    } else if (twitter_usernames_match(account, conv_name, "Timeline: Home")) {
        purple_debug_info(purple_account_get_protocol_id(account), "%s of home timeline\n", G_STRFUNC);
        TwitterEndpointChat *endpoint = twitter_endpoint_chat_find(account, conv_name);
        rv = twitter_endpoint_chat_send(endpoint, stripped_message);
        g_free(stripped_message);
        return rv;
    }
#endif

    //TODO, this should be part of im settings
    im = twitter_conv_name_to_endpoint_im(account, conv_name);
    buddy_name = twitter_conv_name_to_buddy_name(account, conv_name);
    rv = im->settings->send_im(account, buddy_name, stripped_message, flags);
    g_free(stripped_message);
    return rv;
}

void twitter_set_info(PurpleConnection * gc, const char *info)
{
    purple_debug_info(purple_account_get_protocol_id(purple_connection_get_account(gc)), "setting %s's user info to %s\n", gc->account->username, info);
}

void twitter_set_status(PurpleAccount * account, PurpleStatus * status)
{
    gboolean        sync_status = twitter_option_sync_status(account);
    const char     *msg;
    if (!sync_status)
        return;

    //TODO: I'm pretty sure this is broken
    msg = purple_status_get_attr_string(status, "message");
    purple_debug_info(purple_account_get_protocol_id(account), "setting %s's status to %s: %s\n", account->username, purple_status_get_name(status), msg);

    if (msg && strcmp("", msg)) {
        //TODO, sucecss && fail
        twitter_api_set_status(purple_account_get_requestor(account), msg, 0, NULL, twitter_set_status_error_cb, NULL);
    }
}

void twitter_add_buddy(PurpleConnection * gc, PurpleBuddy * buddy, PurpleGroup * group)
{
    //Perform the logic to decide whether this buddy will be online/offline
    twitter_buddy_touch_state(buddy);
}

void twitter_add_buddies(PurpleConnection * gc, GList * buddies, GList * groups)
{
    GList          *buddy = buddies;
    GList          *group = groups;

    purple_debug_info(purple_account_get_protocol_id(purple_connection_get_account(gc)), "adding multiple buddies\n");

    while (buddy && group) {
        twitter_add_buddy(gc, (PurpleBuddy *) buddy->data, (PurpleGroup *) group->data);
        buddy = g_list_next(buddy);
        group = g_list_next(group);
    }
}

void twitter_remove_buddy(PurpleConnection * gc, PurpleBuddy * buddy, PurpleGroup * group)
{
    TwitterUserTweet *twitter_buddy_data = buddy->proto_data;

    purple_debug_info(purple_account_get_protocol_id(purple_connection_get_account(gc)), "removing %s from %s's buddy list\n", buddy->name, gc->account->username);

    if (!twitter_buddy_data)
        return;
    twitter_user_tweet_free(twitter_buddy_data);
    buddy->proto_data = NULL;
}

void twitter_remove_buddies(PurpleConnection * gc, GList * buddies, GList * groups)
{
    GList          *buddy = buddies;
    GList          *group = groups;

    purple_debug_info(purple_account_get_protocol_id(purple_connection_get_account(gc)), "removing multiple buddies\n");

    while (buddy && group) {
        twitter_remove_buddy(gc, (PurpleBuddy *) buddy->data, (PurpleGroup *) group->data);
        buddy = g_list_next(buddy);
        group = g_list_next(group);
    }
}

void twitter_get_cb_info(PurpleConnection * gc, int id, const char *who)
{
    //TODO FIX ME
    PurpleConversation *conv = purple_find_chat(gc, id);
    purple_debug_info(purple_account_get_protocol_id(purple_connection_get_account(gc)), "retrieving %s's info for %s in chat room %s\n", who, gc->account->username, conv->name);

    twitter_api_get_info(gc, who);
}

static void twitter_blist_char_attach_search_toggle(PurpleBlistNode * node, gpointer userdata)
{
    PurpleChat     *chat = PURPLE_CHAT(node);
    PurpleAccount  *account = purple_chat_get_account(chat);
    GHashTable     *components = purple_chat_get_components(chat);
    char           *chat_name = twitter_chat_get_name(components);
    PurpleConversation *conv;

    purple_debug_info(purple_account_get_protocol_id(account), "Setting attach for %s to %d\n", chat_name, (int) userdata);
    conv = purple_find_conversation_with_account(PURPLE_CONV_TYPE_CHAT, chat_name, account);
    g_hash_table_replace(components, g_strdup("attach_search_text"), (g_strdup_printf("%d", (int) userdata)));
    purple_signal_emit(purple_conversations_get_handle(), "prpltwtr-changed-attached-search", conv);
    g_free(chat_name);
}

static void twitter_blist_chat_auto_open_toggle(PurpleBlistNode * node, gpointer userdata)
{
    TwitterEndpointChat *ctx;
    PurpleChat     *chat = PURPLE_CHAT(node);
    PurpleAccount  *account = purple_chat_get_account(chat);
    GHashTable     *components = purple_chat_get_components(chat);
    char           *chat_name = twitter_chat_get_name(components);

    gboolean        new_state = !twitter_blist_chat_is_auto_open(chat);

    //If no conversation exists and we've set this to NOT auto open, let's free some memory
    if (!new_state && !purple_find_conversation_with_account(PURPLE_CONV_TYPE_CHAT, chat_name, account)
        && (ctx = twitter_endpoint_chat_find(account, chat_name))) {
        TwitterConnectionData *twitter = purple_account_get_connection(account)->proto_data;
        purple_debug_info(purple_account_get_protocol_id(account), "No more auto open, destroying context\n");
        g_hash_table_remove(twitter->chat_contexts, purple_normalize(account, ctx->chat_name));
    } else if (new_state && !twitter_endpoint_chat_find(account, chat_name)) {
        //Join the chat, but don't automatically open the conversation
        twitter_chat_join_do(purple_account_get_connection(account), components, FALSE);
    }

    g_hash_table_replace(components, g_strdup("auto_open"), (new_state ? g_strdup("1") : g_strdup("0")));

    g_free(chat_name);
}

//TODO should be handled in twitter_endpoint_reply
static void twitter_blist_buddy_at_msg(PurpleBlistNode * node, gpointer userdata)
{
    PurpleBuddy    *buddy = PURPLE_BUDDY(node);
    char           *name = g_strdup_printf("@%s", buddy->name);
    PurpleConversation *conv = purple_conversation_new(PURPLE_CONV_TYPE_IM, purple_buddy_get_account(buddy),
                                                       name);
    purple_conversation_present(conv);
    g_free(name);
}

static void twitter_blist_buddy_clear_reply(PurpleBlistNode * node, gpointer userdata)
{
    PurpleBuddy    *buddy = PURPLE_BUDDY(node);
    gchar          *conv_name = twitter_endpoint_im_buddy_name_to_conv_name(twitter_endpoint_im_find(purple_buddy_get_account(buddy), TWITTER_IM_TYPE_AT_MSG), buddy->name);
    PurpleConversation *conv = purple_find_conversation_with_account(PURPLE_CONV_TYPE_IM, conv_name, purple_buddy_get_account(buddy));
    purple_conversation_present(conv);
    g_free(conv_name);
    {
        long long      *p = purple_conversation_get_data(conv, "twitter_conv_last_reply_id");
        if (p) {
            g_free(p);
            purple_conversation_set_data(conv, "twitter_conv_last_reply_id", NULL);
        }
    }
    purple_conversation_set_data(conv, "twitter_conv_last_reply_id_manual", NULL);

    purple_debug_info(purple_account_get_protocol_id(purple_buddy_get_account(buddy)), "Cleared reply marker for %p\n", conv);
    purple_signal_emit(purple_conversations_get_handle(), "prpltwtr-set-reply", conv, NULL);
}

//TODO should be handled in twitter_endpoint_dm
static void twitter_blist_buddy_dm(PurpleBlistNode * node, gpointer userdata)
{
    PurpleBuddy    *buddy = PURPLE_BUDDY(node);
    char           *name = g_strdup_printf("d %s", buddy->name);
    PurpleConversation *conv = purple_conversation_new(PURPLE_CONV_TYPE_IM, purple_buddy_get_account(buddy),
                                                       name);
    purple_conversation_present(conv);
    g_free(name);
}

GList          *twitter_blist_node_menu(PurpleBlistNode * node)
{
    GList          *menu = NULL;

    if (PURPLE_BLIST_NODE_IS_CHAT(node)) {
        GList          *submenu = NULL;
        PurpleChat     *chat = PURPLE_CHAT(node);
        GHashTable     *components = purple_chat_get_components(chat);
        char           *label = g_strdup_printf(_("Automatically open chat on new tweets (Currently: %s)"),
                                                (twitter_blist_chat_is_auto_open(chat) ? _("On") : _("Off")));
        const char     *chat_type_str = g_hash_table_lookup(components, "chat_type");
        TwitterChatType chat_type = chat_type_str == NULL ? 0 : strtol(chat_type_str, NULL, 10);

        PurpleMenuAction *action = purple_menu_action_new(label,
                                                          PURPLE_CALLBACK(twitter_blist_chat_auto_open_toggle),
                                                          NULL, /* userdata passed to the callback */
                                                          NULL);    /* child menu items */
        g_free(label);
        purple_debug_info(purple_account_get_protocol_id(purple_chat_get_account(PURPLE_CHAT(node))), "providing buddy list context menu item\n");
        menu = g_list_append(menu, action);
        if (chat_type == TWITTER_CHAT_SEARCH) {
            TWITTER_ATTACH_SEARCH_TEXT cur_attach_search_text = twitter_blist_chat_attach_search_text(chat);

            label = g_strdup_printf(_("No%s"), cur_attach_search_text == TWITTER_ATTACH_SEARCH_TEXT_NONE ? _(" (set)") : "");
            action = purple_menu_action_new(label, PURPLE_CALLBACK(twitter_blist_char_attach_search_toggle), (gpointer) TWITTER_ATTACH_SEARCH_TEXT_NONE, NULL);
            g_free(label);
            submenu = g_list_append(submenu, action);

            label = g_strdup_printf(_("Prepend%s"), cur_attach_search_text == TWITTER_ATTACH_SEARCH_TEXT_PREPEND ? _(" (set)") : "");
            action = purple_menu_action_new(label, PURPLE_CALLBACK(twitter_blist_char_attach_search_toggle), (gpointer) TWITTER_ATTACH_SEARCH_TEXT_PREPEND, NULL);
            g_free(label);
            submenu = g_list_append(submenu, action);

            label = g_strdup_printf(_("Append%s"), cur_attach_search_text == TWITTER_ATTACH_SEARCH_TEXT_APPEND ? _(" (set)") : "");
            action = purple_menu_action_new(label, PURPLE_CALLBACK(twitter_blist_char_attach_search_toggle), (gpointer) TWITTER_ATTACH_SEARCH_TEXT_APPEND, NULL);
            g_free(label);
            submenu = g_list_append(submenu, action);

            label = g_strdup_printf(_("Tag all chats with search term:"));
            action = purple_menu_action_new(label, NULL, NULL, submenu);
            g_free(label);
            menu = g_list_append(menu, action);
        }
    } else if (PURPLE_BLIST_NODE_IS_BUDDY(node)) {
        PurpleMenuAction *action;
        purple_debug_info(purple_account_get_protocol_id(purple_buddy_get_account(PURPLE_BUDDY(node))), "providing buddy list context menu item\n");
        if (twitter_option_default_dm(purple_buddy_get_account(PURPLE_BUDDY(node)))) {
            action = purple_menu_action_new(_("@Message"), PURPLE_CALLBACK(twitter_blist_buddy_at_msg), NULL,   /* userdata passed to the callback */
                                            NULL);  /* child menu items */
        } else {
            action = purple_menu_action_new(_("Direct Message"), PURPLE_CALLBACK(twitter_blist_buddy_dm), NULL, /* userdata passed to the callback */
                                            NULL);  /* child menu items */
        }
        menu = g_list_append(menu, action);
        action = purple_menu_action_new(_("Clear Reply Marker"), PURPLE_CALLBACK(twitter_blist_buddy_clear_reply), NULL, NULL);
        menu = g_list_append(menu, action);
    } else {
    }
    return menu;
}

void twitter_set_buddy_icon(PurpleConnection * gc, PurpleStoredImage * img)
{
    purple_debug_info(purple_account_get_protocol_id(purple_connection_get_account(gc)), "setting %s's buddy icon to %s\n", gc->account->username, purple_imgstore_get_filename(img));
}

void twitter_convo_closed(PurpleConnection * gc, const gchar * conv_name)
{
    TwitterEndpointIm *im = twitter_conv_name_to_endpoint_im(purple_connection_get_account(gc), conv_name);
    if (im) {
        twitter_endpoint_im_convo_closed(im, conv_name);
    }
}

/* We need unique functions here, since the account struct isn't initialized, so we can't check the protocol ID */
GHashTable     *prpltwtr_get_account_text_table_statusnet(PurpleAccount * account)
{
    GHashTable     *table;
    table = g_hash_table_new(g_str_hash, g_str_equal);
    g_hash_table_insert(table, "login_label", _("No passwords with OAuth"));
    return table;
}

/*
 * prpl stuff. see prpl.h for more information.
 */

//borrowed from signals.c
static void twitter_marshal_format_tweet(PurpleCallback cb, va_list args, void *data, void **return_val)
{
    gpointer        ret_val;
    void           *arg1 = va_arg(args, void *);    //account
    void           *arg2 = va_arg(args, void *);    //user
    void           *arg3 = va_arg(args, void *);    //message
    long long       arg4 = va_arg(args, gint64);    //tweet_id
    gint            arg5 = va_arg(args, gint);  //conv type
    void           *arg6 = va_arg(args, void *);    //conv name
    gboolean        arg7 = va_arg(args, gboolean);  //is_tweet
    long long       arg8 = va_arg(args, gint64);    // in_reply_to_status_id
    gboolean        arg9 = va_arg(args, gboolean);  // in_reply_to_status_id

    ret_val = ((gpointer(*)(void *, void *, void *, gint64, gint, void *, gboolean, long long, gboolean, void *)) cb) (arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, data);

    if (return_val != NULL)
        *return_val = ret_val;
}

static void twitter_marshal_received_im(PurpleCallback cb, va_list args, void *data, void **return_val)
{
    void           *arg1 = va_arg(args, void *);    //account
    long long       arg2 = va_arg(args, gint64);    //tweet_id
    void           *arg3 = va_arg(args, void *);    //conv name

    ((gpointer(*)(void *, gint64, void *, void *)) cb) (arg1, arg2, arg3, data);

    if (return_val != NULL)
        *return_val = NULL;
}

static void twitter_marshal_set_reply(PurpleCallback cb, va_list args, void *data, void **return_val)
{
    void           *arg1 = va_arg(args, void *);    // conversation
    void           *arg2 = va_arg(args, void *);    // id_str

    ((gpointer(*)(void *, void *, void *)) cb) (arg1, arg2, data);

    if (return_val != NULL)
        *return_val = NULL;
}

static void twitter_marshal_changed_attached_search(PurpleCallback cb, va_list args, void *data, void **return_val)
{
    void           *arg1 = va_arg(args, void *);    // conversation

    ((gpointer(*)(void *, void *)) cb) (arg1, data);

    if (return_val != NULL)
        *return_val = NULL;
}

void prpltwtr_plugin_init(PurplePlugin * plugin)
{
    PurpleAccountUserSplit *split;
#ifdef ENABLE_NLS
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
#endif                       /* ENABLE_NLS */

    purple_debug_info(plugin->info->id, "starting up\n");

    if (!strcmp(plugin->info->id, TWITTER_PROTOCOL_ID)) {
        plugin->info->summary = _("Twitter for Purple");
        plugin->info->description = _("Access Twitter from within libpurple applications");

        ((PurplePluginProtocolInfo *) plugin->info->extra_info)->protocol_options = prpltwtr_twitter_get_protocol_options();

        /* Only register signals once; we'll use the prpltwtr_twitter plugin */
        purple_signal_register(purple_accounts_get_handle(), "prpltwtr-connecting", purple_marshal_VOID__POINTER, NULL, 1, purple_value_new(PURPLE_TYPE_SUBTYPE, PURPLE_SUBTYPE_ACCOUNT));

        purple_signal_register(purple_accounts_get_handle(), "prpltwtr-disconnected", purple_marshal_VOID__POINTER, NULL, 1, purple_value_new(PURPLE_TYPE_SUBTYPE, PURPLE_SUBTYPE_ACCOUNT));

        purple_signal_register(purple_buddy_icons_get_handle(), "prpltwtr-update-buddyicon", purple_marshal_VOID__POINTER_POINTER_POINTER, NULL, 3, purple_value_new(PURPLE_TYPE_SUBTYPE, PURPLE_SUBTYPE_ACCOUNT), purple_value_new(PURPLE_TYPE_STRING), purple_value_new(PURPLE_TYPE_SUBTYPE, PURPLE_SUBTYPE_BUDDY_ICON));

        purple_signal_register(purple_buddy_icons_get_handle(), "prpltwtr-update-iconurl", purple_marshal_VOID__POINTER_POINTER_POINTER_UINT,   //uint, should be safe for a few decades
                               NULL, 4, purple_value_new(PURPLE_TYPE_SUBTYPE, PURPLE_SUBTYPE_ACCOUNT), purple_value_new(PURPLE_TYPE_STRING), purple_value_new(PURPLE_TYPE_STRING), purple_value_new(PURPLE_TYPE_UINT));

        purple_signal_register(purple_conversations_get_handle(), "prpltwtr-format-tweet", twitter_marshal_format_tweet,    //uint, should be safe for a few decades
                               purple_value_new(PURPLE_TYPE_STRING), 8, purple_value_new(PURPLE_TYPE_SUBTYPE, PURPLE_SUBTYPE_ACCOUNT),  //account
                               purple_value_new(PURPLE_TYPE_STRING),    //user
                               purple_value_new(PURPLE_TYPE_STRING),    //message
                               purple_value_new(PURPLE_TYPE_INT64), //tweet_id
                               purple_value_new(PURPLE_TYPE_INT),   //conv type
                               purple_value_new(PURPLE_TYPE_STRING),    //conv_name
                               purple_value_new(PURPLE_TYPE_BOOLEAN),   // is_tweet
                               purple_value_new(PURPLE_TYPE_INT64), // in_reply_to_status_id
                               purple_value_new(PURPLE_TYPE_BOOLEAN)    // favorited
            );

        purple_signal_register(purple_conversations_get_handle(), "prpltwtr-received-im", twitter_marshal_received_im, NULL, 3, purple_value_new(PURPLE_TYPE_SUBTYPE, PURPLE_SUBTYPE_ACCOUNT),  // account
                               purple_value_new(PURPLE_TYPE_INT64), // tweet_id
                               purple_value_new(PURPLE_TYPE_STRING) // buddy_name
            );
        purple_signal_register(purple_conversations_get_handle(), "prpltwtr-set-reply", twitter_marshal_set_reply, NULL, 2, purple_value_new(PURPLE_TYPE_SUBTYPE, PURPLE_SUBTYPE_CONVERSATION), // conv
                               purple_value_new(PURPLE_TYPE_STRING) // id_str
            );

        purple_signal_register(purple_conversations_get_handle(), "prpltwtr-changed-attached-search", twitter_marshal_changed_attached_search, NULL, 1, purple_value_new(PURPLE_TYPE_SUBTYPE, PURPLE_SUBTYPE_CONVERSATION)  // conv
            );
    } else {
        plugin->info->summary = _("Status.net for Purple (Twitter API)");
        plugin->info->description = _("Access status.net microblogging servers from within libpurple applications");

        split = purple_account_user_split_new(_("Server"), _("server name"), '@');

        ((PurplePluginProtocolInfo *) plugin->info->extra_info)->user_splits = g_list_append(((PurplePluginProtocolInfo *) plugin->info->extra_info)->user_splits, split);

        ((PurplePluginProtocolInfo *) plugin->info->extra_info)->protocol_options = prpltwtr_statusnet_get_protocol_options();
    }

    twitter_endpoint_chat_init(plugin->info->id);
}

void twitter_destroy(PurplePlugin * plugin)
{
    purple_debug_info(plugin->info->id, "shutting down\n");
    if (!strcmp(plugin->info->id, TWITTER_PROTOCOL_ID)) {
        purple_signal_unregister(purple_accounts_get_handle(), "prpltwtr-connecting");
        purple_signal_unregister(purple_accounts_get_handle(), "prpltwtr-disconnected");
        purple_signal_unregister(purple_buddy_icons_get_handle(), "prpltwtr-update-buddyicon");
        purple_signal_unregister(purple_buddy_icons_get_handle(), "prpltwtr-update-iconurl");
        purple_signal_unregister(purple_conversations_get_handle(), "prpltwtr-format-tweet");
        purple_signal_unregister(purple_conversations_get_handle(), "prpltwtr-received-im");
        purple_signals_disconnect_by_handle(plugin);
    }
}
