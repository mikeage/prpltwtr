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

#include "prpltwtr_conn.h"

void twitter_connection_foreach_endpoint_im(TwitterConnectionData * twitter, void (*cb) (TwitterConnectionData * twitter, TwitterEndpointIm * im, gpointer data), gpointer data)
{
    int             i;
    for (i = 0; i < TWITTER_IM_TYPE_UNKNOWN; i++)
        if (twitter->endpoint_ims[i])
            cb(twitter, twitter->endpoint_ims[i], data);
}

TwitterEndpointIm *twitter_connection_get_endpoint_im(TwitterConnectionData * twitter, TwitterImType type)
{
    if (type < TWITTER_IM_TYPE_UNKNOWN)
        return twitter->endpoint_ims[type];
    return NULL;
}

void twitter_connection_set_endpoint_im(TwitterConnectionData * twitter, TwitterImType type, TwitterEndpointIm * endpoint)
{
    twitter->endpoint_ims[type] = endpoint;
}

TwitterRequestor *purple_account_get_requestor(PurpleAccount * account)
{
    PurpleConnection *gc = purple_account_get_connection(account);
    TwitterConnectionData *twitter = NULL;
    if (gc)
        twitter = gc->proto_data;
    if (twitter)
        return twitter->requestor;
    return NULL;
}
