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

#ifndef _TWITTER_SEARCH_H_
#define _TWITTER_SEARCH_H_

#include <glib.h>
#include "prpltwtr_xml.h"
#include "prpltwtr_request.h"

typedef struct _TwitterSearchErrorData TwitterSearchErrorData;

struct _TwitterSearchErrorData {

};

/* @search_result: an array of TwitterUserTweet */
typedef void    (*TwitterSearchSuccessFunc) (PurpleAccount * account, GList * search_results, const gchar * refresh_url, gchar * max_id, gpointer user_data);

typedef         gboolean(*TwitterSearchErrorFunc) (PurpleAccount * account, const TwitterSearchErrorData * error_data, gpointer user_data);

void            twitter_search(TwitterRequestor * r, TwitterRequestParams * params, TwitterSearchSuccessFunc success_cb, TwitterSearchErrorFunc error_cb, gpointer data);

#endif                       /* TWITTER_SEARCH_H_ */
