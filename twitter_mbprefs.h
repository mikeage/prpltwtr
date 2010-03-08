#ifndef _TWITTER_MBPREFS_H_
#define _TWITTER_MBPREFS_H_

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <sys/stat.h>
#include <time.h>
#include <locale.h>

typedef struct
{
	gchar *(*get_user_profile_url)(const gchar *host, const gchar *who);
	gchar *(*get_status_url)(const gchar *host, const gchar *who, long long tweet_id);
} TwitterMbPrefs;

TwitterMbPrefs *twitter_get_mb_pref(const gchar *api_base_url);

#endif
