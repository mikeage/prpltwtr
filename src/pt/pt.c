/**
 * TODO: legal stuff
 *
 * purple
 *
 * Purple is the legal property of its developers, whose names are too numerous
 * to list here. Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111-1301 USA
 */

#include <string.h>
#include <glib/gstdio.h>

#include "prpl.h"
#include "accountopt.h"
#include "debug.h"
#include "version.h"

#include "defines.h"
#include "pt.h"
#include "pt_connection.h"
#include "pt_oauth.h"
#include "pt_requestor.h"


static GList   *pt_protocol_options (void);
static void     pt_init (PurplePlugin * plugin);

static PurplePluginProtocolInfo prpl_info = {
	OPT_PROTO_CHAT_TOPIC | OPT_PROTO_NO_PASSWORD,	/* options */
	NULL,			/* user_splits, initialized in pt_init () */
	NULL,			/* protocol_options, initialized in pt_init () */
	{			/* icon_spec, a PurpleBuddyIconSpec */
	 "png,jpg,gif",		/* format */
	 0,			/* min_width */
	 0,			/* min_height */
	 48,			/* max_width */
	 48,			/* max_height */
	 10000,			/* max_filesize */
	 PURPLE_ICON_SCALE_DISPLAY,	/* scale_rules */
	 },
	pt_list_icon,		/* list_icon */
	NULL,			/* list_emblem */
	pt_status_text,		/* status_text */
	pt_tooltip_text,	/* tooltip_text */
	pt_status_types,	/* status_types */
	pt_blist_node_menu,	/* blist_node_menu */
	pt_chat_info,		/* chat_info */
	pt_chat_info_defaults,	/* chat_info_defaults */
	pt_login,		/* login */
	pt_close,		/* close */
	pt_send_im,		/* send_im */
	pt_set_info,		/* set_info */
	NULL,			/*send_typing */
	pt_api_get_info,	/* get_info */
	pt_set_status,		/* set_status */
	NULL,			/* set_idle */
	NULL,			/* change_passwd */
	pt_add_buddy,		/* add_buddy */
	pt_add_buddies,		/* add_buddies */
	pt_remove_buddy,	/* remove_buddy */
	pt_remove_buddies,	/* remove_buddies */
	NULL,			/* add_permit */
	NULL,			/* add_deny */
	NULL,			/* rem_permit */
	NULL,			/* rem_deny */
	NULL,			/* set_permit_deny */
	pt_chat_join,		/* join_chat */
	NULL,			/* reject_chat */
	pt_chat_get_name,	/* get_chat_name */
	NULL,			/* chat_invite */
	pt_chat_leave,		/* chat_leave */
	NULL,			/* chat_whisper */
	pt_chat_send,		/* chat_send */
	NULL,			/* keepalive */
	NULL,			/* register_user */
	pt_get_cb_info,		/* get_cb_info */
	NULL,			/* get_cb_away */
	NULL,			/* alias_buddy */
	NULL,			/* group_buddy */
	NULL,			/* rename_group */
	NULL,			/* buddy_free */
	pt_convo_closed,	/* convo_closed */
	purple_normalize_nocase,	/* normalize */
	pt_set_buddy_icon,	/* set_buddy_icon */
	NULL,			/* remove_group */
	NULL,			/* get_cb_real_name */
	NULL,			/* set_chat_topic */
	pt_blist_chat_find,	/* find_blist_chat */
	NULL,			/* roomlist_get_list */
	NULL,			/* roomlist_cancel */
	NULL,			/* roomlist_expand_category */
	NULL,			/* can_receive_file */
	NULL,			/* send_file */
	NULL,			/* new_xfer */
	pt_offline_message,	/* offline_message */
	NULL,			/* whiteboard_prpl_ops */
	NULL,			/* send_raw */
	NULL,			/* roomlist_room_serialize */
	NULL,			/* unregister_user... */
	NULL,			/* send_attention */
	NULL,			/* get_attention type */
	sizeof (PurplePluginProtocolInfo),	/* struct_size */
	NULL,			/* get_account_text_table */
	NULL,			/* initiate_media */
	NULL,			/* get_media_caps */
	NULL,			/* get_moods */
	NULL,			/* set_public_alias */
	NULL,			/* get_public_alias */
	NULL,			/* add_buddy_with_invite */
	NULL			/* add_buddies_with_invite */
};

