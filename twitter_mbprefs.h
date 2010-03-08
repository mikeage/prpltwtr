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

typedef enum
{
	TWITTER_MB_TYPE_TWITTER,
	TWITTER_MB_TYPE_STATUS_NET,
	TWITTER_MB_TYPE_UNKNOWN //Keep this last
} TwitterMbType;

typedef struct
{
	gchar *(*get_user_profile_url)(const gchar *host, const gchar *who);
	gchar *(*get_status_url)(const gchar *host, const gchar *who, long long tweet_id);
} TwitterMbPrefs;

#endif
