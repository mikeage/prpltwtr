#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "defaults.h"

#include <gtkplugin.h>
#include <gtkconv.h>
#include <gtkimhtml.h>
#include "twitter.h"
#include "gtkprpltwtr_prefs.h"
#include "twitter_charcount.h"
#include "twitter_convicon.h"
#include "gtkprpltwtr.h"
#include <ctype.h>

#ifdef WIN32
#include "win32dep.h"
#endif

#ifdef ENABLE_NLS
#include <glib/gi18n-lib.h>
#else
#define _(String) ((/* const */ char *) (String))
#define N_(String) ((/* const */ char *) (String))
#endif // ENABLE NLS

static PurplePlugin *gtkprpltwtr_plugin = NULL;

//TODO: Move to separate file
#if PURPLE_VERSION_CHECK(2, 6, 0)
typedef struct
{
	PurpleConversationType type;
	gchar *conv_name;
} TwitterConversationId;

static void twitter_conv_id_write_message(PurpleAccount *account, TwitterConversationId *conv_id,
		PurpleMessageFlags flags, const gchar *message)
{
	PurpleConversation *conv;
	g_return_if_fail(conv_id != NULL);

	conv = purple_find_conversation_with_account(conv_id->type, conv_id->conv_name, account);

	if (conv)
	{
		purple_conversation_write(conv, NULL, message, flags, time(NULL));
	}

	g_free(conv_id->conv_name);
	g_free(conv_id);
}

static void twitter_send_rt_success_cb(TwitterRequestor *r,
		xmlnode *node,
		gpointer user_data)
{
	TwitterConversationId *conv_id = user_data;
	twitter_conv_id_write_message(r->account, conv_id, PURPLE_MESSAGE_SYSTEM, _("Successfully retweeted"));
}

static void twitter_send_rt_error_cb(TwitterRequestor *r,
		const TwitterRequestErrorData *error_data,
		gpointer user_data)
{
	TwitterConversationId *conv_id = user_data;
	gchar * error = g_strdup_printf(_("Couldn't retweet status: %s"), error_data->message ? error_data->message : _("unknown error"));
	twitter_conv_id_write_message(r->account, conv_id, PURPLE_MESSAGE_ERROR, error);
	g_free(error);
}

static void twitter_report_spammer_success_cb(TwitterRequestor *r,
		xmlnode *node,
		gpointer user_data)
{
	TwitterConversationId *conv_id = user_data;
	twitter_conv_id_write_message(r->account, conv_id, PURPLE_MESSAGE_SYSTEM, _("Successfully reported as spam"));
}

static void twitter_report_spammer_error_cb(TwitterRequestor *r,
		const TwitterRequestErrorData *error_data,
		gpointer user_data)
{
	TwitterConversationId *conv_id = user_data;
	gchar * error = g_strdup_printf(_("Couldn't report spammer: %s"), error_data->message ? error_data->message : _("unknown error"));
	twitter_conv_id_write_message(r->account, conv_id, PURPLE_MESSAGE_ERROR, error);
	g_free(error);
}
static void twitter_get_status_success_cb(TwitterRequestor *r,
		xmlnode *node,
		gpointer user_data)
{
	TwitterTweet *status;
	TwitterUserData *user;
	TwitterConversationId *conv_id = user_data;
	TwitterUserTweet * user_tweet;

	PurpleConnection *gc;
	TwitterConnectionData *twitter;

	if (!conv_id)
		return;
	gc = purple_account_get_connection(r->account);
	if (!gc)
		return;

	twitter = gc->proto_data;
	if (!twitter)
		return;

	status = twitter_status_node_parse(node);
	if (!status || !status->text || !status->id)
	{
		purple_debug_info(DEBUG_ID, "Essential information missing from the tweet!\n");
		return;
	}

	user = twitter_user_node_parse(xmlnode_get_child(node, "user"));
	if (!user || !user->screen_name)
	{
		purple_debug_info(DEBUG_ID, "Essential information missing from the user!\n");
		return;
	}

	user_tweet = twitter_user_tweet_new(user->screen_name, user->profile_image_url, user, status);

	switch(conv_id->type) {
		case PURPLE_CONV_TYPE_CHAT:
			{
				TwitterEndpointChat *endpoint_chat = twitter_endpoint_chat_find(r->account, conv_id->conv_name);
				twitter_chat_got_tweet(endpoint_chat, user_tweet);
			}
			break;
		case PURPLE_CONV_TYPE_IM:
			{
				TwitterEndpointIm *endpoint_im = twitter_connection_get_endpoint_im(twitter, TWITTER_IM_TYPE_AT_MSG);
				twitter_status_data_update_conv(endpoint_im, user->screen_name, status);
			}
			break;
		default:
			break;
	}

	twitter_user_tweet_free(user_tweet);
	g_free(conv_id->conv_name);
	g_free(conv_id);
}

static void twitter_get_status_error_cb(TwitterRequestor *r,
		const TwitterRequestErrorData *error_data,
		gpointer user_data)
{
	TwitterConversationId *conv_id = user_data;
	gchar * error = g_strdup_printf(_("Couldn't get status: %s"), error_data->message ? error_data->message : _("unknown error"));
	twitter_conv_id_write_message(r->account, conv_id, PURPLE_MESSAGE_ERROR, error);
	g_free(error);
}

static void twitter_delete_tweet_success_cb(TwitterRequestor *r,
		xmlnode *node,
		gpointer user_data)
{
	TwitterConversationId *conv_id = user_data;
	twitter_conv_id_write_message(r->account, conv_id, PURPLE_MESSAGE_SYSTEM, _("Successfully deleted"));
}

