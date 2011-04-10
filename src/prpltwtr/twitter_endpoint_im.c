#include "twitter_endpoint_im.h"
#include "twitter_util.h"
#include "twitter_conn.h"

static void twitter_endpoint_im_get_last_since_id_error_cb(PurpleAccount *account, const TwitterRequestErrorData *error_data, gpointer user_data);
static void twitter_endpoint_im_start_timer(TwitterEndpointIm *ctx);

static TwitterImType twitter_account_get_default_im_type(PurpleAccount *account)
{
	return twitter_option_default_dm(account) ? TWITTER_IM_TYPE_DM : TWITTER_IM_TYPE_AT_MSG;
}

TwitterEndpointIm *twitter_endpoint_im_find(PurpleAccount *account, TwitterImType type)
{
	PurpleConnection *gc;
	TwitterConnectionData *twitter;

	g_return_val_if_fail(type < TWITTER_IM_TYPE_UNKNOWN, NULL);

	gc = purple_account_get_connection(account);
	if (gc) {
		twitter = gc->proto_data;
		return twitter->endpoint_ims[type];
	} else {
		purple_debug_info(TWITTER_PROTOCOL_ID, "No gc available. Disconnected?");
		return NULL;
	}
}

char *twitter_endpoint_im_buddy_name_to_conv_name(TwitterEndpointIm *im, const char *name)
{
	g_return_val_if_fail(name != NULL && name[0] != '\0' && im != NULL, NULL);
	return twitter_account_get_default_im_type(im->account) == im->settings->type ?
		g_strdup(name) :
		g_strdup_printf("%s%s", im->settings->conv_id, name);
}

const char *twitter_conv_name_to_buddy_name(PurpleAccount *account, const char *name)
{
	g_return_val_if_fail(name != NULL && name[0] != '\0', NULL);
	if (name[0] == '@')
		return name + 1;
	if (name[0] == 'd' && name[1] == ' ' && name[2] != '\0')
		return name + 2;
	return name;
}


TwitterImType twitter_conv_name_to_type(PurpleAccount *account, const char *name)
{
	g_return_val_if_fail(name != NULL && name[0] != '\0', TWITTER_IM_TYPE_UNKNOWN);
	if (name[0] == '@')
		return TWITTER_IM_TYPE_AT_MSG;
	if (name[0] == 'd' && name[1] == ' ' && name[2] != '\0')
		return TWITTER_IM_TYPE_DM;
	if (twitter_option_default_dm(account))
		return TWITTER_IM_TYPE_DM;
	else
		return TWITTER_IM_TYPE_AT_MSG;
}

TwitterEndpointIm *twitter_conv_name_to_endpoint_im(PurpleAccount *account, const char *name)
{
	TwitterImType type = twitter_conv_name_to_type(account, name);
	return twitter_endpoint_im_find(account, type);
}


TwitterEndpointIm *twitter_endpoint_im_new(PurpleAccount *account, TwitterEndpointImSettings *settings, gboolean retrieve_history, gint initial_max_retrieve)
{
	TwitterEndpointIm *endpoint = g_new0(TwitterEndpointIm, 1);
	endpoint->account = account;
	endpoint->settings = settings;
	endpoint->retrieve_history = retrieve_history;
	endpoint->initial_max_retrieve = initial_max_retrieve;
	return endpoint;
}
void twitter_endpoint_im_free(TwitterEndpointIm *ctx)
{
	if (ctx->timer)
	{
		purple_timeout_remove(ctx->timer);
		ctx->timer = 0;
	}
	g_free(ctx);
}

static gboolean twitter_endpoint_im_error_cb(TwitterRequestor *r,
		const TwitterRequestErrorData *error_data,
		gpointer user_data)
{
	TwitterEndpointIm *ctx = (TwitterEndpointIm *) user_data;
	if (ctx->settings->error_cb(r, error_data, NULL))
	{
		twitter_endpoint_im_start_timer(ctx);
	}
	return FALSE;
}


static void twitter_endpoint_im_success_cb(TwitterRequestor *r,
		GList *nodes,
		gpointer user_data)
{
	TwitterEndpointIm *ctx = (TwitterEndpointIm *) user_data;
	ctx->settings->success_cb(r, nodes, NULL);
	ctx->ran_once = TRUE;
	twitter_endpoint_im_start_timer(ctx);
}

static gboolean twitter_im_timer_timeout(gpointer _ctx)
{
	TwitterEndpointIm *ctx = (TwitterEndpointIm *) _ctx;
	ctx->settings->get_im_func(purple_account_get_requestor(ctx->account),
			twitter_endpoint_im_get_since_id(ctx),
			twitter_endpoint_im_success_cb,
			twitter_endpoint_im_error_cb,
			ctx->ran_once ? -1 : ctx->initial_max_retrieve,
			ctx);
	ctx->timer = 0;
	return FALSE;
}

