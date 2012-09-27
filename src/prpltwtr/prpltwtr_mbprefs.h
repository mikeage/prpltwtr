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
    gchar          *(*get_status_url) (TwitterMbPrefs * prefs, const gchar * who, long long tweet_id);
    void            (*mb_prefs_free) (TwitterMbPrefs * mb_prefs);
};

gchar          *twitter_mb_prefs_get_user_profile_url(TwitterMbPrefs * mb_prefs, const gchar * who);
gchar          *twitter_mb_prefs_get_status_url(TwitterMbPrefs * mb_prefs, const gchar * who, long long tweet_id);
void            twitter_mb_prefs_free(TwitterMbPrefs * mb_prefs);
TwitterMbPrefs *twitter_mb_prefs_new(PurpleAccount * account);

#endif
