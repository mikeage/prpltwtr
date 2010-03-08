#include "twitter_mbprefs.h"

/*
 * Copyright (C) 2007 Dossy Shiobara <dossy@panoptic.com>
 * Copyright (C) 2010 Neaveru <neaveru@gmail.com>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02111-1301, USA.
 */
static gchar *get_user_profile_url_twitter(const gchar *host, const gchar *who)
{
	return g_strdup_printf("http://twitter.com/%s", who);
}
static gchar *get_status_url_twitter(const gchar *host, const gchar *who, long long tweet_id)
{
	return g_strdup_printf("http://twitter.com/%s/status/%lld", who, tweet_id);
}
static gchar *get_user_profile_url_statusnet(const gchar *host, const gchar *who)
{
	return NULL; //TODO
}
static gchar *get_status_url_statusnet(const gchar *host, const gchar *who, long long tweet_id)
{
	return NULL; //TODO
}

static TwitterMbPrefs TwitterMbPrefsTwitter =
{
	get_user_profile_url_twitter, //get_user_profile_url
	get_status_url_twitter, //get_status_url
};

static TwitterMbPrefs TwitterMbPrefsStatusNet =
{
	get_user_profile_url_statusnet, //get_user_profile_url
	get_status_url_statusnet, //get_status_url
};

TwitterMbPrefs *twitter_get_mb_pref(const gchar *api_host)
{
	return &TwitterMbPrefsTwitter;
}
