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

#ifndef _TWITTER_BUDDY_H_
#define _TWITTER_BUDDY_H_
#include "prpltwtr_xml.h"
#include "prpltwtr_prefs.h"

TwitterUserTweet *twitter_buddy_get_buddy_data(PurpleBuddy * b);
void            twitter_buddy_set_status_data(PurpleAccount * account, const char *src_user, TwitterTweet * s);
TwitterUserTweet *twitter_buddy_get_buddy_data(PurpleBuddy * b);
PurpleBuddy    *twitter_buddy_new(PurpleAccount * account, const char *screenname, const char *alias);
void            twitter_buddy_set_user_data(PurpleAccount * account, TwitterUserData * u, gboolean add_missing_buddy);
void            twitter_buddy_update_icon_from_username(PurpleAccount * account, const gchar * username, const gchar * url);
void            twitter_buddy_update_icon(PurpleBuddy * buddy);

void            twitter_buddy_touch_state(PurpleBuddy * buddy);
void            twitter_buddy_touch_state_all(PurpleAccount * account);

#endif
