#include "prpl.h"
#include "account.h"
#include "debug.h"

#include "pt.h"
#include "pt_endpoint_im.h"
#include "pt_connection.h"
#include "pt_prefs.h"

static void     pt_endpoint_im_get_last_since_id_error_cb (PurpleAccount * account, const PtRequestorErrorData * error_data, gpointer user_data);
static void     pt_endpoint_im_start_timer (PtEndpointIm * endpoint);

static PtImType pt_account_get_default_im_type (PurpleAccount * account)
{
	return pt_option_default_dm (account) ? PT_IM_TYPE_DM : PT_IM_TYPE_AT_MSG;
}

PtEndpointIm   *pt_endpoint_im_find (PurpleAccount * account, PtImType type)
{
	PurpleConnection *gc;
	PtConnectionData *conn_data;

	g_return_val_if_fail (type < PT_IM_TYPE_UNKNOWN, NULL);

	gc = purple_account_get_connection (account);
	if (gc)
	{
		conn_data = gc->proto_data;
		return conn_data->endpoint_ims[type];
	}
	else
	{
		purple_debug_warning (purple_account_get_protocol_id (account), "No gc available. Disconnected?");
		return NULL;
	}
}

char           *pt_endpoint_im_buddy_name_to_conv_name (PtEndpointIm * im, const char *name)
{
	g_return_val_if_fail (name != NULL && name[0] != '\0' && im != NULL, NULL);
	return pt_account_get_default_im_type (im->account) == im->settings->type ? g_strdup (name) : g_strdup_printf ("%s%s", im->settings->conv_id, name);
}

const char     *pt_conv_name_to_buddy_name (PurpleAccount * account, const char *name)
{
	g_return_val_if_fail (name != NULL && name[0] != '\0', NULL);
	if (name[0] == '@')
		return name + 1;
	if (name[0] == 'd' && name[1] == ' ' && name[2] != '\0')
		return name + 2;
	return name;
}

PtImType pt_conv_name_to_type (PurpleAccount * account, const char *name)
{
	g_return_val_if_fail (name != NULL && name[0] != '\0', PT_IM_TYPE_UNKNOWN);
	if (name[0] == '@')
		return PT_IM_TYPE_AT_MSG;
	if (name[0] == 'd' && name[1] == ' ' && name[2] != '\0')
		return PT_IM_TYPE_DM;
	if (pt_option_default_dm (account))
		return PT_IM_TYPE_DM;
	else
		return PT_IM_TYPE_AT_MSG;
}

PtEndpointIm   *pt_conv_name_to_endpoint_im (PurpleAccount * account, const char *name)
{
	PtImType        type = pt_conv_name_to_type (account, name);

	return pt_endpoint_im_find (account, type);
}

PtEndpointIm   *pt_endpoint_im_new (PurpleAccount * account, PtEndpointImSettings * settings, gboolean retrieve_history, gint initial_max_retrieve)
{
	PtEndpointIm   *endpoint = g_new0 (PtEndpointIm, 1);

	endpoint->account = account;
	endpoint->settings = settings;
	endpoint->retrieve_history = retrieve_history;
	endpoint->initial_max_retrieve = initial_max_retrieve;
	return endpoint;
}

void pt_endpoint_im_free (PtEndpointIm * endpoint)
{
	if (endpoint->timer)
	{
		purple_timeout_remove (endpoint->timer);
		endpoint->timer = 0;
	}
	g_free (endpoint);
}

static gboolean pt_endpoint_im_error_cb (PtRequestor * r, const PtRequestorErrorData * error_data, gpointer user_data)
{
	PtEndpointIm   *endpoint = (PtEndpointIm *) user_data;

	if (endpoint->settings->error_cb (r, error_data, NULL))
	{
		pt_endpoint_im_start_timer (endpoint);
	}
	return FALSE;
}

static void pt_endpoint_im_success_cb (PtRequestor * r, GList * nodes, gpointer user_data)
{
	PtEndpointIm   *endpoint = (PtEndpointIm *) user_data;

	endpoint->settings->success_cb (r, nodes, NULL);
	endpoint->ran_once = TRUE;
	pt_endpoint_im_start_timer (endpoint);
}

static gboolean pt_im_timer_timeout (gpointer user_data)
{
	PtEndpointIm   *endpoint = (PtEndpointIm *) user_data;

	endpoint->settings->get_im_func (pt_requestor_get_requestor (endpoint->account), pt_endpoint_im_get_since_id (endpoint), pt_endpoint_im_success_cb, pt_endpoint_im_error_cb, endpoint->ran_once ? -1 : endpoint->initial_max_retrieve, endpoint);
	endpoint->timer = 0;
	return FALSE;
}