static PurplePluginInfo info = {
	PURPLE_PLUGIN_MAGIC,	/* magic */
	PURPLE_MAJOR_VERSION,	/* major_version */
	PURPLE_MINOR_VERSION,	/* minor_version */
	PURPLE_PLUGIN_PROTOCOL,	/* type */
	NULL,			/* ui_requirement */
	0,			/* flags */
	NULL,			/* dependencies */
	PURPLE_PRIORITY_DEFAULT,	/* priority */
	"pt",			/* id */
	"Twitter v1.1 Protocol",	/* name */
	PACKAGE_VERSION,	/* version */
	"Twitter v1.1 Protocol Plugin",	/* summary */
	"Twitter v1.1 Protocol Plugin",	/* description */
	"mikeage <mikeage@gmail.com>",	/* author */
	"http://code.google.com/p/prpltwtr/",	/* homepage */
	NULL,			/* load */
	NULL,			/* unload */
	pt_destroy,		/* destroy */
	NULL,			/* ui_info */
	&prpl_info,		/* extra_info */
	NULL,			/* prefs_info */
	pt_actions,		/* actions */
	NULL,			/* padding... */
	NULL,
	NULL,
	NULL,
};

static GList   *pt_protocol_options (void)
{
	GList          *options = NULL;

#if 0
	PurpleAccountOption *option;
	option = purple_account_option_bool_new (_("Enable HTTPS"),	/* text shown to user */
						 TWITTER_PREF_USE_HTTPS,	/* pref name */
						 TWITTER_PREF_USE_HTTPS_DEFAULT);	/* default value */
	options = g_list_append (NULL, option);

	/* Default sending im to buddy is to dm */
	option = purple_account_option_bool_new (_("Default IM to buddy is a DM"), TWITTER_PREF_DEFAULT_DM, TWITTER_PREF_DEFAULT_DM_DEFAULT);
	options = g_list_append (options, option);

	/* Retrieve tweets history after login */
	option = purple_account_option_bool_new (_("Retrieve tweets history after login"), TWITTER_PREF_RETRIEVE_HISTORY, TWITTER_PREF_RETRIEVE_HISTORY_DEFAULT);
	options = g_list_append (options, option);

	/* Automatically generate a buddylist based on followers */
	option = purple_account_option_bool_new (_("Add following as friends (NOT recommended for large follower list)"), TWITTER_PREF_GET_FRIENDS, TWITTER_PREF_GET_FRIENDS_DEFAULT);
	options = g_list_append (options, option);

	/* If adding following as friends, what should be the default alias? */
	{
		static const gchar *alias_keys[] = {
			N_("<nickname> | <full name>"),
			N_("<nickname> only"),
			N_("<full name> only"),
			NULL
		};
		static const gchar *alias_values[] = {
			TWITTER_PREF_ALIAS_FORMAT_ALL,
			TWITTER_PREF_ALIAS_FORMAT_NICK,
			TWITTER_PREF_ALIAS_FORMAT_FULLNAME,
			NULL
		};
		GList          *alias_options = NULL;
		int             i;

		for (i = 0; alias_keys[i]; i++)
		{
			PurpleKeyValuePair *kvp = g_new0 (PurpleKeyValuePair, 1);

			kvp->key = g_strdup (_(alias_keys[i]));
			kvp->value = g_strdup (alias_values[i]);
			alias_options = g_list_append (alias_options, kvp);
		}

		option = purple_account_option_list_new (_("Set default alias to:"), TWITTER_PREF_ALIAS_FORMAT, alias_options);
		options = g_list_append (options, option);
	}

	/* Add URL link to each tweet */
	option = purple_account_option_bool_new (_("Add URL link to each tweet"), TWITTER_PREF_ADD_URL_TO_TWEET, TWITTER_PREF_ADD_URL_TO_TWEET_DEFAULT);
	options = g_list_append (options, option);

	/* Idle cutoff time */
	option = purple_account_option_int_new (_("Buddy last tweet hours before set offline (0: Always online)"),	/* text shown to user */
						TWITTER_ONLINE_CUTOFF_TIME_HOURS,	/* pref name */
						TWITTER_ONLINE_CUTOFF_TIME_HOURS_DEFAULT);	/* default value */
	options = g_list_append (options, option);

	/* Max tweets to retrieve when retrieving timeline data */
	option = purple_account_option_int_new (_("Max historical timeline tweets to retrieve (0: infinite)"),	/* text shown to user */
						TWITTER_PREF_HOME_TIMELINE_MAX_TWEETS,	/* pref name */
						TWITTER_PREF_HOME_TIMELINE_MAX_TWEETS_DEFAULT);	/* default value */
	options = g_list_append (options, option);

	/* Timeline tweets refresh interval */
	option = purple_account_option_int_new (_("Refresh timeline every (min)"),	/* text shown to user */
						TWITTER_PREF_TIMELINE_TIMEOUT,	/* pref name */
						TWITTER_PREF_TIMELINE_TIMEOUT_DEFAULT);	/* default value */
	options = g_list_append (options, option);

	/* Mentions/replies tweets refresh interval */
	option = purple_account_option_int_new (_("Refresh replies every (min)"),	/* text shown to user */
						TWITTER_PREF_REPLIES_TIMEOUT,	/* pref name */
						TWITTER_PREF_REPLIES_TIMEOUT_DEFAULT);	/* default value */
	options = g_list_append (options, option);

	/* Dms refresh interval */
	option = purple_account_option_int_new (_("Refresh direct messages every (min)"),	/* text shown to user */
						TWITTER_PREF_DMS_TIMEOUT,	/* pref name */
						TWITTER_PREF_DMS_TIMEOUT_DEFAULT);	/* default value */
	options = g_list_append (options, option);

	/* Lists tweets refresh interval */
	option = purple_account_option_int_new (_("Refresh lists every (min)"),	/* text shown to user */
						TWITTER_PREF_LIST_TIMEOUT,	/* pref name */
						TWITTER_PREF_LIST_TIMEOUT_DEFAULT);	/* default value */
	options = g_list_append (options, option);

	/* Friendlist refresh interval */
	option = purple_account_option_int_new (_("Refresh friendlist every (min)"),	/* text shown to user */
						TWITTER_PREF_USER_STATUS_TIMEOUT,	/* pref name */
						TWITTER_PREF_USER_STATUS_TIMEOUT_DEFAULT);	/* default value */
	options = g_list_append (options, option);

	/* Search results refresh interval */
	option = purple_account_option_int_new (_("Default refresh search results every (min)"),	/* text shown to user */
						TWITTER_PREF_SEARCH_TIMEOUT,	/* pref name */
						TWITTER_PREF_SEARCH_TIMEOUT_DEFAULT);	/* default value */
	options = g_list_append (options, option);

#endif

	return options;
}

