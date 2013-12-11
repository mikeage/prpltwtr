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

#include <glib/gstdio.h>

#include "prpltwtr.h"
#include "prpltwtr_mbprefs.h"
#include "prpltwtr_plugin_twitter.h"
#include "prpltwtr_format_json.h"

static PurplePluginProtocolInfo prpl_info = {
    OPT_PROTO_CHAT_TOPIC | OPT_PROTO_NO_PASSWORD,   /* options */
    NULL,                                        /* user_splits, initialized in twitter_init() */
    NULL,                                        /* protocol_options, initialized in twitter_init() */
    {                                            /* icon_spec, a PurpleBuddyIconSpec */
     "png,jpg,gif",                              /* format */
     0,                                          /* min_width */
     0,                                          /* min_height */
     48,                                         /* max_width */
     48,                                         /* max_height */
     10000,                                      /* max_filesize */
     PURPLE_ICON_SCALE_DISPLAY,                  /* scale_rules */
     },
    twitter_list_icon,                           /* list_icon *///TODO
    NULL,                                        //twitter_list_emblem, /* list_emblem */
    twitter_status_text,                         /* status_text */
    twitter_tooltip_text,                        /* tooltip_text */
    twitter_status_types,                        /* status_types */
    twitter_blist_node_menu,                     /* blist_node_menu */
    twitter_chat_info,                           /* chat_info */
    twitter_chat_info_defaults,                  /* chat_info_defaults */
    prpltwtr_login,                              /* login */
    twitter_close,                               /* close */
    twitter_send_im,                             //twitter_send_dm, /* send_im */
    twitter_set_info,                            /* set_info */
    NULL,                                        //twitter_send_typing, /* send_typing */
    twitter_api_get_info,                        /* get_info */
    twitter_set_status,                          /* set_status */
    NULL,                                        /* set_idle */
    NULL,                                        //TODO? /* change_passwd */
    twitter_add_buddy,                           //TODO /* add_buddy */
    twitter_add_buddies,                         //TODO /* add_buddies */
    twitter_remove_buddy,                        //TODO /* remove_buddy */
    twitter_remove_buddies,                      //TODO /* remove_buddies */
    NULL,                                        //TODO? /* add_permit */
    NULL,                                        //TODO? /* add_deny */
    NULL,                                        //TODO? /* rem_permit */
    NULL,                                        //TODO? /* rem_deny */
    NULL,                                        //TODO? /* set_permit_deny */
    twitter_chat_join,                           /* join_chat */
    NULL,                                        /* reject_chat */
    twitter_chat_get_name,                       /* get_chat_name */
    NULL,                                        /* chat_invite */
    twitter_chat_leave,                          /* chat_leave */
    NULL,                                        //twitter_chat_whisper, /* chat_whisper */
    twitter_chat_send,                           /* chat_send */
    NULL,                                        //TODO? /* keepalive */
    NULL,                                        /* register_user */
    twitter_get_cb_info,                         /* get_cb_info */
    NULL,                                        /* get_cb_away */
    NULL,                                        //TODO? /* alias_buddy */
    NULL,                                        /* group_buddy */
    NULL,                                        /* rename_group */
    NULL,                                        /* buddy_free */
    twitter_convo_closed,                        /* convo_closed */
    purple_normalize_nocase,                     /* normalize */
    twitter_set_buddy_icon,                      /* set_buddy_icon */
    NULL,                                        /* remove_group */
    NULL,                                        //TODO? /* get_cb_real_name */
    NULL,                                        /* set_chat_topic */
    twitter_blist_chat_find,                     /* find_blist_chat */
    NULL,                                        /* roomlist_get_list */
    NULL,                                        /* roomlist_cancel */
    NULL,                                        /* roomlist_expand_category */
    NULL,                                        /* can_receive_file */
    NULL,                                        /* send_file */
    NULL,                                        /* new_xfer */
    prpltwtr_offline_message,                    /* offline_message */
    NULL,                                        /* whiteboard_prpl_ops */
    NULL,                                        /* send_raw */
    NULL,                                        /* roomlist_room_serialize */
    NULL,                                        /* unregister_user... */
    NULL,                                        /* send_attention */
    NULL,                                        /* get_attention type */
    sizeof (PurplePluginProtocolInfo),           /* struct_size */
    NULL,                                        /* get_account_text_table */
    NULL,                                        /* initiate_media */
    NULL,                                        /* get_media_caps */
    NULL,                                        /* get_moods */
    NULL,                                        /* set_public_alias */
    NULL,                                        /* get_public_alias */
    NULL,                                        /* add_buddy_with_invite */
    NULL                                         /* add_buddies_with_invite */
};

