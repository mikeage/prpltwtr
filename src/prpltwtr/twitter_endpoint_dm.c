#include "twitter_endpoint_dm.h"
#include "twitter_util.h"

static void twitter_send_dm_success_cb(PurpleAccount *account, xmlnode *node, gboolean last, gpointer _who)
{
	if (last && _who)
		g_free(_who);
}
static gboolean twitter_send_dm_error_cb(PurpleAccount *account, const TwitterRequestErrorData *error_data, gpointer _who)
{
	gchar *who = _who;
	if (who)
	{
		gchar *conv_name = twitter_endpoint_im_buddy_name_to_conv_name(twitter_endpoint_im_find(account, TWITTER_IM_TYPE_DM), _who);
		gchar * error = g_strdup_printf("Error sending DM: %s", error_data->message ? error_data->message : "unknown error");
		purple_conv_present_error(conv_name, account, error);
		g_free(error);
		g_free(who);
		g_free(conv_name);
	}

	return FALSE; //give up trying
}

static int twitter_send_dm_do(PurpleAccount *account, const char *who,
		const char *message, PurpleMessageFlags flags)
{
	GArray *statuses = twitter_utf8_get_segments(message, MAX_TWEET_LENGTH, NULL);

	twitter_api_send_dms(purple_account_get_requestor(account),
			who,
			statuses,
			twitter_send_dm_success_cb,
			twitter_send_dm_error_cb,
			g_strdup(who)); //TODO

	return 1;
}

typedef struct 
{
	void (*success_cb)(PurpleAccount *account, long long id, gpointer user_data);
	void (*error_cb)(PurpleAccount *account, const TwitterRequestErrorData *error_data, gpointer user_data);
	gpointer user_data;
} TwitterLastSinceIdRequest;

static void _process_dms(PurpleAccount *account,
		GList *statuses,
		TwitterConnectionData *twitter)
{
	GList *l;
	TwitterEndpointIm *ctx = twitter_connection_get_endpoint_im(twitter, TWITTER_IM_TYPE_DM);

	for (l = statuses; l; l = l->next)
	{
		TwitterUserTweet *data = l->data;
		TwitterTweet *status = twitter_user_tweet_take_tweet(data);
		TwitterUserData *user_data = twitter_user_tweet_take_user_data(data);

		if (!user_data)
		{
			twitter_status_data_free(status);
		} else {
			twitter_buddy_set_user_data(account, user_data, FALSE);
			twitter_status_data_update_conv(ctx, data->screen_name, status);
			twitter_status_data_free(status);
		}
		twitter_user_tweet_free(data);
	}
}

static void twitter_get_dms_all_cb(TwitterRequestor *r,
		GList *nodes,
		gpointer user_data)
{
	PurpleConnection *gc = purple_account_get_connection(r->account);
	TwitterConnectionData *twitter = gc->proto_data;

	GList *dms = twitter_dms_nodes_parse(nodes);
	_process_dms(r->account, dms, twitter);

	g_list_free(dms);
}


static gboolean twitter_get_dms_all_timeout_error_cb(TwitterRequestor *r,
		const TwitterRequestErrorData *error_data,
		gpointer user_data)
{
	return TRUE; //restart timer and try again
}

static void twitter_get_dms_get_last_since_id_success_cb(TwitterRequestor *requestor, xmlnode *node, gpointer user_data)
{
	TwitterLastSinceIdRequest *r = user_data;
	long long id = 0;
	xmlnode *status_node = xmlnode_get_child(node, "direct_message");
	purple_debug_info(TWITTER_PROTOCOL_ID, "%s\n", G_STRFUNC);
	if (status_node != NULL)
	{
		TwitterTweet *status_data = twitter_dm_node_parse(status_node);
		if (status_data != NULL)
		{
			id = status_data->id;

			twitter_status_data_free(status_data);
		}
	}
	r->success_cb(requestor->account, id, r->user_data);
	g_free(r);
}

static void twitter_get_last_since_id_error_cb(TwitterRequestor *requestor, const TwitterRequestErrorData *error_data, gpointer user_data)
{
	TwitterLastSinceIdRequest *r = user_data;
	r->error_cb(requestor->account, error_data, r->user_data);
	g_free(r);
}


static void twitter_get_dms_last_since_id(PurpleAccount *account,
	void (*success_cb)(PurpleAccount *account, long long id, gpointer user_data),
	void (*error_cb)(PurpleAccount *account, const TwitterRequestErrorData *error_data, gpointer user_data),
	gpointer user_data)
{
	TwitterLastSinceIdRequest *request = g_new0(TwitterLastSinceIdRequest, 1);
	request->success_cb = success_cb;
	request->error_cb = error_cb;
	request->user_data = user_data;
	/* Simply get the last reply */
	twitter_api_get_dms(purple_account_get_requestor(account),
			0, 1, 1,
			twitter_get_dms_get_last_since_id_success_cb,
			twitter_get_last_since_id_error_cb,
			request);
}


static TwitterEndpointImSettings TwitterEndpointDmSettings =
{
	TWITTER_IM_TYPE_DM,
	"twitter_last_dm_id",
	"d ", //conv_id
	twitter_send_dm_do,
	twitter_option_dms_timeout,
	twitter_api_get_dms_all,
	twitter_get_dms_all_cb,
	twitter_get_dms_all_timeout_error_cb,
	twitter_get_dms_last_since_id,
	NULL, //convo_closed
};

TwitterEndpointImSettings *twitter_endpoint_dm_get_settings()
{
	return &TwitterEndpointDmSettings;
}
