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

#include "prpltwtr_request.h"
#include "prpltwtr_conn.h"
void            prpltwtr_auth_invalidate_token(PurpleAccount * account);
void            prpltwtr_auth_oauth_login(PurpleAccount * account, TwitterConnectionData * twitter);

void            prpltwtr_auth_pre_send_auth_basic(TwitterRequestor * r, gboolean * post, const char **url, TwitterRequestParams ** params, gchar *** header_fields, gpointer * requestor_data);
void            prpltwtr_auth_post_send_auth_basic(TwitterRequestor * r, gboolean * post, const char **url, TwitterRequestParams ** params, gchar *** header_fields, gpointer * requestor_data);
void            prpltwtr_auth_pre_send_oauth(TwitterRequestor * r, gboolean * post, const char **url, TwitterRequestParams ** params, gchar *** header_fields, gpointer * requestor_data);
void            prpltwtr_auth_post_send_oauth(TwitterRequestor * r, gboolean * post, const char **url, TwitterRequestParams ** params, gchar *** header_fields, gpointer * requestor_data);
const gchar    *prpltwtr_auth_get_oauth_key(PurpleAccount * account);
const gchar    *prpltwtr_auth_get_oauth_secret(PurpleAccount * account);
