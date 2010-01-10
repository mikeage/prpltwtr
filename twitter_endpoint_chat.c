#include "twitter_endpoint_chat.h"

static gint twitter_get_next_chat_id()
{
	static gint chat_id = 1;
	return chat_id++;
}

void twitter_endpoint_chat_free(TwitterEndpointChat *ctx)
{
	PurpleConnection *gc;

	if (ctx->settings && ctx->settings->endpoint_data_free)
		ctx->settings->endpoint_data_free(ctx->endpoint_data);
	gc = purple_account_get_connection(ctx->account);

	if (ctx->timer_handle)
	{
		purple_timeout_remove(ctx->timer_handle);
		ctx->timer_handle = 0;
	}
	if (ctx->chat_name)
	{
		g_free(ctx->chat_name);
		ctx->chat_name = NULL;
	}
	g_slice_free(TwitterEndpointChat, ctx);
}

TwitterEndpointChat *twitter_endpoint_chat_new(
	TwitterEndpointChatSettings *settings,
	TwitterChatType type, PurpleAccount *account, const gchar *chat_name,
	GHashTable *components)
{
	TwitterEndpointChat *ctx = g_slice_new0(TwitterEndpointChat);
	ctx->settings = settings;
	ctx->type = type;
	ctx->account = account;
	ctx->chat_name = g_strdup(chat_name);
	ctx->endpoint_data = settings->create_endpoint_data ?
		settings->create_endpoint_data(components) :
		NULL;

	return ctx;
}
TwitterEndpointChatId *twitter_endpoint_chat_id_new(TwitterEndpointChat *chat)
{
	TwitterEndpointChatId *chat_id;
	g_return_val_if_fail(chat != NULL, NULL);

	chat_id = g_slice_new0(TwitterEndpointChatId);
	chat_id->account = chat->account;
	chat_id->chat_name = g_strdup(chat->chat_name);
	return chat_id;
}
void twitter_endpoint_chat_id_free(TwitterEndpointChatId *chat_id)
{
	if (chat_id == NULL)
		return;
	g_free(chat_id->chat_name);
	chat_id->chat_name = NULL;

	g_slice_free(TwitterEndpointChatId, chat_id);
}
TwitterEndpointChat *twitter_endpoint_chat_find_by_id(TwitterEndpointChatId *chat_id)
{
	g_return_val_if_fail(chat_id != NULL, NULL);
	return twitter_find_chat_context(chat_id->account, chat_id->chat_name);
}

PurpleConversation *twitter_chat_context_find_conv(TwitterEndpointChat *ctx)
{
#if _HAZE_
	return purple_find_conversation_with_account(PURPLE_CONV_TYPE_IM, ctx->chat_name, ctx->account);
#else
	return purple_find_conversation_with_account(PURPLE_CONV_TYPE_CHAT, ctx->chat_name, ctx->account);
#endif
}