PURPLE_INIT_PLUGIN (null, pt_init, info);

static void pt_init (PurplePlugin * plugin)
{

#ifdef ENABLE_NLS
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif /* ENABLE_NLS */

	purple_debug_info ("pt", "starting up\n");

	((PurplePluginProtocolInfo *) plugin->info->extra_info)->protocol_options = pt_protocol_options ();

	purple_debug_info ("pt", "Registering signals\n");

	purple_signal_register (purple_accounts_get_handle (), "pt-connecting", purple_marshal_VOID__POINTER, NULL, 1, purple_value_new (PURPLE_TYPE_SUBTYPE, PURPLE_SUBTYPE_ACCOUNT));

	purple_signal_register (purple_accounts_get_handle (), "pt-disconnected", purple_marshal_VOID__POINTER, NULL, 1, purple_value_new (PURPLE_TYPE_SUBTYPE, PURPLE_SUBTYPE_ACCOUNT));

	purple_signal_register (purple_buddy_icons_get_handle (), "pt-update-buddyicon", purple_marshal_VOID__POINTER_POINTER_POINTER, NULL, 3, purple_value_new (PURPLE_TYPE_SUBTYPE, PURPLE_SUBTYPE_ACCOUNT), purple_value_new (PURPLE_TYPE_STRING), purple_value_new (PURPLE_TYPE_SUBTYPE, PURPLE_SUBTYPE_BUDDY_ICON));

	purple_signal_register (purple_buddy_icons_get_handle (), "pt-update-iconurl", purple_marshal_VOID__POINTER_POINTER_POINTER_UINT,	//uint, should be safe for a few decades
				NULL, 4, purple_value_new (PURPLE_TYPE_SUBTYPE, PURPLE_SUBTYPE_ACCOUNT), purple_value_new (PURPLE_TYPE_STRING), purple_value_new (PURPLE_TYPE_STRING), purple_value_new (PURPLE_TYPE_UINT));

	/*purple_signal_register (purple_conversations_get_handle (), "pt-format-tweet", twitter_marshal_format_tweet,  //uint, should be safe for a few decades */
	/*purple_value_new (PURPLE_TYPE_STRING), 8, purple_value_new (PURPLE_TYPE_SUBTYPE, PURPLE_SUBTYPE_ACCOUNT),     //account */
	/*purple_value_new (PURPLE_TYPE_STRING),        //user */
	/*purple_value_new (PURPLE_TYPE_STRING),        //message */
	/*purple_value_new (PURPLE_TYPE_INT64), //tweet_id */
	/*purple_value_new (PURPLE_TYPE_INT),   //conv type */
	/*purple_value_new (PURPLE_TYPE_STRING),        //conv_name */
	/*purple_value_new (PURPLE_TYPE_BOOLEAN),       // is_tweet */
	/*purple_value_new (PURPLE_TYPE_INT64), // in_reply_to_status_id */
	/*purple_value_new (PURPLE_TYPE_BOOLEAN)        // favorited */
	/*); */

	/*purple_signal_register (purple_conversations_get_handle (), "pt-received-im", twitter_marshal_received_im, NULL, 3, purple_value_new (PURPLE_TYPE_SUBTYPE, PURPLE_SUBTYPE_ACCOUNT),   // account */
	/*purple_value_new (PURPLE_TYPE_INT64), // tweet_id */
	/*purple_value_new (PURPLE_TYPE_STRING) // buddy_name */
	/*); */
	/*purple_signal_register (purple_conversations_get_handle (), "pt-set-reply", twitter_marshal_set_reply, NULL, 2, purple_value_new (PURPLE_TYPE_SUBTYPE, PURPLE_SUBTYPE_CONVERSATION),  // conv */
	/*purple_value_new (PURPLE_TYPE_STRING) // id_str */
	/*); */

	/*purple_signal_register (purple_conversations_get_handle (), "pt-changed-attached-search", twitter_marshal_changed_attached_search, NULL, 1, purple_value_new (PURPLE_TYPE_SUBTYPE, PURPLE_SUBTYPE_CONVERSATION)       // conv */
	/*); */

	// twitter_endpoint_chat_init(plugin->info->id);
}

