#include "twitter_endpoint_im.h"
#include "twitter_util.h"
static void twitter_endpoint_im_get_last_since_id_error_cb(PurpleAccount *account, const TwitterRequestErrorData *error_data, gpointer user_data);
static void twitter_endpoint_im_start_timer(TwitterEndpointIm *ctx);

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

static gboolean twitter_endpoint_im_error_cb(PurpleAccount *account,
		const TwitterRequestErrorData *error_data,
		gpointer user_data)
{
	TwitterEndpointIm *ctx = (TwitterEndpointIm *) user_data;
	if (ctx->settings->error_cb(account, error_data, NULL))
	{
		twitter_endpoint_im_start_timer(ctx);
	}
	return FALSE;
}


static void twitter_endpoint_im_success_cb(PurpleAccount *account,
		GList *nodes,
		gpointer user_data)
{
	TwitterEndpointIm *ctx = (TwitterEndpointIm *) user_data;
	ctx->settings->success_cb(account, nodes, NULL);
	ctx->ran_once = TRUE;
	twitter_endpoint_im_start_timer(ctx);
}

static gboolean twitter_im_timer_timeout(gpointer _ctx)
{
	TwitterEndpointIm *ctx = (TwitterEndpointIm *) _ctx;
	ctx->settings->get_im_func(ctx->account,
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