static PurplePluginInfo info = {
    PURPLE_PLUGIN_MAGIC,                         /* magic */
    PURPLE_MAJOR_VERSION,                        /* major_version */
    PURPLE_MINOR_VERSION,                        /* minor_version */
    PURPLE_PLUGIN_PROTOCOL,                      /* type */
    NULL,                                        /* ui_requirement */
    0,                                           /* flags */
    NULL,                                        /* dependencies */
    PURPLE_PRIORITY_DEFAULT,                     /* priority */
    TWITTER_PROTOCOL_ID,                         /* id */
    "Twitter Protocol",                          /* name */
    PACKAGE_VERSION,                             /* version */
    "Twitter Protocol Plugin",                   /* summary */
    "Twitter Protocol Plugin",                   /* description */
    "neaveru <neaveru@gmail.com>",               /* author */
    "http://code.google.com/p/prpltwtr/",        /* homepage */
    NULL,                                        /* load */
    NULL,                                        /* unload */
    twitter_destroy,                             /* destroy */
    NULL,                                        /* ui_info */
    &prpl_info,                                  /* extra_info */
    NULL,                                        /* prefs_info */
    twitter_actions,                             /* actions */
    NULL,                                        /* padding... */
    NULL,
    NULL,
    NULL,
};

PURPLE_INIT_PLUGIN(null, prpltwtr_plugin_init, info);

void prpltwtr_plugin_twitter_setup(TwitterRequestor * requestor)
{
	PurpleAccount *account = requestor->account;
	TwitterFormat *format = requestor->format;
	TwitterUrls   *urls = requestor->urls;

	// Configure the system to use JSON as the communication format.
	prpltwtr_format_json_setup(format);

	// TODO urls->host = twitter_option_api_host(account);
	// TODO urls->subdir = twitter_option_api_subdir(account);
	urls->get_rate_limit_status = g_strdup(twitter_api_create_url_ext(account, TWITTER_PREF_URL_GET_RATE_LIMIT_STATUS, format->extension));
	urls->get_friends = g_strdup(twitter_api_create_url_ext(account, TWITTER_PREF_URL_GET_FRIENDS, format->extension));
	urls->get_home_timeline = g_strdup(twitter_api_create_url_ext(account, TWITTER_PREF_URL_GET_HOME_TIMELINE, format->extension));
	urls->get_mentions = g_strdup(twitter_api_create_url_ext(account, TWITTER_PREF_URL_GET_MENTIONS, format->extension));
	urls->get_dms = g_strdup(twitter_api_create_url_ext(account, TWITTER_PREF_URL_GET_DMS, format->extension));
	urls->update_status = g_strdup(twitter_api_create_url_ext(account, TWITTER_PREF_URL_UPDATE_STATUS, format->extension));
	urls->new_dm = g_strdup(twitter_api_create_url_ext(account, TWITTER_PREF_URL_NEW_DM, format->extension));
	urls->get_saved_searches = g_strdup(twitter_api_create_url_ext(account, TWITTER_PREF_URL_GET_SAVED_SEARCHES, format->extension));
	urls->get_subscribed_lists = g_strdup(twitter_api_create_url_ext(account, TWITTER_PREF_URL_GET_SUBSCRIBED_LISTS, format->extension));
	urls->get_personal_lists = g_strdup(twitter_api_create_url_ext(account, TWITTER_PREF_URL_GET_PERSONAL_LISTS, format->extension));
	urls->get_search_results = g_strdup(twitter_api_create_url_ext(account, TWITTER_PREF_URL_GET_SEARCH_RESULTS, format->extension)); // .atom
	urls->verify_credentials = g_strdup(twitter_api_create_url_ext(account, TWITTER_PREF_URL_VERIFY_CREDENTIALS, format->extension));
	urls->report_spammer = g_strdup(twitter_api_create_url_ext(account, TWITTER_PREF_URL_REPORT_SPAMMER, format->extension));
	urls->get_user_info = g_strdup(twitter_api_create_url(account, TWITTER_PREF_URL_GET_USER_INFO));
}