void pt_destroy(PurplePlugin * plugin)
{
    purple_debug_info("pt", "shutting down\n");
        /*purple_signal_unregister(purple_accounts_get_handle(), "pt-connecting");*/
        /*purple_signal_unregister(purple_accounts_get_handle(), "pt-disconnected");*/
        /*purple_signal_unregister(purple_buddy_icons_get_handle(), "pt-update-buddyicon");*/
        /*purple_signal_unregister(purple_buddy_icons_get_handle(), "pt-update-iconurl");*/
        /*purple_signal_unregister(purple_conversations_get_handle(), "pt-format-tweet");*/
        /*purple_signal_unregister(purple_conversations_get_handle(), "pt-received-im");*/
        purple_signals_disconnect_by_handle(plugin);
}

gboolean pt_offline_message(const PurpleBuddy * buddy)
{
    (void) buddy;
    return TRUE;
}

GList          *pt_actions(PurplePlugin * plugin, gpointer context)
{
    GList          *l = NULL;

#if 0
    PurplePluginAction *action;
    action = purple_plugin_action_new(_("Set status"), twitter_action_set_status);
    l = g_list_append(l, action);

    action = purple_plugin_action_new(_("Rate Limit Status"), twitter_action_get_rate_limit_status);
    l = g_list_append(l, action);

    action = purple_plugin_action_new(_("Debug - Retrieve users"), twitter_action_get_user_info);
    l = g_list_append(l, action);
        l = g_list_append(l, NULL);

        action = purple_plugin_action_new(_("Open Favorites URL"), twitter_api_web_open_favorites);
        l = g_list_append(l, action);
        action = purple_plugin_action_new(_("Open Retweets of Mine"), twitter_api_web_open_retweets_of_mine);
        l = g_list_append(l, action);
        action = purple_plugin_action_new(_("Open Replies"), twitter_api_web_open_replies);
        l = g_list_append(l, action);
        action = purple_plugin_action_new(_("Open Suggested Friends"), twitter_api_web_open_suggested_friends);
        l = g_list_append(l, action);

#endif
    return l;
}

