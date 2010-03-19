#define PLUGIN_ID "gtkprpltwtr"

#include "../config.h"

#include <gtkplugin.h>
#include <gtkconv.h>
#include <gtkimhtml.h>
#include "../twitter.h"
#include "twitter_charcount.h"
#include "twitter_convicon.h"

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
	twitter_conv_id_write_message(r->account, conv_id, PURPLE_MESSAGE_SYSTEM, "Successfully retweeted");
}

static void twitter_send_rt_error_cb(TwitterRequestor *r,
		const TwitterRequestErrorData *error_data,
		gpointer user_data)
{
	TwitterConversationId *conv_id = user_data;
	twitter_conv_id_write_message(r->account, conv_id, PURPLE_MESSAGE_ERROR, "Retweet failed");
}

static void twitter_delete_tweet_success_cb(TwitterRequestor *r,
		xmlnode *node,
		gpointer user_data)
{
	TwitterConversationId *conv_id = user_data;
	twitter_conv_id_write_message(r->account, conv_id, PURPLE_MESSAGE_SYSTEM, "Successfully deleted");
}

static void twitter_delete_tweet_error_cb(TwitterRequestor *r,
		const TwitterRequestErrorData *error_data,
		gpointer user_data)
{
	TwitterConversationId *conv_id = user_data;
	twitter_conv_id_write_message(r->account, conv_id, PURPLE_MESSAGE_ERROR, "Delete failed");
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
	purple_debug_info(TWITTER_PROTOCOL_ID, "%s PROTO %s CMD_ARG %s\n", G_STRFUNC, proto, cmd_arg);

	g_return_val_if_fail(proto != NULL, FALSE);
	g_return_val_if_fail(cmd_arg != NULL, FALSE);

	//don't handle someone elses proto
	if (strcmp(proto, TWITTER_URI))
		return FALSE;

	username = g_hash_table_lookup(params, "account");

	if (username == NULL || username[0] == '\0')
	{
		purple_debug_info(TWITTER_PROTOCOL_ID, "malformed uri. No account username\n");
		return FALSE;
	}

	//ugly hack to fix username highlighting
	account = purple_accounts_find(username+1, TWITTER_PROTOCOL_ID);

	if (account == NULL)
	{
		purple_debug_info(TWITTER_PROTOCOL_ID, "could not find account %s\n", username);
		return FALSE;
	}

	while (cmd_arg[0] == '/')
		cmd_arg++;

	action = g_hash_table_lookup(params, "action");
	if (action)
		cmd_arg = action;
	purple_debug_info(TWITTER_PROTOCOL_ID, "Account %s got action %s\n", username, cmd_arg);
	if (!strcmp(cmd_arg, TWITTER_URI_ACTION_USER))
	{
		purple_notify_info(purple_account_get_connection(account),
				"Clicked URI",
				"@name clicked",
				"Sorry, this has not been implemented yet");
	} else if (!strcmp(cmd_arg, TWITTER_URI_ACTION_REPLY)) {
		const char *id_str, *user;
		long long id;
		PurpleConversation *conv;
		id_str = g_hash_table_lookup(params, "id");
		user = g_hash_table_lookup(params, "user");
		if (id_str == NULL || user == NULL || id_str[0] == '\0' || user[0] == '\0')
		{
			purple_debug_info(TWITTER_PROTOCOL_ID, "malformed uri. Invalid id/user for reply\n");
			return FALSE;
		}
		id = strtoll(id_str, NULL, 10);
		if (id == 0)
		{
			purple_debug_info(TWITTER_PROTOCOL_ID, "malformed uri. Invalid id for reply\n");
			return FALSE;
		}
		conv = twitter_endpoint_reply_conversation_new(twitter_endpoint_im_find(account, TWITTER_IM_TYPE_AT_MSG),
				user,
				id);
		if (!conv)
		{
			return FALSE;
		}
		purple_conversation_present(conv);
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
			purple_debug_info(TWITTER_PROTOCOL_ID, "malformed uri. Invalid id for rt\n");
			return FALSE;
		}
		id = strtoll(id_str, NULL, 10);
		if (id == 0)
		{
			purple_debug_info(TWITTER_PROTOCOL_ID, "malformed uri. Invalid id for rt\n");
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
	} else if (!strcmp(cmd_arg, TWITTER_URI_ACTION_LINK)) {
		const char *id_str, *user;
		long long id;
		gchar *link;
		PurpleConnection *gc = purple_account_get_connection(account);
		TwitterConnectionData *twitter = gc->proto_data;
		id_str = g_hash_table_lookup(params, "id");
		user = g_hash_table_lookup(params, "user");
		if (id_str == NULL || user == NULL || id_str[0] == '\0' || user[0] == '\0')
		{
			purple_debug_info(TWITTER_PROTOCOL_ID, "malformed uri. Invalid id/user for link\n");
			return FALSE;
		}
		id = strtoll(id_str, NULL, 10);
		if (id == 0)
		{
			purple_debug_info(TWITTER_PROTOCOL_ID, "malformed uri. Invalid id for link\n");
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
			purple_debug_info(TWITTER_PROTOCOL_ID, "malformed uri. Invalid id for rt\n");
			return FALSE;
		}
		id = strtoll(id_str, NULL, 10);
		if (id == 0)
		{
			purple_debug_info(TWITTER_PROTOCOL_ID, "malformed uri. Invalid id for rt\n");
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
			purple_debug_info(TWITTER_PROTOCOL_ID, "malformed uri. No text for search\n");
			return FALSE;
		}
		components = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);
		g_hash_table_insert(components, "search", g_strdup(purple_url_decode(text)));
		twitter_endpoint_chat_start(purple_account_get_connection(account), twitter_get_endpoint_chat_settings(TWITTER_CHAT_SEARCH),
				components, TRUE) ;
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
	PurpleAccount *account;

	const gchar *account_name_tmp = url_get_param_value(url, "account", &account_len);
	const gchar *user_name_tmp = url_get_param_value(url, "user", &user_len);
	gchar *account_name, *user_name;
	if (!account_name_tmp || !user_name_tmp)
		return;
	account_name_tmp++;
	account_len--;

	account_name = g_strndup(account_name_tmp, account_len);
	user_name = g_strndup(user_name_tmp, user_len);

	account = purple_accounts_find(account_name, TWITTER_PROTOCOL_ID);

	img = gtk_image_new_from_stock(GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU);
	item = gtk_image_menu_item_new_with_mnemonic(("Retweet"));
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), img);
	g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(twitter_context_menu_retweet), (gpointer)url);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	img = gtk_image_new_from_stock(GTK_STOCK_REDO, GTK_ICON_SIZE_MENU);
	item = gtk_image_menu_item_new_with_mnemonic(("Reply"));
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), img);
	g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(twitter_context_menu_reply), (gpointer)url);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	img = gtk_image_new_from_stock(GTK_STOCK_HOME, GTK_ICON_SIZE_MENU);
	item = gtk_image_menu_item_new_with_mnemonic(("Goto Site"));
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), img);
	g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(twitter_context_menu_link), (gpointer)url);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	if (account && twitter_usernames_match(account, account_name, user_name))
	{
		img = gtk_image_new_from_stock(GTK_STOCK_DELETE, GTK_ICON_SIZE_MENU);
		item = gtk_image_menu_item_new_with_mnemonic(("Delete"));
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), img);
		g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(twitter_context_menu_delete), (gpointer)url);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	}


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
	purple_debug_info(TWITTER_PROTOCOL_ID, "%s\n", G_STRFUNC);

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
	if (twitter_option_enable_conv_icon(account))
		twitter_conv_icon_account_load(account);
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

	purple_signal_connect(purple_buddy_icons_get_handle(),
			"prpltwtr-update-buddyicon",
			plugin, PURPLE_CALLBACK(gtkprpltwtr_update_buddyicon_cb), NULL);
	purple_signal_connect(purple_buddy_icons_get_handle(),
			"prpltwtr-update-iconurl",
			plugin, PURPLE_CALLBACK(gtkprpltwtr_update_iconurl_cb), NULL);

#if PURPLE_VERSION_CHECK(2, 6, 0)
	purple_signal_connect(purple_get_core(), "uri-handler", plugin,
			PURPLE_CALLBACK(twitter_uri_handler), NULL);

	gtk_imhtml_class_register_protocol(TWITTER_URI "://", twitter_url_clicked_cb, twitter_context_menu);
#endif

	twitter_charcount_attach_to_all_windows();

	return TRUE;
}

static gboolean plugin_unload(PurplePlugin *plugin)
{
	purple_signals_disconnect_by_handle(plugin);

	twitter_charcount_detach_from_all_windows();
	return TRUE;
}

static PurplePluginPrefFrame *get_plugin_pref_frame(PurplePlugin *plugin) 
{
	PurplePluginPrefFrame *frame;
	frame = purple_plugin_pref_frame_new();
	return frame;
}

void plugin_destroy(PurplePlugin * plugin)
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
	"0.5.0",                               /**< version */
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
	gtkprpltwtr_plugin = plugin;
	twitter_endpoint_chat_init();
}

PURPLE_INIT_PLUGIN(gtkprpltwtr, plugin_init, info)
