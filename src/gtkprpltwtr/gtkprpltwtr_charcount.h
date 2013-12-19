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

#ifndef _TWITTER_CHARCOUNT_H_
#define _TWITTER_CHARCOUNT_H_

#include "defaults.h"
#include <glib.h>
#include <conversation.h>

void            twitter_charcount_detach_from_all_windows(void);
void            twitter_charcount_attach_to_all_windows(void);
void            twitter_charcount_conv_created_cb(PurpleConversation * conv, gpointer null);
void            twitter_charcount_conv_destroyed_cb(PurpleConversation * conv, gpointer null);
void            twitter_charcount_update_append_text_cb(PurpleConversation * conv);

#endif