static void twitter_delete_tweet_error_cb(TwitterRequestor *r,
		const TwitterRequestErrorData *error_data,
		gpointer user_data)
{
	TwitterConversationId *conv_id = user_data;
	gchar * error = g_strdup_printf(_("Couldn't delete status: %s"), error_data->message ? error_data->message : _("unknown error"));
	twitter_conv_id_write_message(r->account, conv_id, PURPLE_MESSAGE_ERROR, error);
	g_free(error);
}

static void gtkprpltwtr_mark_reply(PurpleConversation * conv, const gchar * id_str)
{
	GtkIMHtml *imhtml = NULL;
	GtkTextBuffer *text_buffer = NULL;
	GtkTextIter insertion_point;
	GtkTextIter old_insertion_point;
	GtkTextIter old_insertion_point_end;
	GtkTextMark * id_mark = NULL; /* new reply location's id_str mark */
	GtkTextMark * reply_mark = NULL; /* reply mark itself */
	GdkPixbuf *reply_icon = NULL;

	imhtml = GTK_IMHTML(PIDGIN_CONVERSATION(conv)->imhtml);
	text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(imhtml));

	purple_debug_info(DEBUG_ID, "Setting the reply mark for %p to %s\n", conv, id_str ? id_str : "DISABLED");

	/* First erase the old reply mark, if any */
	reply_mark = gtk_text_buffer_get_mark(text_buffer, "prpltwtr-reply-mark");
	if (reply_mark)
	{
		gtk_text_buffer_get_iter_at_mark(text_buffer, &old_insertion_point, reply_mark);
		old_insertion_point_end = old_insertion_point;
		gtk_text_iter_forward_char(&old_insertion_point_end);
		gtk_text_buffer_delete(text_buffer, &old_insertion_point, &old_insertion_point_end);
	}

	/* If there's no new mark, delete the old mark, and then we're done here */
	if (!id_str)
	{
		/* Doesn't matter if this fails */
		gtk_text_buffer_delete_mark_by_name(text_buffer, "prpltwtr-reply-mark");
		return;
	}

	/* Move or create the reply mark */
	id_mark = gtk_text_buffer_get_mark(text_buffer, id_str);
	gtk_text_buffer_get_iter_at_mark(text_buffer, &insertion_point, id_mark);
	if (reply_mark)
	{
		gtk_text_buffer_move_mark(text_buffer, reply_mark, &insertion_point);
	} else {
		reply_mark = gtk_text_buffer_create_mark(text_buffer, "prpltwtr-reply-mark", &insertion_point, TRUE);
	}

	/* Add the new icon */
	reply_icon = gtk_widget_render_icon((GtkWidget *)imhtml, GTK_STOCK_OK, GTK_ICON_SIZE_MENU, NULL);
	if(!reply_icon) {
		purple_debug_info(DEBUG_ID, "Couldn't load reply icon!\n");
		return;
	}
	gtk_text_buffer_insert_pixbuf(text_buffer, &insertion_point, reply_icon);
	g_object_unref(reply_icon);
}