const char     *pt_list_icon(PurpleAccount * account, PurpleBuddy * buddy)
{
    return "prpltwtr";
}

char           *pt_status_text(PurpleBuddy * buddy)
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

void pt_tooltip_text(PurpleBuddy * buddy, PurpleNotifyUserInfo * info, gboolean full)
{

    PurplePresence *presence = purple_buddy_get_presence(buddy);
    PurpleStatus   *status = purple_presence_get_active_status(presence);
    char           *msg;

    purple_debug_info(purple_account_get_protocol_id(buddy->account), "showing %s tooltip for %s\n", (full) ? "full" : "short", buddy->name);

    if ((msg = pt_status_text(buddy))) {
        purple_notify_user_info_add_pair(info, purple_status_get_name(status), msg);
        g_free(msg);
    }

    if (full) {
        /*const char *user_info = purple_account_get_user_info(gc->account);
           if (user_info)
           purple_notify_user_info_add_pair(info, _("User info"), user_info); */
    }
}

GList          *pt_status_types(PurpleAccount * account)
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

    type = purple_status_type_new(PURPLE_STATUS_AVAILABLE, "online", "online", TRUE);
    purple_status_type_add_attr(type, "message", ("Online"), purple_value_new(PURPLE_TYPE_STRING));
    types = g_list_prepend(types, type);

    //This is a hack to get notified when another protocol goes into a different status.
    //Eg aim goes "away", we still want to get notified
    for (i = 0; i < status_primitives_count; i++) {
        type = purple_status_type_new(status_primitives[i], "online", "online", FALSE);
        purple_status_type_add_attr(type, "message", ("Online"), purple_value_new(PURPLE_TYPE_STRING));
        types = g_list_prepend(types, type);
    }

    type = purple_status_type_new(PURPLE_STATUS_OFFLINE, "offline", "offline", TRUE);
    purple_status_type_add_attr(type, "message", ("Offline"), purple_value_new(PURPLE_TYPE_STRING));
    types = g_list_prepend(types, type);

    return g_list_reverse(types);
}

