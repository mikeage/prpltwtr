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

#ifndef _TWITTER_UTIL_H_
#define _TWITTER_UTIL_H_

#include <string.h>
#include <glib.h>
#include <xmlnode.h>
#include <debug.h>

#include "defaults.h"
#include "prpltwtr_xml.h"
#include "prpltwtr_prefs.h"                      //TODO move

#define TWITTER_URI_ACTION_USER		"user"       //TODO: move?
#define TWITTER_URI_ACTION_SEARCH	"search"     //TODO: move?
#define TWITTER_URI_ACTION_ACTIONS	"actions"    //TODO: move?
#define TWITTER_URI_ACTION_REPLY	"reply"      //TODO: move?
#define TWITTER_URI_ACTION_REPLYALL	"replyall"   //TODO: move?
#define TWITTER_URI_ACTION_ORT		"ort"        //TODO: move?
#define TWITTER_URI_ACTION_RT		"rt"         //TODO: move?
#define TWITTER_URI_ACTION_LINK		"link"       //TODO: move?
#define TWITTER_URI_ACTION_DELETE	"delete"     //TODO: move?
#define TWITTER_URI_ACTION_SET_REPLY "setreply"  //TODO: move?
#define TWITTER_URI_ACTION_GET_ORIGINAL "getoriginal"   //TODO: move?
#define TWITTER_URI_ACTION_ADD_FAVORITE "addfavorite"   //TODO: move?
#define TWITTER_URI_ACTION_DELETE_FAVORITE "deletefavorite" //TODO: move?
#define TWITTER_URI_ACTION_REPORT_SPAM "reportspam" //TODO: move?

gboolean        twitter_usernames_match(PurpleAccount * account, const gchar * u1, const gchar * u2);
gchar          *twitter_utf8_find_last_pos(const gchar * str, const gchar * needles, glong str_len);
char           *twitter_utf8_get_segment(const gchar * message, int max_len, const gchar * add_text, const gchar ** new_start, gboolean prepend);
GArray         *twitter_utf8_get_segments(const gchar * message, int segment_length, const gchar * add_text, gboolean prepend);

#ifndef g_slice_new0
#define g_slice_new0(a) g_new0(a, 1)
#define g_slice_free(a, b) g_free(b)
#endif

#ifndef g_strcmp0
#define g_strcmp0(a, b) (a == NULL && b == NULL ? 0 : a == NULL ? -1 : b == NULL ? 1 : strcmp(a, b))
#endif

//TODO: move this?
char           *twitter_format_tweet(PurpleAccount * account, const char *src_user, const char *message, gchar * tweet_id, PurpleConversationType conv_type, const gchar * conv_name, gboolean is_tweet, gchar * in_reply_to_status_id, gboolean favorited);
#endif                       /* UTIL_H_ */