static gboolean twitter_uri_handler(const char *proto, const char *cmd_arg, GHashTable *params)
{
	/* TODO: refactor all of this
	 * I'll wait until I create the graphical tweet actions portion
	 */
	const char *text;
	const char *username;
	PurpleAccount *account;
	const gchar *action;
	PidginConversation *gtkconv;
	purple_debug_info(DEBUG_ID, "%s PROTO %s CMD_ARG %s\n", G_STRFUNC, proto, cmd_arg);

	g_return_val_if_fail(proto != NULL, FALSE);
	g_return_val_if_fail(cmd_arg != NULL, FALSE);

	//don't handle someone elses proto
	if (strcmp(proto, TWITTER_URI))
		return FALSE;

	username = g_hash_table_lookup(params, "account");

	if (username == NULL || username[0] == '\0')
	{
		purple_debug_info(DEBUG_ID, "malformed uri. No account username\n");
		return FALSE;
	}

	//ugly hack to fix username highlighting
	account = purple_accounts_find(username+1, TWITTER_PROTOCOL_ID);

	if (account == NULL)
	{
		purple_debug_info(DEBUG_ID, "could not find account %s\n", username);
		return FALSE;
	}

	while (cmd_arg[0] == '/')
		cmd_arg++;

	action = g_hash_table_lookup(params, "action");
	if (action)
		cmd_arg = action;
	purple_debug_info(DEBUG_ID, "Account %s got action %s\n", username, cmd_arg);
	if (!strcmp(cmd_arg, TWITTER_URI_ACTION_USER))
	{
		purple_notify_info(purple_account_get_connection(account),
				_("Clicked URI"),
				_("@name clicked"),
				_("Sorry, this has not been implemented yet"));
	} else if (!strcmp(cmd_arg, TWITTER_URI_ACTION_REPLYALL)) {
		const char *id_str, *user, *text;
		char others[140];
		int i,j;
		long long id;
		PurpleConversation *conv;
		id_str = g_hash_table_lookup(params, "id");
		user = g_hash_table_lookup(params, "user");
		text =  purple_url_decode(g_hash_table_lookup(params, "text"));

		if (id_str == NULL || user == NULL || id_str[0] == '\0' || user[0] == '\0' || text == NULL || text[0] == '\0')
		{
			purple_debug_info(DEBUG_ID, "malformed uri. Invalid id/user for reply\n");
			purple_debug_info(DEBUG_ID, "id_str: 0x%X. user: 0x%X. text: 0x%X", (int) id_str,(int) user,(int) text);
			return FALSE;
		}
		id = strtoll(id_str, NULL, 10);
		if (id == 0)
		{
			purple_debug_info(DEBUG_ID, "malformed uri. Invalid id for reply\n");
			return FALSE;
		}
		conv = twitter_endpoint_reply_conversation_new(twitter_endpoint_im_find(account, TWITTER_IM_TYPE_AT_MSG), user, id, TRUE);
		if (!conv)
		{
			return FALSE;
		}

		purple_conversation_set_data(conv, "twitter_conv_last_reply_id_manual", NULL);
		gtkprpltwtr_mark_reply(conv, NULL);

		i=0;
		memset(others, 0, sizeof(others));
		while(text[i] != '\0') {
			if (text[i] == '@') {
				j=i;
				do {
					j++;
				} while((text[j] != '\0') && (!isspace(text[j]) && !strchr("!@#$%^&*()-=+[]{};:'\"<>?,./`~", text[j])));

				if((i+1) == j) {
					// empty string
					i++;
					continue;
				}
				if (memcmp(&text[i+1], username+1, j-(i+1)) && memcmp(&text[i+1], user, j-(i+1))) {
					memcpy(&others[strlen(others)], &text[i], j-(i));
					others[strlen(others)]=' ';
				}
				i = j - 1; // We always increment by 1, so stop before the null [all other cases are OK]
			}
			i++;
		}
		purple_conversation_present(conv);
		gtkconv = PIDGIN_CONVERSATION(conv);
		gtk_text_buffer_insert_at_cursor(gtkconv->entry_buffer, others, -1);
		gtk_widget_grab_focus(GTK_WIDGET(gtkconv->entry));
	} else if (!strcmp(cmd_arg, TWITTER_URI_ACTION_REPLY)) {
		const char *id_str, *user;
		long long id;
		PurpleConversation *conv;
		id_str = g_hash_table_lookup(params, "id");
		user = g_hash_table_lookup(params, "user");
		if (id_str == NULL || user == NULL || id_str[0] == '\0' || user[0] == '\0')
		{
			purple_debug_info(DEBUG_ID, "malformed uri. Invalid id/user for reply\n");
			return FALSE;
		}
		id = strtoll(id_str, NULL, 10);
		if (id == 0)
		{
			purple_debug_info(DEBUG_ID, "malformed uri. Invalid id for reply\n");
			return FALSE;
		}
		conv = twitter_endpoint_reply_conversation_new(twitter_endpoint_im_find(account, TWITTER_IM_TYPE_AT_MSG),
				user,
				id,
				TRUE);
		if (!conv)
		{
			return FALSE;
		}
		purple_conversation_set_data(conv, "twitter_conv_last_reply_id_manual", NULL);
		gtkprpltwtr_mark_reply(conv, NULL);

		purple_conversation_present(conv);
	} else if (!strcmp(cmd_arg, TWITTER_URI_ACTION_ORT)) {
		const char *user;
		gchar * str, *text;
		PurpleConversation *conv = NULL;
		user = g_hash_table_lookup(params, "user");
		text =  purple_unescape_html(purple_url_decode(g_hash_table_lookup(params, "text")));

		if (user == NULL || user[0] == '\0' || text == NULL || text[0] == '\0')
		{
			purple_debug_info(DEBUG_ID, "malformed uri. Invalid id/user for reply\n");
			return FALSE;
		}
		purple_debug_info(DEBUG_ID, "text is %s\n", text);
		str = g_strdup_printf("RT @%s: %s", user, text);
		conv = purple_find_conversation_with_account(PURPLE_CONV_TYPE_CHAT, "Timeline: Home", account);
		if (!conv) {
			if (twitter_blist_chat_find(account, "Timeline: Home")) {
				PurpleConnection *gc = purple_account_get_connection(account);
				if (gc) {
					TwitterConnectionData *twitter = gc->proto_data;
					if (twitter) {
						conv = serv_got_joined_chat(purple_account_get_connection(account), ++twitter->chat_id, "Timeline: Home");
					}
				}
			}
		}
		if (conv) {
			purple_conversation_present(conv);
			gtkconv = PIDGIN_CONVERSATION(conv);
			gtk_text_buffer_insert_at_cursor(gtkconv->entry_buffer, str, -1);
			gtk_widget_grab_focus(GTK_WIDGET(gtkconv->entry));
		}
		g_free(str);
		g_free(text);
	} else if (!strcmp(cmd_arg, TWITTER_URI_ACTION_RT)) {
		TwitterConversationId *conv_id;

		const char *id_str;
		long long id;

		gchar *conv_type_str;
		PurpleConversationType conv_type;

		gchar *conv_name_encoded;

		id_str = g_hash_table_lookup(params, "id");
		conv_name_encoded = g_hash_table_lookup(params, "conv_name");
		conv_type_str = g_hash_table_lookup(params, "conv_type");

		if (id_str == NULL || id_str[0] == '\0')
		{
			purple_debug_info(DEBUG_ID, "malformed uri. Invalid id for rt\n");
			return FALSE;
		}
		id = strtoll(id_str, NULL, 10);
		if (id == 0)
		{
			purple_debug_info(DEBUG_ID, "malformed uri. Invalid id for rt\n");
			return FALSE;
		}

		conv_type = atoi(conv_type_str);

		conv_id = g_new0(TwitterConversationId, 1);
		conv_id->conv_name = g_strdup(purple_url_decode(conv_name_encoded));
		conv_id->type = conv_type;
		twitter_api_send_rt(purple_account_get_requestor(account),
				id,
				twitter_send_rt_success_cb,
				twitter_send_rt_error_cb,
				conv_id);
	} else if (!strcmp(cmd_arg, TWITTER_URI_ACTION_GET_ORIGINAL)) {
		TwitterConversationId *conv_id;

		const char *in_reply_to_status_id_str;
		long long in_reply_to_status_id;

		gchar *conv_type_str;
		PurpleConversationType conv_type;

		gchar *conv_name_encoded;

		in_reply_to_status_id_str = g_hash_table_lookup(params, "in_reply_to_status_id");
		conv_name_encoded = g_hash_table_lookup(params, "conv_name");
		conv_type_str = g_hash_table_lookup(params, "conv_type");

		if (in_reply_to_status_id_str== NULL || in_reply_to_status_id_str[0] == '\0')
		{
			purple_debug_info(DEBUG_ID, "malformed uri. Invalid in_reply_to_status_id_str for GET_ORIGINAL\n");
			return FALSE;
		}
		in_reply_to_status_id = strtoll(in_reply_to_status_id_str, NULL, 10);
		if (in_reply_to_status_id == 0)
		{
			purple_debug_info(DEBUG_ID, "malformed uri. Invalid in_reply_to_status_id for GET_ORIGINAL \n");
			return FALSE;
		}

		conv_type = atoi(conv_type_str);

		conv_id = g_new0(TwitterConversationId, 1);
		conv_id->conv_name = g_strdup(purple_url_decode(conv_name_encoded));
		conv_id->type = conv_type;
		twitter_api_get_status(purple_account_get_requestor(account),
				in_reply_to_status_id,
				twitter_get_status_success_cb,
				twitter_get_status_error_cb,
				conv_id);
	} else if (!strcmp(cmd_arg, TWITTER_URI_ACTION_LINK)) {
		const char *id_str, *user;
		long long id;
		gchar *link;
		PurpleConnection *gc = purple_account_get_connection(account);
		TwitterConnectionData *twitter;
		if (!gc) {
			purple_debug_info(DEBUG_ID, "disconnected. Exiting\n.");
			return FALSE;
		}
		twitter = gc->proto_data;
		id_str = g_hash_table_lookup(params, "id");
		user = g_hash_table_lookup(params, "user");
		if (id_str == NULL || user == NULL || id_str[0] == '\0' || user[0] == '\0')
		{
			purple_debug_info(DEBUG_ID, "malformed uri. Invalid id/user for link\n");
			return FALSE;
		}
		id = strtoll(id_str, NULL, 10);
		if (id == 0)
		{
			purple_debug_info(DEBUG_ID, "malformed uri. Invalid id for link\n");
			return FALSE;
		}
		link = twitter_mb_prefs_get_status_url(twitter->mb_prefs, user, id);
		if (link)
		{
			purple_notify_uri(NULL, link);
			g_free(link);
		}
	} else if (!strcmp(cmd_arg, TWITTER_URI_ACTION_DELETE)) {
		TwitterConversationId *conv_id;

		const char *id_str;
		long long id;

		gchar *conv_type_str;
		PurpleConversationType conv_type;

		gchar *conv_name_encoded;

		id_str = g_hash_table_lookup(params, "id");
		conv_name_encoded = g_hash_table_lookup(params, "conv_name");
		conv_type_str = g_hash_table_lookup(params, "conv_type");

		if (id_str == NULL || id_str[0] == '\0')
		{
			purple_debug_info(DEBUG_ID, "malformed uri. Invalid id for rt\n");
			return FALSE;
		}
		id = strtoll(id_str, NULL, 10);
		if (id == 0)
		{
			purple_debug_info(DEBUG_ID, "malformed uri. Invalid id for rt\n");
			return FALSE;
		}

		conv_type = atoi(conv_type_str);

		conv_id = g_new0(TwitterConversationId, 1);
		conv_id->conv_name = g_strdup(purple_url_decode(conv_name_encoded));
		conv_id->type = conv_type;
		twitter_api_delete_status(purple_account_get_requestor(account),
				id,
				twitter_delete_tweet_success_cb,
				twitter_delete_tweet_error_cb,
				conv_id);
	} else if (!strcmp(cmd_arg, TWITTER_URI_ACTION_SEARCH)) {
		//join chat with default interval, open in conv window
		GHashTable *components;
		text = g_hash_table_lookup(params, "text");

		if (text == NULL || text[0] == '\0')
		{
			purple_debug_info(DEBUG_ID, "malformed uri. No text for search\n");
			return FALSE;
		}
		components = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);
		g_hash_table_insert(components, "search", g_strdup(purple_url_decode(text)));
		twitter_endpoint_chat_start(purple_account_get_connection(account), twitter_get_endpoint_chat_settings(TWITTER_CHAT_SEARCH),
				components, TRUE) ;
	} else if (!strcmp(cmd_arg, TWITTER_URI_ACTION_SET_REPLY)) {
		const char *id_str, *user;
		long long id;
		PurpleConversation *conv;
		id_str = g_hash_table_lookup(params, "id");
		user = g_hash_table_lookup(params, "user");
		if (id_str == NULL || user == NULL || id_str[0] == '\0' || user[0] == '\0')
		{
			purple_debug_info(DEBUG_ID, "malformed uri. Invalid id/user for reply\n");
			return FALSE;
		}
		id = strtoll(id_str, NULL, 10);
		if (id == 0)
		{
			purple_debug_info(DEBUG_ID, "malformed uri. Invalid id for reply\n");
			return FALSE;
		}
		conv = twitter_endpoint_reply_conversation_new(twitter_endpoint_im_find(account, TWITTER_IM_TYPE_AT_MSG),
				user,
				id,
				TRUE);
		if (!conv)
		{
			return FALSE;
		}
		purple_conversation_set_data(conv, "twitter_conv_last_reply_id_manual", (gpointer)0x10101010);
		purple_debug_info(DEBUG_ID, "Setting reply to %lld for conv %p\n", id, conv);
		gtkprpltwtr_mark_reply(conv, id_str);
	} else if (!strcmp(cmd_arg, TWITTER_URI_ACTION_REPORT_SPAM)) {
		TwitterConversationId *conv_id;
		gchar * user;
		PurpleConversationType conv_type;
		gchar *conv_type_str;
		gchar *conv_name_encoded;

		conv_name_encoded = g_hash_table_lookup(params, "conv_name");
		conv_type_str = g_hash_table_lookup(params, "conv_type");

		conv_type = atoi(conv_type_str);

		user = g_hash_table_lookup(params, "user");
		if (user == NULL || user[0] == '\0')
		{
			purple_debug_info(DEBUG_ID, "malformed uri. Invalid user for marking as spam\n");
			return FALSE;
		}
		conv_id = g_new0(TwitterConversationId, 1);
		conv_id->conv_name = g_strdup(purple_url_decode(conv_name_encoded));
		conv_id->type = conv_type;
		twitter_api_report_spammer(purple_account_get_requestor(account),
				user,
				twitter_report_spammer_success_cb,
				twitter_report_spammer_error_cb,
				conv_id);
	}
	return TRUE;
}