GList          *pt_blist_node_menu(PurpleBlistNode * node)
{
    GList          *menu = NULL;
#if 0
    if (PURPLE_BLIST_NODE_IS_CHAT(node)) {
        GList          *submenu = NULL;
        PurpleChat     *chat = PURPLE_CHAT(node);
        GHashTable     *components = purple_chat_get_components(chat);
        char           *label = g_strdup_printf(_("Automatically open chat on new tweets (Currently: %s)"), (pt_blist_chat_is_auto_open(chat) ? _("On") : _("Off")));
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
#endif
    return menu;
}

GList          *pt_chat_info(PurpleConnection * gc)
{
    struct proto_chat_entry *pce;   
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

GHashTable     *pt_chat_info_defaults(PurpleConnection * gc, const char *chat_name)
{
    GHashTable     *defaults;

    defaults = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);

    g_hash_table_insert(defaults, "search", g_strdup(chat_name));

    //bug in pidgin prevents this from working
#if 0
    g_hash_table_insert(defaults, "interval", g_strdup_printf("%d", twitter_option_search_timeout(purple_connection_get_account(gc))));
#endif 
    return defaults;
}

static void requestor_post_failed(PtRequestor * requestor, const PtRequestorErrorData ** error_data)
{
    purple_debug_error("pt", "post_failed called for account %s, error %d, message %s\n", requestor->account->username, (*error_data)->error, (*error_data)->message ? (*error_data)->message : "");
    switch ((*error_data)->error) {
    case PT_REQUESTOR_ERROR_UNAUTHORIZED:
#if 0
        prpltwtr_auth_invalidate_token(requestor->account);
        prpltwtr_disconnect(r->account, _("Unauthorized"));
#endif
        break;
    default:
        break;
    }
}

void pt_login(PurpleAccount * account)
{
	static gboolean first_connect = TRUE;
    char          **userparts;
    PurpleConnection *gc = purple_account_get_connection(account);
    PtConnectionData *conn_data= g_new0(PtConnectionData, 1);

	if (!gc) {
		purple_debug_fatal("pt", "Can't connect; gc not defined!\n");
		return;
	}

    gc->proto_data = conn_data;

    purple_debug_info("pt", "Logging in %s\n", account->username);
    /* Since protocol plugins are loaded before conversations_init is called
     * we cannot connect these signals in plugin->load.
     * So we have this here, with a global var that tells us to only run this
     * once, regardless of number of accounts connecting 
     * There HAS to be a better way to do this. Need to do some research
     * this is an awful hack (tm)
     */
	if (first_connect) {
        first_connect = FALSE;

        purple_prefs_add_none("/pt");
        purple_prefs_add_bool("/pt/first-load-complete", FALSE);
        if (!purple_prefs_get_bool("/pt/first-load-complete")) {
#if 0
            PurplePlugin   *plugin = purple_plugins_find_with_id("gtkprpltwtr");
            if (plugin) {
                purple_debug_info(purple_account_get_protocol_id(account), "Loading gtk plugin\n");
                purple_plugin_load(plugin);
            }
            purple_prefs_set_bool("/pt/first-load-complete", TRUE);
#endif
        }

    }

    userparts = g_strsplit(account->username, "@", 2);
    purple_connection_set_display_name(gc, userparts[0]);
    g_strfreev(userparts);

    conn_data->requestor = g_new0(PtRequestor, 1);
    conn_data->requestor->account = account;
	conn_data->requestor->pre_send = pt_oauth_pre_send;
    conn_data->requestor->do_send = pt_requestor_send;
	conn_data->requestor->post_send = pt_oauth_post_send;
    conn_data->requestor->post_failed = requestor_post_failed;

#if 0
    /* key: gchar *, value: TwitterEndpointChat */
    twitter->chat_contexts = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify) twitter_endpoint_chat_free);

    /* key: gchar *, value: gchar * (of a long long) */
    twitter->user_reply_id_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

    purple_signal_emit(purple_accounts_get_handle(), "prpltwtr-connecting", account);
#endif 
    /* purple wants a minimum of 2 steps */
    purple_connection_update_progress(gc, ("Connecting"), 0,    /* which connection step this is */
                                      2);        /* total number of steps */

	pt_oauth_login(account, conn_data);
}

void pt_close(PurpleConnection * gc)
{
    /* notify other twitter accounts */
#if 0
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
    gc->proto_data = NULL;
#endif 
}

int pt_send_im(PurpleConnection * gc, const char *conv_name, const char *message, PurpleMessageFlags flags)
{
    int             rv = 0;
#if 0
	TwitterEndpointIm *im;
    const char     *buddy_name;
    PurpleAccount  *account = purple_connection_get_account(gc);
    char           *stripped_message;

    g_return_val_if_fail(message != NULL && message[0] != '\0' && conv_name != NULL && conv_name[0] != '\0', 0);

    stripped_message = purple_markup_strip_html(message);

    //TODO, this should be part of im settings
    im = twitter_conv_name_to_endpoint_im(account, conv_name);
    buddy_name = twitter_conv_name_to_buddy_name(account, conv_name);
    rv = im->settings->send_im(account, buddy_name, stripped_message, flags);
    g_free(stripped_message);
#endif
    return rv;
}

void pt_set_info(PurpleConnection * gc, const char *info)
{
    purple_debug_info(purple_account_get_protocol_id(purple_connection_get_account(gc)), "setting %s's user info to %s\n", gc->account->username, info);
}

