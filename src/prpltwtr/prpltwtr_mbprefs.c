#include "prpltwtr_mbprefs.h"
#include "prpltwtr_prefs.h"

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

static TwitterMbPrefs *twitter_mb_prefs_new_base(TwitterMbPrefsSettings *settings, PurpleAccount *account)
{
	TwitterMbPrefs *mb_prefs = g_new0(TwitterMbPrefs, 1);
	mb_prefs->settings = settings;
	mb_prefs->account = account;
	return mb_prefs;
}

static void twitter_mb_prefs_free_base(TwitterMbPrefs *mb_prefs)
{
	g_free(mb_prefs);
}

static gchar *get_user_profile_url_twitter(TwitterMbPrefs *mb_prefs, const gchar *who)
{
	return g_strdup_printf("http://twitter.com/%s", who);
}
static gchar *get_status_url_twitter(TwitterMbPrefs *mb_prefs, const gchar *who, long long tweet_id)
{
	return g_strdup_printf("http://twitter.com/%s/status/%lld", who, tweet_id);
}

static TwitterMbPrefsSettings TwitterMbPrefsSettingsTwitter =
{
	get_user_profile_url_twitter, //get_user_profile_url
	get_status_url_twitter, //get_status_url
	twitter_mb_prefs_free_base, //mb_prefs_free
};

static TwitterMbPrefs *mb_prefs_new_twitter(PurpleAccount *account)
{
	return twitter_mb_prefs_new_base(&TwitterMbPrefsSettingsTwitter, account);
}

static gchar *get_user_profile_url_statusnet(TwitterMbPrefs *mb_prefs, const gchar *who)
{
	return g_strdup_printf("http://%s/%s", (gchar *) mb_prefs->data, who);
}
static gchar *get_status_url_statusnet(TwitterMbPrefs *mb_prefs, const gchar *who, long long tweet_id)
{
	return g_strdup_printf("http://%s/notice/%lld", (gchar *) mb_prefs->data, tweet_id);
}

static void mb_prefs_free_statusnet(TwitterMbPrefs *mb_prefs)
{
	if (mb_prefs->data)
	{
		g_free(mb_prefs->data);
		mb_prefs->data = NULL;
	}
	twitter_mb_prefs_free_base(mb_prefs);
}

static TwitterMbPrefsSettings TwitterMbPrefsSettingsStatusNet =
{
	get_user_profile_url_statusnet, //get_user_profile_url
	get_status_url_statusnet, //get_status_url
	mb_prefs_free_statusnet, //mb_prefs_free
};

static TwitterMbPrefs *mb_prefs_new_statusnet(PurpleAccount *account, const gchar *host, int host_len)
{
	TwitterMbPrefs *mb_prefs = twitter_mb_prefs_new_base(&TwitterMbPrefsSettingsStatusNet, account);
	mb_prefs->data = g_strndup(host, host_len);
	return mb_prefs;
}

gchar *twitter_mb_prefs_get_user_profile_url(TwitterMbPrefs *mb_prefs, const gchar *who)
{
	return mb_prefs && mb_prefs->settings->get_user_profile_url ?
		mb_prefs->settings->get_user_profile_url(mb_prefs, who) :
		NULL;
}

gchar *twitter_mb_prefs_get_status_url(TwitterMbPrefs *mb_prefs, const gchar *who, long long tweet_id)
{
	return mb_prefs && mb_prefs->settings->get_status_url ?
		mb_prefs->settings->get_status_url(mb_prefs, who, tweet_id) :
		NULL;
}

void twitter_mb_prefs_free(TwitterMbPrefs *mb_prefs)
{
	if (!mb_prefs)
		return;
	if (mb_prefs->settings->mb_prefs_free)
	{
		mb_prefs->settings->mb_prefs_free(mb_prefs);
		mb_prefs = NULL;
	} else {
		twitter_mb_prefs_free_base(mb_prefs);
	}
}

TwitterMbPrefs *twitter_mb_prefs_new(PurpleAccount *account)
{
	const gchar *host = twitter_option_api_host(account);
	const gchar *ptr;
	int twitter_domain_len = strlen("twitter.com");
	int host_len;
	if ((ptr = strchr(host, '/')))
		host_len = ptr - host;
	else
		host_len = strlen(host);
	if (host_len == twitter_domain_len && !strncmp(host, "twitter.com", twitter_domain_len))
		return mb_prefs_new_twitter(account);
	else if (host_len > twitter_domain_len && 
		!strncmp(host + host_len - twitter_domain_len - 1, ".twitter.com", twitter_domain_len + 1))
		return mb_prefs_new_twitter(account);
	else
		return mb_prefs_new_statusnet(account, host, host_len);
}