static void twitter_got_uri_action(const gchar *url, const gchar *action)
{
	gchar *url2 = g_strdup_printf("%s&action=%s", url, action);
	purple_got_protocol_handler_uri(url2);
	g_free(url2);
}

static void twitter_context_menu_retweet(GtkWidget *w, const gchar *url)
{
	twitter_got_uri_action(url, TWITTER_URI_ACTION_RT);
}

static void twitter_context_menu_old_retweet(GtkWidget *w, const gchar *url)
{
	twitter_got_uri_action(url, TWITTER_URI_ACTION_ORT);
}

static void twitter_context_menu_replyall(GtkWidget *w, const gchar *url)
{
	twitter_got_uri_action(url, TWITTER_URI_ACTION_REPLYALL);
}

static void twitter_context_menu_reply(GtkWidget *w, const gchar *url)
{
	twitter_got_uri_action(url, TWITTER_URI_ACTION_REPLY);
}

static void twitter_context_menu_link(GtkWidget *w, const gchar *url)
{
	twitter_got_uri_action(url, TWITTER_URI_ACTION_LINK);
}

static void twitter_context_menu_delete(GtkWidget *w, const gchar *url)
{
	twitter_got_uri_action(url, TWITTER_URI_ACTION_DELETE);
}

static void twitter_context_menu_set_reply(GtkWidget *w, const gchar *url)
{
	twitter_got_uri_action(url, TWITTER_URI_ACTION_SET_REPLY);
}

