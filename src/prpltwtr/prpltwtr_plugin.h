/*
 * prpltwtr 
 *
 * prpltwtr is the legal property of its developers, whose names are too numerous
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

#ifndef _TWITTER_PLUGIN_H_
#define _TWITTER_PLUGIN_H_

typedef struct {
    const gchar    *get_rate_limit_status;
    const gchar    *get_friends;
    const gchar    *get_home_timeline;
    const gchar    *get_mentions;
    const gchar    *get_dms;
    const gchar    *update_status;
    const gchar    *new_dm;
    const gchar    *get_saved_searches;
    const gchar    *get_subscribed_lists;
    const gchar    *get_personal_lists;
    const gchar    *get_list_statuses;
    const gchar    *get_search_results;
    const gchar    *verify_credentials;
    const gchar    *report_spammer;
    const gchar    *add_favorite;
    const gchar    *delete_favorite;
    const gchar    *get_user_info;
} TwitterUrls;

void            twitter_destroy(PurplePlugin * plugin);
void            twitter_tooltip_text(PurpleBuddy * buddy, PurpleNotifyUserInfo * info, gboolean full);
GList          *twitter_actions(PurplePlugin * plugin, gpointer context);
GHashTable     *prpltwtr_get_account_text_table_statusnet(PurpleAccount * account);
const char     *twitter_list_icon(PurpleAccount * account, PurpleBuddy * buddy);
char           *twitter_status_text(PurpleBuddy * buddy);
GList          *twitter_status_types(PurpleAccount * account);
GList          *twitter_blist_node_menu(PurpleBlistNode * node);
GList          *twitter_chat_info(PurpleConnection * gc);
GHashTable     *twitter_chat_info_defaults(PurpleConnection * gc, const char *chat_name);
void            twitter_close(PurpleConnection * gc);
int             twitter_send_im(PurpleConnection * gc, const char *conv_name, const char *message, PurpleMessageFlags flags);
void            twitter_set_info(PurpleConnection * gc, const char *info);
void            twitter_set_status(PurpleAccount * account, PurpleStatus * status);
void            twitter_add_buddy(PurpleConnection * gc, PurpleBuddy * buddy, PurpleGroup * group);
void            twitter_add_buddies(PurpleConnection * gc, GList * buddies, GList * groups);
void            twitter_remove_buddy(PurpleConnection * gc, PurpleBuddy * buddy, PurpleGroup * group);
void            twitter_remove_buddies(PurpleConnection * gc, GList * buddies, GList * groups);
void            twitter_chat_join(PurpleConnection * gc, GHashTable * components);
void            twitter_chat_leave(PurpleConnection * gc, int id);
int             twitter_chat_send(PurpleConnection * gc, int id, const char *message, PurpleMessageFlags flags);
void            twitter_get_cb_info(PurpleConnection * gc, int id, const char *who);
char           *twitter_chat_get_name(GHashTable * components);
void            twitter_convo_closed(PurpleConnection * gc, const gchar * conv_name);
void            twitter_set_buddy_icon(PurpleConnection * gc, PurpleStoredImage * img);
void            prpltwtr_plugin_init(PurplePlugin * plugin);

#endif