//Taken mostly from blist.c
//XXX
static PurpleChat *_twitter_blist_chat_find(PurpleAccount *account, TwitterChatType chat_type,
	const char *component_key, const char *component_value)
{
	const char *node_chat_name;
	gint node_chat_type = 0;
	const char *node_chat_type_str;
	PurpleChat *chat;
	PurpleBlistNode *node, *group;
	char *normname;
	PurpleBuddyList *purplebuddylist = purple_get_blist();
	GHashTable *components;

	g_return_val_if_fail(purplebuddylist != NULL, NULL);
	g_return_val_if_fail((component_value != NULL) && (*component_value != '\0'), NULL);

	normname = g_strdup(purple_normalize(account, component_value));
	purple_debug_info(TWITTER_PROTOCOL_ID, "Account %s searching for chat %s type %d\n",
		account->username,
		component_value == NULL ? "NULL" : component_value,
		chat_type);

	if (normname == NULL)
	{
		return NULL;
	}
	for (group = purplebuddylist->root; group != NULL; group = group->next) {
		for (node = group->child; node != NULL; node = node->next) {
			if (PURPLE_BLIST_NODE_IS_CHAT(node)) {

				chat = (PurpleChat*)node;

				if (account != chat->account)
					continue;

				components = purple_chat_get_components(chat);
				if (components != NULL)
				{
					node_chat_type_str = g_hash_table_lookup(components, "chat_type");
					node_chat_name = g_hash_table_lookup(components, component_key);
					node_chat_type = node_chat_type_str == NULL ? 0 : strtol(node_chat_type_str, NULL, 10);

					if (node_chat_name != NULL && node_chat_type == chat_type && !strcmp(purple_normalize(account, node_chat_name), normname))
					{
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
PurpleChat *twitter_blist_chat_find_search(PurpleAccount *account, const char *name)
{
	return _twitter_blist_chat_find(account, TWITTER_CHAT_SEARCH, "search", name);
}

//XXX
PurpleChat *twitter_blist_chat_find_timeline(PurpleAccount *account, gint timeline_id)
{
	char *tmp = g_strdup_printf("%d", timeline_id);
	PurpleChat *chat = _twitter_blist_chat_find(account, TWITTER_CHAT_TIMELINE, "timeline_id", tmp);
	g_free(tmp);
	return chat;
}

//XXX TODO: fix me
PurpleChat *twitter_find_blist_chat(PurpleAccount *account, const char *name)
{
	static char *timeline = "Timeline: ";
	static char *search = "Search: ";
	PurpleChat *c = NULL;
	if (strlen(name) > strlen(timeline) && !strncmp(timeline, name, strlen(timeline)))
	{
		c = twitter_blist_chat_find_timeline(account, 0);
	} else if (strlen(name) > strlen(search) && !strncmp(search, name, strlen(search))) {
		c = twitter_blist_chat_find_search(account, name + strlen(search));
	} else {
		c = twitter_blist_chat_find_search(account, name);
	}
	return c;
}

//TODO should be static?
gboolean twitter_chat_auto_open(PurpleChat *chat)
{
	g_return_val_if_fail(chat != NULL, FALSE);
	GHashTable *components = purple_chat_get_components(chat);
	char *auto_open = g_hash_table_lookup(components, "auto_open");
	return (auto_open != NULL && auto_open[0] != '0');
}

#if _HAZE_
void twitter_chat_add_tweet(PurpleConvIm *chat, const char *who, const char *message, long long id, time_t time)
#else
void twitter_chat_add_tweet(PurpleConvChat *chat, const char *who, const char *message, long long id, time_t time)
#endif
{
	gchar *tweet;
	purple_debug_info(TWITTER_PROTOCOL_ID, "%s\n", G_STRFUNC);
	g_return_if_fail(chat != NULL);
	g_return_if_fail(who != NULL);
	g_return_if_fail(message != NULL);

	tweet = twitter_format_tweet(
			purple_conversation_get_account(chat->conv),
			who,
			message,
			id);
#if _HAZE_
	//This isn't in twitter_Format_tweet because we can't distinguish between a im and a chat
	gchar *tweet2 = g_strdup_printf("%s: %s", who, tweet);
	g_free(tweet);
	tweet = tweet2;
	serv_got_im(purple_conversation_get_gc(chat->conv), chat->conv->name, tweet, PURPLE_MESSAGE_RECV, time);
#else
	if (!purple_conv_chat_find_user(chat, who))
	{
		purple_debug_info(TWITTER_PROTOCOL_ID, "added %s to chat %s\n",
				who,
				purple_conversation_get_name(purple_conv_chat_get_conversation(chat)));
		purple_conv_chat_add_user(chat,
				who,
				NULL,   /* user-provided join message, IRC style */
				PURPLE_CBFLAGS_NONE,
				FALSE);  /* show a join message */
	}
	purple_debug_info(TWITTER_PROTOCOL_ID, "message %s\n", message);
	serv_got_chat_in(purple_conversation_get_gc(purple_conv_chat_get_conversation(chat)),
			purple_conv_chat_get_id(chat),
			who,
			PURPLE_MESSAGE_RECV,
			tweet,
			time);
#endif
	g_free(tweet);
}

static gboolean twitter_endpoint_chat_interval_timeout(gpointer data)
{
	TwitterEndpointChat *endpoint = data;
	if (endpoint->settings->interval_timeout)
		return endpoint->settings->interval_timeout(endpoint);
	return FALSE;
}


void twitter_endpoint_chat_start(PurpleConnection *gc, TwitterEndpointChatSettings *settings,
		GHashTable *components, gboolean open_conv) 
{
        const char *interval_str = g_hash_table_lookup(components, "interval");
        int interval = 0;

	g_return_if_fail(settings != NULL);

        interval = interval_str == NULL ? 0 : strtol(interval_str, NULL, 10);

	PurpleAccount *account = purple_connection_get_account(gc);
        int default_interval = settings->get_default_interval(account);
	gchar *error = NULL;

	if (settings->verify_components && (error = settings->verify_components(components)))
	{
		purple_notify_info(gc,  /* plugin handle or PurpleConnection */
				("Chat Join"),
				("Invalid Chat"),
				(error));
		g_free(error);
		return;
	}

        if (interval < 1)
                interval = default_interval;

	char *chat_name = settings->get_name(components);

#if _HAZE_
	//HAZE only works when the conv has already been created
	//as opposed to right before the conversation has been created
        if (purple_find_conversation_with_account(PURPLE_CONV_TYPE_IM, chat_name, account)) {
#else
        if (!purple_find_conversation_with_account(PURPLE_CONV_TYPE_CHAT, chat_name, account)) {
		if (open_conv)
		{
			guint chat_id = twitter_get_next_chat_id();
			serv_got_joined_chat(gc, chat_id, chat_name);
		}
#endif
		if (!twitter_find_chat_context(account, chat_name))
		{
			TwitterConnectionData *twitter = gc->proto_data;
			TwitterEndpointChat *endpoint_chat = twitter_endpoint_chat_new(
					settings, settings->type, account, chat_name, components);
			g_hash_table_insert(twitter->chat_contexts, g_strdup(chat_name), endpoint_chat);
			settings->on_start(endpoint_chat);

			endpoint_chat->timer_handle = purple_timeout_add_seconds(
					60 * interval,
					twitter_endpoint_chat_interval_timeout, endpoint_chat);
		}
        } else {
                purple_debug_info(TWITTER_PROTOCOL_ID, "Chat %s is already open.", chat_name);
        }
	g_free(chat_name);
}

TwitterEndpointChat *twitter_find_chat_context(PurpleAccount *account, const char *chat_name)
{
	PurpleConnection *gc = purple_account_get_connection(account);
	TwitterConnectionData *twitter = gc->proto_data;
	return (TwitterEndpointChat *) g_hash_table_lookup(twitter->chat_contexts, chat_name);
}
gpointer twitter_find_chat_context_endpoint_data(PurpleAccount *account, const char *chat_name)
{
	TwitterEndpointChat *ctx_base = twitter_find_chat_context(account, chat_name);
	if (!ctx_base)
		return NULL;
	return ctx_base->endpoint_data;
}

#if _HAZE_
PurpleConvIm *twitter_endpoint_chat_get_conv(TwitterEndpointChat *endpoint_chat)
{
	return PURPLE_CONV_IM(twitter_chat_context_find_conv(endpoint_chat));
}
#else
PurpleConvChat *twitter_endpoint_chat_get_conv(TwitterEndpointChat *endpoint_chat)
{
	PurpleConversation *conv = twitter_chat_context_find_conv(endpoint_chat);
	PurpleChat *blist_chat;
	if (conv == NULL && (blist_chat = twitter_find_blist_chat(endpoint_chat->account, endpoint_chat->chat_name)))
	{
		if (twitter_chat_auto_open(blist_chat))
		{
			purple_debug_info(TWITTER_PROTOCOL_ID, "%s, recreated conv for auto open chat (%s)\n", G_STRFUNC, endpoint_chat->chat_name);
			guint chat_id = twitter_get_next_chat_id();
			conv = serv_got_joined_chat(purple_account_get_connection(endpoint_chat->account), chat_id, endpoint_chat->chat_name);
		}
	}
	if (!conv)
		return NULL;
	return PURPLE_CONV_CHAT(conv);
}
#endif
