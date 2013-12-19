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

#ifndef G_GNUC_NULL_TERMINATED
#if __GNUC__ >= 4
#define G_GNUC_NULL_TERMINATED __attribute__((__sentinel__))
#else
#define G_GNUC_NULL_TERMINATED
#endif                       /* __GNUC__ >= 4 */
#endif                       /* G_GNUC_NULL_TERMINATED */

#ifdef _WIN32
#	include <win32dep.h>
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef PACKAGE_VERSION
#error Version not defined! (command line for MinGW, config.h for autoconf builds)
#endif

#ifdef ENABLE_NLS
#include <glib/gi18n-lib.h>
#else
#define _(String) ((/* const */ char *) (String))
#define N_(String) ((/* const */ char *) (String))
#endif                       // ENABLE NLS

#define TWITTER_SEARCH_COUNT_DEFAULT 100

#define TWITTER_INITIAL_REPLIES_COUNT 200
#define TWITTER_INITIAL_DMS_COUNT 200
#define TWITTER_EVERY_REPLIES_COUNT 200
#define TWITTER_EVERY_DMS_COUNT 200

#define TWITTER_HOME_TIMELINE_INITIAL_COUNT 200
#define TWITTER_HOME_TIMELINE_PAGE_COUNT 150

#define TWITTER_LIST_INITIAL_COUNT 200
#define TWITTER_LIST_PAGE_COUNT 150

//timer (mins) to check to see who hasn't said anything in X hours
#define TWITTER_UPDATE_PRESENCE_TIMEOUT 5

#define TWITTER_PROTOCOL_ID "prpl-twitter"
#define STATUSNET_PROTOCOL_ID "prpl-statusnet"
#define GENERIC_PROTOCOL_ID "prpltwtr"

#define TWITTER_OAUTH_KEY "9hDKG0Lty62lPca2XoA"
#define TWITTER_OAUTH_SECRET "WmCXa0M1Q5k89WTZhnqUhxaebvF3faVkzGWGiwpoZkc"

/* We'll handle TWITTER_URI://foo as an internal action */
#define TWITTER_URI "prpltwtr"

#define MAX_TWEET_LENGTH 140

#define TWITTER_STATUS_ONLINE	"online"
#define TWITTER_STATUS_OFFLINE	"offline"