void pt_set_status(PurpleAccount * account, PurpleStatus * status)
{
    const char     *msg;

    //TODO: I'm pretty sure this is broken
    msg = purple_status_get_attr_string(status, "message");
    purple_debug_info(purple_account_get_protocol_id(account), "setting %s's status to %s: %s\n", account->username, purple_status_get_name(status), msg);

    if (msg && strcmp("", msg)) {
#if 0
        twitter_api_set_status(purple_account_get_requestor(account), msg, 0, NULL, twitter_set_status_error_cb, NULL);
#endif
    }
}

void pt_add_buddy(PurpleConnection * gc, PurpleBuddy * buddy, PurpleGroup * group)
{
    //Perform the logic to decide whether this buddy will be online/offline
#if 0
   pt_buddy_touch_state(buddy);
#endif 
}

void pt_add_buddies(PurpleConnection * gc, GList * buddies, GList * groups)
{
    GList          *buddy = buddies;
    GList          *group = groups;

    purple_debug_info(purple_account_get_protocol_id(purple_connection_get_account(gc)), "adding multiple buddies\n");

    while (buddy && group) {
        pt_add_buddy(gc, (PurpleBuddy *) buddy->data, (PurpleGroup *) group->data);
        buddy = g_list_next(buddy);
        group = g_list_next(group);
    }
}

void pt_remove_buddy(PurpleConnection * gc, PurpleBuddy * buddy, PurpleGroup * group)
{
#if 0
    TwitterUserTweet *twitter_buddy_data = buddy->proto_data;

    purple_debug_info(purple_account_get_protocol_id(purple_connection_get_account(gc)), "removing %s from %s's buddy list\n", buddy->name, gc->account->username);

    if (!twitter_buddy_data)
        return;
    twitter_user_tweet_free(twitter_buddy_data);
    buddy->proto_data = NULL;
#endif
}

void pt_remove_buddies(PurpleConnection * gc, GList * buddies, GList * groups)
{
    GList          *buddy = buddies;
    GList          *group = groups;

    purple_debug_info(purple_account_get_protocol_id(purple_connection_get_account(gc)), "removing multiple buddies\n");

    while (buddy && group) {
        pt_remove_buddy(gc, (PurpleBuddy *) buddy->data, (PurpleGroup *) group->data);
        buddy = g_list_next(buddy);
        group = g_list_next(group);
    }
}

void pt_get_cb_info(PurpleConnection * gc, int id, const char *who)
{
    PurpleConversation *conv = purple_find_chat(gc, id);
    purple_debug_info(purple_account_get_protocol_id(purple_connection_get_account(gc)), "retrieving %s's info for %s in chat room %s\n", who, gc->account->username, conv->name);

    pt_api_get_info(gc, who);
}

void pt_api_get_info(PurpleConnection * gc, const char *username)
{
    //TODO: error check
    //TODO: fix for buddy not on list?
#if 0
    TwitterConnectionData *twitter = gc->proto_data;
    PurpleNotifyUserInfo *info = purple_notify_user_info_new();
    PurpleBuddy    *b = purple_find_buddy(purple_connection_get_account(gc), username);
    gchar          *url;

    if (b) {
        TwitterUserTweet *data = twitter_buddy_get_buddy_data(b);
        if (data) {
            TwitterUserData *user_data = data->user;
            TwitterTweet   *status_data = data->status;

            if (user_data) {
                purple_notify_user_info_add_pair(info, _("Description"), user_data->description);

                if (user_data->friends_count) {
                    purple_notify_user_info_add_pair(info, _("Friends"), user_data->friends_count);
                }
                if (user_data->followers_count) {
                    purple_notify_user_info_add_pair(info, _("Followers"), user_data->followers_count);
                }
                if (user_data->statuses_count) {
                    purple_notify_user_info_add_pair(info, _("Tweets"), user_data->statuses_count);
                }
            }
            if (status_data) {
                purple_notify_user_info_add_pair(info, _("Last status"), status_data->text);
            }
//          twitter_user_tweet_free(data);
        }
    } else {
        purple_notify_user_info_add_pair(info, _("Description"), _("No user info available. Loading from server..."));
        prpltwtr_api_refresh_user(purple_account_get_requestor(purple_connection_get_account(gc)), username, prpltwtr_api_refresh_user_success_cb, prpltwtr_api_refresh_user_error_cb);
    }
    url = twitter_mb_prefs_get_user_profile_url(twitter->mb_prefs, username);
    purple_notify_user_info_add_pair(info, _("Account Link"), url);
    if (url) {
        g_free(url);
    }
    purple_notify_userinfo(gc, username, info, NULL, NULL);
    purple_notify_user_info_destroy(info);
#endif 
}