static void twitter_context_menu_report_spammer(GtkWidget *w, const gchar *url)
{
	twitter_got_uri_action(url, TWITTER_URI_ACTION_REPORT_SPAM);
}

static void twitter_context_menu_get_original(GtkWidget *w, const gchar *url)
{
	twitter_got_uri_action(url, TWITTER_URI_ACTION_GET_ORIGINAL);
}
static const gchar *url_get_param_value(const gchar *url, const gchar *key, gsize *len)
{
	const gchar *start = strchr(url, '?');
	const gchar *end;
	int key_len;
	*len = 0;
	if (!start)
		return NULL;
	key_len = strlen(key);

	do
	{
		start++;
		if (g_str_has_prefix(start, key) && start[key_len] == '=')
		{
			start += key_len + 1;
			end = strchr(start, '&');
			if (!end)
				*len = strlen(start);
			else
				*len = end - start;
			return start;
		}
	} while ((start = strchr(start, '&')) != NULL);

	return NULL;
}

static void twitter_url_menu_actions(GtkWidget *menu, const char *url)
{
	GtkWidget *img, *item;
	gsize account_len;
	gsize user_len;
	gsize in_reply_to_status_id_len;
	gsize conv_type_len;
	PurpleConversationType conv_type;
	PurpleAccount *account;

	const gchar *account_name_tmp = url_get_param_value(url, "account", &account_len);
	const gchar *user_name_tmp = url_get_param_value(url, "user", &user_len);
	const gchar *in_reply_to_status_id_tmp = url_get_param_value(url, "in_reply_to_status_id", &in_reply_to_status_id_len);
	const gchar *conv_type_tmp = url_get_param_value(url, "conv_type", &conv_type_len);
	long long in_reply_to_status_id;
	gchar *account_name, *user_name;
	if (!account_name_tmp || !user_name_tmp)
		return;
	account_name_tmp++;
	account_len--;

	in_reply_to_status_id = strtoll(in_reply_to_status_id_tmp, NULL, 10);
	conv_type = (PurpleConversationType) strtol(conv_type_tmp, NULL, 10);

	account_name = g_strndup(account_name_tmp, account_len);
	user_name = g_strndup(user_name_tmp, user_len);

	account = purple_accounts_find(account_name, TWITTER_PROTOCOL_ID);

	img = gtk_image_new_from_stock(GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU);
	item = gtk_image_menu_item_new_with_mnemonic((_("Retweet")));
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), img);
	g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(twitter_context_menu_retweet), (gpointer)url);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	img = gtk_image_new_from_stock(GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU);
	item = gtk_image_menu_item_new_with_mnemonic((_("Old Retweet")));
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), img);
	g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(twitter_context_menu_old_retweet), (gpointer)url);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);


	img = gtk_image_new_from_stock(GTK_STOCK_REDO, GTK_ICON_SIZE_MENU);
	item = gtk_image_menu_item_new_with_mnemonic((_("Reply")));
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), img);
	g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(twitter_context_menu_reply), (gpointer)url);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	img = gtk_image_new_from_stock(GTK_STOCK_REDO, GTK_ICON_SIZE_MENU);
	item = gtk_image_menu_item_new_with_mnemonic((_("Reply All")));
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), img);
	g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(twitter_context_menu_replyall), (gpointer)url);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);


	img = gtk_image_new_from_stock(GTK_STOCK_HOME, GTK_ICON_SIZE_MENU);
	item = gtk_image_menu_item_new_with_mnemonic((_("Goto Site")));
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), img);
	g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(twitter_context_menu_link), (gpointer)url);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	if (account && twitter_usernames_match(account, account_name, user_name))
	{
		img = gtk_image_new_from_stock(GTK_STOCK_DELETE, GTK_ICON_SIZE_MENU);
		item = gtk_image_menu_item_new_with_mnemonic((_("Delete")));
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), img);
		g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(twitter_context_menu_delete), (gpointer)url);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	}

	if(in_reply_to_status_id)
	{
		img = gtk_image_new_from_stock(GTK_STOCK_HOME, GTK_ICON_SIZE_MENU);
		item = gtk_image_menu_item_new_with_mnemonic((_("In reply to...")));
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), img);
		g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(twitter_context_menu_get_original), (gpointer)url);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	}

	if (conv_type == PURPLE_CONV_TYPE_IM)
	{
		img = gtk_image_new_from_stock(GTK_STOCK_CONVERT, GTK_ICON_SIZE_MENU);
		item = gtk_image_menu_item_new_with_mnemonic((_("Lock reply")));
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), img);
		g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(twitter_context_menu_set_reply), (gpointer)url);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	}

	img = gtk_image_new_from_stock(GTK_STOCK_STOP, GTK_ICON_SIZE_MENU);
	item = gtk_image_menu_item_new_with_mnemonic((_("Report spammer")));
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), img);
	g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(twitter_context_menu_report_spammer), (gpointer)url);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	g_free(account_name);
	g_free(user_name);
}