static void pt_endpoint_im_get_last_since_id_success_cb (PurpleAccount * account, long long id, gpointer user_data)
{
	PtEndpointIm   *endpoint = user_data;

	if (id > pt_endpoint_im_get_since_id (endpoint))
	{
		pt_endpoint_im_set_since_id (endpoint, id);
	}

	endpoint->ran_once = TRUE;
	pt_endpoint_im_start_timer (endpoint);
}

static gboolean pt_endpoint_im_get_since_id_timeout (gpointer user_data)
{
	PtEndpointIm   *endpoint = user_data;

	endpoint->settings->get_last_since_id (endpoint->account, pt_endpoint_im_get_last_since_id_success_cb, pt_endpoint_im_get_last_since_id_error_cb, endpoint);
	endpoint->timer = 0;
	return FALSE;
}

static void pt_endpoint_im_get_last_since_id_error_cb (PurpleAccount * account, const PtRequestorErrorData * error_data, gpointer user_data)
{
	PtEndpointIm   *endpoint = user_data;

	endpoint->timer = purple_timeout_add_seconds (60, pt_endpoint_im_get_since_id_timeout, endpoint);
}

static void pt_endpoint_im_start_timer (PtEndpointIm * endpoint)
{
	endpoint->timer = purple_timeout_add_seconds (60 * endpoint->settings->timespan_func (endpoint->account), pt_im_timer_timeout, endpoint);
}

void pt_endpoint_im_start (PtEndpointIm * endpoint)
{
	if (endpoint->timer)
	{
		purple_timeout_remove (endpoint->timer);
	}
	if (pt_endpoint_im_get_since_id (endpoint) == -1 && endpoint->retrieve_history)
	{
		endpoint->settings->get_last_since_id (endpoint->account, pt_endpoint_im_get_last_since_id_success_cb, pt_endpoint_im_get_last_since_id_error_cb, endpoint);
	}
	else
	{
		pt_im_timer_timeout (endpoint);
	}
}

long long pt_endpoint_im_get_since_id (PtEndpointIm * endpoint)
{
	return (endpoint->since_id > 0 ? endpoint->since_id : pt_endpoint_im_settings_load_since_id (endpoint->account, endpoint->settings));
}

void pt_endpoint_im_set_since_id (PtEndpointIm * endpoint, long long since_id)
{
	endpoint->since_id = since_id;
	pt_endpoint_im_settings_save_since_id (endpoint->account, endpoint->settings, since_id);
}

long long pt_endpoint_im_settings_load_since_id (PurpleAccount * account, PtEndpointImSettings * settings)
{
	return pt_get_long_long (account, settings->since_id_setting_id, -1);
}

void pt_endpoint_im_settings_save_since_id (PurpleAccount * account, PtEndpointImSettings * settings, long long since_id)
{
	pt_set_long_long (account, settings->since_id_setting_id, since_id);
}

void pt_endpoint_im_update_conv(PtEndpointIm * endpoint, char *buddy_name, void /* PtTweet */ * s)
{
	PurpleAccount  *account = endpoint->account;
	PurpleConnection *gc = purple_account_get_connection (account);
	gchar          *conv_name;
	gchar          *tweet;
#if 0
	if (!s || !s->text)
		return;

	if (s->id && s->id > pt_endpoint_im_get_since_id (endpoint))
	{
		purple_debug_info (purple_account_get_protocol_id (account), "saving %s\n", G_STRFUNC);
		pt_endpoint_im_set_since_id (endpoint, s->id);
	}

	conv_name = pt_endpoint_im_buddy_name_to_conv_name (endpoint, buddy_name);

	tweet = pt_format_tweet (account, buddy_name, s->text, s->id, PURPLE_CONV_TYPE_IM, conv_name, endpoint->settings->type == PT_IM_TYPE_AT_MSG, s->in_reply_to_status_id, s->favorited);

	//Account received an im
	/* TODO get in_reply_to_status? s->in_reply_to_screen_name
	 * s->in_reply_to_status_id */

	serv_got_im (gc, conv_name, tweet, PURPLE_MESSAGE_RECV, s->created_at);

	/* Notify the GUI that a new IM was sent. This can't be done in twitter_format_tweet, since the conv window wasn't created yet (if it's the first tweet), and it can't be done by listening to the signal from serv_got_im, since we don't have the tweet there. Shame. Maybe I can refactor by storing the id in a global variable; TBD which per conv (aka per account/conv_name) structs exist ebefore calling serv_got_im */
	purple_signal_emit (purple_conversations_get_handle (), "pt-received-im", account, s->id, conv_name);
	g_free (tweet);
#endif
}

void pt_endpoint_im_convo_closed (PtEndpointIm * im, const gchar * conv_name)
{
	PurpleConversation *conv;

	g_return_if_fail (im != NULL);
	g_return_if_fail (conv_name != NULL);

	if (!im->settings->convo_closed)
		return;

	conv = purple_find_conversation_with_account (PURPLE_CONV_TYPE_IM, conv_name, im->account);
	if (!conv)
		return;

	im->settings->convo_closed (conv);
}