static void pt_chat_join_do(PurpleConnection * gc, GHashTable * components, gboolean open_conv)
{
#if 0
    const char     *conv_type_str = g_hash_table_lookup(components, "chat_type");
    gint            conv_type = conv_type_str == NULL ? 0 : strtol(conv_type_str, NULL, 10);
    twitter_endpoint_chat_start(gc, twitter_get_endpoint_chat_settings(conv_type), components, open_conv);
#endif 
}

void pt_chat_join(PurpleConnection * gc, GHashTable * components)
{
    pt_chat_join_do(gc, components, TRUE);
}

char           *pt_chat_get_name(GHashTable * components)
{
#if 0
    const char     *chat_type_str = g_hash_table_lookup(components, "chat_type");
    TwitterChatType chat_type = chat_type_str == NULL ? 0 : strtol(chat_type_str, NULL, 10);

    TwitterEndpointChatSettings *settings = twitter_get_endpoint_chat_settings(chat_type);
    if (settings && settings->get_name)
        return settings->get_name(components);
#endif 
    return NULL;
}

void pt_chat_leave(PurpleConnection * gc, int id)
{
#if 0
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
#endif 
}

int pt_chat_send(PurpleConnection * gc, int id, const char *message, PurpleMessageFlags flags)
{
    int             rv = 0;
#if 0
    PurpleConversation *conv = purple_find_chat(gc, id);
    PurpleAccount  *account = purple_connection_get_account(gc);
    TwitterEndpointChat *ctx = twitter_endpoint_chat_find(account, purple_conversation_get_name(conv));
    char           *stripped_message;

    g_return_val_if_fail(ctx != NULL, -1);

    stripped_message = purple_markup_strip_html(message);

    rv = twitter_endpoint_chat_send(ctx, stripped_message);
    g_free(stripped_message);
#endif
    return rv;
}

void pt_convo_closed(PurpleConnection * gc, const gchar * conv_name)
{
#if 0
    TwitterEndpointIm *im = twitter_conv_name_to_endpoint_im(purple_connection_get_account(gc), conv_name);
    if (im) {
        twitter_endpoint_im_convo_closed(im, conv_name);
    }
#endif 
}

void pt_set_buddy_icon(PurpleConnection * gc, PurpleStoredImage * img)
{
    purple_debug_info(purple_account_get_protocol_id(purple_connection_get_account(gc)), "setting %s's buddy icon to %s\n", gc->account->username, purple_imgstore_get_filename(img));
}

PurpleChat     *pt_blist_chat_find(PurpleAccount * account, const char *name)
{
    PurpleChat     *c = NULL;
#if 0
    static char    *timeline = "Timeline: ";
    static char    *search = "Search: ";
    static char    *list = "List: ";
    if (strlen(name) > strlen(timeline) && !strncmp(timeline, name, strlen(timeline))) {
        c = pt_blist_chat_find_timeline(account, 0);
    } else if (strlen(name) > strlen(search) && !strncmp(search, name, strlen(search))) {
        c = pt_blist_chat_find_search(account, name + strlen(search));
    } else if (strlen(name) > strlen(list) && !strncmp(list, name, strlen(list))) {
        c = pt_blist_chat_find_list(account, name + strlen(list));
    } else {
        purple_debug_error(purple_account_get_protocol_id(account), "Invalid call to %s; assuming \"search\" for %s\n", G_STRFUNC, name);
        c = pt_blist_chat_find_search(account, name);
    }
#endif
    return c;
}