static gboolean twitter_context_menu(GtkIMHtml *imhtml, GtkIMHtmlLink *link, GtkWidget *menu)
{
	twitter_url_menu_actions(menu, gtk_imhtml_link_get_url(link));
	return TRUE;
}

static gboolean twitter_url_clicked_cb(GtkIMHtml *imhtml, GtkIMHtmlLink *link)
{
	static GtkWidget *menu = NULL;
	gchar *url;
	purple_debug_info(DEBUG_ID, "%s\n", G_STRFUNC);

	if (menu)
	{
		gtk_widget_destroy(menu);
		menu = NULL;
	}

	//If not the action url, handle it by using the uri handler, otherwise, show menu
	if (!g_str_has_prefix(gtk_imhtml_link_get_url(link), TWITTER_URI ":///" TWITTER_URI_ACTION_ACTIONS "?"))
	{
		purple_got_protocol_handler_uri(gtk_imhtml_link_get_url(link));
		return TRUE;
	}

	url = g_strdup(gtk_imhtml_link_get_url(link));

	menu = gtk_menu_new();
	g_object_set_data_full(G_OBJECT(menu), "x-imhtml-url-data", url,
			(GDestroyNotify)g_free);

	twitter_url_menu_actions(menu, url);

	gtk_widget_show_all(menu);
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
			0, gtk_get_current_event_time());

	return TRUE;

}
#endif


static void gtkprpltwtr_connecting_cb(PurpleAccount *account)
{
	if (purple_prefs_get_bool(TWITTER_PREF_ENABLE_CONV_ICON))
		twitter_conv_icon_account_load(account);
}

static void gtkprpltwtr_enable_conv_icon_all_accounts()
{
	if (purple_prefs_get_bool(TWITTER_PREF_ENABLE_CONV_ICON))
	{
		GList *accounts = purple_accounts_get_all();
		GList *l;
		for (l = accounts; l; l = l->next)
		{
			PurpleAccount *account = l->data;
			if (purple_account_is_connected(account) && !strcmp(TWITTER_PROTOCOL_ID, purple_account_get_protocol_id(account)))
			{
				twitter_conv_icon_account_load(account);
			}
		}
	}
}

static void gtkprpltwtr_disable_conv_icon_all_accounts()
{
	GList *accounts = purple_accounts_get_all();
	GList *l;
	for (l = accounts; l; l = l->next)
	{
		PurpleAccount *account = l->data;
		if (purple_account_is_connected(account) && !strcmp(TWITTER_PROTOCOL_ID, purple_account_get_protocol_id(account)))
		{
			twitter_conv_icon_account_unload(account);
		}
	}
}

static void gtkprpltwtr_disconnected_cb(PurpleAccount *account)
{
	twitter_conv_icon_account_unload(account);
}

static void gtkprpltwtr_update_buddyicon_cb(PurpleAccount *account, const gchar *buddy_name, PurpleBuddyIcon *buddy_icon)
{
	twitter_conv_icon_got_buddy_icon(account, buddy_name, buddy_icon);
}

static void gtkprpltwtr_update_iconurl_cb(PurpleAccount *account, const gchar *buddy_name, const gchar *icon_url, time_t created_at)
{
	twitter_conv_icon_got_user_icon(account, buddy_name, icon_url, created_at);
}

#if PURPLE_VERSION_CHECK(2, 6, 0)
static const char *_find_first_delimiter(const char *text, const char *delimiters, int *delim_id)
{
	const char *delimiter;
	if (text == NULL || text[0] == '\0')
		return NULL;
	do
	{
		for (delimiter = delimiters; *delimiter != '\0'; delimiter++)
		{
			if (*text == *delimiter)
			{
				if (delim_id != NULL)
					*delim_id = delimiter - delimiters;
				return text;
			}
		}
	} while (*++text != '\0');
	return NULL;
}

static void _g_string_append_escaped_len(GString *s, const gchar *txt, gssize len)
{
	gchar *tmp = purple_markup_escape_text(txt, len);
	g_string_append(s, tmp);
	g_free(tmp);
}