static void twitter_endpoint_im_get_last_since_id_success_cb(PurpleAccount *account, long long id, gpointer user_data)
{
	TwitterEndpointIm *im = user_data;

	if (id > twitter_endpoint_im_get_since_id(im))
		twitter_endpoint_im_set_since_id(im, id);

	im->ran_once = TRUE;
	twitter_endpoint_im_start_timer(im);
}

static gboolean twitter_endpoint_im_get_since_id_timeout(gpointer user_data)
{
	TwitterEndpointIm *ctx = user_data;
	ctx->settings->get_last_since_id(ctx->account,
			twitter_endpoint_im_get_last_since_id_success_cb,
			twitter_endpoint_im_get_last_since_id_error_cb,
			ctx);
	ctx->timer = 0;
	return FALSE;
}
static void twitter_endpoint_im_get_last_since_id_error_cb(PurpleAccount *account, const TwitterRequestErrorData *error_data, gpointer user_data)
{
	TwitterEndpointIm *ctx = user_data;
	ctx->timer = purple_timeout_add_seconds(60, twitter_endpoint_im_get_since_id_timeout, ctx);
}

static void twitter_endpoint_im_start_timer(TwitterEndpointIm *ctx)
{
	ctx->timer = purple_timeout_add_seconds(
			60 * ctx->settings->timespan_func(ctx->account),
			twitter_im_timer_timeout, ctx);
}

void twitter_endpoint_im_start(TwitterEndpointIm *ctx)
{
	if (ctx->timer)
	{
		purple_timeout_remove(ctx->timer);
	}
	if (twitter_endpoint_im_get_since_id(ctx) == -1 && ctx->retrieve_history)
	{
		ctx->settings->get_last_since_id(ctx->account,
				twitter_endpoint_im_get_last_since_id_success_cb,
				twitter_endpoint_im_get_last_since_id_error_cb,
				ctx);
	} else {
		twitter_im_timer_timeout(ctx);
	}
}

long long twitter_endpoint_im_get_since_id(TwitterEndpointIm *ctx)
{
	return (ctx->since_id > 0
			? ctx->since_id 
			: twitter_endpoint_im_settings_load_since_id(ctx->account, ctx->settings));
}

void twitter_endpoint_im_set_since_id(TwitterEndpointIm *ctx, long long since_id)
{
	ctx->since_id = since_id;
	twitter_endpoint_im_settings_save_since_id(ctx->account, ctx->settings, since_id);
}

long long twitter_endpoint_im_settings_load_since_id(PurpleAccount *account, TwitterEndpointImSettings *settings)
{
	return purple_account_get_long_long(account, settings->since_id_setting_id, -1);
}

void twitter_endpoint_im_settings_save_since_id(PurpleAccount *account, TwitterEndpointImSettings *settings, long long since_id)
{
	purple_account_set_long_long(account, settings->since_id_setting_id, since_id);
}

//TODO IM: rename
void twitter_status_data_update_conv(TwitterEndpointIm *ctx,
		char *buddy_name,
		TwitterTweet *s)
{
	PurpleAccount *account = ctx->account;
	PurpleConnection *gc = purple_account_get_connection(account);
	gchar *conv_name;
	gchar *tweet;

	if (!s || !s->text)
		return;

	if (s->id && s->id > twitter_endpoint_im_get_since_id(ctx))
	{
		purple_debug_info (TWITTER_PROTOCOL_ID, "saving %s\n", G_STRFUNC);
		twitter_endpoint_im_set_since_id(ctx, s->id);
	}

	conv_name = twitter_endpoint_im_buddy_name_to_conv_name(ctx, buddy_name);

	tweet = twitter_format_tweet(account,
			buddy_name,
			s->text,
			s->id,
			PURPLE_CONV_TYPE_IM,
			conv_name,
			ctx->settings->type == TWITTER_IM_TYPE_AT_MSG,
			s->in_reply_to_status_id,
			s->favorited);


	//Account received an im
	/* TODO get in_reply_to_status? s->in_reply_to_screen_name
	 * s->in_reply_to_status_id */

	serv_got_im(gc, conv_name,
			tweet,
			PURPLE_MESSAGE_RECV,
			s->created_at);

	/* Notify the GUI that a new IM was sent. This can't be done in twitter_format_tweet, since the conv window wasn't created yet (if it's the first tweet), and it can't be done by listening to the signal from serv_got_im, since we don't have the tweet there. Shame. Maybe I can refactor by storing the id in a global variable; TBD which per conv (aka per account/conv_name) structs exist ebefore calling serv_got_im */
	purple_signal_emit(purple_conversations_get_handle(),
			"prpltwtr-received-im",
			account,
			s->id,
			conv_name);
	g_free(tweet);
}

void twitter_endpoint_im_convo_closed(TwitterEndpointIm *im, const gchar *conv_name)
{
	PurpleConversation *conv;
	g_return_if_fail(im != NULL);
	g_return_if_fail(conv_name != NULL);

	if (!im->settings->convo_closed)
		return;

	conv = purple_find_conversation_with_account(PURPLE_CONV_TYPE_IM, conv_name, im->account);
	if (!conv)
		return;

	im->settings->convo_closed(conv);
}
