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

#ifndef _TWITTER_ENDPOINT_IM_H_
#define _TWITTER_ENDPOINT_IM_H_

#include "defaults.h"

#include "prpltwtr_api.h"

typedef enum {
    TWITTER_IM_TYPE_AT_MSG = 0,
    TWITTER_IM_TYPE_DM = 1,
    TWITTER_IM_TYPE_UNKNOWN = 2,
} TwitterImType;

typedef void    (*TwitterApiImAllFunc) (TwitterRequestor * r, gchar * since_id, TwitterSendRequestMultiPageAllSuccessFunc success_func, TwitterSendRequestMultiPageAllErrorFunc error_func, gint max_count, gpointer data);

typedef struct {
    TwitterImType   type;
    const gchar    *since_id_setting_id;
    const gchar    *conv_id;
    int             (*send_im) (PurpleAccount * account, const char *buddy_name, const char *message, PurpleMessageFlags flags);
    int             (*timespan_func) (PurpleAccount * account);
    TwitterApiImAllFunc get_im_func;
    TwitterSendRequestMultiPageAllSuccessFunc success_cb;
    TwitterSendRequestMultiPageAllErrorFunc error_cb;
    void            (*get_last_since_id) (PurpleAccount * account, void (*success_cb) (PurpleAccount * account, gchar * id, gpointer user_data), void (*error_cb) (PurpleAccount * account, const TwitterRequestErrorData * error_data, gpointer user_data), gpointer user_data);
    void            (*convo_closed) (PurpleConversation * conv);
} TwitterEndpointImSettings;

typedef struct {
    PurpleAccount  *account;
    gchar          *since_id;
    gboolean        retrieve_history;
    gint            initial_max_retrieve;
    TwitterEndpointImSettings *settings;

    //these should be 'private'
    guint           timer;
    gboolean        ran_once;
} TwitterEndpointIm;

TwitterEndpointIm *twitter_endpoint_im_new(PurpleAccount * account, TwitterEndpointImSettings * settings, gboolean retrieve_history, gint initial_max_retrieve);
void            twitter_endpoint_im_free(TwitterEndpointIm * ctx);

TwitterEndpointIm *twitter_endpoint_im_find(PurpleAccount * account, TwitterImType type);
TwitterEndpointIm *twitter_conv_name_to_endpoint_im(PurpleAccount * account, const char *name);
const char     *twitter_conv_name_to_buddy_name(PurpleAccount * account, const char *name);

void            twitter_endpoint_im_settings_save_since_id(PurpleAccount * account, TwitterEndpointImSettings * settings, gchar * since_id);
const gchar    *twitter_endpoint_im_settings_load_since_id(PurpleAccount * account, TwitterEndpointImSettings * settings);
void            twitter_endpoint_im_set_since_id(TwitterEndpointIm * ctx, gchar * since_id);
const gchar    *twitter_endpoint_im_get_since_id(TwitterEndpointIm * ctx);

void            twitter_endpoint_im_start(TwitterEndpointIm * ctx);
char           *twitter_endpoint_im_buddy_name_to_conv_name(TwitterEndpointIm * im, const char *name);
void            twitter_status_data_update_conv(TwitterEndpointIm * ctx, char *buddy_name, TwitterTweet * s);
TwitterImType   twitter_conv_name_to_type(PurpleAccount * account, const char *name);
void            twitter_endpoint_im_convo_closed(TwitterEndpointIm * im, const gchar * conv_name);

#endif