//TODO: move those
static char *twitter_linkify(PurpleAccount *account, const char *message)
{
	GString *ret;
	static char symbols[] = "#@";
	static char *symbol_actions[] = {TWITTER_URI_ACTION_SEARCH, TWITTER_URI_ACTION_USER};
	static char delims[] = " :"; //I don't know if this is how I want to do this...
	const char *ptr = message;
	const char *end = message + strlen(message);
	const char *delim = NULL;
	g_return_val_if_fail(message != NULL, NULL);

	ret = g_string_new("");

	while (ptr != NULL && ptr < end)
	{
		const char *first_token;
		char *current_action;
		char *link_text;
		int symbol_index = 0;
		first_token = _find_first_delimiter(ptr, symbols, &symbol_index);
		if (first_token == NULL)
		{
			_g_string_append_escaped_len(ret, ptr, -1);
			break;
		}
		current_action = symbol_actions[symbol_index];
		_g_string_append_escaped_len(ret, ptr, first_token - ptr);
		ptr = first_token;
		delim = _find_first_delimiter(ptr, delims, NULL);
		if (delim == NULL)
			delim = end;
		link_text = g_strndup(ptr, delim - ptr);
		//Added the 'a' before the account name because of a highlighting issue... ugly hack
		g_string_append_printf(ret, "<a href=\"" TWITTER_URI ":///%s?account=a%s&text=%s\">",
				current_action,
				purple_account_get_username(account),
				purple_url_encode(link_text));
		_g_string_append_escaped_len(ret, link_text, -1);
		g_string_append(ret, "</a>");
		ptr = delim;

		g_free(link_text);
	}

	return g_string_free(ret, FALSE);
}

static gchar *gtkprpltwtr_format_tweet_cb(PurpleAccount *account,
		const char *src_user,
		const char *message,
		long long tweet_id,
		PurpleConversationType conv_type,
		const gchar *conv_name,
		gboolean is_tweet,
		long long in_reply_to_status_id)
{
	gchar *linkified_message;
	GString *tweet;

	linkified_message = twitter_linkify(account, message);

	g_return_val_if_fail(linkified_message != NULL, NULL);

	tweet = g_string_new(linkified_message);

	if (is_tweet && tweet_id && conv_type != PURPLE_CONV_TYPE_UNKNOWN && conv_name)
	{
		const gchar *account_name = purple_account_get_username(account);
		//TODO: make this an image
		/* purple_url_encode uses a static array (!) */
		g_string_append_printf(tweet,
				" <a href=\"" TWITTER_URI ":///" TWITTER_URI_ACTION_ACTIONS "?account=a%s&user=%s&id=%lld",
				account_name,
				purple_url_encode(src_user),
				tweet_id);
		g_string_append_printf(tweet, "&text=%s", purple_url_encode(message));
		g_string_append_printf(tweet,
				"&conv_type=%d&conv_name=%s&in_reply_to_status_id=%lld\">*</a>",
				conv_type,
				purple_url_encode(conv_name),
				in_reply_to_status_id);
	}

	g_free(linkified_message);
	return g_string_free(tweet, FALSE);
}


static void gtkprpltwtr_received_im_cb(PurpleAccount *account,
		long long tweet_id,
		const gchar *buddy_name)
{
	gchar *conv_name = twitter_endpoint_im_buddy_name_to_conv_name(twitter_endpoint_im_find(account, TWITTER_IM_TYPE_AT_MSG), buddy_name);
	PurpleConversation *conv = purple_find_conversation_with_account(PURPLE_CONV_TYPE_IM, conv_name, account);
	gchar *id_str = NULL;
	GtkTextBuffer *text_buffer;
	GtkTextIter insertion_point;
	g_free(conv_name);

	if (conv)
	{
		text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(GTK_IMHTML(PIDGIN_CONVERSATION(conv)->imhtml)));
		gtk_text_buffer_get_end_iter(text_buffer, &insertion_point);
		gtk_text_iter_forward_to_line_end(&insertion_point);
		id_str = g_strdup_printf("%lld",tweet_id);
		gtk_text_buffer_create_mark(text_buffer, id_str, &insertion_point, TRUE);
		g_free(id_str);
	}
}
#endif

static void gtkprpltwtr_enable_conv_icon()
{
	purple_signal_connect(purple_buddy_icons_get_handle(),
			"prpltwtr-update-buddyicon",
			gtkprpltwtr_plugin, PURPLE_CALLBACK(gtkprpltwtr_update_buddyicon_cb), NULL);
	purple_signal_connect(purple_buddy_icons_get_handle(),
			"prpltwtr-update-iconurl",
			gtkprpltwtr_plugin, PURPLE_CALLBACK(gtkprpltwtr_update_iconurl_cb), NULL);
	gtkprpltwtr_enable_conv_icon_all_accounts();
}

static void gtkprpltwtr_disable_conv_icon()
{
	purple_signal_disconnect(purple_buddy_icons_get_handle(),
			"prpltwtr-update-buddyicon",
			gtkprpltwtr_plugin, PURPLE_CALLBACK(gtkprpltwtr_update_buddyicon_cb));
	purple_signal_disconnect(purple_buddy_icons_get_handle(),
			"prpltwtr-update-iconurl",
			gtkprpltwtr_plugin, PURPLE_CALLBACK(gtkprpltwtr_update_iconurl_cb));
	gtkprpltwtr_disable_conv_icon_all_accounts();
}

static void gtkprpltwtr_pref_enable_conv_icon_change(const char *name, PurplePrefType type,
		gconstpointer val, gpointer data)
{
	if (purple_prefs_get_bool(TWITTER_PREF_ENABLE_CONV_ICON))
	{
		gtkprpltwtr_enable_conv_icon();
	} else {
		gtkprpltwtr_disable_conv_icon();
	}
}

