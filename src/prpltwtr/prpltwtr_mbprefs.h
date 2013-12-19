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

#ifndef _TWITTER_MBPREFS_H_
#define _TWITTER_MBPREFS_H_

#include "defaults.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <sys/stat.h>
#include <time.h>
#include <locale.h>
#include <account.h>

typedef struct _TwitterMbPrefsSettings TwitterMbPrefsSettings;

typedef struct {
    TwitterMbPrefsSettings *settings;
    PurpleAccount  *account;
    gpointer        data;
} TwitterMbPrefs;

struct _TwitterMbPrefsSettings {
    gchar          *(*get_user_profile_url) (TwitterMbPrefs * prefs, const gchar * who);
    gchar          *(*get_status_url) (TwitterMbPrefs * prefs, const gchar * who, gchar * tweet_id);
    void            (*mb_prefs_free) (TwitterMbPrefs * mb_prefs);
};

gchar          *twitter_mb_prefs_get_user_profile_url(TwitterMbPrefs * mb_prefs, const gchar * who);
gchar          *twitter_mb_prefs_get_status_url(TwitterMbPrefs * mb_prefs, const gchar * who, gchar * tweet_id);
void            twitter_mb_prefs_free(TwitterMbPrefs * mb_prefs);
TwitterMbPrefs *twitter_mb_prefs_new(PurpleAccount * account);

#endif
