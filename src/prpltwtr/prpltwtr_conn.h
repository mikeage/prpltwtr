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

#ifndef _TWITTER_CONN_H_
#define _TWITTER_CONN_H_

#include <glib.h>
#include "prpltwtr_endpoint_im.h"
#include "prpltwtr_request.h"
#include "prpltwtr_mbprefs.h"

typedef struct {
    TwitterRequestor *requestor;

    long long       failed_get_replies_count;

    guint           get_friends_timer;
    guint           update_presence_timer;

    gchar          *last_home_timeline_id;

    /* a table of TwitterEndpointChat
     * where the key will be the chat name
     * Alternatively, we only really need chat contexts when
     * we have a blist node with auto_open = TRUE or a chat
     * already open. So we could pass the context between the two
     * but that would be much more annoying to write/maintain */
    GHashTable     *chat_contexts;

    /* key: gchar *screen_name,
     * value: gchar *reply_id (then converted to gchar *)
     * Store the id of last reply sent from any user to @me
     * This is used as in_reply_to_status_id
     * when @me sends a tweet to others */
    GHashTable     *user_reply_id_table;

    /* key: gchar *screen_name
     * value: TwitterConvIcon
     * Store purple buddy icons for nonbuddies (for conversations)
     */
    GHashTable     *icons;

    gboolean        requesting;
    TwitterEndpointIm *endpoint_ims[TWITTER_IM_TYPE_UNKNOWN];

    gchar          *oauth_token;
    gchar          *oauth_token_secret;

    TwitterMbPrefs *mb_prefs;

    int             chat_id;
} TwitterConnectionData;

void            twitter_connection_foreach_endpoint_im(TwitterConnectionData * twitter, void (*cb) (TwitterConnectionData * twitter, TwitterEndpointIm * im, gpointer data), gpointer data);
TwitterEndpointIm *twitter_connection_get_endpoint_im(TwitterConnectionData * twitter, TwitterImType type);
void            twitter_connection_set_endpoint_im(TwitterConnectionData * twitter, TwitterImType type, TwitterEndpointIm * endpoint);

TwitterRequestor *purple_account_get_requestor(PurpleAccount * account);

#endif