static gboolean plugin_load(PurplePlugin *plugin) 
{
	gtkprpltwtr_plugin = plugin;
	purple_signal_connect(purple_conversations_get_handle(),
			"conversation-created",
			plugin, PURPLE_CALLBACK(twitter_charcount_conv_created_cb), NULL);
	purple_signal_connect(purple_conversations_get_handle(),
			"deleting-conversation",
			plugin, PURPLE_CALLBACK(twitter_charcount_conv_destroyed_cb), NULL);

	purple_signal_connect(purple_accounts_get_handle(),
			"prpltwtr-connecting",
			plugin, PURPLE_CALLBACK(gtkprpltwtr_connecting_cb), NULL);

	purple_signal_connect(purple_accounts_get_handle(),
			"prpltwtr-disconnected",
			plugin, PURPLE_CALLBACK(gtkprpltwtr_disconnected_cb), NULL);

	if (purple_prefs_get_bool(TWITTER_PREF_ENABLE_CONV_ICON))
		gtkprpltwtr_enable_conv_icon();

#if PURPLE_VERSION_CHECK(2, 6, 0)
	purple_signal_connect(purple_conversations_get_handle(),
			"prpltwtr-format-tweet",
			plugin, PURPLE_CALLBACK(gtkprpltwtr_format_tweet_cb), NULL);

	purple_signal_connect(purple_conversations_get_handle(),
			"prpltwtr-received-im",
			plugin, PURPLE_CALLBACK(gtkprpltwtr_received_im_cb), NULL);

	purple_signal_connect(purple_conversations_get_handle(),
			"prpltwtr-set-reply",
			plugin, PURPLE_CALLBACK(gtkprpltwtr_mark_reply), NULL);

	purple_signal_connect(purple_get_core(), "uri-handler", plugin,
			PURPLE_CALLBACK(twitter_uri_handler), NULL);

	purple_signal_connect(purple_conversations_get_handle(),
		   "prpltwtr-changed-attached-search",
		  plugin, PURPLE_CALLBACK(twitter_charcount_update_append_text_cb), NULL);

	gtk_imhtml_class_register_protocol(TWITTER_URI "://", twitter_url_clicked_cb, twitter_context_menu);
#endif

	twitter_charcount_attach_to_all_windows();

	purple_prefs_connect_callback(plugin,
			TWITTER_PREF_ENABLE_CONV_ICON,
			gtkprpltwtr_pref_enable_conv_icon_change,
			NULL);

	return TRUE;
}

static gboolean plugin_unload(PurplePlugin *plugin)
{
	purple_prefs_disconnect_by_handle(plugin);
	purple_signals_disconnect_by_handle(plugin);

	twitter_charcount_detach_from_all_windows();
	gtkprpltwtr_disable_conv_icon();
	return TRUE;
}

static PurplePluginPrefFrame *get_plugin_pref_frame(PurplePlugin *plugin) 
{
	PurplePluginPrefFrame *frame;
	PurplePluginPref *ppref;

	frame = purple_plugin_pref_frame_new();

	ppref = purple_plugin_pref_new_with_name_and_label(
			TWITTER_PREF_ENABLE_CONV_ICON, _("Enable Icons in Chat"));
	purple_plugin_pref_frame_add(frame, ppref);

	ppref = purple_plugin_pref_new_with_name_and_label(TWITTER_PREF_CONV_ICON_SIZE, _("Icon Size"));
	purple_plugin_pref_set_bounds(ppref, 16, 64);
	purple_plugin_pref_frame_add(frame, ppref);

	return frame;
}

static void plugin_destroy(PurplePlugin * plugin)
{
}

static PurplePluginUiInfo prefs_info = {
	get_plugin_pref_frame,
	0,   /* page_num (Reserved) */
	NULL, /* frame (Reserved) */
	/* Padding */
	NULL,
	NULL,
	NULL,
	NULL
};

static PurplePluginInfo info =
{
	PURPLE_PLUGIN_MAGIC,
	PURPLE_MAJOR_VERSION,                           /**< major version */
	PURPLE_MINOR_VERSION,                           /**< minor version */
	PURPLE_PLUGIN_STANDARD,                         /**< type */
	PIDGIN_PLUGIN_TYPE,                             /**< ui_requirement */
	0,                                              /**< flags */
	NULL,                                           /**< dependencies */
	PURPLE_PRIORITY_DEFAULT,                        /**< priority */
	PLUGIN_ID,                                   /**< id */
	"GtkPrplTwtr",                                      /**< name */
	PACKAGE_VERSION,                               /**< version */
	"PrplTwtr Pidgin Support",                        /**< summary */
	"PrplTwtr Pidgin Support",               /**< description */
	"neaveru <neaveru@gmail.com>",		     /* author */
	"http://code.google.com/p/prpltwtr/",  /* homepage */
	plugin_load,                                    /**< load */
	plugin_unload,                                  /**< unload */
	plugin_destroy,                                 /**< destroy */
	NULL,                                           /**< ui_info */
	NULL,                                           /**< extra_info */
	&prefs_info,                                    /**< prefs_info */
	NULL,                                           /**< actions */
	NULL,						    /* padding... */
	NULL,
	NULL,
	NULL,
};

static void plugin_init(PurplePlugin *plugin) 
{	
#ifdef ENABLE_NLS
	bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
#endif /* ENABLE_NLS */
	info.summary     = _("GUI Enhancements for PrplTwtr");
	info.description = _("Pidgin specific GUI options for the purple plugin PrplTwtr");

	gtkprpltwtr_plugin = plugin;
	twitter_endpoint_chat_init();
	gtkprpltwtr_prefs_init(plugin);
}

PURPLE_INIT_PLUGIN(gtkprpltwtr, plugin_init, info)
